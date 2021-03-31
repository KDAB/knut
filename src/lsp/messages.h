#pragma once

#include "requestmessage.h"

namespace Lsp {

struct InitiliazeParams
{
    IntegerOrNull processId = nullptr;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InitiliazeParams, processId);

static constexpr char initializeName[] = "initialize";
struct InitializeRequest : public RequestMessage<initializeName, InitiliazeParams>
{
};

}
