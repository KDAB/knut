/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "codedocument.h"
#include "astnode.h"
#include "codedocument_p.h"
#include "logger.h"
#include "lsp_utils.h"
#include "project.h"
#include "querymatch.h"
#include "rangemark.h"
#include "settings.h"
#include "symbol.h"
#include "treesitter/predicates.h"
#include "utils/log.h"
#include "utils/string_helper.h"

#include <QFile>
#include <QJSEngine>
#include <QMap>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextStream>
#include <algorithm>
#include <kdalgorithms.h>
#include <memory>

namespace Core {

/*!
 * \qmltype CodeDocument
 * \brief Base document object for any code that Knut can parse.
 * \ingroup CodeDocument/@first
 * \inherits TextDocument
 *
 * Knut uses Tree-sitter to parse the code and provide additional information about it.
 * For a better user experience, the Knut GUI also uses a Language server (LSP), if available.
 * For each language that Knut can work with, this class should be subclassed to provide language-specific
 * functionality.
 *
 * This class provides the language-independent basis of integration with Tree-sitter and the LSP.
 *
 * Currently supported languages are: C/C++, C#, Dart, Qml and Rust
 */

CodeDocument::~CodeDocument() = default;

CodeDocument::CodeDocument(Type type, QObject *parent)
    : TextDocument(type, parent)
    , m_treeSitterHelper(std::make_unique<TreeSitterHelper>(this))
{
    connect(textEdit()->document(), &QTextDocument::contentsChange, this, &CodeDocument::changeContent);
}

void CodeDocument::setLspClient(Lsp::Client *client)
{
    m_lspClient = client;
}

bool CodeDocument::hasLspClient() const
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
Symbol *CodeDocument::currentSymbol(const std::function<bool(const Symbol &)> &filterFunc) const
{
    const int pos = textEdit()->textCursor().position();

    const auto symbolList = symbols();
    for (auto symbol : symbolList | std::views::reverse) {
        if (symbol->range().contains(pos) && (!filterFunc || filterFunc(*symbol)))
            return symbol;
    }
    return {};
}

/**
 * Deletes the specified symbols text range, as well as leading whitespace
 * and trailing semicolon/newline character.
 */
void CodeDocument::deleteSymbol(const Symbol &symbol)
{
    const auto range = symbol.range();

    // Include any leading whitespace (excluding newlines).
    int start = range.start();
    while (start > 0) {
        selectRegion(start - 1, start);

        if (selectedText() != " " && selectedText() != "\t") {
            break;
        }

        start--;
    }

    // Include a trailing semicolon and up to one trailing newline
    int end = range.end();
    selectRegion(end, end + 1);
    if (selectedText() == ";") {
        end++;
        selectRegion(end, end + 1);
    }

    if (selectedText() == "\n") {
        end++;
    }

    this->selectRegion(start, end);
    this->deleteSelection();
}

/*!
 * \qmlmethod array<Symbol> CodeDocument::symbols()
 * Returns the list of symbols in the current document.
 *
 * Note that the returned `Symbol` pointers are only valid until the document they
 * originate from is deconstructed.
 */
Core::SymbolList CodeDocument::symbols() const
{
    LOG();
    RangeMark::temporaryDocumentText = text();
    const auto &symbols = m_treeSitterHelper->symbols();
    RangeMark::temporaryDocumentText.clear();
    return symbols;
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
 * \qmlmethod Symbol CodeDocument::symbolUnderCursor()
 * Returns the symbol that's at the current cursor position.
 *
 * This function may return symbols that are not returned by the `symbols()` or `currentSymbol()` function,
 * as these only return high-level symbols, like classes and functions, but not symbols within functions.
 * `symbolUnderCursor()` can however return these Symbols.
 */
const Core::Symbol *CodeDocument::symbolUnderCursor() const
{
    const auto containsCursor = [this](const Core::Symbol *symbol) {
        return symbol->selectionRange().contains(textEdit()->textCursor().position());
    };

    const auto symbols = this->symbols();
    const auto symbolIter = kdalgorithms::find_if(symbols, containsCursor);
    if (symbolIter) {
        return *symbolIter;
    }

    return nullptr;
}

/*!
 * \qmlmethod string CodeDocument::hover()
 *
 * Returns information about the symbol at the current cursor position.
 * The result of this call is a plain string that may be formatted in Markdown.
 */
QString CodeDocument::hover() const
{
    return hover(textEdit()->textCursor().position());
}

QString CodeDocument::hover(int position, std::function<void(const QString &)> asyncCallback /*  = {} */) const
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

std::pair<QString, std::optional<RangeMark>> CodeDocument::hoverWithRange(
    int position, std::function<void(const QString &, std::optional<RangeMark>)> asyncCallback /*  = {} */) const
{
    spdlog::debug("{}", FUNCTION_NAME);

    if (!checkClient())
        return {"", {}};

    Lsp::HoverParams params;
    params.textDocument.uri = toUri();
    params.position = Utils::lspFromPos(*this, position);

    QPointer<const CodeDocument> safeThis(this);

    auto convertResult = [safeThis](const auto &result) -> std::pair<QString, std::optional<RangeMark>> {
        if (!std::holds_alternative<Lsp::Hover>(result)) {
            return {"", {}};
        }

        auto hover = std::get<Lsp::Hover>(result);

        std::optional<RangeMark> range;
        if (hover.range && !safeThis.isNull()) {
            range = Utils::lspToRange(*safeThis, hover.range.value());
        }

        Lsp::MarkupContent markupContent;
        if (const auto *content = std::get_if<Lsp::MarkupContent>(&hover.contents)) {
            return {QString::fromStdString(content->value), range};
        } else {
            spdlog::warn(
                "{}: LSP returned deprecated MarkedString type which is unsupported by Knut\n - Consider updating "
                "your LSP server",
                FUNCTION_NAME);
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
                spdlog::debug("{}: LSP server returned no result for Hover", FUNCTION_NAME);
            }
            return convertResult(result.value());
        }
    }

    return {"", {}};
}

RangeMarkList CodeDocument::references(int position) const
{
    spdlog::debug("{}", FUNCTION_NAME);

    if (!checkClient()) {
        return {};
    }

    Lsp::ReferenceParams params;
    params.textDocument.uri = toUri();
    params.position = Utils::lspFromPos(*this, position);

    if (auto result = client()->references(std::move(params))) {
        const auto &value = result.value();
        if (const auto *locations = std::get_if<std::vector<Lsp::Location>>(&value)) {
            return Utils::lspToRangeMarkList(*locations);
        } else {
            spdlog::warn("{}: Language server returned unsupported references type!", FUNCTION_NAME);
        }
    } else {
        spdlog::warn("{}: LSP call to references returned nothing!", FUNCTION_NAME);
    }

    return {};
}

// Follows the symbol under the cursor.
Document *CodeDocument::followSymbol()
{
    spdlog::debug("{}", FUNCTION_NAME);
    if (!checkClient())
        return {};

    // Set the cursor position to the beginning of any selected text.
    // That way, calling followSymbol twice in a row causes Clangd
    // to switch between declaration and definition.
    auto cursor = textEdit()->textCursor();
    return followSymbol(cursor.selectionStart());
}

// At least with clangd, the "declaration" LSP call acts like followSymbol, it will:
// - Go to the declaration, if the symbol under cursor is a use
// - Go to the declaration, if the symbol under cursor is a definition
// - Go to the definition, if the symbol under cursor is a declaration
Document *CodeDocument::followSymbol(int pos)
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
        const auto locationLinks = std::get<std::vector<Lsp::DeclarationLink>>(*result);
        for (const auto &link : locationLinks)
            locations.push_back({link.targetUri, link.targetSelectionRange});
    }

    if (locations.empty())
        return nullptr;

    if (locations.size() > 1)
        spdlog::warn("{}: Multiple locations returned!", FUNCTION_NAME);
    // Heuristic: If multiple locations were found, use the last one.
    auto location = locations.back();

    auto url = QUrl::fromEncoded(QByteArray::fromStdString(location.uri));

    Q_ASSERT(url.isLocalFile());
    auto filepath = url.toLocalFile();

    auto *document = Project::instance()->open(filepath);

    if (document) {
        if (auto *codeDocument = qobject_cast<CodeDocument *>(document)) {
            codeDocument->selectRange(Utils::lspToRange(*codeDocument, location.range));
        } else {
            spdlog::warn("{}: Opened document '{}' is not an CodeDocument", FUNCTION_NAME, document->fileName());
        }
    }

    return document;
}

