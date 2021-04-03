#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <variant>

namespace Lsp {

using MessageId = std::variant<int, std::string>;

using IntegerOrNull = std::variant<int, nullptr_t>;
}

// Serialization
////////////////
namespace nlohmann {
template <>
struct adl_serializer<Lsp::MessageId>
{
    static void to_json(nlohmann::json &j, const Lsp::MessageId &data)
    {
        if (std::holds_alternative<int>(data))
            j = std::get<int>(data);
        else
            j = std::get<std::string>(data);
    }

    static void from_json(const nlohmann::json &j, Lsp::MessageId &data)
    {
        if (j.is_number())
            data = j.get<int>();
        else
            data = j.get<std::string>();
    }
};

template <>
struct adl_serializer<Lsp::IntegerOrNull>
{
    static void to_json(nlohmann::json &j, const Lsp::IntegerOrNull &data)
    {
        if (std::holds_alternative<int>(data))
            j = std::get<int>(data);
        else
            j = nullptr;
    }

    static void from_json(const nlohmann::json &j, Lsp::IntegerOrNull &data)
    {
        if (j.is_number())
            data = j.get<int>();
        else
            data = nullptr;
    }
};

}
