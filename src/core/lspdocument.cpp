#include "lspdocument.h"

#include "private/lspcache.h"

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

LspDocument::~LspDocument() = default;

LspDocument::LspDocument(Type type, QObject *parent)
    : TextDocument(type, parent)
    , m_cache(std::make_unique<LspCache>(this))
{
    connect(textEdit()->document(), &QTextDocument::contentsChange, this, &LspDocument::changeContent);
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

    return m_cache->symbols();
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

void LspDocument::changeContent(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(position)
    Q_UNUSED(charsRemoved)
    Q_UNUSED(charsAdded)
    m_cache->clear();
}

} // namespace Core
