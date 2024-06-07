/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "qt_fmt_format.h"

#include <optional>
#include <spdlog/spdlog.h>

namespace Log::detail {
inline bool disableLogging = false;
}

#define TRACE(fmt, ...)                                                                                                \
    do {                                                                                                               \
        if (Log::detail::disableLogging)                                                                               \
            break;                                                                                                     \
        spdlog::trace(fmt, ##__VA_ARGS__);                                                                             \
    } while (false)

#define DEBUG(fmt, ...)                                                                                                \
    do {                                                                                                               \
        if (Log::detail::disableLogging)                                                                               \
            break;                                                                                                     \
        spdlog::debug(fmt, ##__VA_ARGS__);                                                                             \
    } while (false)

#define INFO(fmt, ...)                                                                                                 \
    do {                                                                                                               \
        if (Log::detail::disableLogging)                                                                               \
            break;                                                                                                     \
        spdlog::info(fmt, ##__VA_ARGS__);                                                                              \
    } while (false)

#define WARN(fmt, ...)                                                                                                 \
    do {                                                                                                               \
        if (Log::detail::disableLogging)                                                                               \
            break;                                                                                                     \
        spdlog::warn(fmt, ##__VA_ARGS__);                                                                              \
    } while (false)

#define ERROR(fmt, ...)                                                                                                \
    do {                                                                                                               \
        if (Log::detail::disableLogging)                                                                               \
            break;                                                                                                     \
        spdlog::error(fmt, ##__VA_ARGS__);                                                                             \
    } while (false)

#define CRITICAL(fmt, ...)                                                                                             \
    do {                                                                                                               \
        if (Log::detail::disableLogging)                                                                               \
            break;                                                                                                     \
        spdlog::critical(fmt, ##__VA_ARGS__);                                                                          \
    } while (false)
