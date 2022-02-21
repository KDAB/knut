#pragma once

#include "symbol.h"
#include "textdocument.h"

namespace Lsp {
class Client;
struct Position;
struct Range;
}

namespace Core {

class LspDocument : public TextDocument
{
    Q_OBJECT

public:
    explicit LspDocument(QObject *parent = nullptr);
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
    QPointer<Lsp::Client> m_lspClient;
};

} // namespace Core
