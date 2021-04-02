#pragma once

#include "notificationmessage.h"
#include "requestmessage.h"

// LSP specification:
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#shutdown
namespace Lsp {

static constexpr char shutdownName[] = "shutdown";

struct ShutdownRequest : public RequestMessage<shutdownName, std::nullptr_t, std::nullptr_t, std::nullptr_t>
{
};

static constexpr char exitName[] = "exit";

struct ExitNotification : public NotificationMessage<exitName, std::nullptr_t>
{
};
}
