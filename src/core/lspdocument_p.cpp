#include "lspdocument_p.h"
#include "lspdocument.h"

#include "lsp/client.h"
#include "lsp/types.h"

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

} // namespace Core
