/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <spdlog/spdlog.h>

#if defined(SPDLOG_USE_STD_FORMAT)
#include "qt_std_format.h"
#else
#include "qt_fmt_format.h"
#endif
