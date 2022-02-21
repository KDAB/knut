#include "lspdocument.h"

#include "lsp/client.h"
#include "lsp/types.h"

#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextDocument>

#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype LspDocument
 * \brief Base document object for document using LSP.
 * \instantiates Core::LspDocument
 * \inqmlmodule Script
 * \since 4.0
 * \inherits TextDocument
 */

LspDocument::LspDocument(QObject *parent)
    : TextDocument(parent)
{
}

LspDocument::~LspDocument() = default;

LspDocument::LspDocument(Type type, QObject *parent)
    : TextDocument(type, parent)
{
}

void LspDocument::setLspClient(Lsp::Client *client)
{
    m_lspClient = client;
}

/*!
 * \qmlmethod vector<Symbol> LspDocument::symbols()
 * Returns the list of symbols in the current document.
 */
QVector<Core::Symbol> LspDocument::symbols() const
{
    spdlog::trace("LspDocument::symbols");

    if (!client())
        return {};

    // TODO cache the data
    Lsp::DocumentSymbolParams params;
    params.textDocument.uri = toUri();
    auto result = client()->documentSymbol(std::move(params));
    if (!result)
        return {};

    // Wee only supports Lsp::DocumentSymbol for now
    Q_ASSERT(std::holds_alternative<std::vector<Lsp::DocumentSymbol>>(result.value()));
    const auto lspSymbols = std::get<std::vector<Lsp::DocumentSymbol>>(result.value());
    QVector<Symbol> symbols;

    // Create a recusive lambda to flatten the hierarchy
    // Add the full symbol name (with namespaces/classes)
    const std::function<void(const std::vector<Lsp::DocumentSymbol> &, QString)> fillSymbols =
        [this, &symbols, &fillSymbols](const std::vector<Lsp::DocumentSymbol> &lspSymbols, QString context) {
            for (const auto &lspSymbol : lspSymbols) {
                const QString description = lspSymbol.detail ? QString::fromStdString(lspSymbol.detail.value()) : "";
                QString name = QString::fromStdString(lspSymbol.name);
                if (!context.isEmpty())
                    name = context + "::" + name;
                symbols.push_back(Symbol {name, description, static_cast<Core::Symbol::Kind>(lspSymbol.kind),
                                          toRange(lspSymbol.range), toRange(lspSymbol.selectionRange)});
                if (lspSymbol.children) {
                    if (lspSymbol.kind == Lsp::SymbolKind::String) // case for BEGIN_MESSAGE_MAP
                        fillSymbols(lspSymbol.children.value(), context);
                    else
                        fillSymbols(lspSymbol.children.value(), name);
                }
            }
        };
    fillSymbols(lspSymbols, "");

    return symbols;
}

void LspDocument::didOpen()
{
    if (!m_lspClient)
        return;

    Lsp::DidOpenTextDocumentParams params;
    params.textDocument.uri = toUri();
    params.textDocument.version = revision();
    params.textDocument.text = textEdit()->toPlainText().toStdString();
    params.textDocument.languageId = m_lspClient->languageId();

    m_lspClient->didOpen(std::move(params));
}

void LspDocument::didClose()
{
    if (!m_lspClient)
        return;

    Lsp::DidCloseTextDocumentParams params;
    params.textDocument.uri = toUri();

    m_lspClient->didClose(std::move(params));
}

Lsp::Client *LspDocument::client() const
{
    return m_lspClient;
}

std::string LspDocument::toUri() const
{
    return QUrl::fromLocalFile(fileName()).toString().toStdString();
}

int LspDocument::toPos(const Lsp::Position &pos) const
{
    // Internally, columns are 0-based, like in LSP
    const int blockNumber = qMin((int)pos.line, textEdit()->document()->blockCount() - 1);
    const QTextBlock &block = textEdit()->document()->findBlockByNumber(blockNumber);
    if (block.isValid()) {
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.character);
        return cursor.position();
    }
    return 0;
}

TextRange LspDocument::toRange(const Lsp::Range &range) const
{
    return {toPos(range.start), toPos(range.end)};
}

} // namespace Core
