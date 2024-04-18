#pragma once

#include "utils/json.h"

class QPlainTextEdit;

namespace Core {

//! Store tab settings for text editor
struct TabSettings
{
    bool insertSpaces = true;
    int tabSize = 4;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TabSettings, insertSpaces, tabSize);

void indentTextInTextEdit(QPlainTextEdit *textEdit, int tabCount);
void gotoLineInTextEdit(QPlainTextEdit *textEdit, int line, int column = 1);

} // namespace Core
