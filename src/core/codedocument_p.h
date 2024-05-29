#pragma once

#include "rangemark.h"
#include "symbol.h"
#include "treesitter/node.h"
#include "treesitter/parser.h"
#include "treesitter/query.h"
#include "treesitter/tree.h"

#include <QVector>

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
    QVector<treesitter::Node> nodesInRange(const RangeMark &range);

    const QVector<Core::Symbol *> &symbols();

private:
    void assignSymbolContexts();

    QVector<Core::Symbol *> functionSymbols() const;
    QVector<Core::Symbol *> classSymbols() const;
    QVector<Core::Symbol *> memberSymbols() const;
    QVector<Core::Symbol *> enumSymbols() const;

    enum Flags {
        HasSymbols = 0x01,
    };

    CodeDocument *const m_document;
    std::optional<treesitter::Parser> m_parser;
    std::optional<treesitter::Tree> m_tree;
    QVector<Core::Symbol *> m_symbols;
    int m_flags = 0;
};

} // namespace Core
