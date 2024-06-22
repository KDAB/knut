# FunctionSymbol

!!! Warning "Experimental API"
    The API here is still experimental, and may change in follow-up release. Use it at your own risk.


Represents a function or a method in the current file [More...](#detailed-description)

```qml
import Script 1.0
```

## Properties

| | Name |
|-|-|
|vector<[FunctionArgument](../script/functionargument.md)>|**[arguments](#arguments)**|
|[TextRange](../script/textrange.md)|**[range](#range)**|
|string|**[returnType](#returnType)**|

## Property Documentation

#### <a name="arguments"></a>vector<[FunctionArgument](../script/functionargument.md)> **arguments**

Returns the list of arguments being passed to this function.

#### <a name="range"></a>[TextRange](../script/textrange.md) **range**

The range enclosing this function, not including leading/trailing
whitespace but everything else like comments.

#### <a name="returnType"></a>string **returnType**

Returns the return type of this function.
