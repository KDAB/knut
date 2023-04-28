#pragma once

#include "querymatch.h"
#include "symbol.h"
#include "textdocument.h"

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

class LspDocument : public TextDocument
{
    Q_OBJECT

public:
    virtual ~LspDocument();

    void setLspClient(Lsp::Client *client);

    Q_INVOKABLE Core::Symbol *findSymbol(const QString &name, int options = NoFindFlags) const;
    Q_INVOKABLE QVector<Core::Symbol *> symbols() const;
    Q_INVOKABLE QString hover() const;
    Q_INVOKABLE const Core::Symbol *symbolUnderCursor() const;
    Q_INVOKABLE QVector<Core::TextLocation> references(int position) const;

    Q_INVOKABLE QVector<Core::QueryMatch> query(const QString &query);
    Q_INVOKABLE QVector<Core::QueryMatch> queryInRange(const Core::RangeMark &range, const QString &query);

    bool hasLspClient() const;

    Symbol *currentSymbol(std::function<bool(const Symbol &)> filterFunc) const;
    void deleteSymbol(const Symbol &symbol);

    QString hover(int position, std::function<void(const QString &)> asyncCallback = {}) const;

    int toPos(const Lsp::Position &pos) const;
    TextRange toRange(const Lsp::Range &range) const;

public slots:
    Core::Document *followSymbol();
    Core::Document *switchDeclarationDefinition();
    void selectSymbol(const QString &name, int options = NoFindFlags);

protected:
    explicit LspDocument(Type type, QObject *parent = nullptr);

    void didOpen() override;
    void didClose() override;

    Lsp::Client *client() const;
    std::string toUri() const;
    Lsp::Position fromPos(int pos) const;

    int revision() const;

    std::pair<QString, std::optional<TextRange>>
    hoverWithRange(int position,
                   std::function<void(const QString &, std::optional<TextRange>)> asyncCallback = {}) const;

private:
    bool checkClient() const;
    Document *followSymbol(int pos);

    void changeContent(int position, int charsRemoved, int charsAdded);
    void changeContentLsp(int position, int charsRemoved, int charsAdded);
    void changeContentTreeSitter(int position, int charsRemoved, int charsAdded);

    // Language Server
    friend LspCache;
    QPointer<Lsp::Client> m_lspClient;
    std::unique_ptr<LspCache> m_cache;
    int m_revision = 0;

    // TreeSitter
    friend TreeSitterHelper;
    std::unique_ptr<TreeSitterHelper> m_treeSitterHelper;
};

} // namespace Core
