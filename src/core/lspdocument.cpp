#include "lspdocument.h"

#include "logger.h"
#include "private/lspcache.h"
#include "string_utils.h"

#include "lsp/client.h"
#include "lsp/types.h"

#include "project.h"
#include "symbol.h"

#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextDocument>

#include <spdlog/spdlog.h>

#include <algorithm>

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

bool LspDocument::hasLspClient() const
{
    return m_lspClient != nullptr;
}

/*!
 * \qmlmethod vector<Symbol> LspDocument::symbols()
 * Returns the list of symbols in the current document.
 */
QVector<Core::Symbol> LspDocument::symbols() const
{
    LOG("LspDocument::symbols");

    return m_cache->symbols();
}

Document *LspDocument::followSymbol()
{
    LOG("LspDocument::followSymbol");

    if (!client()) {
        spdlog::error("followSymbol: LspDocument is missing LSP client!");
        return nullptr;
    }

    Q_ASSERT(textEdit());

    auto cursor = textEdit()->textCursor();
    // Set the cursor position to the beginning of any selected text.
    // That way, calling followSymbol twice in a row causes Clangd
    // to switch between delcaration and definition.
    cursor.setPosition(cursor.selectionStart());
    auto line = cursor.blockNumber();
    auto character = cursor.positionInBlock();

    Lsp::DeclarationParams params;
    params.textDocument.uri = toUri();
    params.position.line = line;
    params.position.character = character;

    // At least with clangd, the "declaration" LSP call acts like followSymbol, it will:
    // - Go to the declaration, if the symbol under cursor is a use
    // - Go to the declaration, if the symbol under cursor is a definition
    // - Go to the definition, if the symbol under cursor is a declaration
    auto result = client()->declaration(std::move(params));

    if (!result.has_value()) {
        spdlog::warn("followSymbol: Empty return value!");
        return nullptr;
    }

    auto locations = std::vector<Lsp::Location>();

    if (std::holds_alternative<Lsp::Location>(*result)) {
        auto location = std::get<Lsp::Location>(*result);
        locations.push_back(location);
    } else if (std::holds_alternative<std::vector<Lsp::Location>>(*result)) {
        locations = std::move(std::get<std::vector<Lsp::Location>>(*result));

    } else if (std::holds_alternative<std::vector<Lsp::LocationLink>>(*result)) {
        auto locationLinks = std::get<std::vector<Lsp::LocationLink>>(*result);

        for (const auto &link : locationLinks) {
            Lsp::Location location = {.uri = link.targetUri, .range = link.targetSelectionRange};
            locations.push_back(location);
        }
    } else {
        spdlog::warn("followSymbol: Unknown return type!");
    }

    if (locations.empty()) {
        return nullptr;
    }

    if (locations.size() > 1) {
        spdlog::warn("followSymbol: Multiple locations returned!");
    }
    // Heuristic: If multiple locations were found, use the last one.
    auto location = locations.back();

    QUrl url(QString::fromStdString(location.uri));

    Q_ASSERT(url.isLocalFile());
    auto filepath = url.toLocalFile();

    auto *document = Project::instance()->open(filepath);

    if (document) {
        auto *lspDocument = dynamic_cast<LspDocument *>(document);
        if (lspDocument) {
            auto cursor = lspDocument->textEdit()->textCursor();

            lspDocument->selectRange(lspDocument->toRange(location.range));
        } else {
            spdlog::warn("followSymbol: Opened document '{}' is not an LspDocument",
                         document->fileName().toStdString());
        }
    }

    return document;
}

Document *LspDocument::switchDeclarationDefinition()
{
    LOG("LspDocument::switchDeclarationDefinition");

    Q_ASSERT(textEdit());

    auto cursor = textEdit()->textCursor();
    auto symbolList = symbols();

    auto currentFunction = std::find_if(symbolList.begin(), symbolList.end(), [&cursor](const auto &symbol) {
        auto isInRange = symbol.range.start <= cursor.position() && cursor.position() <= symbol.range.end;
        auto isFunction = symbol.kind == Symbol::Kind::Function || symbol.kind == Symbol::Kind::Constructor
            || symbol.kind == Symbol::Kind::Method;
        return isInRange && isFunction;
    });

    if (currentFunction == symbolList.end()) {
        spdlog::info("switchDeclarationDefinition: Cursor is currently not within a function!");
        return nullptr;
    }

    selectRange(currentFunction->selectionRange);

    return followSymbol();
}

/*!
 * \qmlmethod void LspDocument::selectSymbol( string name, int options = TextDocument.NoFindFlags)
 * Select a symbol based on its `name`, using different find `options`.
 *
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only fully qualified symbol
 * - `TextDocument.FindRegexp`: use a regexp
 *
 * If no symbols are found, do nothing.
 */
void LspDocument::selectSymbol(const QString &name, int options)
{
    LOG("LspDocument::selectSymbol", name, options);
    auto symbol = findSymbol(name, options);
    if (!symbol.isNull())
        selectRange(symbol.selectionRange);
}

/*!
 * \qmlmethod Symbol LspDocument::findSymbol( string name, int options = TextDocument.NoFindFlags)
 * Find a symbol based on its `name`, using different find `options`.
 *
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only fully qualified symbol
 * - `TextDocument.FindRegexp`: use a regexp
 */
Symbol LspDocument::findSymbol(const QString &name, int options) const
{
    LOG("LspDocument::findSymbol", name, options);

    const auto &symbols = m_cache->symbols();
    const auto regexp = (options & FindRegexp) ? createRegularExpression(name, options) : QRegularExpression {};
    auto byName = [name, options, regexp](const Symbol &symbol) {
        if (options & FindWholeWords)
            return symbol.name.compare(name, (options & FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive)
                == 0;
        else if (options & FindRegexp)
            return regexp.match(symbol.name).hasMatch();
        else
            return symbol.name.contains(name,
                                        (options & FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive);
    };
    auto it = std::ranges::find_if(symbols, byName);
    if (it != symbols.end())
        return *it;
    return {};
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
