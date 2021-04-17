#pragma once

#include "capabilities.h"
#include "notificationmessage.h"
#include "requestmessage.h"

// LSP specification:
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialize
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialized
namespace Lsp {

struct InitializeParams
{
    struct ClientInfo
    {
        std::string name;
        std::optional<std::string> version;
    };

    std::variant<int, std::nullptr_t> processId = nullptr;
    std::optional<ClientInfo> clientInfo;
    std::optional<std::string> locale;
    // deprecated: rootPath?: string | null;
    // deprecated: rootUri: DocumentUri | null;
    // TODO user provided initializationOptions?: any;
    ClientCapabilities capabilities;
    std::optional<TraceValue> trace;
    std::optional<std::vector<WorkspaceFolder>> workspaceFolders;
};
JSONIFY(InitializeParams::ClientInfo, name, version);
JSONIFY(InitializeParams, processId, clientInfo, locale, capabilities, trace, workspaceFolders);

struct InitializeResult
{
    struct ServerInfo
    {
        std::string name;
        std::optional<std::string> version;
    };

    // TODO capabilities
    std::optional<ServerInfo> serverInfo;
};
JSONIFY(InitializeResult::ServerInfo, name, version);
JSONIFY(InitializeResult, serverInfo);

struct InitializeError
{
    bool retry;
};
JSONIFY(InitializeError, retry);

inline constexpr char initializeName[] = "initialize";
struct InitializeRequest : public RequestMessage<initializeName, InitializeParams, InitializeResult, InitializeError>
{
};

inline constexpr char initializedName[] = "initialized";
struct InitializedNotification : public NotificationMessage<initializedName, std::nullptr_t>
{
};

}
