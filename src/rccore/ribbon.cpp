/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ribbon.h"
#include "utils/log.h"

#include <pugixml.hpp>

namespace RcCore {
/*!
 * \qmltype RibbonElement
 * \brief An item in the ribbon (button, separator...).
 * \inqmlmodule Knut
 * \ingroup RcDocument
 * \sa Ribbon
 */
/*!
 * \qmlproperty string RibbonElement::type
 * This property holds the type of the element item (button, separator...).
 */
/*!
 * \qmlproperty string RibbonElement::id
 * This property holds the id of the element item.
 */
/*!
 * \qmlproperty string RibbonElement::text
 * This property holds the text of the element item.
 */
/*!
 * \qmlproperty string RibbonElement::keys
 * This property holds the keys of the element item.
 */
/*!
 * \qmlproperty int RibbonElement::smallIndex
 * This property holds the index for the small icon (16x16 generally) used for the item.
 */
/*!
 * \qmlproperty int RibbonElement::largeIndex
 * This property holds the index for the large icon (32x32 generally) used for the item.
 */
/*!
 * \qmlproperty array<RibbonElement> RibbonElement::elements
 * This property holds the children of this item, for creating menus.
 */
/*!
 * \qmlproperty bool RibbonElement::isSeparator
 * This property returns `true` if the element is a separator.
 */

/*!
 * \qmltype RibbonPanel
 * \brief An panel (group of elements) in the ribbon.
 * \inqmlmodule Knut
 * \ingroup RcDocument
 * \sa Ribbon
 */
/*!
 * \qmlproperty string RibbonPanel::text
 * This property holds the text title of the panel.
 */
/*!
 * \qmlproperty string RibbonPanel::keys
 * This property holds the keys of the panel.
 */
/*!
 * \qmlproperty array<RibbonElement> RibbonPanel::elements
 * This property holds the children elements of this panel.
 */

/*!
 * \qmltype RibbonCategory
 * \brief A tab (made of panels) in the ribbon.
 * \inqmlmodule Knut
 * \ingroup RcDocument
 * \sa Ribbon
 */
/*!
 * \qmlproperty string RibbonCategory::text
 * This property holds the text title of the category.
 */
/*!
 * \qmlproperty string RibbonCategory::keys
 * This property holds the keys of the category.
 */
/*!
 * \qmlproperty string RibbonCategory::smallImage
 * This property holds the image resource for small icons.
 */
/*!
 * \qmlproperty string RibbonCategory::largeImage
 * This property holds the image resource for large icons.
 */
/*!
 * \qmlproperty array<RibbonPanel> RibbonCategory::panels
 * This property holds the children panels of this category.
 */

/*!
 * \qmltype RibbonContext
 * \brief A context (tabs with a title) in the ribbon.
 * \inqmlmodule Knut
 * \ingroup RcDocument
 * \sa Ribbon
 */
/*!
 * \qmlproperty string RibbonContext::id
 * This property holds the id of the context.
 */
/*!
 * \qmlproperty string RibbonContext::text
 * This property holds the text of the context.
 */
/*!
 * \qmlproperty array<RibbonCategory> RibbonContext::categories
 * This property holds the children categories of this context.
 */

/*!
 * \qmltype RibbonMenu
 * \brief A menu showing when clicking on the left/top icon in the ribbon.
 * \inqmlmodule Knut
 * \ingroup RcDocument
 * \sa Ribbon
 */
/*!
 * \qmlproperty string RibbonMenu::text
 * This property holds the text label of the menu.
 */
/*!
 * \qmlproperty string RibbonMenu::smallImage
 * This property holds the image resource for small icons.
 */
/*!
 * \qmlproperty string RibbonMenu::largeImage
 * This property holds the image resource for large icons.
 */
/*!
 * \qmlproperty array<RibbonElement> RibbonMenu::elements
 * This property holds the children elements of this menu.
 */
/*!
 * \qmlproperty string RibbonMenu::recentFilesText
 * This property holds the text for the recent files menu.
 */

/*!
 * \qmltype Ribbon
 * \brief The ribbon description (not everything is read yet).
 * \inqmlmodule Knut
 * \ingroup RcDocument
 * \sa RcFile
 *
 * A ribbon is made of multiple items:
 *
 * - a file menu, top/left, which displays a popup menu when clicking (see RibbonMenu)
 * - multiple categories displayed as tabs
 *     - each with multiple panels (displayed as group of actions)
 *     - each panel contains multiple elements (displayed as buttons, separators...)
 * - multiple contexts, a context showing another tab with it's name in the titlebar
 *     - each context contains multiple categories
 */
/*!
 * \qmlproperty RibbonMenu Ribbon::menu
 * This property holds the menu of the ribbon.
 */
/*!
 * \qmlproperty array<RibbonCategory> Ribbon::categories
 * This property holds the children categories of this ribbon.
 */
/*!
 * \qmlproperty array<RibbonContext> Ribbon::contexts
 * This property holds the children contexts of this ribbon.
 */

template <typename T, typename Func>
QList<T> readItems(pugi::xml_node node, Func readFunc)
{
    QList<T> collection;
    for (const auto &childNode : node.children())
        collection.push_back(readFunc(childNode));
    return collection;
}

static RibbonElement readElement(pugi::xml_node node)
{
    if (node.empty())
        return {};

    RibbonElement element;
    element.type = node.child("ELEMENT_NAME").text().as_string();
    element.id = node.select_node("ID/NAME").node().text().as_string();
    element.text = node.child("TEXT").text().as_string();
    element.keys = node.child("KEYS").text().as_string();
    element.smallIndex = node.child("INDEX_SMALL").text().as_int(-1);
    element.largeIndex = node.child("INDEX_LARGE").text().as_int(-1);
    element.elements = readItems<RibbonElement>(node.child("ELEMENTS"), readElement);
    return element;
}

static RibbonMenu readMenu(pugi::xml_node node)
{
    RibbonMenu menu;
    menu.text = node.child("NAME").text().as_string();
    menu.smallImage = node.select_node("IMAGE_SMALL/ID/NAME").node().text().as_string();
    menu.largeImage = node.select_node("IMAGE_LARGE/ID/NAME").node().text().as_string();
    menu.elements = readItems<RibbonElement>(node.child("ELEMENTS"), readElement);
    menu.recentFilesText = node.select_node("RECENT_FILE_LIST/LABEL").node().text().as_string();
    return menu;
}

static RibbonPanel readPanel(pugi::xml_node node)
{
    RibbonPanel panel;
    panel.text = node.child("NAME").text().as_string();
    panel.keys = node.child("KEYS").text().as_string();
    panel.elements = readItems<RibbonElement>(node.child("ELEMENTS"), readElement);
    return panel;
}

static RibbonCategory readCategory(pugi::xml_node node)
{
    RibbonCategory category;
    category.text = node.child("NAME").text().as_string();
    category.keys = node.child("KEYS").text().as_string();
    category.smallImage = node.select_node("IMAGE_SMALL/ID/NAME").node().text().as_string();
    category.largeImage = node.select_node("IMAGE_LARGE/ID/NAME").node().text().as_string();
    category.panels = readItems<RibbonPanel>(node.child("PANELS"), readPanel);
    return category;
}

static RibbonContext readContext(pugi::xml_node node)
{
    RibbonContext context;
    context.id = node.select_node("ID/NAME").node().text().as_string();
    context.text = node.child("TEXT").text().as_string();
    context.categories = readItems<RibbonCategory>(node.child("CATEGORIES"), readCategory);
    return context;
}

bool Ribbon::load()
{
    pugi::xml_document document;
    pugi::xml_parse_result result = document.load_file(fileName.toLatin1().constData());

    if (!result) {
        spdlog::critical("{}({}): {}", fileName, result.offset, result.description());
        return false;
    }

    auto menuNode = document.select_nodes("/AFX_RIBBON/RIBBON_BAR/CATEGORY_MAIN").first().node();
    menu = readMenu(menuNode);

    auto categoriesNode = document.select_nodes("/AFX_RIBBON/RIBBON_BAR/CATEGORIES").first().node();
    categories = readItems<RibbonCategory>(categoriesNode, readCategory);

    auto contextsNode = document.select_nodes("/AFX_RIBBON/RIBBON_BAR/CONTEXTS").first().node();
    contexts = readItems<RibbonContext>(contextsNode, readContext);

    return true;
}

bool operator==(const RibbonElement &lhs, const RibbonElement &rhs)
{
    return lhs.id == rhs.id;
}

} // namespace RcCore
