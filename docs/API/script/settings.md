# Settings

Singleton for accessing and editing persistent settings. [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|bool|**[isTesting](#isTesting)**|

## Methods

| | Name |
|-|-|
|bool |**[hasValue](#hasValue)**(string path)|
|variant |**[setValue](#setValue)**(string path, variant value)|
|variant |**[value](#value)**(string path, variant defaultValue = null)|

## Detailed Description

The settings are stored in a json file, and could be:

- bool
- int
- double
- string
- array&lt;string>

## Property Documentation

#### <a name="isTesting"></a>bool **isTesting**

Returns true if Knut is currently in a test, and false otherwise

## Method Documentation

#### <a name="hasValue"></a>bool **hasValue**(string path)

Returns true if the project settings has a settings `path`.

#### <a name="setValue"></a>variant **setValue**(string path, variant value)

Adds a new value `value` to the project settings at the given `path`. Returns `true` if the operation succeeded.

#### <a name="value"></a>variant **value**(string path, variant defaultValue = null)

Returns the value of the settings `path`, or `defaultValue` if the settings does not exist.
