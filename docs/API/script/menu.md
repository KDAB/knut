# Menu

Description of a RC file menu. [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|array&lt;string>|**[actionIds](#actionIds)**|
|array&lt;[MenuItem](../script/menuitem.md)>|**[children](#children)**|
|string|**[id](#id)**|

## Methods

| | Name |
|-|-|
|bool |**[contains](#contains)**(string id)|

## Property Documentation

#### <a name="actionIds"></a>array&lt;string> **actionIds**

This property holds all action ids used in the menu.

#### <a name="children"></a>array&lt;[MenuItem](../script/menuitem.md)> **children**

This property holds the list of menu items inside the menu.

#### <a name="id"></a>string **id**

This property holds the id of the menu.

## Method Documentation

#### <a name="contains"></a>bool **contains**(string id)

Returns true if the menu contains the given `id`
