/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "textview.h"
#include "treesitter/parser.h"

namespace Gui {

class CodeView : public TextView
{
    Q_OBJECT

public:
    explicit CodeView(QWidget *parent = nullptr);

signals:
    void treeSitterExplorerRequested();
};

} // namespace Gui
