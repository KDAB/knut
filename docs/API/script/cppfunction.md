# CppFunction

Represents a function or a method in the current file [More...](#detailed-description)

```qml
import Script 1.0
```

## Properties

| | Name |
|-|-|
|QVector<[Argument](../script/argument.md)>|**[arguments](#arguments)**|
|string|**[name](#name)**|
|[TextRange](../script/textrange.md)|**[range](#range)**|
|string|**[returnType](#returnType)**|

## Property Documentation

#### <a name="arguments"></a>QVector<[Argument](../script/argument.md)> **arguments**

Returns the list of arguments being passed to this function.

#### <a name="name"></a>string **name**

Returns the fully qualified name of this function.

#### <a name="range"></a>[TextRange](../script/textrange.md) **range**

The range enclosing this function, not including leading/trailing
whitespace but everything else like comments.

#### <a name="returnType"></a>string **returnType**

Returns the return type of this function.
