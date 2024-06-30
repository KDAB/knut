# RibbonElement

An item in the ribbon (button, separator...). [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|array&lt;[RibbonElement](../script/ribbonelement.md)>|**[elements](#elements)**|
|string|**[id](#id)**|
|bool|**[isSeparator](#isSeparator)**|
|string|**[keys](#keys)**|
|int|**[largeIndex](#largeIndex)**|
|int|**[smallIndex](#smallIndex)**|
|string|**[text](#text)**|
|string|**[type](#type)**|

## Property Documentation

#### <a name="elements"></a>array&lt;[RibbonElement](../script/ribbonelement.md)> **elements**

This property holds the children of this item, for creating menus.

#### <a name="id"></a>string **id**

This property holds the id of the element item.

#### <a name="isSeparator"></a>bool **isSeparator**

This property returns `true` if the element is a separator.

#### <a name="keys"></a>string **keys**

This property holds the keys of the element item.

#### <a name="largeIndex"></a>int **largeIndex**

This property holds the index for the large icon (32x32 generally) used for the item.

#### <a name="smallIndex"></a>int **smallIndex**

This property holds the index for the small icon (16x16 generally) used for the item.

#### <a name="text"></a>string **text**

This property holds the text of the element item.

#### <a name="type"></a>string **type**

This property holds the type of the element item (button, separator...).
