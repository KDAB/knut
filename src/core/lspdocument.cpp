#include "lspdocument.h"
#include "lspdocument_p.h"

#include "json_utils.h"
#include "logger.h"
#include "string_utils.h"

#include "lsp/client.h"
#include "lsp/types.h"

#include "project.h"
#include "symbol.h"

#include "scriptmanager.h"

#include <QFile>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextStream>

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

/**
 * Returns the symbol the cursor is in, or an empty symbol otherwise
 * The function is used to filter out the symbol
 *
 * Note that the returned \c Symbol pointer is only valid until the document
 * it originates from is deconstructed.
 */
Symbol *LspDocument::currentSymbol(std::function<bool(const Symbol &)> filterFunc) const
{
    const int pos = textEdit()->textCursor().position();

    auto symbolList = symbols();
    std::ranges::reverse(symbolList);
    for (auto symbol : symbolList) {
        if (symbol->range().contains(pos) && (!filterFunc || filterFunc(*symbol)))
            return symbol;
    }
    return {};
}

/**
 * Deletes the specified symbols text range, as well as leading whitespace
 * and trailing semicolon/newline character.
 */
void LspDocument::deleteSymbol(const Symbol &symbol)
{
    auto range = symbol.range();

    // Include any leading whitespace (excluding newlines).
    auto leading = Core::TextRange {range.start, range.start + 1};
    while (leading.start > 0) {
        leading.start--;
        leading.end--;
        selectRange(leading);

        if (selectedText() != " " && selectedText() != "\t") {
            break;
        }

        range.start--;
    }

    // Include a trailing semicolon and up to one trailing newline
    auto trailing = Core::TextRange {range.end, range.end + 1};
    selectRange(trailing);
    if (selectedText() == ";") {
        range.end++;

        trailing.start++;
        trailing.end++;
        selectRange(trailing);
    }

    if (selectedText() == "\n") {
        range.end++;
    }

    this->selectRange(range);
    this->deleteSelection();
}

/*!
 * \qmlmethod vector<Symbol> LspDocument::symbols()
 * Returns the list of symbols in the current document.
 *
 * Note that the returned \c Symbol pointers are only valid until the document they
 * originate from is deconstructed.
 */
QVector<Core::Symbol *> LspDocument::symbols() const
{
    LOG("LspDocument::symbols");
    if (!checkClient())
        return {};
    return m_cache->symbols();
}

struct RegexpTransform
{
    QString from;
    QString to;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RegexpTransform, from, to);

struct Transforms
{
    std::vector<RegexpTransform> patterns;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Transforms, patterns);

void LspDocument::regexpTransform(const RegexpTransform &transform,
                                  const std::unordered_map<QString, QString> &regexpContext)
{
    auto from = transform.from;
    auto to = transform.to;

    for (const auto &contextPair : regexpContext) {
        auto key = QString("${%1}").arg(QRegularExpression::escape(contextPair.first));
        auto value = QRegularExpression::escape(contextPair.second);
        from.replace(key, value);
        to.replace(key, value);
    }

    // Use the FindBackward flag, so that nested Regexp Transforms will
    // replace the "inner" item first.
    replaceAllRegexp(from, to, TextDocument::FindBackward | TextDocument::PreserveCase);
}

/*!
 * \qmlmethod LspDocument::transformSymbol(const QString &symbolName, const QString &jsonFileName)
 *
 * Runs a list of transformations defined in a JSON file on the given `symbolName`.
 * The JSON file is loaded from the path specified in `jsonFileName`.
 * \todo
 */
void LspDocument::transformSymbol(const QString &symbolName, const QString &jsonFileName)
{
    LOG("LspDocument::transformSymbol", LOG_ARG("text", symbolName), jsonFileName);

    QFile file(jsonFileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        spdlog::error("LspDocument::transformSymbol - Could not open JSON file: '{}'", jsonFileName.toStdString());
        return;
    }
    QTextStream in(&file);

    nlohmann::json transformJson;
    try {
        transformJson = nlohmann::json::parse(in.readAll().toStdString());
    } catch (nlohmann::json::exception &exception) {
        spdlog::error("LspDocument::transformSymbol - JSON parsing failed: {}", exception.what());
        return;
    }

    try {
        auto transforms = transformJson.get<Transforms>();

        std::unordered_map<QString, QString> regexpContext;
        regexpContext["symbol"] = symbolName;
        for (const auto &pattern : transforms.patterns) {
            regexpTransform(pattern, regexpContext);
        }
    } catch (nlohmann::json::exception &exception) {
        spdlog::error("LspDocument::transformSymbol - Not a valid Transform JSON: {}", exception.what());
        return;
    }
}

