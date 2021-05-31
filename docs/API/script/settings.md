# Settings

Singleton for accessing and editing persistent settings. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>knut 4.0</td></tr>
</table>

## Methods

| | Name |
|-|-|
|variant |**[value](#value)**(string path, variant defaultValue = null)|

## Detailed Description

The settings are stored in a json file, and could be:

- bool
- int
- double
- string
- array<string>

## Method Documentation

#### <a name="value"></a>variant **value**(string path, variant defaultValue = null)

Returns the value of the settings `path`, or `defaultValue` if the settings does not exist.
