#pragma once

#include "requests.h"
#include "types.h"
#include "utils/log.h"

#include <QObject>
#include <string>

namespace Lsp {

class ClientBackend;

class Client : public QObject
{
    Q_OBJECT

public:
    enum State {
        Uninitialized,
        Initialized,
        Shutdown,
        Error,
    };

    explicit Client(std::string languageId, QString program, QStringList arguments, QObject *parent = nullptr);
    ~Client() override;

    std::string languageId() const;

    bool initialize(const QString &rootPath = {});
    bool shutdown();

    /**
     * Opens a new project, this will add a new workspace on the server
     */
    void openProject(const QString &rootPath);
    /**
     * Closes a project, the project will be removed from the server's workspaces
     */
    void closeProject(const QString &rootPath);

    /**
     * Sends the didOpen notification, when a document has been opened
     */
    void didOpen(DidOpenTextDocumentParams &&params);
    /**
     * Sends the didClose notification, when a document has been closed
     */
    void didClose(DidCloseTextDocumentParams &&params);

    /**
     * Sends the didChange notification, when a document has been changed
     */
    void didChange(DidChangeTextDocumentParams &&params);

    /**
     * Query which kind of document changes can be sent to the server.
     * Either Full or Incremental.
     */
    bool canSendDocumentChanges(TextDocumentSyncKind kind) const;

    /**
     * ##### LSP requests #####
     * If asyncCallback is not null, the request will be sent asynchronously and the callback called once the response
     * has arrived. Otherwise, the request is synchronous, and the result is returned. An empty optional means there was
     * an error.
     */
    std::optional<TextDocumentDocumentSymbolRequest::Result>
    documentSymbol(DocumentSymbolParams &&params,
                   std::function<void(TextDocumentDocumentSymbolRequest::Result)> asyncCallback = {});

    std::optional<TextDocumentDeclarationRequest::Result>
    declaration(DeclarationParams &&params,
                std::function<void(TextDocumentDeclarationRequest::Result)> asyncCallback = {});

    std::optional<TextDocumentHoverRequest::Result>
    hover(HoverParams &&params, std::function<void(TextDocumentHoverRequest::Result)> asyncCallback = {});

    std::optional<TextDocumentReferencesRequest::Result>
    references(ReferenceParams &&params, std::function<void(TextDocumentReferencesRequest::Result)> asyncCallback = {});

    State state() const { return m_state; }

    static std::string toUri(const QString &path);

signals:
    void stateChanged(Lsp::Client::State state);

private:
    void setState(State newState);
    bool initializeCallback(InitializeRequest::Response response);
    bool shutdownCallback(ShutdownRequest::Response response);

    bool canSendWorkspaceFoldersChanges() const;
    bool canSendOpenCloseChanges() const;
    bool canSendDocumentSymbol() const;
    bool canSendDeclaration() const;
    bool canSendHover() const;
    bool canSendReferences() const;

    template <typename Request, typename Params>
    std::optional<typename Request::Result>
    sendGenericRequest(bool (Client::*canSend)() const, const char *name, Params &&params,
                       std::function<void(typename Request::Result)> asyncCallback)
    {
        if (!(this->*canSend)()) {
            spdlog::error("{} not supported by LSP server", name);
            return {};
        }

        Request request;
        request.id = m_nextRequestId++;
        request.params = std::forward<Params>(params);

        return sendRequest(m_backend, request, asyncCallback);
    }

    template <typename Options, typename Variant>
    bool canSend(Variant Lsp::ServerCapabilities::*pProvider) const
    {
        Q_ASSERT(m_state == Initialized);
        // TODO handle dynamic capabilities
        if (auto provider = m_serverCapabilities.*pProvider) {
            if (std::holds_alternative<Options>(provider.value()) || std::get<bool>(provider.value())) {
                return true;
            }
        }
        return false;
    }

private:
    mutable int m_nextRequestId = 1;
    std::string m_languageId;
    ClientBackend *m_backend = nullptr;
    State m_state = Uninitialized;

    ServerCapabilities m_serverCapabilities;
};

} // namespace Lsp
