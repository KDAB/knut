#include "data.h"

#include <algorithm>

namespace RcCore {
/*!
 * \qmltype Asset
 * \brief Description of a RC file asset.
 * \inqmlmodule Script
 * \since 4.0
 * \sa RcFile
 */
/*!
 * \qmlproperty string Asset::id
 * This property holds the id of the asset.
 */
/*!
 * \qmlproperty string Asset::fileName
 * This property holds the fileName of the asset. The fileName is absolute if the asset's file
 * exists.
 */
/*!
 * \qmlproperty bool Asset::exist
 * This property returns `true` if the asset's file exists.
 */

/*!
 * \qmltype ToolBarItem
 * \brief Description of a RC file toolbar item.
 * \inqmlmodule Script
 * \since 4.0
 * \sa RcFile
 */
/*!
 * \qmlproperty string ToolBarItem::id
 * This property holds the id of the toolbar item.
 */
/*!
 * \qmlproperty bool ToolBarItem::isSeparator
 * This property returns `true` if the toolbar item is a separatior (vertical line visually).
 */

/*!
 * \qmltype ToolBar
 * \brief Description of a RC file toolbar.
 * \inqmlmodule Script
 * \since 4.0
 * \sa RcFile
 */
/*!
 * \qmlproperty string ToolBar::id
 * This property holds the id of the toolbar.
 */
/*!
 * \qmlproperty array<ToolBarItem> ToolBarItem::children
 * This property holds the list of toolbar items inside in the toolbar.
 */
/*!
 * \qmlproperty size ToolBarItem::iconSize
 * This property holds the size of the icon associated to the toolbar.
 */

/*!
 * \qmltype Widget
 * \brief Description of a RC file widget.
 * \inqmlmodule Script
 * \since 4.0
 * \sa RcFile
 */
/*!
 * \qmlproperty string Widget::id
 * This property holds the id of the widget.
 */
/*!
 * \qmlproperty string Widget::className
 * This property holds the Qt class name equivalent of the widget.
 */
/*!
 * \qmlproperty rect Widget::geometry
 * This property holds the geometry of the widget.
 *
 * The geometry is in dialog system, different from pixel size.
 * Usually, you need to apply a scale of 1.5 x 1.65 to get the pixel size, but it could change
 * depending on the project.
 */
/*!
 * \qmlproperty object Widget::properties
 * This property holds the list of Qt properties for the widget.
 */
/*!
 * \qmlproperty array<Widget> Widget::children
 * This property holds the list of children of the current widget.
 */

/*!
 * \qmltype MenuItem
 * \brief Description of a RC file menu item.
 * \inqmlmodule Script
 * \since 4.0
 * \sa RcFile
 */
/*!
 * \qmlproperty string MenuItem::id
 * This property holds the id of the menu item.
 */
/*!
 * \qmlproperty string MenuItem::text
 * This property holds the text of the menu item.
 */
/*!
 * \qmlproperty array<MenuItem> MenuItem::children
 * This property holds the children of the menu item, in case this one is another level of menu.
 */
/*!
 * \qmlproperty bool MenuItem::isSeparator
 * This property returns `true` if the menu item is a separator (horizontal line in the menu).
 */
/*!
 * \qmlproperty bool MenuItem::isAction
 * This property returns `true` if the menu item is an action.
 */
/*!
 * \qmlproperty bool MenuItem::isTopLevel
 * This property returns `true` if the menu item is a top level item (direct children of the Menu).
 */

/*!
 * \qmltype Menu
 * \brief Description of a RC file menu.
 * \inqmlmodule Script
 * \since 4.0
 * \sa RcFile
 */
/*!
 * \qmlproperty string Menu::id
 * This property holds the id of the menu.
 */
/*!
 * \qmlproperty array<MenuItem Menu::children
 * This property holds the list of menu items inside the menu.
 */

/*!
 * \qmltype Shortcut
 * \brief Description of a RC file shortcut.
 * \inqmlmodule Script
 * \since 4.0
 * \sa RcFile
 */
/*!
 * \qmlproperty string Shortcut::event
 * This property holds the key combination for the event.
 */
/*!
 * \qmlproperty bool Shortcut::unknown
 * This property returns `true` if the key combination can't be parsed.
 */

/*!
 * \qmltype Action
 * \brief Description of a RC file action.
 * \inqmlmodule Script
 * \since 4.0
 * \sa RcFile
 */
/*!
 * \qmlproperty string Action::id
 * This property holds the id of the action.
 */
/*!
 * \qmlproperty array<Shortcut> Action::shortcuts
 * This property holds all the shortcuts associated with the action.
 */
/*!
 * \qmlproperty string Action::toolTip
 * This property holds the toolTip of the action.
 */
/*!
 * \qmlproperty string Action::statusTip
 * This property holds the statusTip of the action.
 */
/*!
 * \qmlproperty string Action::iconPath
 * This property holds the fileName for the icon of the action.
 */
/*!
 * \qmlproperty bool Action::checked
 * This property is `true` if the action is checkabled and checked by default.
 */

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
