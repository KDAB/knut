# TestCase

Provides a way to create unit tests as script. [More...](#detailed-description)

```qml
import Knut.Test
```

## Properties

| | Name |
|-|-|
|string|**[name](#name)**|

## Methods

| | Name |
|-|-|
||**[compare](#compare)**(actual, expected, msg)|
||**[verify](#verify)**(value, msg)|

## Detailed Description

Run unit tests as a script, and returns the number of failed tests.

## Property Documentation

#### <a name="name"></a>string **name**

This property defines the name of the unit test. **This is a mandatory property**.

## Method Documentation

#### <a name="compare"></a>**compare**(actual, expected, msg)

Compares `actual` vs `expected`, and display the `msg` if it's not the same.

#### <a name="verify"></a>**verify**(value, msg)

Verifies that `value` is true, and display the `msg` if it's not.
