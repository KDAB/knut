#include "menumodel.h"

#include "global.h"

static void createParentMap(Data::MenuItem *menu,
                            QHash<Data::MenuItem *, Data::MenuItem *> &parentMap,
                            QHash<Data::MenuItem *, int> indexMap)
{
    for (int i = 0; i < menu->children.size(); ++i) {
        auto child = &(menu->children[i]);
        parentMap[child] = menu;
        indexMap[child] = i;
        createParentMap(child, parentMap, indexMap);
    }
}

MenuModel::MenuModel(Data *data, Data::MenuItem *menu, QObject *parent)
    : QAbstractItemModel(parent)
    , m_data(data)
    , m_menu(menu)
{
    createParentMap(m_menu, m_parentMap, m_indexMap);
}

QModelIndex MenuModel::index(int row, int column, const QModelIndex &parent) const
{
    auto parentMenu = indexToMenu(parent);
    auto menu = &(parentMenu->children[row]);
    return createIndex(row, column, menu);
}

QModelIndex MenuModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return {};

    auto menu = indexToMenu(child);
    auto parentMenu = m_parentMap.value(menu);
    if (parentMenu == m_menu)
        return {};
    return createIndex(m_indexMap.value(menu), 0, parentMenu);
}

int MenuModel::rowCount(const QModelIndex &parent) const
{
    auto menu = indexToMenu(parent);
    return menu->children.size();
}

int MenuModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColumnCount;
}

QVariant MenuModel::data(const QModelIndex &index, int role) const
{
    auto menu = indexToMenu(index);
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        switch (index.column()) {
        case Title:
            if (menu->text.isEmpty())
                return "--------";
            return menu->text;
        case ID:
            return menu->id;
        case Shortcut:
            return menu->shortcut;
        case ToolTip: {
            const auto &text = m_data->strings.value(menu->id);
            if (text.text.contains(QLatin1Char('\n')))
                return text.text.split(QLatin1Char('\n')).value(1);
            break;
        }
        case StatusTip: {
            const auto &text = m_data->strings.value(menu->id);
            if (!text.text.isEmpty())
                return text.text.split(QLatin1Char('\n')).first();
            break;
        }
        }
        return {};
    }

    if (role == Knut::LineRole)
        return menu->line;

    if (role == Qt::CheckStateRole && index.column() == Checked) {
        if (menu->flags & Data::MenuItem::Checked)
            return Qt::Checked;
    }

    return {};
}

QVariant MenuModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {"Title", "Checked", "ID", "Shortcut", "ToolTip", "StatusTip"};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}

Data::MenuItem *MenuModel::indexToMenu(const QModelIndex &index) const
{
    if (index.isValid())
        return reinterpret_cast<Data::MenuItem *>(index.internalPointer());
    return m_menu;
}