/*!
 * \qmlmethod string LspDocument::hover()
 *
 * Returns information about the symbol at the current cursor position.
 * The result of this call is a plain string that may be formatted in Markdown.
 */
QString LspDocument::hover() const
{
    return hover(textEdit()->textCursor().position());
}

QString LspDocument::hover(int position) const
{
    LOG("LspDocument::hover");

    if (!checkClient())
        return "";

    Lsp::HoverParams params;
    params.textDocument.uri = toUri();
    params.position = fromPos(position);

    auto result = client()->hover(std::move(params));
    if (!result) {
        return "";
    }

    if (!std::holds_alternative<Lsp::Hover>(*result)) {
        spdlog::warn("LSP server returned no result for Hover");
        return "";
    }

    auto hover = std::get<Lsp::Hover>(*result);

    Lsp::MarkupContent markupContent;
    if (const auto *content = std::get_if<Lsp::MarkupContent>(&hover.contents)) {
        return QString::fromStdString(content->value);
    } else {
        spdlog::warn("LSP returned deprecated MarkedString type which is unsupported by Knut\n - Consider updating "
                     "your LSP server");
        return "";
    }
}

/*!
 * \qmlmethod LspDocument::followSymbol()
 * Follow the symbol under the cursor.
 *
 * - Go to the declaration, if the symbol under cursor is a use
 * - Go to the declaration, if the symbol under cursor is a function definition
 * - Go to the definition, if the symbol under cursor is a function declaration
 * \todo
 */
Document *LspDocument::followSymbol()
{
    LOG("LspDocument::followSymbol");
    if (!checkClient())
        return {};

    // Set the cursor position to the beginning of any selected text.
    // That way, calling followSymbol twice in a row causes Clangd
    // to switch between delcaration and definition.
    auto cursor = textEdit()->textCursor();
    LOG_RETURN("document", followSymbol(cursor.selectionStart()));
}

/**
 * At least with clangd, the "declaration" LSP call acts like followSymbol, it will:
 * - Go to the declaration, if the symbol under cursor is a use
 * - Go to the declaration, if the symbol under cursor is a definition
 * - Go to the definition, if the symbol under cursor is a declaration
 */
Document *LspDocument::followSymbol(int pos)
{
    auto cursor = textEdit()->textCursor();
    cursor.setPosition(pos);

    Lsp::DeclarationParams params;
    params.textDocument.uri = toUri();
    params.position.line = cursor.blockNumber();
    params.position.character = cursor.positionInBlock();

    auto result = client()->declaration(std::move(params));

    Q_ASSERT(result.has_value());

    auto locations = std::vector<Lsp::Location>();

    if (std::holds_alternative<Lsp::Location>(*result)) {
        auto location = std::get<Lsp::Location>(*result);
        locations.push_back(location);

    } else if (std::holds_alternative<std::vector<Lsp::Location>>(*result)) {
        locations = std::move(std::get<std::vector<Lsp::Location>>(*result));

    } else if (std::holds_alternative<std::vector<Lsp::LocationLink>>(*result)) {
        auto locationLinks = std::get<std::vector<Lsp::LocationLink>>(*result);
        for (const auto &link : locationLinks)
            locations.push_back({link.targetUri, link.targetSelectionRange});
    }

    if (locations.empty())
        return nullptr;

    if (locations.size() > 1)
        spdlog::warn("LspDocument::followSymbol: Multiple locations returned!");
    // Heuristic: If multiple locations were found, use the last one.
    auto location = locations.back();

    auto url = QUrl::fromEncoded(QByteArray::fromStdString(location.uri));

    Q_ASSERT(url.isLocalFile());
    auto filepath = url.toLocalFile();

    auto *document = Project::instance()->open(filepath);

    if (document) {
        auto *lspDocument = qobject_cast<LspDocument *>(document);
        if (lspDocument) {
            lspDocument->selectRange(lspDocument->toRange(location.range));
        } else {
            spdlog::warn("LspDocument::followSymbol: Opened document '{}' is not an LspDocument",
                         document->fileName().toStdString());
        }
    }

    return document;
}

/*!
 * \qmlmethod LspDocument::switchDeclarationDefinition()
 * Switch between the function declaration or definition.
 * \todo
 */
