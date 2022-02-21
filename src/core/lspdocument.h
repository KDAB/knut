#pragma once

#include "symbol.h"
#include "textdocument.h"

namespace Lsp {
class Client;
struct Position;
struct Range;
}

namespace Core {

class LspCache;

class LspDocument : public TextDocument
{
    Q_OBJECT

public:
    ~LspDocument();

    void setLspClient(Lsp::Client *client);

    Q_INVOKABLE QVector<Core::Symbol> symbols() const;

protected:
    explicit LspDocument(Type type, QObject *parent = nullptr);

    void didOpen() override;
    void didClose() override;

    Lsp::Client *client() const;
    std::string toUri() const;
    int toPos(const Lsp::Position &pos) const;
    TextRange toRange(const Lsp::Range &range) const;

private:
    void changeContent(int position, int charsRemoved, int charsAdded);

    friend LspCache;
    QPointer<Lsp::Client> m_lspClient;
    std::unique_ptr<LspCache> m_cache;
};

} // namespace Core
