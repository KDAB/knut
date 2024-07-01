/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "codedocument_p.h"
#include "codedocument.h"
#include "treesitter/languages.h"
#include "treesitter/tree_cursor.h"
#include "utils/log.h"

#include <kdalgorithms.h>

namespace Core {

///////////////////////////////////////////////////////////////////////////////
// TreeSitterHelper
///////////////////////////////////////////////////////////////////////////////
TreeSitterHelper::TreeSitterHelper(CodeDocument *document)
    : m_document(document)
{
}

void TreeSitterHelper::clear()
{
    m_tree = {};
    m_symbols.clear();
    m_flags &= ~HasSymbols;
}

treesitter::Parser &TreeSitterHelper::parser()
{
    if (!m_parser) {
        m_parser = treesitter::Parser(treesitter::Parser::getLanguage(m_document->type()));
    }

    // Regarding const-ness:
    // The TreeSitter parser can't change anything about the documents contents.
    // Therefore it is okay to be returned by non-const reference, even in a const function.
    return const_cast<treesitter::Parser &>(*m_parser);
}

std::optional<treesitter::Tree> &TreeSitterHelper::syntaxTree()
{
    if (!m_tree) {
        auto &parser = this->parser();
        if (!parser.setIncludedRanges(m_document->includedRanges())) {
            spdlog::warn("TreeSitterHelper::syntaxTree: Unable to set the included ranges on the treesitter parser!");
            parser.setIncludedRanges({});
        }
        m_tree = parser.parseString(m_document->text());
        if (!m_tree) {
            spdlog::warn("CodeDocument::syntaxTree: Failed to parse document {}!", m_document->fileName());
        }
    }
    return m_tree;
}

std::shared_ptr<treesitter::Query> TreeSitterHelper::constructQuery(const QString &query)
{
    std::shared_ptr<treesitter::Query> tsQuery;
    try {
        tsQuery = std::make_shared<treesitter::Query>(parser().language(), query);
    } catch (treesitter::Query::Error &error) {
        spdlog::error("CodeDocument::constructQuery: Failed to parse query `{}` error: {} at: {}", query,
                      error.description, error.utf8_offset);
        return {};
    }
    return tsQuery;
}

// `nodesInRange` returns only the outermost nodes that fit entirely in the given range.
// The subsequent children of these outermost nodes are *not* returned, even though
// they are also technically in the range!
// This is used by queryInRange to find on which nodes to run the query on.
QList<treesitter::Node> TreeSitterHelper::nodesInRange(const RangeMark &range)
{
    enum RangeComparison { Overlaps, Contains, Disjoint };

    const auto &tree = syntaxTree();

    if (!tree) {
        return {};
    }

    QList<treesitter::Node> nodesToVisit;
    QList<treesitter::Node> nodesInRange;
    nodesToVisit.emplace_back(tree->rootNode());

    auto compareToRange = [&range](const treesitter::Node &node) {
        if (range.contains(node.startPosition()) && range.contains(node.endPosition() - 1))
            return RangeComparison::Contains;
        else if (static_cast<int>(node.startPosition()) <= range.end()
                 && static_cast<int>(node.endPosition()) >= range.start())
            return RangeComparison::Overlaps;
        return RangeComparison::Disjoint;
    };

    // Note: This could be improved performance-wise using the first_child_for_byte
    // functions on either TSNode, or TSTreeCursor.
    // However, these functions aren't currently wrapped and would make the search a bit more complex.
    while (!nodesToVisit.isEmpty()) {
        auto node = nodesToVisit.takeLast();
        switch (compareToRange(node)) {
        case RangeComparison::Contains:
            nodesInRange.emplace_back(node);
            break;
        case RangeComparison::Overlaps:
            nodesToVisit.append(node.children());
            break;
        default:
            break;
        }
    }

    return nodesInRange;
}

treesitter::Node TreeSitterHelper::nodeCoveringRange(int start, int end)
{
    auto coversRange = [start, end](const treesitter::Node &node) {
        return static_cast<int>(node.startPosition()) <= start && end <= static_cast<int>(node.endPosition());
    };

    auto coveringNode = syntaxTree()->rootNode();

    auto cursor = treesitter::TreeCursor(coveringNode);

    bool node_changed = cursor.gotoFirstChild();
    while (node_changed) {
        if (coversRange(cursor.currentNode())) {
            coveringNode = cursor.currentNode();
            node_changed = cursor.gotoFirstChild();
        } else {
            node_changed = cursor.gotoNextSibling();
        }
    }

    return coveringNode;
}

void TreeSitterHelper::assignSymbolContexts()
{
    auto contextForSymbol = [this](Symbol *symbol) {
        auto surroundsSymbol = [&symbol](const Symbol *otherSymbol) {
            return symbol != otherSymbol && otherSymbol->range().contains(symbol->range());
        };
        auto surroundingSymbols = kdalgorithms::filtered(m_symbols, surroundsSymbol);

        kdalgorithms::sort_by(
            surroundingSymbols,
            [](const auto &symbol) {
                return symbol->range().length();
            },
            kdalgorithms::descending);
        return surroundingSymbols;
    };

    for (const auto &symbol : m_symbols | std::views::reverse) {
        symbol->assignContext(contextForSymbol(symbol));
    }
}

const QList<Core::Symbol *> &TreeSitterHelper::symbols()
{
    if (m_flags & HasSymbols)
        return m_symbols;

    m_flags |= HasSymbols;

    m_symbols = querySymbols(m_document);

    kdalgorithms::sort_by(m_symbols, [](const auto &symbol) {
        return symbol->range().start();
    });

    assignSymbolContexts();

    return m_symbols;
}

} // namespace Core
