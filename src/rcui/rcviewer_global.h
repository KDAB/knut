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
