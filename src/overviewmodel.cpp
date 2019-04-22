#include "overviewmodel.h"

#include "data.h"
#include "global.h"

namespace {
static const char *DataTypeStr[] = {
    "Dialogs", "Menus", "ToolBars", "Accelerators", "Assets", "Icons", "Strings", "Includes",
};
}

OverviewModel::OverviewModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

void OverviewModel::setResourceData(Data *data)
{
    beginResetModel();
    m_data = data;
    endResetModel();
}

void OverviewModel::updateModel()
{
    beginResetModel();
    endResetModel();
}

QModelIndex OverviewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column, static_cast<quintptr>(Knut::NoData));

    return createIndex(row, column, static_cast<quintptr>(parent.row()));
}

QModelIndex OverviewModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return {};

    int data = static_cast<int>(child.internalId());
    if (data == Knut::NoData)
        return {};

    return createIndex(data, 0, static_cast<quintptr>(Knut::NoData));
}

int OverviewModel::rowCount(const QModelIndex &parent) const
{
    if (!m_data || !m_data->isValid)
        return 0;
    if (!parent.isValid())
        return static_cast<int>(std::size(DataTypeStr));

    int data = static_cast<int>(parent.internalId());
    if (data == Knut::NoData) {
        switch (parent.row()) {
        case Knut::DialogData:
            return m_data->dialogs.size();
        case Knut::MenuData:
            return m_data->menus.size();
        case Knut::ToolBarData:
            return m_data->toolBars.size();
        case Knut::AcceleratorData:
            return m_data->acceleratorTables.size();
        case Knut::AssetData:
        case Knut::IconData:
        case Knut::StringData:
        case Knut::IncludeData:
            return 0;
        }
    }
    return 0;
}

int OverviewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!m_data || !m_data->isValid)
        return 0;
    return 1;
}

QVariant OverviewModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        int data = static_cast<int>(index.internalId());
        if (data == Knut::NoData)
            return DataTypeStr[index.row()];

        switch (data) {
        case Knut::DialogData:
            return m_data->dialogs.value(index.row()).id;
        case Knut::MenuData:
            return m_data->menus.value(index.row()).id;
        case Knut::ToolBarData:
            return m_data->toolBars.value(index.row()).id;
        case Knut::AcceleratorData:
            return m_data->acceleratorTables.value(index.row()).id;
        }
        return {};
    }

    if (role == Knut::LineRole) {
        const int data = static_cast<int>(index.internalId());
        switch (data) {
        case Knut::DialogData:
            return m_data->dialogs.value(index.row()).line;
        case Knut::MenuData:
            return m_data->menus.value(index.row()).line;
        case Knut::ToolBarData:
            return m_data->toolBars.value(index.row()).line;
        case Knut::AcceleratorData:
            return m_data->acceleratorTables.value(index.row()).line;
        case Knut::AssetData:
        case Knut::IconData:
        case Knut::StringData:
        case Knut::IncludeData:
        case Knut::NoData:
            return -1;
        }
        return -1;
    }

    if (role == TypeRole) {
        const int data = static_cast<int>(index.internalId());
        if (data == Knut::NoData)
            return index.row();
        else
            return index.parent().row();
    }

    if (role == IndexRole) {
        const int data = static_cast<int>(index.internalId());
        if (data == Knut::NoData)
            return -1;
        else
            return index.row();
    }
    return {};
}
