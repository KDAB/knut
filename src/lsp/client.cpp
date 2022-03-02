#include "client.h"

#include "clientbackend.h"
#include "lsp_utils.h"
#include "notificationmessage_json.h"
#include "notifications.h"
#include "requestmessage_json.h"
#include "types_json.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QUrl>

#include <spdlog/spdlog.h>

namespace Lsp {

template <typename Request>
std::optional<typename Request::Result> sendRequest(ClientBackend *backend, Request request,
                                                    std::function<void(typename Request::Result)> callback)
{
    auto checkResponse = [&](typename Request::Response response) {
        if (!response.isValid() || response.error) {
            spdlog::warn("Response error for request {} - {}", request.method,
                         response.error ? response.error->message : "");
            return false;
        }
        return true;
    };

    if (callback) {
        auto requestCallBack = [&](typename Request::Response response) {
            if (checkResponse(response))
                callback(response.result.value());
        };
        backend->sendAsyncRequest(request, requestCallBack);
    } else {
        QElapsedTimer time;
        time.start();
        auto response = backend->sendRequest(request);
        spdlog::trace("{} ms for handling request {}", static_cast<int>(time.elapsed()), request.method);
        if (checkResponse(response))
            return response.result;
    }
    return {};
}

Client::Client(std::string languageId, QString program, QStringList arguments, QObject *parent)
    : QObject(parent)
    , m_languageId(std::move(languageId))
    , m_backend(new ClientBackend(m_languageId, std::move(program), std::move(arguments), this))
{
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

    spdlog::debug("LSP server started");

    InitializeRequest request;
    request.id = m_nextRequestId++;
    request.params.processId = static_cast<int>(QCoreApplication::applicationPid());
    request.params.clientInfo = {"knut", "4.0"};

    // Workspace capabilities
    {
        ClientCapabilities::WorkspaceType workspaceCapabilities;
        workspaceCapabilities.workspaceFolders = true;
        request.params.capabilities.workspace = workspaceCapabilities;
    }

    // TextDocument capabilities
    {
        TextDocumentSyncClientCapabilities synchronization;
        synchronization.dynamicRegistration = false;

        DocumentSymbolClientCapabilities symbol;
        symbol.dynamicRegistration = false;
        symbol.hierarchicalDocumentSymbolSupport = true;
        symbol.labelSupport = true;
        std::vector<SymbolKind> symbolKinds = {
            SymbolKind::File,        SymbolKind::Module,       SymbolKind::Namespace, SymbolKind::Package,
            SymbolKind::Class,       SymbolKind::Method,       SymbolKind::Property,  SymbolKind::Field,
            SymbolKind::Constructor, SymbolKind::Enum,         SymbolKind::Interface, SymbolKind::Function,
            SymbolKind::Variable,    SymbolKind::Constant,     SymbolKind::String,    SymbolKind::Number,
            SymbolKind::Boolean,     SymbolKind::Array,        SymbolKind::Object,    SymbolKind::Key,
            SymbolKind::Null,        SymbolKind::EnumMember,   SymbolKind::Struct,    SymbolKind::Event,
            SymbolKind::Operator,    SymbolKind::TypeParameter};
        symbol.symbolKind = DocumentSymbolClientCapabilities::SymbolKindType {symbolKinds};

        SemanticTokensClientCapabilities semanticTokens;
        semanticTokens.dynamicRegistration = false;
        semanticTokens.requests.full = true;
        semanticTokens.formats = {TokenFormat::Relative};

        DeclarationClientCapabilities declaration;
        declaration.dynamicRegistration = false;
        declaration.linkSupport = true;

        TextDocumentClientCapabilities textDocument;
        textDocument.synchronization = synchronization;
        textDocument.documentSymbol = symbol;
        textDocument.declaration = declaration;
        textDocument.semanticTokens = semanticTokens;
        request.params.capabilities.textDocument = textDocument;
    }

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
    if (!fi.exists() || !canSendWorkspaceFoldersChanges())
        return;

    DidChangeWorkspaceFoldersNotification notification;
    notification.params.event.added.push_back({Utils::toDocumentUri(rootPath), fi.baseName().toStdString()});
    m_backend->sendNotification(notification);
}

void Client::closeProject(const QString &rootPath)
{
    Q_ASSERT(m_state == Initialized);
    QFileInfo fi(rootPath);
    if (!fi.exists() || !canSendWorkspaceFoldersChanges())
        return;

    DidChangeWorkspaceFoldersNotification notification;
    notification.params.event.removed.push_back({Utils::toDocumentUri(rootPath), fi.baseName().toStdString()});
    m_backend->sendNotification(notification);
}

void Client::didOpen(DidOpenTextDocumentParams &&params)
{
    if (!canSendOpenCloseChanges())
        return;

    DidOpenNotification notification;
    notification.params = std::move(params);
    m_backend->sendNotification(notification);
}

void Client::didClose(DidCloseTextDocumentParams &&params)
{
    if (!canSendOpenCloseChanges())
        return;

    DidCloseNotification notification;
    notification.params = std::move(params);
    m_backend->sendNotification(notification);
}

std::optional<DocumentSymbolRequest::Result>
Client::documentSymbol(DocumentSymbolParams &&params, std::function<void(DocumentSymbolRequest::Result)> asyncCallback)
{
    if (!canSendDocumentSymbol()) {
        spdlog::error("{} not supported by LSP server", DocumentSymbolName);
        return {};
    }

    DocumentSymbolRequest request;
    request.id = m_nextRequestId++;
    request.params = std::move(params);
    return sendRequest(m_backend, request, asyncCallback);
}

std::optional<DeclarationRequest::Result>
Client::declaration(DeclarationParams &&params, std::function<void(DeclarationRequest::Result)> asyncCallback /* = {}*/)
{
    if (!canSendDeclaration()) {
        spdlog::error("{} not supported by LSP server", DeclarationName);
        return {};
    }

    DeclarationRequest request;
    request.id = m_nextRequestId++;
    request.params = std::move(params);
    return sendRequest(m_backend, request, asyncCallback);
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
        spdlog::error("Error initializing the server");
        setState(Error);
        return false;
    }

    m_serverCapabilities = response.result->capabilities;
    m_backend->sendNotification(InitializedNotification());
    spdlog::debug("LSP server initialized");
    setState(Initialized);
    return true;
}

bool Client::shutdownCallback(ShutdownRequest::Response response)
{
    if (!response.isValid() || response.error) {
        spdlog::error("Error shutting down the server");
        setState(Error);
        return false;
    }

    m_backend->sendNotification(ExitNotification());
    spdlog::debug("LSP server exited");
    setState(Shutdown);
    return true;
}

bool Client::canSendWorkspaceFoldersChanges() const
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

bool Client::canSendOpenCloseChanges() const
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

bool Client::canSendDocumentSymbol() const
{
    Q_ASSERT(m_state == Initialized);
    // TODO handle dynamic capabilities
    if (auto documentSymbolProvider = m_serverCapabilities.documentSymbolProvider) {
        if (std::holds_alternative<DocumentSymbolOptions>(documentSymbolProvider.value())
            || std::get<bool>(documentSymbolProvider.value())) {
            return true;
        }
    }
    return false;
}

bool Client::canSendDeclaration() const
{
    Q_ASSERT(m_state == Initialized);
    // TODO handle dynamic capabilities
    if (auto declarationProvider = m_serverCapabilities.declarationProvider) {
        if (std::holds_alternative<DeclarationOptions>(declarationProvider.value())
            || std::get<bool>(declarationProvider.value())) {
            return true;
        }
    }
    return false;
}
} // namespace Lsp
