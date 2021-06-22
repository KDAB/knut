#include "data.h"

#include <algorithm>

namespace RcCore {

bool operator==(const ToolBarItem &left, const ToolBarItem &right)
{
    return left.id == right.id;
}

template <typename T>
const T *findById(const QVector<T> &collection, const QString &id)
{
    auto it = std::find_if(collection.cbegin(), collection.cend(), [id](const auto &data) {
        return data.id == id;
    });
    if (it == collection.cend())
        return nullptr;
    return it;
}

const Asset *Data::asset(const QString &id) const
{
    return findById(assets, id);
}

const ToolBar *Data::toolBar(const QString &id) const
{
    return findById(toolBars, id);
}

const Data::Dialog *Data::dialog(const QString &id) const
{
    return findById(dialogs, id);
}

const Menu *Data::menu(const QString &id) const
{
    return findById(menus, id);
}

const Data::AcceleratorTable *Data::acceleratorTable(const QString &id) const
{
    return findById(acceleratorTables, id);
}

bool operator==(const Widget &left, const Widget &right)
{
    return left.id == right.id;
}

bool operator==(const MenuItem &left, const MenuItem &right)
{
    return left.id == right.id && left.text == right.text;
}

bool operator==(const Shortcut &left, const Shortcut &right)
{
    return left.event == right.event && left.unknown == right.unknown;
}

} // namespace RcCore
