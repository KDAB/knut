/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

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