// Switches between the function declaration or definition.
Document *CodeDocument::switchDeclarationDefinition()
{
    spdlog::debug("{}", FUNCTION_NAME);
    if (!checkClient())
        return {};

    auto cursor = textEdit()->textCursor();
    auto symbolList = symbols();

    auto currentFunction = kdalgorithms::find_if(symbolList, [&cursor](const auto &symbol) {
        auto isInRange = symbol->range().contains(cursor.position());
        return isInRange && symbol->isFunction();
    });

    if (!currentFunction) {
        spdlog::info("{}: Cursor is currently not within a function!", FUNCTION_NAME);
        return nullptr;
    }

    return followSymbol((*currentFunction)->selectionRange().start());
}

/*!
 * \qmlmethod CodeDocument::selectSymbol(string name, int options = TextDocument.NoFindFlags)
 * Selects a symbol based on its `name`, using different find `options`.
 *
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only fully qualified symbol
 * - `TextDocument.FindRegexp`: use a regexp
 *
 * If no symbols are found, do nothing.
 */
void CodeDocument::selectSymbol(const QString &name, int options)
{
    LOG(LOG_ARG("text", name), options);

    if (auto symbol = findSymbol(name, options))
        selectRange(symbol->selectionRange());
}

/*!
 * \qmlmethod int CodeDocument::selectLargerSyntaxNode(int count = 1)
 *
 * Selects the text of the next larger syntax node that the selection is in.
 *
 * It does so `count` times and returns the resulting cursor position.
 */