Document *LspDocument::switchDeclarationDefinition()
{
    LOG("LspDocument::switchDeclarationDefinition");
    if (!checkClient())
        return {};

    auto cursor = textEdit()->textCursor();
    auto symbolList = symbols();

    auto currentFunction = std::find_if(symbolList.begin(), symbolList.end(), [&cursor](const auto &symbol) {
        auto isInRange = symbol->range().start <= cursor.position() && cursor.position() <= symbol->range().end;
        return isInRange && symbol->isFunction();
    });

    if (currentFunction == symbolList.end()) {
        spdlog::info("LspDocument::switchDeclarationDefinition: Cursor is currently not within a function!");
        return nullptr;
    }

    LOG_RETURN("document", followSymbol((*currentFunction)->selectionRange().start));
}

/*!
 * \qmlmethod LspDocument::selectSymbol(string name, int options = TextDocument.NoFindFlags)
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
    LOG("LspDocument::selectSymbol", LOG_ARG("text", name), options);
    if (!checkClient())
        return;

    if (auto symbol = findSymbol(name, options))
        selectRange(symbol->selectionRange());
}

/*!
 * \qmlmethod Symbol LspDocument::findSymbol(string name, int options = TextDocument.NoFindFlags)
 * Find a symbol based on its `name`, using different find `options`.
 *
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only fully qualified symbol
 * - `TextDocument.FindRegexp`: use a regexp
 *
 * Note that the returned \c Symbol pointer is only valid until the document it originates
 * from is deconstructed.
 */
Symbol *LspDocument::findSymbol(const QString &name, int options) const
{
    LOG("LspDocument::findSymbol", LOG_ARG("text", name), options);
    if (!checkClient())
        return {};

    const auto &symbols = m_cache->symbols();
    const auto regexp = (options & FindRegexp) ? createRegularExpression(name, options) : QRegularExpression {};
    auto byName = [name, options, regexp](Symbol *symbol) {
        if (options & FindWholeWords)
            return symbol->name().compare(name,
                                          (options & FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive)
                == 0;
        else if (options & FindRegexp)
            return regexp.match(symbol->name()).hasMatch();
        else
            return symbol->name().contains(name,
                                           (options & FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive);
    };
    auto it = std::ranges::find_if(symbols, byName);
    if (it != symbols.end())
        return *it;
    return nullptr;
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

Lsp::Position LspDocument::fromPos(int pos) const
{
    Lsp::Position position;

    auto cursor = textEdit()->textCursor();
    cursor.setPosition(pos, QTextCursor::MoveAnchor);

    position.line = cursor.blockNumber();
    position.character = cursor.positionInBlock();
    return position;
}

TextRange LspDocument::toRange(const Lsp::Range &range) const
{
    return {toPos(range.start), toPos(range.end)};
}

int LspDocument::revision() const
{
    return m_revision;
}

bool LspDocument::checkClient() const
{
    Q_ASSERT(textEdit());
    if (!client()) {
        spdlog::error("LspDocument {} has no LSP client - API not available", fileName().toStdString());
        return false;
    }
    return true;
}

void LspDocument::changeContent(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(position)
    Q_UNUSED(charsRemoved)
    Q_UNUSED(charsAdded)
    m_cache->clear();

    if (!checkClient()) {
        return;
    }

    if (client()->canSendDocumentChanges(Lsp::TextDocumentSyncKind::Full)
        || client()->canSendDocumentChanges(Lsp::TextDocumentSyncKind::Incremental)) {
        // TODO: We currently always send the entire document to the Language server, even
        // if it suppports incremental changes.
        // Change this, so we also send incremental updates.
        //
        // This currently isn't implemented, as changeContent only gets called *after*
        // the change has happened, but the LSP server needs the locations from *before* the change
        // has happened, which can no longer be queried.

        Lsp::VersionedTextDocumentIdentifier document;
        document.version = ++m_revision;
        document.uri = toUri();

        std::vector<Lsp::TextDocumentContentChangeEvent> events;
        Lsp::TextDocumentContentChangeEvent event;

        event.text = text().toStdString();
        events.emplace_back(std::move(event));

        Lsp::DidChangeTextDocumentParams params;
        params.textDocument = document;
        params.contentChanges = events;

        client()->didChange(std::move(params));
    } else {
        spdlog::error("LSP server does not support Document changes!");
    }
}

} // namespace Core
