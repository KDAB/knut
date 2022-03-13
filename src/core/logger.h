#pragma once

#include <QAbstractItemModel>
#include <QMetaEnum>
#include <QString>
#include <QVariantList>

#include <vector>

namespace Core {

class HistoryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns { NameCol = 0, ParamCol, ColumnCount };

    explicit HistoryModel(QObject *parent = nullptr);
    ~HistoryModel();

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void clear();

    void logData(const QString &name) { addData(LogData {name, {}}, false); }
    template <typename... Ts>
    void logData(const QString &name, bool merge, Ts... params)
    {
        auto data = LogData {name, QVariantList({(QVariant::fromValue(params), ...)})};
        addData(std::move(data), merge);
    }

private:
    struct LogData
    {
        QString name;
        QVariantList params;
    };

    void addData(LogData data, bool merge);

    std::vector<LogData> m_data;
};

template <typename T>
concept HasToString = requires(const T &t)
{
    t.toString();
};

template <typename T>
concept HasPointerToString = requires(const T &t)
{
    t->toString();
};

template <class T>
QString toString(const T &data)
{
    if constexpr (std::is_same_v<std::remove_cv_t<T>, QString>)
        return data;
    else if constexpr (std::is_same_v<std::remove_cv_t<T>, bool>)
        return data ? "true" : "false";
    else if constexpr (std::is_floating_point_v<T> || std::is_integral_v<T>)
        return QString::number(data);
    else if constexpr (std::is_same_v<std::remove_cv_t<T>, QStringList>)
        return '{' + data.join(", ") + '}';
    else if constexpr (std::is_enum_v<T>)
        return QMetaEnum::fromType<T>().valueToKey(data);
    else if constexpr (HasToString<T>)
        return data.toString();
    else if constexpr (HasPointerToString<T>)
        return data->toString();
    else
        Q_UNREACHABLE();
    return {};
}

/**
 * @brief The LoggerDisabler class is a RAII class to temporary disable logging
 */
class LoggerDisabler
{
public:
    LoggerDisabler();
    ~LoggerDisabler();
};

/**
 * @brief The LoggerObject class is a utility class to help logging API calls
 *
 * This class ensure that only the first API call is logged, subsequent calls done by the first one won't.
 */
class LoggerObject
{
public:
    explicit LoggerObject(const QString &name, bool /*unused*/)
        : LoggerObject()
    {
        if (!m_canLog)
            return;
        if (m_model)
            m_model->logData(name);
        log(name);
    }
    template <typename... Ts>
    explicit LoggerObject(const QString &name, bool merge, Ts... params)
        : LoggerObject()
    {
        if (!m_canLog)
            return;
        if (m_model)
            m_model->logData(name, merge, params...);

        QStringList paramStrings({(toString(params), ...)});
        QString result = name + " - " + paramStrings.join("' ");
        log(result);
    }

    ~LoggerObject();

private:
    friend LoggerDisabler;
    friend HistoryModel;

    LoggerObject();
    void log(const QString &string);

    inline static bool m_canLog = true;
    bool m_firstLogger = false;

    inline static HistoryModel *m_model = nullptr;
};

/**
 * Log a method, with all its parameters.
 */
#define LOG(name, ...) LoggerObject __sl(name, false, ##__VA_ARGS__)

/**
 * Log a method, with all its parameters. If the previous log is also the same method, it will be merged into one
 * operation
 */
#define LOG_AND_MERGE(name, ...) LoggerObject __sl(name, true, ##__VA_ARGS__)

} // namespace Core
