/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "node.h"

namespace treesitter {

class TreeCursor
{
public:
    TreeCursor(Node);
    TreeCursor(const TreeCursor &) = delete;
    TreeCursor(TreeCursor &&) = delete;

    Node currentNode() const;
    /// This returns a null QString if there is no field name
    QString currentFieldName() const;

    bool gotoFirstChild();
    bool gotoNextSibling();
    bool gotoParent();

private:
    TSTreeCursor m_cursor;
};
}
