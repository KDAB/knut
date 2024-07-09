/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "rangemark.h"
#include "symbol.h"
#include "treesitter/node.h"
#include "treesitter/parser.h"
#include "treesitter/query.h"
#include "treesitter/tree.h"

#include <QList>

namespace Core {

class CodeDocument;

class TreeSitterHelper
{
public:
    explicit TreeSitterHelper(CodeDocument *document);

    void clear();

    treesitter::Parser &parser();
    std::optional<treesitter::Tree> &syntaxTree();

    std::shared_ptr<treesitter::Query> constructQuery(const QString &query);
    QList<treesitter::Node> nodesInRange(const RangeMark &range);

    const QList<Core::Symbol *> &symbols();

private:
    void assignSymbolContexts();

    QList<Core::Symbol *> functionSymbols() const;
    QList<Core::Symbol *> classSymbols() const;
    QList<Core::Symbol *> memberSymbols() const;
    QList<Core::Symbol *> enumSymbols() const;

    enum Flags {
        HasSymbols = 0x01,
    };

    CodeDocument *const m_document;
    std::optional<treesitter::Parser> m_parser;
    std::optional<treesitter::Tree> m_tree;
    QList<Core::Symbol *> m_symbols;
    int m_flags = 0;
};

} // namespace Core
