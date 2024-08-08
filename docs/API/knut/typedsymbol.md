# TypedSymbol

Represents a symbol with a type [More...](#detailed-description)

```qml
import Knut
```

## Properties

| | Name |
|-|-|
|string|**[type](#type)**|

## Detailed Description

This symbol has a type associated with it, like a variable or a member of a class.

## Property Documentation

#### <a name="type"></a>string **type**

The type of this symbol.

The type is the part of the symbol that describes what kind of value it holds. For example, the type of a variable.
This symbol will extract the type as-written in the original source, but with whitespace
[simplified](https://doc.qt.io/qt-6/qstring.html#simplified). So if e.g. the source code is `const string  &`, it
will be extracted as `const string &`. The type will **not** be resolved to a fully qualified path (like
`std::string` for the previous example).

This property is read-only.
