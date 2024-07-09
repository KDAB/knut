# QtUiWidget

Provides access to widget attributes in the ui files. [More...](#detailed-description)

```qml
import Knut
```

## Properties

| | Name |
|-|-|
|string|**[className](#className)**|
|bool|**[isRoot](#isRoot)**|
|string|**[name](#name)**|

## Methods

| | Name |
|-|-|
||**[addProperty](#addProperty)**(string name, var value, object attributes = {})|
|var |**[getProperty](#getProperty)**(string name)|

## Property Documentation

#### <a name="className"></a>string **className**

Name of the widget's class.

#### <a name="isRoot"></a>bool **isRoot**

Read-only property returning `true` if the widget is the root widget.

#### <a name="name"></a>string **name**

Name of the widget.

## Method Documentation

#### <a name="addProperty"></a>**addProperty**(string name, var value, object attributes = {})

Adds a new property with the given `name`, `value` and `attributes`.

Attributes is a has<string, string> object, where the key is the attribute name and the value is the attribute value.
For example:

```
widget.setProperty("text", "My text", { "comment": "some comment for translation" });
```

#### <a name="getProperty"></a>var **getProperty**(string name)

Returns the value of the property `name`.
