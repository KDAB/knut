# CppDocument

Document object for a C++ file (source or header) [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.0</td></tr>
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
||**[addMember](#addMember)**(string member, string className, AccessSpecifier)|
||**[addMethodDeclaration](#addMethodDeclaration)**(string member, string className, AccessSpecifier)|
||**[addMethodDefintion](#addMethodDefintion)**(string method, string className)|
||**[commentSelection](#commentSelection)**()|
|string |**[correspondingHeaderSource](#correspondingHeaderSource)**()|
|void |**[deleteMethod](#deleteMethod)**()|
|void |**[deleteMethod](#deleteMethod)**(string methodName)|
|void |**[deleteMethod](#deleteMethod)**(string methodName, string signature)|
|int |**[gotoBlockEnd](#gotoBlockEnd)**(int count)|
|int |**[gotoBlockStart](#gotoBlockStart)**(int count)|
||**[insertCodeInMethod](#insertCodeInMethod)**(string methodName, string code, Position insertAt)|
||**[insertForwardDeclaration](#insertForwardDeclaration)**(string fwddecl)|
||**[insertInclude](#insertInclude)**(string include, bool newGroup = false)|
||**[mfcExtractMessageMap](#mfcExtractMessageMap)**(className = "")|
|[CppDocument](../script/cppdocument.md) |**[openHeaderSource](#openHeaderSource)**()|
||**[removeInclude](#removeInclude)**(string include)|
|int |**[selectBlockEnd](#selectBlockEnd)**()|
|int |**[selectBlockStart](#selectBlockStart)**()|
|int |**[selectBlockUp](#selectBlockUp)**()|
||**[toggleSection](#toggleSection)**()|

Inherited methods: [LspDocument methods](../script/lspdocument.md#methods)

## Property Documentation

#### <a name="isHeader"></a>bool **isHeader**

Return true if the current document is a header.

## Method Documentation

#### <a name="addMember"></a>**addMember**(string member, string className, AccessSpecifier)

!!! note ""
    Since: Knut 1.1

Adds a new member in a specific class under the specefic access specifier.

If the class does not exist, log error can't find the class, but if the
specifier is valid but does not exist in the class, we will add that specifier in the end of the
class and add the member under it.
The specifier can take these values:

- `CppDocument.Public`
- `CppDocument.Protected`
- `CppDocument.Private`

#### <a name="addMethodDeclaration"></a>**addMethodDeclaration**(string member, string className, AccessSpecifier)

!!! note ""
    Since: Knut 1.1

Declares a new method in a specific class under the specefic access specifier.

If the class does not exist, log error can't find the class, but if the
specifier is valid but does not exist in the class, we will add that specifier in the end of the
class and declare the method under it.
The specifier can take these values:

- `CppDocument.Public`
- `CppDocument.Protected`
- `CppDocument.Private`

#### <a name="addMethodDefintion"></a>**addMethodDefintion**(string method, string className)

!!! note ""
    Since: Knut 1.1

Defines a new method `method` for class `className` in the current file.

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

#### <a name="deleteMethod"></a>void **deleteMethod**()

!!! note ""
    Since: Knut 1.1

Deletes the method/function at the current cursor position.
Overloads of the function will not be deleted!

Also see: CppDocument::deleteMethod(const QString& methodName, const QString& signature)

#### <a name="deleteMethod"></a>void **deleteMethod**(string methodName)

!!! note ""
    Since: Knut 1.1

Deletes a method of the specified `methodName`, without matching a specific `signature`.
Therefore, all overloads of the function will be deleted.

Also see: CppDocument::deleteMethod(string methodName, string signature)

#### <a name="deleteMethod"></a>void **deleteMethod**(string methodName, string signature)

!!! note ""
    Since: Knut 1.1

Delete the method or function with the specified `methodName` and optional `signature`.
The method definition/declaration will be deleted from the current file,
as well as the corresponding header/source file.
References to the method will not be deleted.

The `methodName` must be fully qualified, i.e. "<Namespaces>::<Class>::<Method>".

The `signature` must be in the form: "<return type> (<first parameter type>, <second parameter type>, <...>)".
i.e. for a function with the following declaration:

``` cpp
void myFunction(const QString& a, int b);
```

The `signature` would be:

``` cpp
void (const QString&, int)
```

If an empty string is provided as the `signature`, all overloads of the function are deleted as well.

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

#### <a name="mfcExtractMessageMap"></a>**mfcExtractMessageMap**(className = "")

!!! note ""
    Since: Knut 1.1

Extracts information contained in the MFC MESSAGE_MAP.
The `className` parameter can be used to ensure the result matches to a specific class.
Returns a `MessageMap` object.

#### <a name="openHeaderSource"></a>[CppDocument](../script/cppdocument.md) **openHeaderSource**()

Opens the corresponding source or header files, the current document is the new file.
If no files have been found, it's a no-op.

#### <a name="removeInclude"></a>**removeInclude**(string include)

Remove `include` from the file. If the include is not in the file, do nothing (and returns true).

The `include` string should be either `<foo.h>` or `"foo.h"`, it will returns false otherwise.

#### <a name="selectBlockEnd"></a>int **selectBlockEnd**()

Selects the text from current cursor position to the end of the block, and returns the new cursor position.
A block is definied by {} or () or [].
Does it `count` times.

#### <a name="selectBlockStart"></a>int **selectBlockStart**()

Selects the text from current cursor position to the start of the block, and returns the new cursor position.
A block is definied by {} or () or [].
Does it `count` times.

#### <a name="selectBlockUp"></a>int **selectBlockUp**()

!!! note ""
    Since: Knut 1.1

Selects the text of the block the cursor is in, and returns the new cursor position.
A block is definied by {} or () or [].
Does it `count` times.

#### <a name="toggleSection"></a>**toggleSection**()

Comments out a section of the code using `#ifdef` / `#endif`. The variable used is defined by the settings.
```json
"toggle_section": {
    "tag": "KDAB_TEMPORARILY_REMOVED",
    "debug": "qDebug(\"%1 is commented out\")"
    "return_values": {
        "BOOL": "false"
    }
}
```
`debug` is the debug line to show, if empty it won't show anything. `return_values` gives a mapping for the value
returned by the function. In this example, if the returned type is `BOOL`, it will return `false`. If text is
selected, it comment out the lines of the selected text. Otherwise, it will comment the function the cursor is in. In
the latter case, if the function is already commented, it will remove the commented section.
