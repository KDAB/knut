# ScriptDialog

QML Item for writing visual scripts. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|QQmlPropertyMap|**[data](#data)**|
|bool|**[showProgress](#showProgress)**|

## Signals

| | Name |
|-|-|
||**[accepted](#accepted)**()|
||**[clicked](#clicked)**(string name)|
||**[rejected](#rejected)**()|

## Detailed Description

The `ScriptDialog` allows creating a scipt dialog based on a ui file. It requires creating a ui file with the same
name as the qml script.

Widget's main properties are mapped to a property inside the data property, using the same name as the `objectName`.
Buttons (`QPushButton` or `QToolButton`) `clicked` signal is mapped to the `clicked` signal of this class, with the
button `objectName` as parameter. `QDialogButtonBox` `accepted` or `rejected` signals are automatically connected.

```qml
import Script 1.0

ScriptDialog {
    property string text1: data.lineEdit
    property string text2: data.comboBox
    property int number: data.spinBox
    property int number2: data.doubleSpinBox
    property bool check: data.checkBox
    property bool check2: data.radioButton
    onClicked: (name) => {
        if (name == "pushButton" || name == "toolButton")
            console.log(name)
    }
}
```

## Property Documentation

#### <a name="data"></a>QQmlPropertyMap **data**

This read-only property contains all properties mapping the widgets.

#### <a name="showProgress"></a>bool **showProgress**

!!! note ""
    Since: Knut 1.1

If set to true, a progress dialog will be shown when the dialog is accepted.
This is useful for long-running scripts.

## Signal Documentation

#### <a name="accepted"></a>**accepted**()

This handler is called when a button with an accept role from a `QDialogButtonBox` is pressed (usually the OK
button).

#### <a name="clicked"></a>**clicked**(string name)

This handler is called when a button is cliked, the `name` is the name of the button.

#### <a name="rejected"></a>**rejected**()

This handler is called when a button with a reject role from a `QDialogButtonBox` is pressed (usually the Cancel
button).
