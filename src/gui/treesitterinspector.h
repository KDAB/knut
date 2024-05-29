/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "treesitter/parser.h"
#include "treesittertreemodel.h"

#include <QDialog>
#include <QSyntaxHighlighter>

namespace treesitter {
class Transformation;
class Predicates;
}

namespace Core {
class Document;
class CodeDocument;
}

namespace Gui {
namespace Ui {
    class TreeSitterInspector;
}

class QueryErrorHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    QueryErrorHighlighter(QTextDocument *parent);

    void highlightBlock(const QString &text) override;

    void setUtf8Position(int position);

private:
    int m_errorUtf8Position = -1;
};

class TreeSitterInspector : public QDialog
{
    Q_OBJECT

public:
    explicit TreeSitterInspector(QWidget *parent = nullptr);
    ~TreeSitterInspector() override;

private:
    void showUnnamedChanged();
    void changeCurrentDocument(Core::Document *document);
    void setDocument(Core::CodeDocument *document);
    void changeText();
    void changeCursor();
    void changeQuery();
    void changeQueryState();
    void previewTransformation();
    void runTransformation();
    void prepareTransformation(const std::function<void(treesitter::Transformation &transformation)> &runFunction);

    std::unique_ptr<treesitter::Predicates> makePredicates();

    QString preCheckTransformation() const;

    void changeTreeSelection(const QModelIndex &current, const QModelIndex &previous);

    QString highlightQueryError(const treesitter::Query::Error &error) const;

    Ui::TreeSitterInspector *ui;

    treesitter::Parser m_parser;
    TreeSitterTreeModel m_treemodel;
    QueryErrorHighlighter *m_errorHighlighter;

    Core::CodeDocument *m_document;

    QString m_queryText;
};

} // namespace Gui
