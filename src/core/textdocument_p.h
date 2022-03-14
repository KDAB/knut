#pragma once

#include "json_utils.h"

namespace Core {

//! Store tab settings for text editor
struct TabSettings
{
    bool insertSpaces = true;
    int tabSize = 4;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TabSettings, insertSpaces, tabSize);

} // namespace Core
