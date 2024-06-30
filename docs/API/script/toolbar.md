# ToolBar

Description of a RC file toolbar. [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|array&lt;string>|**[actionIds](#actionIds)**|
|array&lt;[ToolBarItem](../script/toolbaritem.md)>|**[children](#children)**|
|size|**[iconSize](#iconSize)**|
|string|**[id](#id)**|

## Methods

| | Name |
|-|-|
|bool |**[contains](#contains)**(string id)|

## Property Documentation

#### <a name="actionIds"></a>array&lt;string> **actionIds**

This property holds all action ids used in the toolbar.

#### <a name="children"></a>array&lt;[ToolBarItem](../script/toolbaritem.md)> **children**

This property holds the list of toolbar items inside in the toolbar.

#### <a name="iconSize"></a>size **iconSize**

This property holds the size of the icon associated to the toolbar.

#### <a name="id"></a>string **id**

This property holds the id of the toolbar.

## Method Documentation

#### <a name="contains"></a>bool **contains**(string id)

Returns true if the toolbar contains the given `id`