int CodeDocument::selectLargerSyntaxNode(int count /* = 1*/)
{
    LOG_AND_MERGE(LOG_ARG("count", count));

    auto currentNode = m_treeSitterHelper->nodeCoveringRange(selectionStart(), selectionEnd());

    auto matchesCurrentSelection = static_cast<int>(currentNode.startPosition()) == selectionStart()
        && static_cast<int>(currentNode.endPosition()) == selectionEnd();
    if (!matchesCurrentSelection) {
        // finding the node that covers the current selection already produced a larger syntax node.
        // So we're already up one level.
        --count;
    }

    for (/*count already initialized*/; count > 0 && !currentNode.parent().isNull(); --count) {
        auto largerNode = currentNode.parent();
        if (largerNode.startPosition() == currentNode.startPosition()
            && largerNode.endPosition() == currentNode.endPosition()) {
            // If the parent node matches the current selection exactly, that's not a real change, so search one
            // additional time.
            ++count;
        }
        currentNode = largerNode;
    }

    selectRegion(currentNode.startPosition(), currentNode.endPosition());

    LOG_RETURN("pos", position());
}

/*!
 * \qmlmethod int CodeDocument::selectSmallerSyntaxNode(int count = 1)
 *
 * Selects the left-most next smaller syntax node within the current selection.
 *
 * It does so `count` times and returns the resulting cursor position.
 *
 * Note that this only selects "named" Tree-sitter nodes, so punctuation and other unnamed nodes are skipped.
 */
