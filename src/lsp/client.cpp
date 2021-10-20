#include "client.h"

#include "clientbackend.h"
#include "lsp_utils.h"
#include "notificationmessage_json.h"
#include "notifications.h"
#include "requestmessage_json.h"
#include "types_json.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Lsp {

Client::Client(std::string languageId, QString program, QStringList arguments, QObject *parent)
    : QObject(parent)
    , m_languageId(std::move(languageId))
    , m_backend(new ClientBackend(m_languageId, std::move(program), std::move(arguments), this))
{
    const auto clientLogName = m_languageId + "_client";
    m_clientLogger = spdlog::get(clientLogName);
    if (!m_clientLogger) {
        m_clientLogger = spdlog::stdout_color_mt(clientLogName);
        m_clientLogger->set_level(spdlog::level::debug);
    }

    connect(m_backend, &ClientBackend::errorOccured, this, [this]() {
        setState(Error);
    });
    connect(m_backend, &ClientBackend::finished, this, [this]() {
        setState(Shutdown);
    });
}

Client::~Client() { }

std::string Client::languageId() const
{
    return m_languageId;
}

bool Client::initialize(const QString &rootPath)
{
    if (!m_backend->start())
        return false;

    m_clientLogger->debug("LSP server started");

    InitializeRequest request;
    request.id = m_nextRequestId++;
    request.params.processId = static_cast<int>(QCoreApplication::applicationPid());
    request.params.clientInfo = {"knut", "4.0"};

    // Workspace capabilities
    ClientCapabilities::WorkspaceType workspaceCapabilities;
    workspaceCapabilities.workspaceFolders = true;
    request.params.capabilities.workspace = workspaceCapabilities;

    // TextDocument capabilities
    TextDocumentSyncClientCapabilities synchronization;
    synchronization.dynamicRegistration = false;
    TextDocumentClientCapabilities textDocument;
    textDocument.synchronization = synchronization;
    request.params.capabilities.textDocument = textDocument;

    QFileInfo fi(rootPath);
    if (fi.exists()) {
        request.params.rootPath = QDir::toNativeSeparators(rootPath).toStdString();
        request.params.rootUri = Utils::toDocumentUri(rootPath);
        std::vector<WorkspaceFolder> wsf = {{Utils::toDocumentUri(rootPath), fi.baseName().toStdString()}};
        request.params.workspaceFolders = wsf;
    }

    return initializeCallback(m_backend->sendRequest(request));
}

bool Client::shutdown()
{
    Q_ASSERT(m_state == Initialized);
    ShutdownRequest request;
    request.id = m_nextRequestId++;
    return shutdownCallback(m_backend->sendRequest(request));
}

void Client::openProject(const QString &rootPath)
{
    Q_ASSERT(m_state == Initialized);
    QFileInfo fi(rootPath);
    if (!fi.exists() || !sendWorkspaceFoldersChanges())
        return;

    DidChangeWorkspaceFoldersNotification notification;
    notification.params.event.added.push_back({Utils::toDocumentUri(rootPath), fi.baseName().toStdString()});
    m_backend->sendNotification(notification);
}

void Client::closeProject(const QString &rootPath)
{
    Q_ASSERT(m_state == Initialized);
    QFileInfo fi(rootPath);
    if (!fi.exists() || !sendWorkspaceFoldersChanges())
        return;

    DidChangeWorkspaceFoldersNotification notification;
    notification.params.event.removed.push_back({Utils::toDocumentUri(rootPath), fi.baseName().toStdString()});
    m_backend->sendNotification(notification);
}

void Client::didOpen(DidOpenTextDocumentParams params)
{
    if (!sendOpenCloseChanges())
        return;

    DidOpenNotification notification;
    notification.params = std::move(params);
    m_backend->sendNotification(notification);
}

void Client::didClose(DidCloseTextDocumentParams params)
{
    if (!sendOpenCloseChanges())
        return;

    DidCloseNotification notification;
    notification.params = std::move(params);
    m_backend->sendNotification(notification);
}

std::string Client::toUri(const QString &path)
{
    QFileInfo fi(path);
    return QUrl::fromLocalFile(fi.absoluteFilePath()).toString().toStdString();
}

void Client::setState(State newState)
{
    if (m_state == newState)
        return;
    m_state = newState;
    emit stateChanged(m_state);
}

bool Client::initializeCallback(InitializeRequest::Response response)
{
    if (!response.isValid() || response.error) {
        m_clientLogger->error("Error initializing the server");
        setState(Error);
        return false;
    }

    m_serverCapabilities = response.result->capabilities;
    m_backend->sendNotification(InitializedNotification());
    m_clientLogger->debug("LSP server initialized");
    setState(Initialized);
    return true;
}

bool Client::shutdownCallback(ShutdownRequest::Response response)
{
    if (!response.isValid() || response.error) {
        m_clientLogger->error("Error shutting down the server");
        setState(Error);
        return false;
    }

    m_backend->sendNotification(ExitNotification());
    m_clientLogger->debug("LSP server exited");
    setState(Shutdown);
    return true;
}

bool Client::sendWorkspaceFoldersChanges() const
{
    Q_ASSERT(m_state == Initialized);
    // TODO handle dynamic capabilities
    if (auto workspace = m_serverCapabilities.workspace) {
        if (auto workspaceFolders = workspace.value().workspaceFolders) {
            if (workspaceFolders.value().supported.value_or(false)) {
                auto notifications = workspaceFolders.value().changeNotifications.value_or(false);
                return std::holds_alternative<std::string>(notifications) || std::get<bool>(notifications);
            }
        }
    }
    return false;
}

bool Client::sendOpenCloseChanges() const
{
    Q_ASSERT(m_state == Initialized);
    // TODO handle dynamic capabilities
    if (auto textDocument = m_serverCapabilities.textDocumentSync) {
        if (std::holds_alternative<TextDocumentSyncKind>(textDocument.value())) {
            auto syncKind = std::get<TextDocumentSyncKind>(textDocument.value());
            return syncKind != TextDocumentSyncKind::None;
        } else {
            auto syncOptions = std::get<TextDocumentSyncOptions>(textDocument.value());
            return syncOptions.openClose.value_or(false);
        }
    }
    return false;
}

} // namespace Lsp
