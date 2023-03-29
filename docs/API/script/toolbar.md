# ToolBar

Description of a RC file toolbar. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|array<string>|**[actionIds](#actionIds)**|
|array<[ToolBarItem](../script/toolbaritem.md)>|**[children](#children)**|
|size|**[iconSize](#iconSize)**|
|string|**[id](#id)**|

## Methods

| | Name |
|-|-|
|bool |**[contains](#contains)**(string id)|

## Property Documentation

#### <a name="actionIds"></a>array<string> **actionIds**

!!! note ""
    Since: Knut 1.1

This property holds all action ids used in the toolbar.

#### <a name="children"></a>array<[ToolBarItem](../script/toolbaritem.md)> **children**

This property holds the list of toolbar items inside in the toolbar.

#### <a name="iconSize"></a>size **iconSize**

This property holds the size of the icon associated to the toolbar.

#### <a name="id"></a>string **id**

This property holds the id of the toolbar.

## Method Documentation

#### <a name="contains"></a>bool **contains**(string id)

!!! note ""
    Since: Knut 1.1

Returns true if the toolbar contains the given `id`
