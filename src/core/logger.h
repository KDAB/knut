#pragma once

#include <QMetaEnum>
#include <QString>
#include <QVariantList>

namespace Core {

template <typename T>
concept HasToString = requires(const T &t)
{
    t.toString();
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
    else
        Q_UNREACHABLE();
    return {};
}

/**
 * @brief The LoggerObject class is a utility class to help logging API calls
 *
 * This class ensure that only the first API call is logged, subsequent calls done by the first one won't.
 */
class LoggerObject
{
public:
    explicit LoggerObject(const QString &name)
        : LoggerObject()
    {
        log(name);
    }
    template <typename... Ts>
    explicit LoggerObject(const QString &name, Ts... params)
        : LoggerObject()
    {
        QStringList paramStrings({(toString(params), ...)});
        QString result = name + " - " + paramStrings.join("' ");
        log(result);
    }

    ~LoggerObject();

private:
    LoggerObject();
    void log(const QString &string);

    inline static bool m_isLogging = false;
    bool m_firstLogger = false;
};

/**
 * Log a method, with all its parameters.
 */
#define LOG(name, ...) LoggerObject __sl(name, ##__VA_ARGS__)

/**
 * Log a method, with all its parameters. If the previous log is also the same method, it will be merged into one
 * operation
 */
#define LOG_AND_MERGE(name, ...) LoggerObject __sl(name, ##__VA_ARGS__)

} // namespace Core
