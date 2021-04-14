#pragma once

#include "lsp_types.h"
#include "notificationmessage.h"
#include "requestmessage.h"

// LSP specification:
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialize
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialized
namespace Lsp {

struct InitializeParams
{
    IntegerOrNull processId = nullptr;
    // TODO everything else...
};
JSONIFY(InitializeParams, processId);

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
