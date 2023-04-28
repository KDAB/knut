#include "lspdocument.h"
#include "lspdocument_p.h"

#include "json_utils.h"
#include "logger.h"
#include "project.h"
#include "querymatch.h"
#include "rangemark.h"
#include "scriptmanager.h"
#include "string_utils.h"
#include "symbol.h"
#include "textlocation.h"

#include <QFile>
#include <QJSEngine>
#include <QMap>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextStream>

#include <kdalgorithms.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <memory>

namespace Core {

/*!
 * \qmltype LspDocument
 * \brief Base document object for document using LSP.
 * \inqmlmodule Script
 * \ingroup LspDocument/@first
 * \since 1.0
 * \inherits TextDocument
 */

LspDocument::~LspDocument() = default;

LspDocument::LspDocument(Type type, QObject *parent)
    : TextDocument(type, parent)
    , m_cache(std::make_unique<LspCache>(this))
    , m_treeSitterHelper(std::make_unique<TreeSitterHelper>(this))
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
 * Note that the returned `Symbol` pointer is only valid until the document
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
 * \qmlmethod array<Symbol> LspDocument::symbols()
 * Returns the list of symbols in the current document.
 *
 * Note that the returned `Symbol` pointers are only valid until the document they
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

/*!
 * \qmlmethod Symbol LspDocument::symbolUnderCursor()
 * \since 1.1
 * Returns the symbol that's at the current cursor position.
 *
 * This function may return symbols that are not returned by the `symbols()` or `currentSymbol()` function,
 * as these only return high-level symbols, like classes and functions, but not symbols within functions.
 * `symbolUnderCursor()` can however return these Symbols.
 */
const Core::Symbol *LspDocument::symbolUnderCursor() const
{
    const auto containsCursor = [this](const Core::Symbol *symbol) {
        return symbol->selectionRange().contains(textEdit()->textCursor().position());
    };

    const auto symbols = this->symbols();
    const auto symbolIter = kdalgorithms::find_if(symbols, containsCursor);
    if (symbolIter) {
        return *symbolIter;
    }

    auto hover = hoverWithRange(textEdit()->textCursor().position());
    if (hover.second) {
        return m_cache->inferSymbol(hover.first, hover.second.value());
    } else {
        spdlog::warn("LspDocument::symbolUnderCursor: Cannot infer Symbol - Language Server did not return a range!");
    }

    return nullptr;
}

/*!
 * \qmlmethod string LspDocument::hover()
 * \since 1.1
 *
 * Returns information about the symbol at the current cursor position.
 * The result of this call is a plain string that may be formatted in Markdown.
 */
QString LspDocument::hover() const
{
    return hover(textEdit()->textCursor().position());
}

QString LspDocument::hover(int position, std::function<void(const QString &)> asyncCallback /*  = {} */) const
{
    if (asyncCallback) {
        return hoverWithRange(position,
                              [asyncCallback = std::move(asyncCallback)](const auto &hoverText, auto) {
                                  asyncCallback(hoverText);
                              })
            .first;
    } else {
        return hoverWithRange(position).first;
    }
}

std::pair<QString, std::optional<TextRange>> LspDocument::hoverWithRange(
    int position, std::function<void(const QString &, std::optional<TextRange>)> asyncCallback /*  = {} */) const
{
    LOG("LspDocument::hover");

    if (!checkClient())
        return {"", {}};

    Lsp::HoverParams params;
    params.textDocument.uri = toUri();
    params.position = fromPos(position);

    QPointer<const LspDocument> safeThis(this);

    auto convertResult = [safeThis](const auto &result) -> std::pair<QString, std::optional<TextRange>> {
        if (!std::holds_alternative<Lsp::Hover>(result)) {
            return {"", {}};
        }

        auto hover = std::get<Lsp::Hover>(result);

        std::optional<TextRange> range;
        if (hover.range && !safeThis.isNull()) {
            range = safeThis->toRange(hover.range.value());
        }

        Lsp::MarkupContent markupContent;
        if (const auto *content = std::get_if<Lsp::MarkupContent>(&hover.contents)) {
            return {QString::fromStdString(content->value), range};
        } else {
            spdlog::warn("LSP returned deprecated MarkedString type which is unsupported by Knut\n - Consider updating "
                         "your LSP server");
            return {"", {}};
        }
    };

    if (asyncCallback) {
        client()->hover(std::move(params),
                        [convertResult, asyncCallback = std::move(asyncCallback)](const auto result) {
                            auto hoverText = convertResult(result);
                            asyncCallback(hoverText.first, hoverText.second);
                        });
    } else {
        auto result = client()->hover(std::move(params));
        if (result) {
            // We can't have this in "convertResult", as that would spam the log due to Hover being called when
            // a Tooltip is requested.
            // See: TextView::eventFilter.
            if (!std::holds_alternative<Lsp::Hover>(result.value())) {
                spdlog::debug("LSP server returned no result for Hover");
            }
            return convertResult(result.value());
        }
    }

    return {"", {}};
}

/*!
 * \qmlmethod array<TextLocation> LspDocument::references()
 * \since 1.1
 * Returns the list of references at the given position.
 */
QVector<Core::TextLocation> LspDocument::references(int position) const
{
    LOG("LspDocument::references");

    if (!checkClient()) {
        return {};
    }

    Lsp::ReferenceParams params;
    params.textDocument.uri = toUri();
    params.position = fromPos(position);

    QVector<Core::TextLocation> textLocations;
    if (auto result = client()->references(std::move(params))) {
        const auto &value = result.value();
        if (const auto *locations = std::get_if<std::vector<Lsp::Location>>(&value)) {
            return TextLocation::fromLsp(*locations);
        } else {
            spdlog::warn("LspDocument::references: Language server returned unsupported references type!");
        }
    } else {
        spdlog::warn("LspDocument::references: LSP call to references returned nothing!");
    }

    return textLocations;
}

/*!
 * \qmlmethod LspDocument::followSymbol()
 * Follows the symbol under the cursor.
 *
 * - Go to the declaration, if the symbol under cursor is a use
 * - Go to the declaration, if the symbol under cursor is a function definition
 * - Go to the definition, if the symbol under cursor is a function declaration
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

    if (std::holds_alternative<Lsp::Declaration>(*result)) {
        auto &declaration = std::get<Lsp::Declaration>(*result);
        if (std::holds_alternative<Lsp::Location>(declaration)) {
            auto location = std::get<Lsp::Location>(declaration);
            locations.push_back(location);
        } else if (std::holds_alternative<std::vector<Lsp::Location>>(declaration)) {
            locations = std::move(std::get<std::vector<Lsp::Location>>(declaration));
        }
    } else if (std::holds_alternative<std::vector<Lsp::DeclarationLink>>(*result)) {
        auto locationLinks = std::get<std::vector<Lsp::DeclarationLink>>(*result);
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
 * Switches between the function declaration or definition.
 * \todo
 */
Document *LspDocument::switchDeclarationDefinition()
{
    LOG("LspDocument::switchDeclarationDefinition");
    if (!checkClient())
        return {};

    auto cursor = textEdit()->textCursor();
    auto symbolList = symbols();

    auto currentFunction = kdalgorithms::find_if(symbolList, [&cursor](const auto &symbol) {
        auto isInRange = symbol->range().start <= cursor.position() && cursor.position() <= symbol->range().end;
        return isInRange && symbol->isFunction();
    });

    if (!currentFunction) {
        spdlog::info("LspDocument::switchDeclarationDefinition: Cursor is currently not within a function!");
        return nullptr;
    }

    LOG_RETURN("document", followSymbol((*currentFunction)->selectionRange().start));
}

/*!
 * \qmlmethod LspDocument::selectSymbol(string name, int options = TextDocument.NoFindFlags)
 * Selects a symbol based on its `name`, using different find `options`.
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
 * Finds a symbol based on its `name`, using different find `options`.
 *
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only fully qualified symbol
 * - `TextDocument.FindRegexp`: use a regexp
 *
 * Note that the returned `Symbol` pointer is only valid until the document it originates
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

/*!
 * \qmlmethod array<QueryMatch> LspDocument::query(string query)
 * \since 1.1
 * Runs the `query` passed in parameter and returns the list of matches.
 *
 * The query is using the [TreeSitter](https://tree-sitter.github.io/tree-sitter/) query, you can find more information
 * on this page: [Pattern Matching with
 * Queries](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries).
 */
QVector<QueryMatch> LspDocument::query(const QString &query)
{
    LOG("LspDocument::query", LOG_ARG("query", query));

    const auto &tree = m_treeSitterHelper->syntaxTree();
    auto tsQuery = m_treeSitterHelper->constructQuery(query);

    if (!tree || !tsQuery) {
        return {};
    }

    treesitter::QueryCursor cursor;
    cursor.execute(tsQuery, tree->rootNode(), std::make_unique<treesitter::Predicates>(text()));
    auto matches = cursor.allRemainingMatches();

    return kdalgorithms::transformed<QVector<QueryMatch>>(matches, [this](const treesitter::QueryMatch &match) {
        return QueryMatch(*this, match);
    });
}

/**
 * \qmlmethod array<QueryMatch> LspDocument::queryInRange(RangeMark range, string query)
 * \since 1.1
 *
 * Searches for the given `query`, but only in the provided `range`.
 *
 * \sa LspDocument::query
 */
QVector<QueryMatch> LspDocument::queryInRange(const Core::RangeMark &range, const QString &query)
{
    LOG("LspDocument::queryInRange", LOG_ARG("range", range), LOG_ARG("query", query));

    if (!range.isValid()) {
        spdlog::warn("LspDocument::queryInRange: Range is not valid");
        return {};
    }

    auto nodes = m_treeSitterHelper->nodesInRange(range);

    if (nodes.isEmpty()) {
        spdlog::warn("LspDocument::queryInRange: No nodes in range");
        return {};
    }
    spdlog::debug("LspDocument::queryInRange: Found {} nodes in range", nodes.size());

    auto tsQuery = m_treeSitterHelper->constructQuery(query);
    if (!tsQuery)
        return {};

    treesitter::QueryCursor cursor;
    QVector<QueryMatch> matches;
    for (const treesitter::Node &node : nodes) {
        cursor.execute(tsQuery, node, std::make_unique<treesitter::Predicates>(text()));
        matches.append(kdalgorithms::transformed<QVector<QueryMatch>>(cursor.allRemainingMatches(),
                                                                      [this](const treesitter::QueryMatch &match) {
                                                                          return QueryMatch(*this, match);
                                                                      }));
    }
    return matches;
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

void LspDocument::changeContentLsp(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(position)
    Q_UNUSED(charsRemoved)
    Q_UNUSED(charsAdded)
    m_cache->clear();

    // TODO: Keep copy of previous string around, so we can find the oldEndPosition.
    // const auto document = textEdit()->document();
    // const auto startblock = document->findBlock(position);
    // spdlog::warn("start point: {}, {}", startblock.blockNumber(), position - startblock.position());

    // const auto newEndPosition = position + charsAdded;
    // const auto newEndBlock = document->findBlock(newEndPosition);
    // spdlog::warn("new end point: {}, {}", newEndBlock.blockNumber(), newEndPosition - newEndBlock.position());

    // const auto plain = document->toPlainText();
    // spdlog::warn("added: {}", plain.sliced(position, charsAdded).toStdString());

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

        // Set text
        Lsp::TextDocumentContentChangeEventFull event {};
        event.text = text().toStdString();

        events.push_back(std::move(event));

        Lsp::DidChangeTextDocumentParams params;
        params.textDocument = document;
        params.contentChanges = events;

        client()->didChange(std::move(params));
    } else {
        spdlog::error("LSP server does not support Document changes!");
    }
}

void LspDocument::changeContentTreeSitter(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(position)
    Q_UNUSED(charsRemoved)
    Q_UNUSED(charsAdded)

    // Note: This invalidates all existing treesitter::Node instances of this tree!
    // Only use treesitter nodes as long as you're certain the document isn't edited!
    // TODO: If we keep the old string around, we could implement incremental parsing here.
    // If TreeSitter parsing ever becomes a performance bottleneck, this would be where to fix it.
    m_treeSitterHelper->clear();
}

void LspDocument::changeContent(int position, int charsRemoved, int charsAdded)
{
    changeContentLsp(position, charsRemoved, charsAdded);
    changeContentTreeSitter(position, charsRemoved, charsAdded);
}

} // namespace Core
