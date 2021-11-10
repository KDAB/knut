# Menu

Description of a RC file menu. [More...](#detailed-description)

```qml
import Script.Mfc 1.0
```

<table>
<tr><td>Since:</td><td>Knut 4.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|array<[MenuItem](../script.mfc/menuitem.md)|**[children](#children)**|
|string|**[id](#id)**|

## Methods

| | Name |
|-|-|
|bool |**[contains](#contains)**(string id)|

## Property Documentation

#### <a name="children"></a>array<[MenuItem](../script.mfc/menuitem.md) **children**

This property holds the list of menu items inside the menu.

#### <a name="id"></a>string **id**

This property holds the id of the menu.

## Method Documentation

#### <a name="contains"></a>bool **contains**(string id)

Returns true if the menu contains the given `id`
