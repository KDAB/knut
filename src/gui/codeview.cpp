/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "codeview.h"
#include "guisettings.h"

namespace Gui {

CodeView::CodeView(QWidget *parent)
    : TextView(parent)
{
    auto *action = new QAction(tr("Show TreeSitter Explorer"));
    GuiSettings::setIcon(action, ":/gui/file-tree.png");
    connect(action, &QAction::triggered, this, &CodeView::treeSitterExplorerRequested);
    addAction(action);
}

} // namespace Gui
