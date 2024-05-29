/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <Qt>

namespace RcUi {

enum Roles {
    LineRole = Qt::UserRole + 1024,
};

enum DataType {
    NoData = -1,
    DialogData = 0,
    MenuData,
    ToolBarData,
    AcceleratorData,
    AssetData,
    IconData,
    StringData,
    IncludeData,
};

} // namespace RcUi
