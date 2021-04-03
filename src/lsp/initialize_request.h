#pragma once

#include "lsp_types.h"
#include "notificationmessage.h"
#include "requestmessage.h"

// LSP specification:
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialize
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialized
namespace Lsp {

struct InitiliazeParams
{
    IntegerOrNull processId = nullptr;
    // TODO everything else...
};

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

struct InitializeError
{
    bool retry;
};

static constexpr char initializeName[] = "initialize";
struct InitializeRequest : public RequestMessage<initializeName, InitiliazeParams, InitializeResult, InitializeError>
{
};

static constexpr char initializedName[] = "initialized";
struct InitializedNotification : public NotificationMessage<initializedName, std::nullptr_t>
{
};

///////////////////////////////////////////////////////////////////////////////
// Serialization
///////////////////////////////////////////////////////////////////////////////
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InitiliazeParams, processId);

void to_json(nlohmann::json &j, const InitializeResult::ServerInfo &serverInfo);
void from_json(const nlohmann::json &j, InitializeResult::ServerInfo &serverInfo);
void to_json(nlohmann::json &j, const InitializeResult &result);
void from_json(const nlohmann::json &j, InitializeResult &result);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InitializeError, retry);
}
