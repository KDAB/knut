#include "lspdocument_p.h"
#include "lspdocument.h"

#include "functionsymbol.h"

#include "lsp/client.h"
#include "lsp/lsp_utils.h"
#include "lsp/types.h"
#include "treesitter/languages.h"

#include <kdalgorithms.h>
#include <spdlog/spdlog.h>

namespace Core {

///////////////////////////////////////////////////////////////////////////////
// LspCache
///////////////////////////////////////////////////////////////////////////////
LspCache::LspCache(LspDocument *document)
    : m_document(document)
{
}

void LspCache::clear()
{
    m_symbols.clear();
    m_flags = 0;
}

const QVector<Symbol *> &LspCache::symbols()
{
    if (m_flags & HasSymbols)
        return m_symbols;

    if (!m_document->m_lspClient)
        return m_symbols;

    Lsp::DocumentSymbolParams params;
    params.textDocument.uri = m_document->toUri();
    auto result = m_document->m_lspClient->documentSymbol(std::move(params));
    if (!result)
        return m_symbols;

    // We only supports Lsp::DocumentSymbol for now
    if (!std::holds_alternative<std::vector<Lsp::DocumentSymbol>>(result.value())) {
        Q_ASSERT(std::get<std::vector<Lsp::SymbolInformation>>(result.value()).empty());
        return m_symbols;
    }
    const auto lspSymbols = std::get<std::vector<Lsp::DocumentSymbol>>(result.value());

    // Create a recusive lambda to flatten the hierarchy
    // Add the full symbol name (with namespaces/classes)
    m_symbols.clear();
    const std::function<void(const std::vector<Lsp::DocumentSymbol> &, QString)> fillSymbols =
        [this, &fillSymbols](const std::vector<Lsp::DocumentSymbol> &lspSymbols, QString context) {
            for (const auto &lspSymbol : lspSymbols) {
                auto symbol = Symbol::makeSymbol(m_document, lspSymbol, m_document->toRange(lspSymbol.range),
                                                 m_document->toRange(lspSymbol.selectionRange), context);
                m_symbols.push_back(symbol);

                if (lspSymbol.children) {
                    if (lspSymbol.kind == Lsp::SymbolKind::String) // case for BEGIN_MESSAGE_MAP
                        fillSymbols(lspSymbol.children.value(), context);
                    else
                        fillSymbols(lspSymbol.children.value(), symbol->name());
                }
            }
        };
    fillSymbols(lspSymbols, "");
    m_flags |= HasSymbols;
    return m_symbols;
}

const Core::Symbol *LspCache::inferVariable(QStringList lines, TextRange range, Symbol::Kind kind)
{
    static QString typePrefix("Type: ");
    auto words = lines.first().split(' ');

    if (words.size() < 2) {
        return nullptr;
    }
    // Remove the `param` or `variable` prefix
    words.removeFirst();
    auto name = words.join(' ');

    QString type;
    for (auto line : lines) {
        if (line.startsWith(typePrefix)) {
            type = line.remove(0, typePrefix.size());
            type = Lsp::Utils::removeTypeAliasInformation(type);
        }
    }

    auto importLocation = inferImportLocation(lines);

    return Symbol::makeSymbol(m_document, name, type, importLocation, kind, range, range);
}

QString LspCache::inferImportLocation(QStringList &lines)
{
    auto line = kdalgorithms::mutable_find_if(lines, [](const QString &line) {
        return line.startsWith("provided by ");
    });
    if (line.has_result()) {
        QString importLocation = line->remove(0, 12);
        lines.removeOne(*line);
        return importLocation;
    }
    return "";
}

const Core::Symbol *LspCache::inferMethod(QStringList lines, TextRange range, Symbol::Kind kind)
{
    auto words = lines.first().split(' ');

    if (words.size() < 2) {
        return nullptr;
    }

    words.removeFirst();
    auto name = words.join(' ');

    auto importLocation = inferImportLocation(lines);

    return Symbol::makeSymbol(m_document, name, "" /* fill description later */, importLocation, kind, range, range);
}

const Core::Symbol *LspCache::inferGenericSymbol(QStringList lines, TextRange range)
{
    static const std::unordered_map<QString, Symbol::Kind> nameToSymbol {{"namespace", Symbol::Namespace},
                                                                         {"enumerator", Symbol::Enum},
                                                                         {"class", Symbol::Class},
                                                                         {"struct", Symbol::Struct}};
    auto words = lines.first().split(' ');

    if (words.size() < 2) {
        return nullptr;
    }

    auto kindIter = nameToSymbol.find(words.first());
    if (kindIter == nameToSymbol.cend()) {
        return nullptr;
    }
    auto kind = kindIter->second;
    words.removeFirst();
    auto name = words.join(' ');

    lines.removeFirst();
    auto importLocation = inferImportLocation(lines);

    while (!lines.isEmpty() && lines.first().isEmpty()) {
        lines.removeFirst();
    }
    auto description = lines.join('\n');

    return Symbol::makeSymbol(m_document, name, description, importLocation, kind, range, range);
}

const Core::Symbol *LspCache::inferSymbol(const QString &hoverText, TextRange range)
{
    spdlog::debug("Trying to infer Symbol from Hover text:\n{}", hoverText.toStdString());

    auto cached = kdalgorithms::find_if(m_inferredSymbols, [&range](const auto symbol) {
        return symbol->range() == range;
    });
    if (cached) {
        return *cached;
    }

    auto lines = hoverText.split('\n');
    if (lines.isEmpty()) {
        return nullptr;
    }

    auto words = lines.first().split(' ');
    if (words.size() < 2) {
        return nullptr;
    }

    const Symbol *result;
    auto kind = words.first();
    if (kind == "param" || kind == "variable") {
        result = inferVariable(lines, range, Symbol::Variable);
    } else if (kind == "field") {
        result = inferVariable(lines, range, Symbol::Field);
    } else if (kind == "instance-method") {
        result = inferMethod(lines, range, Symbol::Method);
    } else if (kind == "function") {
        result = inferMethod(lines, range, Symbol::Function);
    } else {
        result = inferGenericSymbol(std::move(lines), range);
    }

    m_inferredSymbols.emplace_back(result);

    return result;
}

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
    } catch (treesitter::Query::Error error) {
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

} // namespace Core
