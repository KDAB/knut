#pragma once

#include "types.h"
#include "notificationmessage.h"
#include "requestmessage.h"

// LSP specification:
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialize
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialized
namespace Lsp {

inline constexpr char initializeName[] = "initialize";
struct InitializeRequest : public RequestMessage<initializeName, InitializeParams, InitializeResult, InitializeError>
{
};

inline constexpr char initializedName[] = "initialized";
struct InitializedNotification : public NotificationMessage<initializedName, std::nullptr_t>
{
};

}
