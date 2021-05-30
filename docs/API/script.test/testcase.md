# TestCase

Provides a way to create unit tests as script. [More...](#detailed-description)

```qml
import Script.Test 1.0
```

## Properties

- string **[name](#name)**

## Methods

- **[compare](#compare)**(actual, expected, msg)
- **[verify](#verify)**(value, msg)

## Detailed Description

Run unit tests as a script, and display the result in the Script Output pane. A test \b must
have a \c name.

## Property Documentation

#### <a name="name"></a>string **name**

This property defines the name of the unit test.

## Method Documentation

#### <a name="compare"></a>**compare**(actual, expected, msg)

Compares \a actual vs \a expected, and display the \a msg if it's not the same.

#### <a name="verify"></a>**verify**(value, msg)

Verifies that \a value is \c true, and display the \a msg if it's not.
