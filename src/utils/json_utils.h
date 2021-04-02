#pragma once

#include <nlohmann/json.hpp>

#include <optional>

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

}
