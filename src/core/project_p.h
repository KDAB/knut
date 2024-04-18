#pragma once

#include "document.h"
#include "utils/json.h"

namespace Core {

//! Store settings relative to a LSP server
struct LspServer
{
    Document::Type type;
    QString program;
    QStringList arguments;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LspServer, type, program, arguments);

} // namespace Core
