/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "astnode.h"
#include "lsp/client.h"
#include "querymatch.h"
#include "symbol.h"
#include "textdocument.h"
#include "treesitter/query.h"

#include <functional>
#include <memory>

namespace Lsp {
class Client;
struct Position;
struct Range;
}

namespace treesitter {
class Query;
}

namespace Core {

class LspCache;
class TreeSitterHelper;
struct RegexpTransform;
class AstNode;

class CodeDocument : public TextDocument
{
    Q_OBJECT

public:
    ~CodeDocument() override;

    void setLspClient(Lsp::Client *client);

    Q_INVOKABLE Core::Symbol *findSymbol(const QString &name, int options = NoFindFlags) const;
    Q_INVOKABLE Core::SymbolList symbols() const;
    Q_INVOKABLE QString hover() const;
    Q_INVOKABLE const Core::Symbol *symbolUnderCursor() const;

    Q_INVOKABLE Core::QueryMatchList query(const QString &query);
    Q_INVOKABLE Core::QueryMatch queryFirst(const QString &query);
    Q_INVOKABLE Core::QueryMatchList queryInRange(const Core::RangeMark &range, const QString &query);

    // This overload exists for improved performance. It's not user-facing API.
    //
    // It turns out that constructing Query instances is relatively expensive.
    // Therefore it's better to construct them once and reuse them.
    // So allow this for outside users.
    QList<Core::QueryMatch> query(const std::shared_ptr<treesitter::Query> &query);
    Core::QueryMatch queryFirst(const std::shared_ptr<treesitter::Query> &query);

    bool hasLspClient() const;

    Symbol *currentSymbol(const std::function<bool(const Symbol &)> &filterFunc) const;
    void deleteSymbol(const Symbol &symbol);

    // As per KUT-163, KNUT-164 and KNUT-165, these are no longer public API.
    // They are only used internally by the editor/GUI and not available from QML/JS.
    // As they rely on the clangd LSP, they are not reliable enough to use for scripting.
    Core::Document *switchDeclarationDefinition();
    Core::Document *followSymbol();
    Core::RangeMarkList references(int position) const;

    QString hover(int position, std::function<void(const QString &)> asyncCallback = {}) const;

    Q_INVOKABLE Core::AstNode astNodeAt(int pos);

public slots:
    void selectSymbol(const QString &name, int options = NoFindFlags);

    int selectLargerSyntaxNode(int count = 1);
    int selectSmallerSyntaxNode(int count = 1);
    int selectNextSyntaxNode(int count = 1);
    int selectPreviousSyntaxNode(int count = 1);

protected:
    explicit CodeDocument(Type type, QObject *parent = nullptr);

    void didOpen() override;
    void didClose() override;

    Lsp::Client *client() const;
    std::string toUri() const;

    int revision() const;

    std::pair<QString, std::optional<TextRange>>
    hoverWithRange(int position,
                   std::function<void(const QString &, std::optional<TextRange>)> asyncCallback = {}) const;

private:
    bool checkClient() const;
    Document *followSymbol(int pos);

    std::optional<treesitter::QueryCursor> createQueryCursor(const std::shared_ptr<treesitter::Query> &query);

    void changeContent(int position, int charsRemoved, int charsAdded);
    void changeContentLsp(int position, int charsRemoved, int charsAdded);
    void changeContentTreeSitter(int position, int charsRemoved, int charsAdded);

    // Language Server
    QPointer<Lsp::Client> m_lspClient;
    int m_revision = 0;

    // TreeSitter
    friend TreeSitterHelper;
    std::unique_ptr<TreeSitterHelper> m_treeSitterHelper;

    friend class AstNode;
};

} // namespace Core
