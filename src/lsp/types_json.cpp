#include "types_json.h"

namespace Lsp {

void to_json(nlohmann::json &j, const FormattingOptions &value)
{
    j["tabSize"] = value.tabSize;
    j["insertSpaces"] = value.insertSpaces;
    nlohmann::optional_to_json(j, "trimTrailingWhitespace", value.trimTrailingWhitespace);
    nlohmann::optional_to_json(j, "insertFinalNewline", value.insertFinalNewline);
    nlohmann::optional_to_json(j, "trimFinalNewlines", value.trimFinalNewlines);

    for (const auto &[key, val] : value.propertyMap)
        j[key] = val;
}

void from_json(const nlohmann::json &j, FormattingOptions &value)
{
    for (const auto &[key, val] : j.items()) {
        if (key == "tabSize")
            value.tabSize = j["tabSize"].get<unsigned int>();
        else if (key == "insertSpaces")
            value.insertSpaces = j["insertSpaces"].get<bool>();
        else if (key == "trimTrailingWhitespace")
            nlohmann::optional_from_json(j, "trimTrailingWhitespace", value.trimTrailingWhitespace);
        else if (key == "insertFinalNewline")
            nlohmann::optional_from_json(j, "insertFinalNewline", value.insertFinalNewline);
        else if (key == "trimFinalNewlines")
            nlohmann::optional_from_json(j, "trimFinalNewlines", value.trimFinalNewlines);
        else {
            for (const auto &[k, v] : val.items())
                value.propertyMap[k] = val.get<std::variant<bool, int, std::string>>();
        }
    }
}

void to_json(nlohmann::json &j, const SelectionRange &value)
{
    j["range"] = value.range;
    if (value.parent)
        j["parent"] = *(value.parent);
}

void from_json(const nlohmann::json &j, SelectionRange &value)
{
    value.range = j["range"].get<Range>();
    if (j.contains("parent")) {
        auto parent = j["parent"].get<SelectionRange>();
        value.parent.reset(new SelectionRange(std::move(parent)));
    }
}

void to_json(nlohmann::json &j, const WorkspaceEdit::ChangeAnnotationsType &value)
{
    for (const auto &[key, val] : value.propertyMap)
        j[key] = val;
}

void from_json(const nlohmann::json &j, WorkspaceEdit::ChangeAnnotationsType &value)
{
    for (const auto &[key, val] : j.items())
        value.propertyMap[key] = val.get<ChangeAnnotation>();
}
}
