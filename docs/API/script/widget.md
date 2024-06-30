# Widget

Description of a RC file widget. [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|array&lt;[Widget](../script/widget.md)>|**[children](#children)**|
|string|**[className](#className)**|
|rect|**[geometry](#geometry)**|
|string|**[id](#id)**|
|object|**[properties](#properties)**|

## Property Documentation

#### <a name="children"></a>array&lt;[Widget](../script/widget.md)> **children**

This property holds the list of children of the current widget.

#### <a name="className"></a>string **className**

This property holds the Qt class name equivalent of the widget.

#### <a name="geometry"></a>rect **geometry**

This property holds the geometry of the widget.

The geometry is in dialog system, different from pixel size.
Usually, you need to apply a scale of 1.5 x 1.65 to get the pixel size, but it could change
depending on the project.

#### <a name="id"></a>string **id**

This property holds the id of the widget.

#### <a name="properties"></a>object **properties**

This property holds the list of Qt properties for the widget.
