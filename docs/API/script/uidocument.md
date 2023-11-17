# UiDocument

Provides access to the content of a Ui file (Qt designer file). [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|array<[UiWidget](../script/uiwidget.md)>|**[widgets](#widgets)**|

## Methods

| | Name |
|-|-|
|[UiWidget](../script/uiwidget.md) |**[findWidget](#findWidget)**(string name)|
||**[preview](#preview)**()|

## Property Documentation

#### <a name="widgets"></a>array<[UiWidget](../script/uiwidget.md)> **widgets**

List of all widgets in the ui file.

## Method Documentation

#### <a name="findWidget"></a>[UiWidget](../script/uiwidget.md) **findWidget**(string name)

Returns the widget for the given `name`.

#### <a name="preview"></a>**preview**()

Open a dialog to preview the current ui file.
