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
|Kind|**[kind](#kind)**|
|string|**[name](#name)**|
|[TextRange](../script/textrange.md)|**[range](#range)**|
|[TextRange](../script/textrange.md)|**[selectionRange](#selectionRange)**|

## Property Documentation

#### <a name="description"></a>string **description**

Return more detail for this symbol, e.g the signature of a function.

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

The range enclosing this symbol not including leading/trailing whitespace
but everything else like comments. This information is typically used to
determine if the clients cursor is inside the symbol to reveal in the
symbol in the UI.

#### <a name="selectionRange"></a>[TextRange](../script/textrange.md) **selectionRange**

The range that should be selected and revealed when this symbol is being
picked, e.g. the name of a function. Must be contained by the `range`.
