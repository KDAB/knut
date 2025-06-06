# Ribbon

The ribbon description (not everything is read yet). [More...](#detailed-description)

```qml
import Knut
```

## Properties

| | Name |
|-|-|
|array&lt;[RibbonCategory](../knut/ribboncategory.md)>|**[categories](#categories)**|
|array&lt;[RibbonContext](../knut/ribboncontext.md)>|**[contexts](#contexts)**|
|string|**[id](#id)**|
|[RibbonMenu](../knut/ribbonmenu.md)|**[menu](#menu)**|

## Methods

| | Name |
|-|-|
|[RibbonElement](../knut/ribbonelement.md) |**[elementFromId](#elementFromId)**(string id)|

## Detailed Description

A ribbon is made of multiple items:

- a file menu, top/left, which displays a popup menu when clicking (see RibbonMenu)
- multiple categories displayed as tabs
    - each with multiple panels (displayed as group of actions)
    - each panel contains multiple elements (displayed as buttons, separators...)
- multiple contexts, a context showing another tab with it's name in the titlebar
    - each context contains multiple categories

## Property Documentation

#### <a name="categories"></a>array&lt;[RibbonCategory](../knut/ribboncategory.md)> **categories**

This property holds the children categories of this ribbon.

#### <a name="contexts"></a>array&lt;[RibbonContext](../knut/ribboncontext.md)> **contexts**

This property holds the children contexts of this ribbon.

#### <a name="id"></a>string **id**

This property holds the id of the ribbon.

#### <a name="menu"></a>[RibbonMenu](../knut/ribbonmenu.md) **menu**

This property holds the menu of the ribbon.

## Method Documentation

#### <a name="elementFromId"></a>[RibbonElement](../knut/ribbonelement.md) **elementFromId**(string id)

This method returns RibbonElement from identifier `id`.
