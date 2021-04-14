#pragma once

#include <QString>
#include <QStringList>

#include <nlohmann/json.hpp>
#include <magic_enum.hpp>

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

template <typename T>
void optional_enum_to_json(nlohmann::json &j, const char *name, const std::optional<T> &value)
{
    if (value) {
        auto jsonString = std::string(magic_enum::enum_name(*value));
        jsonString[0] = std::tolower(jsonString[0]);
        j[name] = jsonString;
    }
}

template <typename E>
void optional_enum_from_json(const nlohmann::json &j, const char *name, std::optional<E> &value)
{
    const auto it = j.find(name);
    if (it != j.end()) {
        auto jsonString = it->get<std::string>();
        jsonString[0] = std::toupper(jsonString[0]);
        value = magic_enum::enum_cast<E>(jsonString);
    } else {
        value = std::nullopt;
    }
}

}
