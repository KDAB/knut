#include "lspdocument_p.h"
#include "lspdocument.h"

#include "treesitter/languages.h"

#include <kdalgorithms.h>
#include <spdlog/spdlog.h>

namespace Core {

///////////////////////////////////////////////////////////////////////////////
// TreeSitterHelper
///////////////////////////////////////////////////////////////////////////////
TreeSitterHelper::TreeSitterHelper(LspDocument *document)
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
        // TODO: Make language configurable
        m_parser = treesitter::Parser(tree_sitter_cpp());
    }

    // Regarding const-ness:
    // The TreeSitter parser can't change anything about the documents contents.
    // Therefore it is okay to be returned by non-const reference, even in a const function.
    return const_cast<treesitter::Parser &>(*m_parser);
}

std::optional<treesitter::Tree> &TreeSitterHelper::syntaxTree()
{
    if (!m_tree) {
        m_tree = parser().parseString(m_document->text());
        if (!m_tree) {
            spdlog::warn("LspDocument::syntaxTree: Failed to parse document {}!", m_document->fileName().toStdString());
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
        spdlog::error("LspDocument::constructQuery: Failed to parse query `{}` error: {} at: {}", query.toStdString(),
                      error.description.toStdString(), error.utf8_offset);
        return {};
    }
    return tsQuery;
}

// `nodesInRange` returns only the outermost nodes that fit entirely in the given range.
// The subsequent children of these outermost nodes are *not* returned, even though
// they are also technically in the range!
// This is used by queryInRange to find on which nodes to run the query on.
QVector<treesitter::Node> TreeSitterHelper::nodesInRange(const RangeMark &range)
{
    enum RangeComparison { Overlaps, Contains, Disjoint };

    const auto &tree = syntaxTree();

    if (!tree) {
        return {};
    }

    QVector<treesitter::Node> nodesToVisit;
    QVector<treesitter::Node> nodesInRange;
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

QVector<Core::Symbol *> TreeSitterHelper::functionSymbols() const
{
    auto functionDeclarator = R"EOF(
            (function_declarator
              declarator: [
                (identifier) @selectionRange
                (field_identifier) @selectionRange
                (_ [(identifier) (field_identifier)] @selectionRange)
                (_ (_ [(identifier) (field_identifier)] @selectionRange))
              ] @name

              parameters: (parameter_list
                ; Cool trick: We can use [(type) @capture _]* to capture specific node types
                ; and ignore all others, like comments, "," and so on.
                [(parameter_declaration) @parameter _]*) @parameters

              (trailing_return_type (_) @return)? )
    )EOF";

    auto pointerDeclarator = QString(R"EOF(
        [%1
        (_ "*"? @return "&"? @return "&&"? @return %1)
        (_ ["*" "&" "&&"]? @return
            (_ ["*" "&" "&&"]? @return %1))]
    )EOF")
                                 .arg(functionDeclarator);

    // TODO: Add support for pointers & references
    auto functions = m_document->query(QString(R"EOF(
                        [; Free functions
                        (function_definition
                          type: (_)? @return
                          ; If using trailing return type, we need to remove the auto type at the start
                          (#exclude! @return placeholder_type_specifier)
                          declarator: %2) @range

                        ; Constructor/Destructors
                        (declaration
                          declarator: %1) @range

                        ; Member functions
                        (field_declaration
                          type: (_)? @return
                          ; If using trailing return type, we need to remove the auto type at the start
                          (#exclude! @return placeholder_type_specifier)
                          declarator: %2) @range

                        ])EOF")
                                           .arg(functionDeclarator, pointerDeclarator));

    auto function_to_symbol = [this](const QueryMatch &match) {
        auto kind = Symbol::Kind::Function;
        if (!match.get("return").isValid()) {
            // No return type, this is a Constructor/Destructor
            // Clangd also assigned the Constructor kind to Destructors, so we'll do the same
            kind = Symbol::Kind::Constructor;
        } else if (match.get("name").text().contains("::")) {
            // This is a bit of a guesstimate, but if the function name contains "::", it's likely a method.
            // It may also be a member of a namespace, but this information isn't really available unless we try
            // to resolve the original declaration.
            kind = Symbol::Kind::Method;
        }
        return Symbol::makeSymbol(m_document, match, kind);
    };

    return kdalgorithms::transformed<QVector<Symbol *>>(functions, function_to_symbol);
}

QVector<Core::Symbol *> TreeSitterHelper::classSymbols() const
{
    auto classesAndStructs = m_document->query(QString(R"EOF(
            (class_specifier
              name: (_) @name @selectionRange
              body: (field_declaration_list)) @range

            (struct_specifier
              name: (_) @name @selectionRange
              body: (field_declaration_list)) @range
    )EOF"));
    auto class_to_symbol = [this](const QueryMatch &match) {
        return Symbol::makeSymbol(m_document, match, Symbol::Kind::Class);
    };

    return kdalgorithms::transformed<QVector<Symbol *>>(classesAndStructs, class_to_symbol);
}

QVector<Core::Symbol *> TreeSitterHelper::memberSymbols() const
{
    auto fieldIdentifier = "(field_identifier) @name @selectionRange";
    auto members = m_document->query(QString(R"EOF(
                                        (field_declaration
                                          type: (_) @type
                                          declarator: [
                                            %1
                                            (_ %1) @decl_type
                                            (_ (_ %1) @decl_type) @decl_type
                                          ]
                                          ; We need to filter out functions, they are already captured
                                          ; by the functionSymbols query
                                          (#not_is? @decl_type function_declarator)) @range)EOF")
                                         .arg(fieldIdentifier));

    auto member_to_symbol = [this](const QueryMatch &match) {
        return Symbol::makeSymbol(m_document, match, Symbol::Kind::Field);
    };

    return kdalgorithms::transformed<QVector<Symbol *>>(members, member_to_symbol);
}

QVector<Core::Symbol *> TreeSitterHelper::enumSymbols() const
{
    auto enums = m_document->query(R"EOF(
        (enum_specifier
          name: (_) @name @selectionRange) @range
    )EOF");
    auto enum_to_symbol = [this](const QueryMatch &match) {
        return Symbol::makeSymbol(m_document, match, Symbol::Kind::Enum);
    };
    auto result = kdalgorithms::transformed<QVector<Symbol *>>(enums, enum_to_symbol);

    auto enumerators = m_document->query(R"EOF(
        (enumerator
          name: (_) @name @selectionRange
          value: (_)? @value) @range
    )EOF");
    result.append(kdalgorithms::transformed<QVector<Symbol *>>(enumerators, enum_to_symbol));

    return result;
}

const QVector<Core::Symbol *> &TreeSitterHelper::symbols()
{
    if (m_flags & HasSymbols)
        return m_symbols;

    m_flags |= HasSymbols;

    m_symbols = classSymbols();
    m_symbols.append(functionSymbols());
    m_symbols.append(memberSymbols());
    m_symbols.append(enumSymbols());

    kdalgorithms::sort_by(m_symbols, [](const auto &symbol) {
        return symbol->range().start;
    });

    assignSymbolContexts();

    return m_symbols;
}

} // namespace Core
