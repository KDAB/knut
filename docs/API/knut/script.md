# Script

Script object for writing non visual scripts. [More...](#detailed-description)

```qml
import Knut
```

## Methods

| | Name |
|-|-|
||**[compare](#compare)**(var actual, var expected, string message = {})|
||**[verify](#verify)**(bool value, string message = {})|

## Detailed Description

The `Script` is the base class for all creatable items in QML. It is needed as a `QtObject`
can't have any children in QML. It can be used as the basis for non visual QML scripts:

```qml
import Knut

Script {
// ...
}
```

You can also integrate unit-tests as part of the script dialog. This is done by adding methods prefixed with `test_`.
Those methods will be executed when the script is run in test mode (using `--test` command line option). If a test is
failing, the script will show a critical log message, and the script will return a non-zero exit code.

```qml
import Knut

Script {
    function test_foo() {
        compare(1, 2, "test should failed")
    }
    function test_bar() {
        verify(1 == 1, "test should pass")
    }
}
```

## Method Documentation

#### <a name="compare"></a>**compare**(var actual, var expected, string message = {})

Compare `actual` with `expected` and log an error `message` if they are not equal.

This method will increment the number of failed test internally, and when the script is finished, the test runner
will return the number of failed tests.

This is usually used in tests method like that:

```qml
Script {
    function test_foo() {
        compare(1, 2, "1 should be equal to 2"); // This will log an error
    }
}
```

#### <a name="verify"></a>**verify**(bool value, string message = {})

Compare `actual` with `expected` and log an error `message` if they are not equal.

This method will increment the number of failed test internally, and when the script is finished, the test runner
will return the number of failed tests.

This is usually used in tests method like that:

```qml
Script {
    function test_foo() {
        let answerToEverything = 42;
        verify(answerToEverything == 42, "What else?");
    }
}
```
