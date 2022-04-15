# CppFunction

!!! Warning "Experimental API"
    The API here is still experimental, and may change in follow-up release. Use it at your own risk.


Represents a function or a method in the current file [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 4.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|vector<[Argument](../script/argument.md)>|**[arguments](#arguments)**|
|bool|**[isNull](#isNull)**|
|string|**[name](#name)**|
|[TextRange](../script/textrange.md)|**[range](#range)**|
|string|**[returnType](#returnType)**|

## Property Documentation

#### <a name="arguments"></a>vector<[Argument](../script/argument.md)> **arguments**

Returns the list of arguments being passed to this function.

#### <a name="isNull"></a>bool **isNull**

This property returns `true` if the function is null.

#### <a name="name"></a>string **name**

Returns the fully qualified name of this function.

#### <a name="range"></a>[TextRange](../script/textrange.md) **range**

The range enclosing this function, not including leading/trailing
whitespace but everything else like comments.

#### <a name="returnType"></a>string **returnType**

Returns the return type of this function.
