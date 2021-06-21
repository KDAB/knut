#include "datamodel.h"

#include "rcfile/data.h"
#include "rcviewer_global.h"

using namespace RcFile;

namespace {
const char *DataTypeStr[] = {
    "Dialogs", "Menus", "ToolBars", "Accelerators", "Assets", "Icons", "Strings", "Includes",
};
}

namespace RcUi {

DataModel::DataModel(const Data &data, QObject *parent)
    : QAbstractItemModel(parent)
    , m_data(data)
{
}

QModelIndex DataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column, static_cast<quintptr>(NoData));

    return createIndex(row, column, static_cast<quintptr>(parent.row()));
}

QModelIndex DataModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return {};

    int data = static_cast<int>(child.internalId());
    if (data == NoData)
        return {};

    return createIndex(data, 0, static_cast<quintptr>(NoData));
}

int DataModel::rowCount(const QModelIndex &parent) const
{
    if (!m_data.isValid)
        return 0;
    if (!parent.isValid())
        return static_cast<int>(std::size(DataTypeStr));

    int data = static_cast<int>(parent.internalId());
    if (data == NoData) {
        switch (parent.row()) {
        case DialogData:
            return m_data.dialogs.size();
        case MenuData:
            return m_data.menus.size();
        case ToolBarData:
            return m_data.toolBars.size();
        case AcceleratorData:
            return m_data.acceleratorTables.size();
        case AssetData:
        case IconData:
        case StringData:
        case IncludeData:
            return 0;
        }
    }
    return 0;
}

int DataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (!m_data.isValid)
        return 0;
    return 1;
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        int data = static_cast<int>(index.internalId());
        if (data == NoData)
            return QLatin1String(DataTypeStr[index.row()]);

        switch (data) {
        case DialogData:
            return m_data.dialogs.value(index.row()).id;
        case MenuData:
            return m_data.menus.value(index.row()).id;
        case ToolBarData:
            return m_data.toolBars.value(index.row()).id;
        case AcceleratorData:
            return m_data.acceleratorTables.value(index.row()).id;
        }
        return {};
    }

    if (role == LineRole) {
        const int data = static_cast<int>(index.internalId());
        switch (data) {
        case DialogData:
            return m_data.dialogs.value(index.row()).line;
        case MenuData:
            return m_data.menus.value(index.row()).line;
        case ToolBarData:
            return m_data.toolBars.value(index.row()).line;
        case AcceleratorData:
            return m_data.acceleratorTables.value(index.row()).line;
        case AssetData:
        case IconData:
        case StringData:
        case IncludeData:
        case NoData:
            return -1;
        }
        return -1;
    }

    if (role == TypeRole) {
        const int data = static_cast<int>(index.internalId());
        if (data == NoData)
            return index.row();
        return index.parent().row();
    }

    if (role == IndexRole) {
        const int data = static_cast<int>(index.internalId());
        if (data == NoData)
            return -1;
        return index.row();
    }
    return {};
}

} // namespace RcUi