int CodeDocument::selectSmallerSyntaxNode(int count /* = 1*/)
{
    LOG_AND_MERGE(LOG_ARG("count", count));

    auto smallerNodes =
        kdalgorithms::filtered(m_treeSitterHelper->nodesInRange(createRangeMark()), [](const auto &node) {
            return node.isNamed();
        });

    std::optional<treesitter::Node> node;
    for (/*count already initialized*/; count > 0; --count) {
        if (smallerNodes.isEmpty()) {
            break;
        }
        node = smallerNodes.first();
        auto matchesCurrentSelection = static_cast<int>(node->startPosition()) == selectionStart()
            && static_cast<int>(node->endPosition()) == selectionEnd();
        // If the first found node matches the current selection exactly, we need to search one additional time.
        if (matchesCurrentSelection) {
            ++count;
        }
        smallerNodes = node->namedChildren();
    }

    if (node.has_value()) {
        selectRegion(node->startPosition(), node->endPosition());
    } else {
        spdlog::warn("{}: No smaller node found! Do you currently not have a selection?", FUNCTION_NAME);
    }

    LOG_RETURN("pos", position());
}

static std::optional<treesitter::Node>
findSibling(const treesitter::Node &start, treesitter::Node (treesitter::Node::*nextFunction)() const, int count)
{
    auto node = start;

    std::optional<treesitter::Node> target = node;
    for (/*count already initialized*/; count > 0; --count) {
        auto next = std::invoke(nextFunction, node);
        while (next.isNull() && !node.parent().isNull()) {
            node = node.parent();
            next = std::invoke(nextFunction, node);
        }
        if (next.isNull()) {
            // We're already at the very end/top, nowhere else to go
            break;
        }
        node = next;
        target = next;
    }

    return target;
}

/*!
 * \qmlmethod int CodeDocument::selectNextSyntaxNode(int count = 1)
 *
 * Selects the next syntax node following the current selection.
 *
 * If there is no next syntax node in the current level, it increases the selection to the next larger syntax node and
 * searches from there. See also: `CodeDocument::selectLargerSyntaxNode`
 *
 * It does so `count` times and returns the resulting cursor position.
 *
 * Note that this only selects "named" Tree-sitter nodes, so punctuation and other unnamed nodes are skipped.
 */
int CodeDocument::selectNextSyntaxNode(int count /*= 1*/)
{
    LOG_AND_MERGE(LOG_ARG("count", count));

    auto node = m_treeSitterHelper->nodeCoveringRange(selectionStart(), selectionEnd());

    auto target = findSibling(node, &treesitter::Node::nextNamedSibling, count);

    if (target.has_value()) {
        selectRegion(target->startPosition(), target->endPosition());
    }

    LOG_RETURN("pos", position());
}

/*!
 * \qmlmethod int CodeDocument::selectPreviousSyntaxNode(int count = 1)
 *
 * Selects the previous syntax node before the current selection.
 *
 * If there is no previous syntax node in the current level, it increases the selection to the next larger syntax node
 * and searches from there. See also: `CodeDocument::selectLargerSyntaxNode`
 *
 * It does so `count` times and returns the resulting cursor position.
 *
 * Note that this only selects "named" Tree-sitter nodes, so punctuation and other unnamed nodes are skipped.
 */
int CodeDocument::selectPreviousSyntaxNode(int count /*= 1*/)
{
    LOG_AND_MERGE(LOG_ARG("count", count));

    auto node = m_treeSitterHelper->nodeCoveringRange(selectionStart(), selectionEnd());

    auto target = findSibling(node, &treesitter::Node::previousNamedSibling, count);

    if (target.has_value()) {
        selectRegion(target->startPosition(), target->endPosition());
    }

    LOG_RETURN("pos", position());
}

/*!
 * \qmlmethod Symbol CodeDocument::findSymbol(string name, int options = TextDocument.NoFindFlags)
 * Finds a symbol based on its `name`, using different find `options`.
 *
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only fully qualified symbol
 * - `TextDocument.FindRegexp`: use a regexp
 *
 * Note that the returned `Symbol` pointer is only valid until the document it originates
 * from is deconstructed.
 */
