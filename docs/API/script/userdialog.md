# UserDialog

Singleton with methods to display common dialog to the user. [More...](#detailed-description)

```qml
import Script
```

## Methods

| | Name |
|-|-|
||**[critical](#critical)**(string title, string text)|
|double |**[getDouble](#getDouble)**(string title, string label, real value = 0.0, int decimals = 1, real step = 1.0, real min = -2147483647.0, real max = 2147483647.0)|
|string |**[getExistingDirectory](#getExistingDirectory)**(string caption, string dir = "")|
|int |**[getInt](#getInt)**(string title, string label, int value = 0, int step = 1, int min = -2147483647, int max = 2147483647)|
|string |**[getItem](#getItem)**(string title, string label, array&lt;string> items, int current = 0, bool editable = false)|
|string |**[getOpenFileName](#getOpenFileName)**(string caption, string dir = "", string filters = "")|
|string |**[getSaveFileName](#getSaveFileName)**(string caption, string dir = "", string filters = "")|
|string |**[getText](#getText)**(string title, string label, string text = "")|
||**[information](#information)**(string title, string text)|
||**[warning](#warning)**(string title, string text)|

## Detailed Description

The `UserDialog` singleton provides methods to display common dialog that could be used in
scripts. If the user cancel the dialog, it will return a null value you can test directly:

```js
let s = UserDialog.getText("Get text", "Value")
if (s !== null)
    Message.log("Text: " + s)
else
    Message.log("Cancelled")
```

## Method Documentation

#### <a name="critical"></a>**critical**(string title, string text)

Show a critical dialog.

#### <a name="getDouble"></a>double **getDouble**(string title, string label, real value = 0.0, int decimals = 1, real step = 1.0, real min = -2147483647.0, real max = 2147483647.0)

Returns a double entered by the user. Returns `null` if the user cancel.

The dialog's caption is set to `title`, the `label` is displayed and the initial value is
`value`. The dialog will show `decimals` number of decimals.
`min` and `max` are the minimum and maximum values the user may choose.
`step` is the amount by which the values change when incrementing or decrementing the value.

#### <a name="getExistingDirectory"></a>string **getExistingDirectory**(string caption, string dir = "")

Returns an existing directory selected by the user. Returns `null` if the user cancel.

The dialog's caption is set to `caption`, the initial directory set to `dir`.

#### <a name="getInt"></a>int **getInt**(string title, string label, int value = 0, int step = 1, int min = -2147483647, int max = 2147483647)

Returns an integer entered by the user. Returns `null` if the user cancel.

The dialog's caption is set to `title`, the `label` is displayed and the initial value is
`value`. `min` and `max` are the minimum and maximum values the user may choose.
`step` is the amount by which the values change when incrementing or decrementing the value.

#### <a name="getItem"></a>string **getItem**(string title, string label, array&lt;string> items, int current = 0, bool editable = false)

Returns a string selected among a pre-defined list. Returns `null` if the user cancel.

The dialog's caption is set to `title`, the `label` is displayed and `items` contain the
initial list of values. The value `current` is selected at the start, and if `editable` is
true, the user will be able to enter his own value.

#### <a name="getOpenFileName"></a>string **getOpenFileName**(string caption, string dir = "", string filters = "")

Returns an existing file selected by the user. Returns `null` if the user cancel.

The dialog's caption is set to `caption`, the initial directory set to `dir` and only files
that matches the given `filters` are shown.

#### <a name="getSaveFileName"></a>string **getSaveFileName**(string caption, string dir = "", string filters = "")

Returns a file name selected by the user. Returns `null` if the user cancel.

The dialog's caption is set to `caption`, the initial directory set to `dir` and only files
that matches the given `filters` are shown.

#### <a name="getText"></a>string **getText**(string title, string label, string text = "")

Returns a string entered by the user. Returns `null` if the user cancel.

The dialog's caption is set to `title`, the `label` is displayed and the initial value is
`text`.

#### <a name="information"></a>**information**(string title, string text)

Show an information dialog.

#### <a name="warning"></a>**warning**(string title, string text)

Show a warning dialog.
