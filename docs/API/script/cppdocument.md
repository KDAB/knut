# CppDocument

Document object for a C++ file (source or header) [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 4.0</td></tr>
<tr><td>Inherits:</td><td><a href="LspDocument.html">LspDocument</a></td></tr>
</table>

## Properties

| | Name |
|-|-|
|bool|**[isHeader](#isHeader)**|

Inherited properties: [LspDocument properties](../script/lspdocument.md#properties)

## Methods

| | Name |
|-|-|
|string |**[correspondingHeaderSource](#correspondingHeaderSource)**()|
|int |**[gotoBlockEnd](#gotoBlockEnd)**()|
|int |**[gotoBlockStart](#gotoBlockStart)**()|
||**[insertForwardDeclaration](#insertForwardDeclaration)**(string fwddecl)|
|[CppDocument](../script/cppdocument.md) |**[openHeaderSource](#openHeaderSource)**()|

Inherited methods: [LspDocument methods](../script/lspdocument.md#methods)

## Property Documentation

#### <a name="isHeader"></a>bool **isHeader**

Return true if the current document is a header.

## Method Documentation

#### <a name="correspondingHeaderSource"></a>string **correspondingHeaderSource**()

Returns the corresponding source or header file path.

#### <a name="gotoBlockEnd"></a>int **gotoBlockEnd**()

Move the cursor to the end of the block it's in, and returns the new cursor position.
A block is definied by {} or () or [].

#### <a name="gotoBlockStart"></a>int **gotoBlockStart**()

Move the cursor to the start of the block it's in, and returns the new cursor position.
A block is definied by {} or () or [].

#### <a name="insertForwardDeclaration"></a>**insertForwardDeclaration**(string fwddecl)

Inserts the forward declaration `fwddecl` into the current file.
The method will check if the file is a header file, and also that the forward declaration starts with 'class ' or
'struct '. Fully qualified the forward declaration to add namespaces: `class Foo::Bar::FooBar` will result in:

```cpp
namespace Foo {
namespace Bar {
class FooBar
}
}
```

#### <a name="openHeaderSource"></a>[CppDocument](../script/cppdocument.md) **openHeaderSource**()

Opens the corresponding source or header files, the current document is the new file.
If no files have been found, it's a no-op.
