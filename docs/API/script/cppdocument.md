# CppDocument

!!! Warning "Experimental API"
    The API here is still experimental, and may change in follow-up release. Use it at your own risk.


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
||**[commentSelection](#commentSelection)**()|
|string |**[correspondingHeaderSource](#correspondingHeaderSource)**()|
|int |**[gotoBlockEnd](#gotoBlockEnd)**(int count)|
|int |**[gotoBlockStart](#gotoBlockStart)**(int count)|
||**[insertCodeInMethod](#insertCodeInMethod)**(string methodName, string code, Position insertAt)|
||**[insertForwardDeclaration](#insertForwardDeclaration)**(string fwddecl)|
|[CppDocument](../script/cppdocument.md) |**[openHeaderSource](#openHeaderSource)**()|
|int |**[selectBlockEnd](#selectBlockEnd)**()|
|int |**[selectBlockStart](#selectBlockStart)**()|

Inherited methods: [LspDocument methods](../script/lspdocument.md#methods)

## Property Documentation

#### <a name="isHeader"></a>bool **isHeader**

Return true if the current document is a header.

## Method Documentation

#### <a name="commentSelection"></a>**commentSelection**()

Comments the selected lines (or current line if there's no selection) in current document.

- If there's no selection, current line is commented using `//`.
- If there's a valid selection and the start and end position of the selection are before any text of the lines,
  all of the selected lines are commented using `//`.
- If there's a valid selection and the start and/or end position of the selection are between any text of the
  lines, all of the selected lines are commented using multi-line comment.
- If selection or position is invalid or out of range, or the position is on an empty line, the document remains
  unchanged.

#### <a name="correspondingHeaderSource"></a>string **correspondingHeaderSource**()

Returns the corresponding source or header file path.

#### <a name="gotoBlockEnd"></a>int **gotoBlockEnd**(int count)

Moves the cursor to the end of the block it's in, and returns the new cursor position.
A block is definied by {} or () or [].
Does it `count` times.

#### <a name="gotoBlockStart"></a>int **gotoBlockStart**(int count)

Moves the cursor to the start of the block it's in, and returns the new cursor position.
A block is definied by {} or () or [].
Does it `count` times.

#### <a name="insertCodeInMethod"></a>**insertCodeInMethod**(string methodName, string code, Position insertAt)

Provides a fast way to add some code in an existing method definition. Does nothing if the method does not exist in
the current document.

This method will find a method in the current file with name matching with `methodName`. If the method exists in the
current document, then it will insert the supplied `code` either at the beginning of the method, or at the end of the
method, depending on the `insertAt` argument.

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

#### <a name="selectBlockEnd"></a>int **selectBlockEnd**()

Selects the text from current cursor position to the end of the block, and returns the new cursor position.
A block is definied by {} or () or [].
Does it `count` times.

#### <a name="selectBlockStart"></a>int **selectBlockStart**()

Selects the text from current cursor position to the start of the block, and returns the new cursor position.
A block is definied by {} or () or [].
Does it `count` times.
