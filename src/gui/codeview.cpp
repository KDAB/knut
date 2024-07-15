/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "codeview.h"
#include "core/logger.h"
#include "core/textdocument.h"
#include "guisettings.h"
#include "treesitter/languages.h"
#include "treesitter/predicates.h"
#include "treesittertreemodel.h"

#include <QCheckBox>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

namespace Gui {

CodeView::CodeView(QWidget *parent)
    : TextView(parent)
    , m_parser(nullptr)
{
    auto *action = new QAction(tr("Show TreeSitter Explorer"));
    action->setCheckable(true);
    action->setChecked(false);
    GuiSettings::setIcon(action, ":/gui/file-tree.png");
    connect(action, &QAction::triggered, this, &CodeView::toggleTreeView);
    addAction(action);
}

void CodeView::setDocument(Core::TextDocument *document)
{
    TextView::setDocument(document);

    m_treeView = new QTreeView(this);
    m_showUnnamed = new QCheckBox(tr("Show unnamed nodes"), this);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(document->textEdit());
    splitter->setCollapsible(0, false);

    QWidget *rightWidget = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(rightWidget);
    vbox->setContentsMargins({});
    vbox->addWidget(m_treeView);
    vbox->addWidget(m_showUnnamed);
    splitter->addWidget(rightWidget);

    layout()->addWidget(splitter);

    // By default, the treesitter view is not visible
    rightWidget->setVisible(false);
}

void CodeView::toggleTreeView()
{
    Q_ASSERT(m_treeView && document());
    initializeCodeModel();
    m_treeView->parentWidget()->setVisible(!m_treeView->isVisible());
}

void CodeView::toggleUnnamedNodes()
{
    // technically the text didn't change, but this will force
    // a complete re-parse and re-build of the entire tree.
    changeText();
}

void CodeView::initializeCodeModel()
{
    if (m_treemodel)
        return;

    m_treemodel = new TreeSitterTreeModel(this);
    m_treeView->setModel(m_treemodel);
    m_parser = treesitter::Parser::getLanguage(document()->type());
    connect(document(), &Core::TextDocument::textChanged, this, &CodeView::changeText);
    connect(document(), &Core::TextDocument::positionChanged, this, &CodeView::changeCursor);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &CodeView::changeTreeSelection);
    connect(m_showUnnamed, &QCheckBox::toggled, this, &CodeView::toggleUnnamedNodes);

    changeCursor();
    changeText();
}

void CodeView::changeText()
{
    QString text;
    {
        Core::LoggerDisabler disableLogging;
        text = document()->text();
    }
    auto tree = m_parser.parseString(text);
    if (tree.has_value()) {
        m_treemodel->setTree(std::move(tree.value()), makePredicates(), m_showUnnamed->isChecked());
        m_treeView->expandAll();
        for (int i = 0; i < 2; i++) {
            m_treeView->resizeColumnToContents(i);
        }
    }
}

void CodeView::changeCursor()
{
    int position;
    {
        Core::LoggerDisabler disableLogging;
        position = document()->position();
    }
    m_treemodel->setCursorPosition(position);
}

void CodeView::changeTreeSelection(const QModelIndex &current)
{
    const auto node = m_treemodel->tsNode(current);
    if (node.has_value()) {
        Core::LoggerDisabler disableLogging;
        document()->selectRegion(node->startPosition(), node->endPosition());
    }
}

std::unique_ptr<treesitter::Predicates> CodeView::makePredicates()
{
    Q_ASSERT(document());
    // No need to always log the call to TextDocument::text
    Core::LoggerDisabler disabler;
    return std::make_unique<treesitter::Predicates>(document()->text());
}

} // namespace Gui
