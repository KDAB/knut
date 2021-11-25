#pragma once

#include "requests.h"

#include <QObject>

#include <spdlog/spdlog.h>

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
    ~Client();

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
     * Sends the documentSymbol requests, and returns the list of symbols
     * If asyncCallback is not null, the request will be sent asynchronously and the callback called once the respone
     * has arrive. Otherwise, the request is synchronous, and the result is returned. An empty optional means there was
     * an error.
     */
    std::optional<DocumentSymbolRequest::Result>
    documentSymbol(DocumentSymbolParams &&params,
                   std::function<void(DocumentSymbolRequest::Result)> asyncCallback = {});

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

private:
    mutable int m_nextRequestId = 1;
    std::string m_languageId;
    std::shared_ptr<spdlog::logger> m_clientLogger;
    ClientBackend *m_backend = nullptr;
    State m_state = Uninitialized;

    ServerCapabilities m_serverCapabilities;
};

} // namespace Lsp
