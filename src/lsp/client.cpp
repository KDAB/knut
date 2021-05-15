#include "client.h"

#include "clientbackend.h"
#include "notificationmessage_json.h"
#include "notifications.h"
#include "requestmessage_json.h"
#include "types_json.h"

#include "utils/test_utils.h"

#include <QCoreApplication>

#include <doctest/doctest.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Lsp {

Client::Client(const std::string &language, QString program, QStringList arguments, QObject *parent)
    : QObject(parent)
    , m_backend(new ClientBackend(language, program, arguments, this))
{
    const auto serverLogName = language + "_client";
    m_clientLogger = spdlog::get(serverLogName);
    if (!m_clientLogger) {
        m_clientLogger = spdlog::stdout_color_mt(serverLogName);
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

bool Client::initialize()
{
    if (!m_backend->start())
        return false;

    m_clientLogger->debug("LSP server started");

    InitializeRequest request;
    request.id = m_nextRequestId++;
    request.params.processId = QCoreApplication::applicationPid();
    request.params.clientInfo = {"knut", "4.0"};
    ClientCapabilities::WorkspaceType workspaceCapabilities;
    workspaceCapabilities.workspaceFolders = true;
    request.params.capabilities.workspace = workspaceCapabilities;

    m_clientLogger->debug("==> Sending InitializeRequest");
    return initializeCallback(m_backend->sendRequest(request));
}

bool Client::shutdown()
{
    ShutdownRequest request;
    request.id = m_nextRequestId++;
    m_clientLogger->debug("==> Sending ShutdownRequest");
    return shutdownCallback(m_backend->sendRequest(request));
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
        m_clientLogger->debug("Error initializing the server");
        setState(Error);
        return false;
    }

    // TODO initialize some client internal flags
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

} // namespace Lsp

///////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////
TEST_SUITE("lsp")
{
    TEST_CASE("client initialize and shutdown")
    {
        Lsp::Client client("cpp", "clangd", {});
        Test::LogSilencer clientLog("cpp_client");

        CHECK(client.initialize());
        CHECK(client.state() == Lsp::Client::Initialized);
        CHECK(client.shutdown());
        CHECK(client.state() == Lsp::Client::Shutdown);
    }
}
