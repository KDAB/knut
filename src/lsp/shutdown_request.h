#pragma once

#include "requestmessage.h"

// LSP specification:
// https://microsoft.github.io/language-server-protocol/specifications/specification-current/#shutdown
namespace Lsp {

static constexpr char shutdownName[] = "shutdown";

struct ShutdownRequest : public RequestMessage<shutdownName, std::nullptr_t, std::nullptr_t, std::nullptr_t>
{
};

}
