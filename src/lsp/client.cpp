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
    auto checkResponse = [request](typename Request::Response response) {
        if (!response.isValid() || response.error) {
            spdlog::warn("Response error for request {} - {}", request.method,
                         response.error ? response.error->message : "");
            return false;
        }
        return true;
    };

    if (callback) {
        auto requestCallBack = [checkResponse, callback = std::move(callback)](typename Request::Response response) {
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

    spdlog::debug("LSP server started in: {}", rootPath.toStdString());

    InitializeRequest request;
    request.id = m_nextRequestId++;
    request.params.processId = static_cast<int>(QCoreApplication::applicationPid());
    request.params.clientInfo = {"knut", "4.0"};

    // Workspace capabilities
    {
        WorkspaceClientCapabilities workspaceCapabilities;
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

    WorkspaceDidChangeWorkspaceFoldersNotification notification;
    notification.params.event.added.push_back({Utils::toDocumentUri(rootPath), fi.baseName().toStdString()});
    m_backend->sendNotification(notification);
}

void Client::closeProject(const QString &rootPath)
{
    Q_ASSERT(m_state == Initialized);
    QFileInfo fi(rootPath);
    if (!fi.exists() || !canSendWorkspaceFoldersChanges())
        return;

    WorkspaceDidChangeWorkspaceFoldersNotification notification;
    notification.params.event.removed.push_back({Utils::toDocumentUri(rootPath), fi.baseName().toStdString()});
    m_backend->sendNotification(notification);
}

void Client::didOpen(DidOpenTextDocumentParams &&params)
{
    if (!canSendOpenCloseChanges())
        return;

    TextDocumentDidOpenNotification notification;
    notification.params = std::move(params);
    m_backend->sendNotification(notification);
}

void Client::didClose(DidCloseTextDocumentParams &&params)
{
    if (!canSendOpenCloseChanges())
        return;

    TextDocumentDidCloseNotification notification;
    notification.params = std::move(params);
    m_backend->sendNotification(notification);
}

void Client::didChange(DidChangeTextDocumentParams &&params)
{
    if (!canSendOpenCloseChanges())
        return;

    TextDocumentDidChangeNotification notification;
    notification.params = std::move(params);
    m_backend->sendNotification(notification);
}

std::optional<TextDocumentDocumentSymbolRequest::Result>
Client::documentSymbol(DocumentSymbolParams &&params,
                       std::function<void(TextDocumentDocumentSymbolRequest::Result)> asyncCallback)
{
    return sendGenericRequest<TextDocumentDocumentSymbolRequest>(
        &Client::canSendDocumentSymbol, TextDocumentDocumentSymbolName, std::move(params), asyncCallback);
}

std::optional<TextDocumentDeclarationRequest::Result>
Client::declaration(DeclarationParams &&params,
                    std::function<void(TextDocumentDeclarationRequest::Result)> asyncCallback /* = {}*/)
{
    return sendGenericRequest<TextDocumentDeclarationRequest>(&Client::canSendDeclaration, TextDocumentDeclarationName,
                                                              std::move(params), asyncCallback);
}

std::optional<TextDocumentHoverRequest::Result>
Client::hover(HoverParams &&params, std::function<void(TextDocumentHoverRequest::Result)> asyncCallback /* = {} */)
{
    return sendGenericRequest<TextDocumentHoverRequest>(&Client::canSendHover, TextDocumentHoverName, std::move(params),
                                                        asyncCallback);
}

std::optional<TextDocumentReferencesRequest::Result>
Client::references(ReferenceParams &&params,
                   std::function<void(TextDocumentReferencesRequest::Result)> asyncCallback /* = {} */)
{
    return sendGenericRequest<TextDocumentReferencesRequest>(&Client::canSendReferences, TextDocumentReferencesName,
                                                             std::move(params), asyncCallback);
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

bool Client::canSendDocumentChanges(TextDocumentSyncKind kind) const
{
    Q_ASSERT(m_state == Initialized);
    // TODO handle dynamic capabilities
    if (auto textDocument = m_serverCapabilities.textDocumentSync) {
        if (std::holds_alternative<TextDocumentSyncKind>(textDocument.value())) {
            auto syncKind = std::get<TextDocumentSyncKind>(textDocument.value());
            return syncKind == kind;
        } else {
            auto syncOptions = std::get<TextDocumentSyncOptions>(textDocument.value());
            return syncOptions.change.value_or(TextDocumentSyncKind::None) == kind;
        }
    }
    return false;
}

bool Client::canSendDocumentSymbol() const
{
    return canSend<DocumentSymbolOptions>(&Lsp::ServerCapabilities::documentSymbolProvider);
}

bool Client::canSendDeclaration() const
{
    return canSend<DeclarationOptions>(&Lsp::ServerCapabilities::declarationProvider);
}

bool Client::canSendHover() const
{
    return canSend<HoverOptions>(&Lsp::ServerCapabilities::hoverProvider);
}

bool Client::canSendReferences() const
{
    return canSend<ReferenceOptions>(&Lsp::ServerCapabilities::referencesProvider);
}

} // namespace Lsp
