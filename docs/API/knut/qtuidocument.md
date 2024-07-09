# QtUiDocument

Provides access to the content of a Ui file (Qt designer file). [More...](#detailed-description)

```qml
import Knut
```

## Properties

| | Name |
|-|-|
|array&lt;[QtUiWidget](../knut/qtuiwidget.md)>|**[widgets](#widgets)**|

## Methods

| | Name |
|-|-|
|[QtUiWidget](../knut/qtuiwidget.md) |**[addCustomWidget](#addCustomWidget)**(string className, string baseClassName, string header, bool isContainer)|
|[QtUiWidget](../knut/qtuiwidget.md) |**[addWidget](#addWidget)**(string className, string name, [QtUiWidget](../knut/qtuiwidget.md) parent)|
|[QtUiWidget](../knut/qtuiwidget.md) |**[findWidget](#findWidget)**(string name)|
||**[preview](#preview)**()|

## Property Documentation

#### <a name="widgets"></a>array&lt;[QtUiWidget](../knut/qtuiwidget.md)> **widgets**

List of all widgets in the ui file.

## Method Documentation

#### <a name="addCustomWidget"></a>[QtUiWidget](../knut/qtuiwidget.md) **addCustomWidget**(string className, string baseClassName, string header, bool isContainer)

Adds a new custom widget to the ui file.

The `className` is the name of the custom widget, `baseClassName` is the name of the base class, `header` is the
include. If `isContainer` is true, the widget is a container.

The `header` should be in the form of `<foo.h>` or `"foo.h"`.

#### <a name="addWidget"></a>[QtUiWidget](../knut/qtuiwidget.md) **addWidget**(string className, string name, [QtUiWidget](../knut/qtuiwidget.md) parent)

Creates a new widget with the given `className` and `name`, and adds it to the `parent` widget.

If `parent` is null, it creates the top level widget (there can only be one).

#### <a name="findWidget"></a>[QtUiWidget](../knut/qtuiwidget.md) **findWidget**(string name)

Returns the widget for the given `name`.

#### <a name="preview"></a>**preview**()

Opens a dialog to preview the current ui file.
