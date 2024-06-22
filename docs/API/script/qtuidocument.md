# QtUiDocument

Provides access to the content of a Ui file (Qt designer file). [More...](#detailed-description)

```qml
import Script 1.0
```

## Properties

| | Name |
|-|-|
|array<[QtUiWidget](../script/qtuiwidget.md)>|**[widgets](#widgets)**|

## Methods

| | Name |
|-|-|
|[QtUiWidget](../script/qtuiwidget.md) |**[findWidget](#findWidget)**(string name)|
||**[preview](#preview)**()|

## Property Documentation

#### <a name="widgets"></a>array<[QtUiWidget](../script/qtuiwidget.md)> **widgets**

List of all widgets in the ui file.

## Method Documentation

#### <a name="findWidget"></a>[QtUiWidget](../script/qtuiwidget.md) **findWidget**(string name)

Returns the widget for the given `name`.

#### <a name="preview"></a>**preview**()

Open a dialog to preview the current ui file.
