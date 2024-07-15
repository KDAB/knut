/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "textview.h"
#include "treesitter/parser.h"

class QTreeView;
class QCheckBox;

namespace treesitter {
class Predicates;
}

namespace Gui {

class TreeSitterTreeModel;

class CodeView : public TextView
{
    Q_OBJECT

public:
    explicit CodeView(QWidget *parent = nullptr);

    void setDocument(Core::TextDocument *document) override;

private:
    void toggleTreeView();
    void toggleUnnamedNodes();

    void initializeCodeModel();
    void changeText();
    void changeCursor();
    void changeTreeSelection(const QModelIndex &current);
    std::unique_ptr<treesitter::Predicates> makePredicates();

    QTreeView *m_treeView = nullptr;
    QCheckBox *m_showUnnamed = nullptr;
    treesitter::Parser m_parser;
    TreeSitterTreeModel *m_treemodel = nullptr;
};

} // namespace Gui
