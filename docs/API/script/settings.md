# Settings

Provides a singleton for accessing and editing persistent settings. [More...](#detailed-description)

```qml
import Script 1.0
```

## Methods

- variant **[value](#value)**(string path, variant defaultValue = null)

## Detailed Description

Settings are stored in a json file, and can be anything.

## Method Documentation

#### <a name="value"></a>variant **value**(string path, variant defaultValue = null)

Returns the value of the settings \a path, or \a defaultValue if the settings does not exist.
