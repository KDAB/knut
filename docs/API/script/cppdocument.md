# CppDocument

Document object for a C++ file (source or header) [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 4.0</td></tr>
<tr><td>Inherits:</td><td><a href="TextDocument.html">TextDocument</a></td></tr>
</table>

## Properties

| | Name |
|-|-|
|bool|**[isHeader](#isHeader)**|

Inherited properties: [TextDocument properties](../script/textdocument.md#properties)

## Methods

| | Name |
|-|-|
|string |**[correspondingHeaderSource](#correspondingHeaderSource)**()|
|[CppDocument](../script/cppdocument.md) |**[openHeaderSource](#openHeaderSource)**()|

Inherited methods: [TextDocument methods](../script/textdocument.md#methods)

## Property Documentation

#### <a name="isHeader"></a>bool **isHeader**

Return true if the current document is a header.

## Method Documentation

#### <a name="correspondingHeaderSource"></a>string **correspondingHeaderSource**()

Returns the corresponding source or header file path.

#### <a name="openHeaderSource"></a>[CppDocument](../script/cppdocument.md) **openHeaderSource**()

Opens the corresponding source or header files, the current document is the new file.
If no files have been found, it's a no-op.
