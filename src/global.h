#ifndef TYPES_H
#define TYPES_H

#include <QMetaType>

namespace Knut {

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

}

#endif // TYPES_H
