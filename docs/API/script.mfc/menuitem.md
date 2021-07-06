# MenuItem

Description of a RC file menu item. [More...](#detailed-description)

```qml
import Script.Mfc 1.0
```

<table>
<tr><td>Since:</td><td>Knut 4.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|array<[MenuItem](../script.mfc/menuitem.md)>|**[children](#children)**|
|string|**[id](#id)**|
|bool|**[isAction](#isAction)**|
|bool|**[isSeparator](#isSeparator)**|
|bool|**[isTopLevel](#isTopLevel)**|
|string|**[text](#text)**|

## Property Documentation

#### <a name="children"></a>array<[MenuItem](../script.mfc/menuitem.md)> **children**

This property holds the children of the menu item, in case this one is another level of menu.

#### <a name="id"></a>string **id**

This property holds the id of the menu item.

#### <a name="isAction"></a>bool **isAction**

This property returns `true` if the menu item is an action.

#### <a name="isSeparator"></a>bool **isSeparator**

This property returns `true` if the menu item is a separator (horizontal line in the menu).

#### <a name="isTopLevel"></a>bool **isTopLevel**

This property returns `true` if the menu item is a top level item (direct children of the Menu).

#### <a name="text"></a>string **text**

This property holds the text of the menu item.
