#pragma once

#include "json_utils.h"

#include <unordered_map>

namespace Core {

//! Store ToggleSection settings
struct ToggleSectionSettings
{
    QString tag;
    QString debug;
    std::unordered_map<std::string, std::string> return_values;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ToggleSectionSettings, tag, debug, return_values);

} // namespace Core
