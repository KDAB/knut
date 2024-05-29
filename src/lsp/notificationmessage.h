/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <string>

namespace Lsp {

/**
 *  \brief Notification message (client or server)
 */
template <const char *MethodName, typename NotificationParams>
struct NotificationMessage
{
    std::string jsonrpc = "2.0";
    std::string method = MethodName;
    // params could be optional, we are doing that by passing std::nullptr_t as the type
    NotificationParams params;
};

}
