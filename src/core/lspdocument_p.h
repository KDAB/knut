#pragma once

#include "rangemark.h"
#include "symbol.h"

#include "lsp/client.h"
#include "lsp/types.h"
#include "treesitter/node.h"
#include "treesitter/parser.h"
#include "treesitter/predicates.h"
#include "treesitter/query.h"
#include "treesitter/tree.h"

#include <QVector>

namespace Core {

class LspDocument;

class LspCache
{
public:
    explicit LspCache(LspDocument *document);

    void clear();

    const QVector<Core::Symbol *> &symbols();

    const Core::Symbol *inferSymbol(const QString &hoverText, TextRange range);

private:
    enum Flags {
        HasSymbols = 0x01,
    };

    const Core::Symbol *inferVariable(const QStringList &lines, TextRange range, Symbol::Kind kind);
    const Core::Symbol *inferMethod(const QStringList &lines, TextRange range, Symbol::Kind kind);
    const Core::Symbol *inferGenericSymbol(QStringList lines, TextRange range);

    LspDocument *const m_document;
    QVector<Core::Symbol *> m_symbols;
    QVector<const Core::Symbol *> m_inferredSymbols;
    int m_flags = 0;
};

class TreeSitterHelper
{
public:
    explicit TreeSitterHelper(LspDocument *document);

    void clear();

    treesitter::Parser &parser();
    std::optional<treesitter::Tree> &syntaxTree();

    std::shared_ptr<treesitter::Query> constructQuery(const QString &query);
    QVector<treesitter::Node> nodesInRange(const RangeMark &range);

private:
    LspDocument *const m_document;
    std::optional<treesitter::Parser> m_parser;
    std::optional<treesitter::Tree> m_tree;
};

} // namespace Core
