# Ribbon

The ribbon description (not everything is read yet). [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|array&lt;[RibbonCategory](../script/ribboncategory.md)>|**[categories](#categories)**|
|array&lt;[RibbonContext](../script/ribboncontext.md)>|**[contexts](#contexts)**|
|[RibbonMenu](../script/ribbonmenu.md)|**[menu](#menu)**|

## Detailed Description

A ribbon is made of multiple items:

- a file menu, top/left, which displays a popup menu when clicking (see RibbonMenu)
- multiple categories displayed as tabs
    - each with multiple panels (displayed as group of actions)
    - each panel contains multiple elements (displayed as buttons, separators...)
- multiple contexts, a context showing another tab with it's name in the titlebar
    - each context contains multiple categories

## Property Documentation

#### <a name="categories"></a>array&lt;[RibbonCategory](../script/ribboncategory.md)> **categories**

This property holds the children categories of this ribbon.

#### <a name="contexts"></a>array&lt;[RibbonContext](../script/ribboncontext.md)> **contexts**

This property holds the children contexts of this ribbon.

#### <a name="menu"></a>[RibbonMenu](../script/ribbonmenu.md) **menu**

This property holds the menu of the ribbon.