Symbol *CodeDocument::findSymbol(const QString &name, int options) const
{
    LOG(LOG_ARG("text", name), options);

    auto symbols = this->symbols();
    const auto regexp =
        (options & FindRegexp) ? ::Utils::createRegularExpression(name, options) : QRegularExpression {};
    auto byName = [name, options, regexp](Symbol *symbol) {
        if (options & FindWholeWords)
            return symbol->name().compare(name,
                                          (options & FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive)
                == 0;
        else if (options & FindRegexp)
            return regexp.match(symbol->name()).hasMatch();
        else
            return symbol->name().endsWith(name,
                                           (options & FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive);
    };
    auto it = std::ranges::find_if(symbols, byName);
    if (it != symbols.end())
        return *it;
    return nullptr;
}

void CodeDocument::didOpen()
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

void CodeDocument::didClose()
{
    if (!m_lspClient)
        return;

    Lsp::DidCloseTextDocumentParams params;
    params.textDocument.uri = toUri();

    m_lspClient->didClose(std::move(params));
}

Lsp::Client *CodeDocument::client() const
{
    return m_lspClient;
}

std::string CodeDocument::toUri() const
{
    return QUrl::fromLocalFile(fileName()).toString().toStdString();
}

std::unique_ptr<TreeSitterHelper> &CodeDocument::helper()
{
    return m_treeSitterHelper;
}

std::optional<treesitter::QueryCursor> CodeDocument::createQueryCursor(const std::shared_ptr<treesitter::Query> &query)
{
    const auto &tree = m_treeSitterHelper->syntaxTree();
    if (!tree || !query) {
        return {};
    }

    treesitter::QueryCursor cursor;
    cursor.setProgressCallback(ScriptDialogItem::updateProgress);
    cursor.execute(query, tree->rootNode(), std::make_unique<treesitter::Predicates>(text()));
    return cursor;
}

Core::QueryMatch CodeDocument::queryFirst(const std::shared_ptr<treesitter::Query> &query)
{
    auto cursor = createQueryCursor(query);
    if (!cursor.has_value()) {
        return {};
    }

    auto match = cursor->nextMatch();
    if (match.has_value()) {
        return QueryMatch(*this, match.value());
    } else {
        return QueryMatch();
    }
}

Core::QueryMatchList CodeDocument::query(const std::shared_ptr<treesitter::Query> &query)
{
    auto cursor = createQueryCursor(query);
    if (!cursor.has_value()) {
        return {};
    }

    auto matches = cursor->allRemainingMatches();

    return kdalgorithms::transformed<Core::QueryMatchList>(matches, [this](const treesitter::QueryMatch &match) {
        return QueryMatch(*this, match);
    });
}

/*!
 * \qmlmethod array<QueryMatch> CodeDocument::query(string query)
 * Runs the given Tree-sitter `query` and returns the list of matches.
 *
 * The query is using [Tree-sitter
 * queries](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries).
 *
 * Also see: [Tree-sitter in Knut](../../getting-started/treesitter.md)
 */
Core::QueryMatchList CodeDocument::query(const QString &query)
{
    LOG(LOG_ARG("query", query));

    return this->query(m_treeSitterHelper->constructQuery(query));
}

/*!
 * \qmlmethod QueryMatch CodeDocument::queryFirst(string query)
 * Runs the given Tree-sitter `query` and returns the first match.
 * If no match can be found an empty match will be returned.
 *
 * This can be a lot faster than `query` if you only need the first match.
 *
 * The query is using [Tree-sitter
 * queries](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries).
 *
 * Also see: [Tree-sitter in Knut](../../getting-started/treesitter.md)
 Core::QueryMatchList CodeDocument::query(const QString &query)
 */
Core::QueryMatch CodeDocument::queryFirst(const QString &query)
{
    LOG(LOG_ARG("query", query));

    return this->queryFirst(m_treeSitterHelper->constructQuery(query));
}

/**
 * \qmlmethod array<QueryMatch> CodeDocument::queryInRange(RangeMark range, string query)
 *
 * Searches for the given `query`, but only in the provided `range`.
 *
 * \sa CodeDocument::query
 */
Core::QueryMatchList CodeDocument::queryInRange(const Core::RangeMark &range, const QString &query)
{
    LOG(LOG_ARG("range", range), LOG_ARG("query", query));

    if (!range.isValid()) {
        spdlog::warn("{}: Range is not valid", FUNCTION_NAME);
        return {};
    }

    const auto nodes = m_treeSitterHelper->nodesInRange(range);

    if (nodes.isEmpty()) {
        spdlog::warn("{}: No nodes in range", FUNCTION_NAME);
        return {};
    }
    spdlog::debug("{}: Found {} nodes in range", FUNCTION_NAME, nodes.size());

    auto tsQuery = m_treeSitterHelper->constructQuery(query);
    if (!tsQuery)
        return {};

    treesitter::QueryCursor cursor;
    Core::QueryMatchList matches;
    for (const treesitter::Node &node : nodes) {
        cursor.execute(tsQuery, node, std::make_unique<treesitter::Predicates>(text()));
        matches.append(kdalgorithms::transformed<QList<QueryMatch>>(cursor.allRemainingMatches(),
                                                                    [this](const treesitter::QueryMatch &match) {
                                                                        return QueryMatch(*this, match);
                                                                    }));
    }
    return matches;
}

int CodeDocument::revision() const
{
    return m_revision;
}

bool CodeDocument::checkClient() const
{
    Q_ASSERT(textEdit());
    if (!Settings::instance()->hasLsp())
        return false;
    if (!client()) {
        spdlog::error("{}: CodeDocument {} has no LSP client - API not available", FUNCTION_NAME, fileName());
        return false;
    }
    return true;
}

void CodeDocument::changeContentLsp(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(position)
    Q_UNUSED(charsRemoved)
    Q_UNUSED(charsAdded)

    // TODO: Keep copy of previous string around, so we can find the oldEndPosition.
    // const auto document = textEdit()->document();
    // const auto startblock = document->findBlock(position);
    // spdlog::warn("{} - start point: {}, {}", FUNCTION_NAME, startblock.blockNumber(), position -
    // startblock.position());

    // const auto newEndPosition = position + charsAdded;
    // const auto newEndBlock = document->findBlock(newEndPosition);
    // spdlog::warn("{} - new end point: {}, {}", FUNCTION_NAME, newEndBlock.blockNumber(), newEndPosition -
    // newEndBlock.position());

    // const auto plain = document->toPlainText();
    // spdlog::warn("{} - added: {}", FUNCTION_NAME, plain.sliced(position, charsAdded));

    if (!checkClient()) {
        return;
    }

    if (client()->canSendDocumentChanges(Lsp::TextDocumentSyncKind::Full)
        || client()->canSendDocumentChanges(Lsp::TextDocumentSyncKind::Incremental)) {
        // TODO: We currently always send the entire document to the Language server, even
        // if it supports incremental changes.
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

        events.emplace_back(std::move(event));

        Lsp::DidChangeTextDocumentParams params;
        params.textDocument = document;
        params.contentChanges = events;

        client()->didChange(std::move(params));
    } else {
        spdlog::error("{}: LSP server does not support Document changes!", FUNCTION_NAME);
    }
}

void CodeDocument::changeContentTreeSitter(int position, int charsRemoved, int charsAdded)
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

void CodeDocument::changeContent(int position, int charsRemoved, int charsAdded)
{

    // We don't want to cause extra logging here, we're probably already in a situation where we're changing text, which
    // has itself already been logged.
    //
    // Additionally, when running scripts that show progress, this could be problematic,
    // as then logging causes a redraw.
    // As we're not quite done with updating the text at this point, we cannot redraw yet!
    LoggerDisabler disabler;

    changeContentLsp(position, charsRemoved, charsAdded);
    changeContentTreeSitter(position, charsRemoved, charsAdded);
}

AstNode CodeDocument::astNodeAt(int pos)
{
    const auto root = m_treeSitterHelper->syntaxTree()->rootNode();
    if (const auto node = root.descendantForRange(pos, pos); !node.isNull()) {
        return AstNode(node, this);
    }
    return {};
}

QList<treesitter::Range> CodeDocument::includedRanges() const
{
    // An empty list tells the parser to include the entire document.
    return {};
}

} // namespace Core
