# ScriptDialog

QML Item for writing visual scripts. [More...](#detailed-description)

```qml
import Script 1.0
```

## Properties

| | Name |
|-|-|
|QQmlPropertyMap|**[data](#data)**|
|bool|**[interactive](#interactive)**|
|bool|**[showProgress](#showProgress)**|

## Methods

| | Name |
|-|-|
||**[nextStep](#nextStep)**(string title)|
||**[runSteps](#runSteps)**(function generator)|
||**[startProgress](#startProgress)**(string firstStep, int numSteps)|

## Signals

| | Name |
|-|-|
||**[accepted](#accepted)**()|
||**[clicked](#clicked)**(string name)|
||**[rejected](#rejected)**()|

## Detailed Description

The `ScriptDialog` allows creating a script dialog based on a ui file. It requires creating a ui file with the same
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

#### <a name="interactive"></a>bool **interactive**

If set to false, runSteps will not ask for user input, the entire script will be run at once.
This is especially useful for testing.

#### <a name="showProgress"></a>bool **showProgress**

If set to true, a progress dialog will be shown when the dialog is accepted.
This is useful for long-running scripts.

## Method Documentation

#### <a name="nextStep"></a>**nextStep**(string title)

Indicate a new progress step.

This will update the progress bar and the title of the progress dialog.
Make sure that the number of steps is set correctly before calling this method.

#### <a name="runSteps"></a>**runSteps**(function generator)

Run a script in multiple (interactive) steps.
The argument to this function must be a JavaScript generator object
([See this documentation on JS
Generators](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Generator)).

The generator should yield a string with the next step title,
whenever the user should be able to pause the script and inspect the changes.
This will behave the same as calling `nextStep`, but pauses the script, until the user continues or aborts the
script.
You can also mix and match between `yield` and `nextStep` calls.

For the best experience, we recommend to use `startProgress` and `nextStep` to indicate the remaining progress.

Example:
```javascript
function *conversionSteps() {
   startProgress("Adding member", 2)
   document.addMember("test", "int", CppDocument.Public)

   yield "Inserting include" // <--- The user can check that the member was inserted correctly
   document.insertInclude("<iostream>")
}

function convert() {
   runSteps(conversionSteps())
}
```

#### <a name="startProgress"></a>**startProgress**(string firstStep, int numSteps)

Start a progress bar with the given `firstStep` title and number of steps.

The number of following `nextStep` calls (or yield calls if using runSteps) should be one less than the number of
steps set here.

## Signal Documentation

#### <a name="accepted"></a>**accepted**()

This handler is called when a button with an accept role from a `QDialogButtonBox` is pressed (usually the OK
button).

#### <a name="clicked"></a>**clicked**(string name)

This handler is called when a button is cliked, the `name` is the name of the button.

#### <a name="rejected"></a>**rejected**()

This handler is called when a button with a reject role from a `QDialogButtonBox` is pressed (usually the Cancel
button).
