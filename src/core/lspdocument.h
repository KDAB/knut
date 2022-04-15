#pragma once

#include "symbol.h"
#include "textdocument.h"

#include <functional>
#include <memory>

namespace Lsp {
class Client;
struct Position;
struct Range;
}

namespace Core {

class LspCache;
struct RegexpTransform;

class LspDocument : public TextDocument
{
    Q_OBJECT

public:
    ~LspDocument();

    void setLspClient(Lsp::Client *client);

    Q_INVOKABLE Core::Symbol findSymbol(const QString &name, int options = NoFindFlags) const;
    Q_INVOKABLE QVector<Core::Symbol> symbols() const;
    Q_INVOKABLE void transformSymbol(const QString &symbolName, const QString &jsonFileName);

    bool hasLspClient() const;

    Symbol currentSymbol(std::function<bool(const Symbol &)> filterFunc) const;

public slots:
    Document *followSymbol();
    Document *switchDeclarationDefinition();
    void selectSymbol(const QString &name, int options = NoFindFlags);

protected:
    explicit LspDocument(Type type, QObject *parent = nullptr);

    void didOpen() override;
    void didClose() override;

    Lsp::Client *client() const;
    std::string toUri() const;
    int toPos(const Lsp::Position &pos) const;
    TextRange toRange(const Lsp::Range &range) const;

    int revision() const;

private:
    bool checkClient() const;
    Document *followSymbol(int pos);
    void changeContent(int position, int charsRemoved, int charsAdded);

    // JSON Transformations
    void regexpTransform(const RegexpTransform &transform, const std::unordered_map<QString, QString> &regexpContext);

    friend LspCache;
    QPointer<Lsp::Client> m_lspClient;
    std::unique_ptr<LspCache> m_cache;

    int m_revision = 0;
};

} // namespace Core
