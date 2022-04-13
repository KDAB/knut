# Symbol

Represent a symbol in the current file [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 4.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|string|**[description](#description)**|
|bool|**[isNull](#isNull)**|
|Kind|**[kind](#kind)**|
|string|**[name](#name)**|
|[TextRange](../script/textrange.md)|**[range](#range)**|
|[TextRange](../script/textrange.md)|**[selectionRange](#selectionRange)**|

## Methods

| | Name |
|-|-|
|[CppClass](../script/cppclass.md) |**[toClass](#toClass)**()|
|[CppFunction](../script/cppfunction.md) |**[toFunction](#toFunction)**()|

## Property Documentation

#### <a name="description"></a>string **description**

Return more detail for this symbol, e.g the signature of a function.

#### <a name="isNull"></a>bool **isNull**

This property returns `true` if the symbol is null.

#### <a name="kind"></a>Kind **kind**

Return the kind of this symbol. Available symbol kinds are:

- `Symbol.File`
- `Symbol.Module`
- `Symbol.Namespace`
- `Symbol.Package`
- `Symbol.Class`
- `Symbol.Method`
- `Symbol.Property`
- `Symbol.Field`
- `Symbol.Constructor`
- `Symbol.Enum`
- `Symbol.Interface`
- `Symbol.Function`
- `Symbol.Variable`
- `Symbol.Constant`
- `Symbol.String`
- `Symbol.Number`
- `Symbol.Boolean`
- `Symbol.Array`
- `Symbol.Object`
- `Symbol.Key`
- `Symbol.Null`
- `Symbol.EnumMember`
- `Symbol.Struct`
- `Symbol.Event`
- `Symbol.Operator`
- `Symbol.TypeParameter`

#### <a name="name"></a>string **name**

Return the name of this symbol.

#### <a name="range"></a>[TextRange](../script/textrange.md) **range**

The range enclosing this symbol not including leading/trailing whitespace but everything else like comments. This
information is typically used to determine if the clients cursor is inside the symbol to reveal in the symbol in the
UI.

#### <a name="selectionRange"></a>[TextRange](../script/textrange.md) **selectionRange**

The range that should be selected and revealed when this symbol is being picked, e.g. the name of a function. Must be
contained by the `range`.

## Method Documentation

#### <a name="toClass"></a>[CppClass](../script/cppclass.md) **toClass**()


!!! Warning "Experimental API"
    The API here is still experimental, and may change in follow-up release. Use it at your own risk.

Returns a structure representing the class for the current symbol.
The method checks if the `kind` of the symbol for `Symbol::Class` or `Symbol::Struct`. If so then it finds all the
members of the class from the list of symbols in current document, adds them in `CppClass` structure, and returns it.
If not, then it returns an empty structure.

#### <a name="toFunction"></a>[CppFunction](../script/cppfunction.md) **toFunction**()


!!! Warning "Experimental API"
    The API here is still experimental, and may change in follow-up release. Use it at your own risk.

Returns a `CppFunction` structure for current `Symbol::Symbol`.
The method checks if the `kind` of the symbol for `Symbol::Method` or `Symbol::Function`. If so then it extracts
information from `Symbol::description`, fills it in `CppFunction` structure, and returns it. If not, then it returns
an empty structure.
