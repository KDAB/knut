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
||**[insertInclude](#insertInclude)**(string include, bool newGroup = false)|
|[CppDocument](../script/cppdocument.md) |**[openHeaderSource](#openHeaderSource)**()|
|int |**[selectBlockEnd](#selectBlockEnd)**()|
|int |**[selectBlockStart](#selectBlockStart)**()|
||**[toggleSection](#toggleSection)**()|

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

#### <a name="insertInclude"></a>**insertInclude**(string include, bool newGroup = false)

Inserts a new include line in the file. If the include is already in, do nothing (and returns true).

The `include` string should be either `<foo.h>` or `"foo.h"`, it will returns false otherwise.
The method will try to find the best group of includes to insert into, a group of includes being consecutive includes
in the file.

If `newGroup` is true, it will insert the include at the end, with a new line separating the other includes.

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

#### <a name="toggleSection"></a>**toggleSection**()

Comments out a section of the code using `#ifdef` / `#endif`. The variable used is defined by the settings.
```
"toggle_section": {
    "tag": "KDAB_TEMPORARILY_REMOVED",
    "debug": "qDebug("%1 is commented out")"
    "return_values": {
        "BOOL": "false"
    }
}
```
`debug` is the debug line to show, if empty it won't show anything. `return_values` gives a mapping for the value
returned by the function. In this example, if the returned type is `BOOL`, it will return `false`. If text is
selected, it comment out the lines of the selected text. Otherwise, it will comment the function the cursor is in. In
the latter case, if the function is already commented, it will remove the commented section.
