#pragma once

#include <QString>
#include <QStringList>

#include <nlohmann/json.hpp>

#include <optional>

inline void to_json(nlohmann::json &j, const QString &str)
{
    j = nlohmann::json(str.toStdString());
}

inline void from_json(const nlohmann::json &j, QString &str)
{
    str = QString::fromStdString(j.get<std::string>());
}

inline void to_json(nlohmann::json &j, const QStringList &strList)
{
    std::vector<QString> list(strList.cbegin(), strList.cend());
    j = list;
}

inline void from_json(const nlohmann::json &j, QStringList &strList)
{
    if (j.is_array()) {
        auto list = j.get<std::vector<QString>>();
        strList = QStringList(list.cbegin(), list.cend());
    }
}

namespace nlohmann {

template <class T>
void optional_to_json(nlohmann::json &j, const char *name, const std::optional<T> &value)
{
    if (value)
        j[name] = *value;
}

template <class T>
void optional_from_json(const nlohmann::json &j, const char *name, std::optional<T> &value)
{
    const auto it = j.find(name);
    if (it != j.end())
        value = it->get<T>();
    else
        value = std::nullopt;
}

template <typename>
constexpr bool is_optional = false;
template <typename T>
constexpr bool is_optional<std::optional<T>> = true;

template <typename T>
void knut_to_json(const char *key, nlohmann::json &j, const T &value)
{
    if constexpr (is_optional<T>)
        nlohmann::optional_to_json(j, key, value);
    else
        j[key] = value;
}
template <typename T>
void knut_from_json(const char *key, const nlohmann::json &j, T &value)
{
    if constexpr (is_optional<T>)
        nlohmann::optional_from_json(j, key, value);
    else
        j.at(key).get_to(value);
}
}

#define JSON_TO(v1) knut_to_json(#v1, nlohmann_json_j, nlohmann_json_t.v1);
#define JSON_FROM(v1) knut_from_json(#v1, nlohmann_json_j, nlohmann_json_t.v1);

/**
 * \brief Macro use to define serialize and deserialize function for structures
 *
 *   This macro is based on NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE, but takes optional members into account.
 */
#define JSONIFY(Type, ...)                                                                                             \
    inline void to_json(nlohmann::json &nlohmann_json_j, const Type &nlohmann_json_t)                                  \
    {                                                                                                                  \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(JSON_TO, __VA_ARGS__))                                                \
    }                                                                                                                  \
    inline void from_json(const nlohmann::json &nlohmann_json_j, Type &nlohmann_json_t)                                \
    {                                                                                                                  \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(JSON_FROM, __VA_ARGS__))                                              \
    }
