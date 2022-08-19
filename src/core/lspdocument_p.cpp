#include "lspdocument_p.h"
#include "lspdocument.h"

#include "cppfunctionsymbol.h"
#include "lsp/client.h"
#include "lsp/types.h"

#include <spdlog/spdlog.h>

namespace Core {

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

const Core::Symbol *LspCache::inferVariable(const QStringList &lines, TextRange range, Symbol::Kind kind)
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
        }
    }

    return Symbol::makeSymbol(m_document, name, type, kind, range, range);
}

const Core::Symbol *LspCache::inferMethod(const QStringList &lines, TextRange range, Symbol::Kind kind)
{
    auto words = lines.first().split(' ');

    if (words.size() < 2) {
        return nullptr;
    }

    words.removeFirst();
    auto name = words.join(' ');

    return Symbol::makeSymbol(m_document, name, "" /* fill description later */, kind, range, range);
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
    while (!lines.isEmpty() && lines.first().isEmpty()) {
        lines.removeFirst();
    }
    auto description = lines.join('\n');

    return Symbol::makeSymbol(m_document, name, description, kind, range, range);
}

const Core::Symbol *LspCache::inferSymbol(const QString &hoverText, TextRange range)
{
    spdlog::debug("Trying to infer Symbol from Hover text:\n{}", hoverText.toStdString());

    auto cached = std::find_if(m_inferredSymbols.cbegin(), m_inferredSymbols.cend(), [&range](const auto symbol) {
        return symbol->range() == range;
    });
    if (cached != m_inferredSymbols.cend()) {
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

} // namespace Core
