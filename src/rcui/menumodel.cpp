#include "menumodel.h"

#include "rcviewer_global.h"

using namespace RcCore;

namespace RcUi {

MenuModel::MenuModel(const Data &data, int index, QObject *parent)
    : QAbstractItemModel(parent)
{
    const Menu &menu = data.menus.value(index);
    createTree(m_root, menu.children, data);
}

QModelIndex MenuModel::index(int row, int column, const QModelIndex &parent) const
{
    Item *parentItem = indexToItem(parent);
    Item *item = &(parentItem->children[row]);
    return createIndex(row, column, item);
}

QModelIndex MenuModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return {};

    Item *item = indexToItem(child);
    Item *parentItem = item->parent;
    if (parentItem == &m_root)
        return {};
    return createIndex(parentItem->index, 0, parentItem);
}

int MenuModel::rowCount(const QModelIndex &parent) const
{
    auto menu = indexToItem(parent);
    return menu->children.size();
}

int MenuModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant MenuModel::data(const QModelIndex &index, int role) const
{
    auto menu = indexToItem(index);
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        switch (index.column()) {
        case Title:
            return menu->title;
        case ID:
            return menu->id;
        case Shortcut:
            return menu->shortcut;
        case ToolTip:
            return menu->tooltip;
        case StatusTip:
            return menu->statusTip;
        }
        return {};
    }

    if (role == LineRole)
        return menu->line;

    if (role == Qt::CheckStateRole && index.column() == Checked) {
        if (menu->isChecked)
            return Qt::Checked;
    }

    return {};
}

QVariant MenuModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {tr("Title"), tr("Checked"), tr("ID"), tr("Shortcut"), tr("ToolTip"), tr("StatusTip")};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}

void MenuModel::createTree(Item &item, const QVector<MenuItem> &children, const Data &data)
{
    item.children.resize(children.size());
    for (int i = 0; i < children.size(); ++i) {
        const auto &childMenu = children.value(i);
        Item &childItem = item.children[i];
        childItem.index = i;
        childItem.parent = &item;
        childItem.title = childMenu.text;
        if (childMenu.text.isEmpty())
            childItem.title = QStringLiteral("--------");
        childItem.id = childMenu.id;
        childItem.shortcut = childMenu.shortcut;
        childItem.line = childMenu.line;
        childItem.isChecked = (childMenu.flags & MenuItem::Checked);

        const auto &text = data.strings.value(childMenu.id);
        if (!text.text.isEmpty()) {
            auto split = text.text.split(QLatin1Char('\n'));
            childItem.statusTip = split.first();
            if (split.size() > 1)
                childItem.tooltip = split.value(1);
        }
        createTree(childItem, childMenu.children, data);
    }
}

MenuModel::Item *MenuModel::indexToItem(const QModelIndex &index) const
{
    if (index.isValid())
        return reinterpret_cast<Item *>(index.internalPointer());
    return const_cast<Item *>(&m_root);
}

} // namespace RcUi
