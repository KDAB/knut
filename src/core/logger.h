/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "scriptdialogitem.h"
#include "utils/log.h"

#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QMetaEnum>
#include <QString>
#include <QVariantList>
#include <concepts>
#include <vector>

/**
 * Create a return value, the name will depend on the type returned.
 */
#define LOG_ARG(name, value) Core::LoggerArg(name, value)

/**
 * Log a method, with all its parameters.
 */
#define LOG(...)                                                                                                       \
    Core::LoggerObject __loggerObject(                                                                                 \
        Core::LoggerObject::canLog() ? Core::formatToClassNameFunctionName(std::source_location::current()) : "",      \
        false, ##__VA_ARGS__)

/**
 * Log a method, with all its parameters. If the previous log is also the same method, it will be merged into one
 * operation
 */
#define LOG_AND_MERGE(...)                                                                                             \
    Core::LoggerObject __loggerObject(                                                                                 \
        Core::LoggerObject::canLog() ? Core::formatToClassNameFunctionName(std::source_location::current()) : "",      \
        true, ##__VA_ARGS__)

/**
 * Macro to save the returned value in the historymodel
 */
#define LOG_RETURN(name, value)                                                                                        \
    do {                                                                                                               \
        if (Core::LoggerObject::canLog()) {                                                                            \
            const auto &__value = value;                                                                               \
            __loggerObject.setReturnValue(name, __value);                                                              \
            return __value;                                                                                            \
        } else {                                                                                                       \
            return value;                                                                                              \
        }                                                                                                              \
    } while (false)

namespace Core {

/**
 * @brief The LoggerDisabler class is a RAII class to temporary disable logging
 */
class LoggerDisabler
{
public:
    LoggerDisabler(bool silenceAll = false);
    ~LoggerDisabler();

private:
    bool m_originalCanLog = true;
    bool m_silenceAll = false;
    spdlog::level::level_enum m_level = spdlog::level::off;
};

///////////////////////////////////////////////////////////////////////////////
// Internal structs and classes
///////////////////////////////////////////////////////////////////////////////
template <typename T>
concept HasToString = requires(const T &t) { t.toString(); };

template <typename T>
concept HasPointerToString = requires(const T &t) { t->toString(); };

template <class T>
QString flagsToString(const QFlags<T> &flags)
{
    const auto metaEnum = QMetaEnum::fromType<T>();
    QString className = QMetaType::fromType<T>().metaObject()->className();
    // Remove potential namespace, we only want the class name
    className = className.split("::").last();
    QString text;
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        if (flags.testFlag(static_cast<T>(metaEnum.value(i)))) {
            if (!text.isEmpty())
                text += " | ";
            text += QString("%1.%2").arg(className, metaEnum.key(i));
        }
    }
    return text;
}

/**
 * @brief toString
 * Returns a string for any kind of data you can pass as a parameter.
 */
template <class T>
QString valueToString(const T &data, bool escape = false)
{
    if constexpr (std::is_same_v<std::remove_cvref_t<T>, QString>) {
        QString text = data;
        text.replace('\n', "\\n");
        text.replace('\t', "\\t");
        if (escape) {
            text.replace('\\', R"(\\)");
            text.replace('"', R"(\")");
            text.append('"');
            text.prepend('"');
        }
        return text;
    } else if constexpr (std::is_same_v<std::remove_cvref_t<T>, bool>)
        return data ? "true" : "false";
    else if constexpr (std::is_floating_point_v<T> || std::is_integral_v<T>)
        return QString::number(data);
    else if constexpr (std::is_same_v<std::remove_cvref_t<T>, QStringList>)
        return '{' + data.join(", ") + '}';
    else if constexpr (std::is_enum_v<T>) {
        const auto metaEnum = QMetaEnum::fromType<T>();
        QString className = QMetaType::fromType<T>().metaObject()->className();
        // Remove potential namespace, we only want the class name
        className = className.split("::").last();
        return QString("%1.%2").arg(className, metaEnum.valueToKey(data));
    } else if constexpr (HasToString<T>)
        return data.toString();
    else if constexpr (HasPointerToString<T>)
        return data->toString();
    else if constexpr (std::is_same_v<std::remove_cvref_t<T>, QFlags<typename T::enum_type>>) {
        return flagsToString(data);
    } else
        Q_UNREACHABLE();
    return {};
}

struct LoggerArgBase
{
};
/**
 * @brief Argument for a call
 * The argName will be matched to an existing returned value from a previous method, when recording a script.
 * If empty, or not set by a previous method, the value will be used.
 */
template <typename T>
struct LoggerArg : public LoggerArgBase
{
    LoggerArg(QString &&name, T v)
        : argName(name)
        , value(v)
    {
    }
    QString argName;
    T value;
    QString toString() const { return valueToString(value); }
};

class HistoryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns { NameCol = 0, ParamCol, ColumnCount };

    explicit HistoryModel(QObject *parent = nullptr);
    ~HistoryModel() override;

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void clear();

    /**
     * @brief Create a script from 2 points in the history
     * The script is created using 2 rows in the history model. It will create a javascript script.
     */
    QString createScript(int start, int end);
    QString createScript(const QModelIndex &startIndex, const QModelIndex &endIndex);

private:
    friend class LoggerObject;

    struct Arg
    {
        QString name;
        QString value;
        int type;
    };
    struct ReturnArg
    {
        QString name;
        QVariant value;
        bool isEmpty() const { return name.isEmpty(); }
    };
    struct LogData
    {
        QString name;
        std::vector<Arg> params;
        ReturnArg returnArg;
    };

    void logData(const QString &name);
    template <typename... Ts>
    void logData(const QString &name, bool merge, Ts... params)
    {
        LogData data;
        data.name = name;
        fillLogData(data, params...);
        addData(std::move(data), merge);
    }

    template <typename T>
    void setReturnValue(QString &&name, const T &value)
    {
        m_data.back().returnArg.name = std::move(name);
        m_data.back().returnArg.value = QVariant::fromValue(value);
    }

    void fillLogData(LogData &) { }

    template <typename T, typename... Ts>
    void fillLogData(LogData &data, T param, Ts... params)
    {
        if constexpr (std::derived_from<T, LoggerArgBase>)
            data.params.push_back(
                {param.argName, valueToString(param.value, true), qMetaTypeId<decltype(param.value)>()});
        else
            data.params.push_back({"", valueToString(param, true), qMetaTypeId<T>()});

        fillLogData(data, params...);
    }

    void addData(LogData &&data, bool merge);

    std::vector<LogData> m_data;
};

/**
 * @brief The LoggerObject class is a utility class to help logging API calls
 *
 * This class ensure that only the first API call is logged, subsequent calls done by the first one won't.
 * Do not use this class directly, but use the macros LOG and LOG_AND_MERGE
 */
class LoggerObject
{
public:
    explicit LoggerObject(QString location, bool /*unused*/)
        : LoggerObject()
    {
        if (!m_canLog)
            return;

        // When we're running a script, we ideally want to show some kind of feedback.
        // As our scripts currently have to run on the GUI thread, the GUI is blocked.
        // So we need to update the progress bar to show that the script is still running.
        //
        // We're doing this here, as logging happens quite often in pretty much all scripts.
        // This has nothing to do with logging itself, but is just a good place to do it.
        ScriptDialogItem::updateProgress();

        if (m_model)
            m_model->logData(location);
        log(std::move(location));
    }

    template <typename... Ts>
    explicit LoggerObject(QString location, bool merge, Ts... params)
        : LoggerObject()
    {
        if (!m_canLog)
            return;

        if (m_model)
            m_model->logData(location, merge, params...);

        QStringList paramList;
        (paramList.push_back(valueToString(params)), ...);

        QString result = location + " - " + paramList.join(", ");
        log(std::move(result));
    }

    ~LoggerObject();

    template <typename T>
    void setReturnValue(QString &&name, const T &value)
    {
        if (m_firstLogger && m_model)
            m_model->setReturnValue(std::move(name), value);
    }

    static bool canLog() { return m_canLog; }

private:
    friend HistoryModel;
    friend LoggerDisabler;

    LoggerObject();
    void log(QString &&string);

    inline static bool m_canLog = true;
    bool m_firstLogger = false;

    inline static HistoryModel *m_model = nullptr;
};

} // namespace Core
