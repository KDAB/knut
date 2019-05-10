#ifndef TYPES_H
#define TYPES_H

#include <QMetaType>

#include <algorithm>

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

template<typename T, typename Compare>
void sort(T &container, Compare func)
{
    std::sort(container.begin(), container.end(), func);
}

}

#endif // TYPES_H
