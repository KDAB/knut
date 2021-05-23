#include "types_json.h"

#include "types.h"

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
            value.tabSize = j["tabSize"];
        else if (key == "insertSpaces")
            value.insertSpaces = j["insertSpaces"];
        else if (key == "trimTrailingWhitespace")
            value.trimTrailingWhitespace = j["trimTrailingWhitespace"];
        else if (key == "insertFinalNewline")
            value.insertFinalNewline = j["insertFinalNewline"];
        else if (key == "trimFinalNewlines")
            value.trimFinalNewlines = j["trimFinalNewlines"];
        else
            value.propertyMap[key] = val;
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
    value.range = j["range"];
    if (j.contains("parent")) {
        SelectionRange parent = j["parent"];
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
        value.propertyMap[key] = val;
}
}
