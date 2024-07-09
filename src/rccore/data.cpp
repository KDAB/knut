/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "data.h"

#include <algorithm>

namespace RcCore {
/*!
 * \qmltype Asset
 * \brief Description of a RC file asset.
 * \inqmlmodule Knut
 * \ingroup RcDocument
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
 * \inqmlmodule Knut
 * \ingroup RcDocument
 * \sa RcFile
 */
/*!
 * \qmlproperty string ToolBarItem::id
 * This property holds the id of the toolbar item.
 */
/*!
 * \qmlproperty bool ToolBarItem::isSeparator
 * This property returns `true` if the toolbar item is a separator (vertical line visually).
 */

/*!
 * \qmltype ToolBar
 * \brief Description of a RC file toolbar.
 * \inqmlmodule Knut
 * \ingroup RcDocument
 * \sa RcFile
 */
/*!
 * \qmlproperty string ToolBar::id
 * This property holds the id of the toolbar.
 */
/*!
 * \qmlproperty array<ToolBarItem> ToolBar::children
 * This property holds the list of toolbar items inside in the toolbar.
 */
/*!
 * \qmlproperty size ToolBar::iconSize
 * This property holds the size of the icon associated to the toolbar.
 */
/*!
 * \qmlproperty array<string> ToolBar::actionIds
 * This property holds all action ids used in the toolbar.
 */

/*!
 * \qmltype Widget
 * \brief Description of a RC file widget.
 * \inqmlmodule Knut
 * \ingroup RcDocument
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
 * \inqmlmodule Knut
 * \ingroup RcDocument
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
 * \inqmlmodule Knut
 * \ingroup RcDocument
 * \sa RcFile
 */
/*!
 * \qmlproperty string Menu::id
 * This property holds the id of the menu.
 */
/*!
 * \qmlproperty array<MenuItem> Menu::children
 * This property holds the list of menu items inside the menu.
 */
/*!
 * \qmlproperty array<string> Menu::actionIds
 * This property holds all action ids used in the menu.
 */

/*!
 * \qmltype Shortcut
 * \brief Description of a RC file shortcut.
 * \inqmlmodule Knut
 * \ingroup RcDocument
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
 * \inqmlmodule Knut
 * \ingroup RcDocument
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

/*!
 * \qmltype String
 * \brief Description of a RC file string.
 * \inqmlmodule Knut
 * \ingroup RcDocument
 * \sa RcFile
 */
/*!
 * \qmlproperty string String::id
 * This property holds the id of the string.
 */
/*!
 * \qmlproperty string String::text
 * This property holds the text of the string.
 */

bool operator==(const ToolBarItem &left, const ToolBarItem &right)
{
    return left.id == right.id;
}

template <typename T>
const T *findById(const QList<T> &collection, const QString &id)
{
    auto it = std::find_if(collection.cbegin(), collection.cend(), [id](const auto &data) {
        return data.id == id;
    });
    if (it == collection.cend())
        return nullptr;
    return &*it;
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

const Data::DialogData *Data::dialogData(const QString &id) const
{
    return findById(dialogDataList, id);
}

const Menu *Data::menu(const QString &id) const
{
    return findById(menus, id);
}

const Data::AcceleratorTable *Data::acceleratorTable(const QString &id) const
{
    return findById(acceleratorTables, id);
}

const Ribbon *Data::ribbon(const QString &id) const
{
    return findById(ribbons, id);
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

bool operator==(const String &left, const String &right)
{
    return left.id == right.id && left.text == right.text;
}

static bool containActionId(const QString &id, const MenuItem &menuItem)
{
    if (menuItem.id == id)
        return true;

    return std::ranges::any_of(menuItem.children, [&id](const MenuItem &child) {
        return containActionId(id, child);
    });
}

/*!
 * \qmlmethod bool Menu::contains(string id)
 * Returns true if the menu contains the given `id`
 */
bool Menu::contains(const QString &id) const
{
    return std::ranges::any_of(children, [&id](const MenuItem &child) {
        return containActionId(id, child);
    });
}

static void fillActionIds(QStringList &result, const MenuItem &menuItem)
{
    if (menuItem.isAction())
        result.push_back(menuItem.id);
    for (const auto &child : menuItem.children)
        fillActionIds(result, child);
}

QStringList Menu::actionIds() const
{
    QStringList result;
    for (const auto &child : children)
        fillActionIds(result, child);
    return result;
}

/*!
 * \qmlmethod bool ToolBar::contains(string id)
 * Returns true if the toolbar contains the given `id`
 */
bool ToolBar::contains(const QString &id) const
{
    auto isSame = [id](const ToolBarItem &item) {
        return item.id == id;
    };
    return std::ranges::any_of(children, isSame);
}

QStringList ToolBar::actionIds() const
{
    QStringList result;
    for (const auto &child : children) {
        if (!child.isSeparator())
            result.push_back(child.id);
    }
    return result;
}

} // namespace RcCore
