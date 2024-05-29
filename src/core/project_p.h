/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

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
