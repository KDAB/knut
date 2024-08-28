# ScriptDialog

QML Item for writing visual scripts. [More...](#detailed-description)

```qml
import Knut
```

## Properties

| | Name |
|-|-|
|var|**[data](#data)**|
|bool|**[interactive](#interactive)**|
|int|**[stepCount](#stepCount)**|

## Methods

| | Name |
|-|-|
||**[compare](#compare)**(var actual, var expected, string message = {})|
||**[firstStep](#firstStep)**(string firstStep)|
||**[nextStep](#nextStep)**(string title)|
||**[runSteps](#runSteps)**(function generator)|
||**[setDisplayLogs](#setDisplayLogs)**(const QString &level)|
||**[setStepCount](#setStepCount)**(int stepCount)|
||**[verify](#verify)**(bool value, string message = {})|

## Signals

| | Name |
|-|-|
||**[onAccepted](#onAccepted)**()|
||**[onClicked](#onClicked)**(string name)|
||**[onRejected](#onRejected)**()|

## Detailed Description

The `ScriptDialog` allows creating a script dialog based on a ui file. It requires creating a ui file with the same
name as the qml script.

Inside the dialog, all widget's main property is mapped to a property inside the data property, using the same
name as the `objectName`. For example, the text of a `QLineEdit` with `objectName` set to `lineEdit` will be mapped
to `data.lineEdit`.

Buttons (`QPushButton` or `QToolButton`) `clicked` signal is available through the `onClikced` signal handler, with
the button `objectName` as parameter.

`QDialogButtonBox` `accepted` or `rejected` signals are
automatically connected and available through the `onAccepted` and `onRejected` signal handlers.

```qml
import Knut

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
    onAccepted: {
        console.log("Accepted")
    }
    onRejected: {
        console.log("Rejected")
    }
}
```

You can also integrate unit-tests as part of the script dialog. This is done by adding methods prefixed with `test_`.
Those methods will be executed when the script is run in test mode (using `--test` command line option). If a test is
failing, the script will show a critical log message, and the script will return a non-zero exit code.

```qml
import Knut

ScriptDialog {
    function test_foo() {
        compare(1, 2, "test should failed")
    }
    function test_bar() {
        verify(1 == 1, "test should pass")
    }
}
```

## Property Documentation

#### <a name="data"></a>var **data**

This read-only property contains all properties mapping the widgets.

Use `data.objectName` to access the main property of a widget (the text for a `QLineEdit` for example).

#### <a name="interactive"></a>bool **interactive**

If set to false, `runSteps` will not ask for user input, the entire script will be run at once.
This is especially useful for testing.

#### <a name="stepCount"></a>int **stepCount**

Number of steps to display in the progress bar.

## Method Documentation

#### <a name="compare"></a>**compare**(var actual, var expected, string message = {})

Compare `actual` with `expected` and log an error `message` if they are not equal.

This method will increment the number of failed test internally, and when the script is finished, the test runner
will return the number of failed tests.

This is usually used in tests method like that:

```qml
ScriptDialog {
    function test_foo() {
        compare(1, 2, "1 should be equal to 2"); // This will log an error
    }
}
```

#### <a name="firstStep"></a>**firstStep**(string firstStep)

Starts a progress bar with the given `firstStep` title.

The number of following `nextStep` calls (or yield calls if using runSteps) should be one less than the number of
steps set here.

#### <a name="nextStep"></a>**nextStep**(string title)

Changes the progression to a new progress step.

This will update the progress bar and the title of the progress dialog.
Make sure that the number of steps is set correctly before calling this method.

#### <a name="runSteps"></a>**runSteps**(function generator)

Runs a script in multiple (interactive) steps.

The argument to this function must be a JavaScript generator object
([See this documentation on JS
Generators](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Generator)).

The generator should yield a string with the next step title, whenever the user should be able to pause the script
and inspect the changes. This will behave the same as calling `nextStep`, but pauses the script, until the user
continues or aborts the script.

You can also mix and match between `yield` and `nextStep` calls.

For the best experience, we recommend to use `setStepCount`, `firstStep` and `yield` to indicate the remaining
progress.

Example:
```javascript
function *conversionSteps() {
   setStepCount(2)            // <--- Initialize the number of steps
   firstStep("Adding member") // <--- Start the first step
   document.addMember("test", "int", CppDocument.Public)

   yield "Inserting include"  // <--- The user can check that the member was inserted correctly
   document.insertInclude("<iostream>")
}

function convert() {
   runSteps(conversionSteps())
}
```

#### <a name="setDisplayLogs"></a>**setDisplayLogs**(const QString &level)

This method enables real-time logging display in the progress dialog by configuring a QPlainTextEdit widget as a log
output panel. It filters the logs to show only those that are of the specified log level or higher.

#### <a name="setStepCount"></a>**setStepCount**(int stepCount)

Sets the number of steps to show in the progress bar.

By default the value is 0, meaning there are no steps set. This will show an indeterminate progress bar. You can use
the `stepCount` property to set the number of steps too.

#### <a name="verify"></a>**verify**(bool value, string message = {})

Compare `actual` with `expected` and log an error `message` if they are not equal.

This method will increment the number of failed test internally, and when the script is finished, the test runner
will return the number of failed tests.

This is usually used in tests method like that:

```qml
ScriptDialog {
    function test_foo() {
        let answerToEverything = 42;
        verify(answerToEverything == 42, "What else?");
    }
}
```

## Signal Documentation

#### <a name="onAccepted"></a>**onAccepted**()

This handler is called when a button with an accept role from a `QDialogButtonBox` is pressed (usually the OK
button).

#### <a name="onClicked"></a>**onClicked**(string name)

This handler is called when a button is cliked, the `name` is the name of the button.

#### <a name="onRejected"></a>**onRejected**()

This handler is called when a button with a reject role from a `QDialogButtonBox` is pressed (usually the Cancel
button).
