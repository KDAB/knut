#pragma once

#include <QString>
#include <QStringList>

#include <nlohmann/json.hpp>
#include <magic_enum.hpp>

#include <optional>
#include <variant>

///////////////////////////////////////////////////////////////////////////////
// QString
///////////////////////////////////////////////////////////////////////////////
inline void to_json(nlohmann::json &j, const QString &str)
{
    j = nlohmann::json(str.toStdString());
}

inline void from_json(const nlohmann::json &j, QString &str)
{
    str = QString::fromStdString(j.get<std::string>());
}

///////////////////////////////////////////////////////////////////////////////
// QStringList
///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// Enums
///////////////////////////////////////////////////////////////////////////////
// the LSP spec has lower case strings for enums, but we cannot use
// lower case in C++ due to reserved keywords, like 'delete'.
// we do the serialization here by converting the first character to/from lowercase
template <class T>
void enum_to_json(nlohmann::json &j, const char *name, const T &value)
{
    auto jsonString = std::string(magic_enum::enum_name(value));
    jsonString[0] = std::tolower(jsonString[0]);
    j[name] = jsonString;
}

template <class T>
void enum_from_json(const nlohmann::json &j, T &value)
{
    auto jsonString = j.get<std::string>();
    jsonString[0] = std::toupper(jsonString[0]);
    value = magic_enum::enum_cast<T>(jsonString).value();
}

///////////////////////////////////////////////////////////////////////////////
// std::variant
///////////////////////////////////////////////////////////////////////////////
// Try to set the value of type T into the variant data
// if it fails, do nothing
template <typename T, typename... Ts>
void variant_from_json(const nlohmann::json &j, std::variant<Ts...> &data)
{
    try {
        data = j.get<T>();
    } catch (...) {
    }
}

template <typename... Ts>
struct adl_serializer<std::variant<Ts...>>
{
    static void to_json(nlohmann::json &j, const std::variant<Ts...> &data)
    {
        // Will call j = v automatically for the right type
        std::visit([&j](const auto &v) { j = v; }, data);
    }

    static void from_json(const nlohmann::json &j, std::variant<Ts...> &data)
    {
        // Call variant_from_json for all types, only one will succeed
        (variant_from_json<Ts>(j, data), ...);
    }
};

///////////////////////////////////////////////////////////////////////////////
// std::optional
///////////////////////////////////////////////////////////////////////////////
template <typename>
constexpr bool is_optional = false;
template <typename T>
constexpr bool is_optional<std::optional<T>> = true;

template <class T>
void optional_to_json(nlohmann::json &j, const char *name, const std::optional<T> &value)
{
    if (value) {
        if constexpr (std::is_enum<T>::value) {
            enum_to_json(j, name, *value);
        } else {
            j[name] = *value;
        }
    }
}

template <class T>
void optional_from_json(const nlohmann::json &j, const char *name, std::optional<T> &value)
{
    const auto it = j.find(name);
    if (it != j.end()) {
        if constexpr (std::is_enum<T>::value) {
            T enumValue;
            enum_from_json(*it, enumValue);
            value = enumValue;
        } else {
            value = it->get<T>();
        }
    } else {
        value = std::nullopt;
    }
}

///////////////////////////////////////////////////////////////////////////////
// all together
///////////////////////////////////////////////////////////////////////////////
template <typename T>
void knut_to_json(const char *key, nlohmann::json &j, const T &value)
{
    if constexpr (is_optional<T>) {
        nlohmann::optional_to_json(j, key, value);
    } else if constexpr (std::is_enum<T>::value) {
        enum_to_json(j, key, value);
    } else {
        j[key] = value;
    }
}
template <typename T>
void knut_from_json(const char *key, const nlohmann::json &j, T &value)
{
    if constexpr (is_optional<T>) {
        nlohmann::optional_from_json(j, key, value);
    } else if constexpr (std::is_enum<T>::value) {
        enum_from_json(j.at(key), value);
    } else {
        j.at(key).get_to(value);
    }
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
