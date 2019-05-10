#include "toolbarmodel.h"

#include "global.h"

#include <QColor>

ToolBarModel::ToolBarModel(const Data::ToolBar &toolBar, QObject *parent)
    : QAbstractListModel(parent)
    , m_items(toolBar.children)
{
}

int ToolBarModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant ToolBarModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        const auto &item = m_items.at(index.row());
        if (item.id.isEmpty())
            return "--------";
        return item.id;
    }

    if (role == Knut::LineRole) {
        const auto &item = m_items.at(index.row());
        return item.line;
    }

    return {};
}

QVariant ToolBarModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {"Id"};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}
