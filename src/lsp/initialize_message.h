#pragma once

#include "message_types.h"
#include "requestmessage.h"

// LSP specification:
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/
namespace Lsp {

///////////////////////////////////////////////////////////////////////////////
// Initialize Request
///////////////////////////////////////////////////////////////////////////////
static constexpr char initializeName[] = "initialize";

struct InitiliazeParams
{
    IntegerOrNull processId = nullptr;
    // TODO everything else...
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InitiliazeParams, processId);

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
void to_json(nlohmann::json &j, const InitializeResult &result);
void from_json(const nlohmann::json &j, InitializeResult &result);

struct InitializeError
{
    bool retry;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InitializeError, retry);

struct InitializeRequest : public RequestMessage<initializeName, InitiliazeParams, InitializeResult, InitializeError>
{
};

}
