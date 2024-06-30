# CppDocument

Document object for a C++ file (source or header) [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|bool|**[isHeader](#isHeader)**|

Inherited properties: [CodeDocument properties](../script/codedocument.md#properties)

## Methods

| | Name |
|-|-|
||**[addMember](#addMember)**(string member, string className, AccessSpecifier)|
||**[addMethod](#addMethod)**(string declaration, string className, AccessSpecifier, string body)|
||**[addMethod](#addMethod)**(string declaration, string className, AccessSpecifier)|
||**[addMethodDeclaration](#addMethodDeclaration)**(string method, string className, AccessSpecifier specifier)|
||**[addMethodDefinition](#addMethodDefinition)**(string method, string className)|
||**[addMethodDefinition](#addMethodDefinition)**(string method, string className, string body)|
|void |**[changeBaseClass](#changeBaseClass)**()|
||**[commentSelection](#commentSelection)**()|
|string |**[correspondingHeaderSource](#correspondingHeaderSource)**()|
|void |**[deleteMethod](#deleteMethod)**()|
|void |**[deleteMethod](#deleteMethod)**(string methodName)|
|void |**[deleteMethod](#deleteMethod)**(string method, string signature)|
|int |**[gotoBlockEnd](#gotoBlockEnd)**(int count)|
|int |**[gotoBlockStart](#gotoBlockStart)**(int count)|
||**[insertCodeInMethod](#insertCodeInMethod)**(string methodName, string code, Position insertAt)|
||**[insertForwardDeclaration](#insertForwardDeclaration)**(string forwardDeclaration)|
||**[insertInclude](#insertInclude)**(string include, bool newGroup = false)|
|QStringList |**[keywords](#keywords)**()|
|[DataExchange](../script/dataexchange.md) |**[mfcExtractDDX](#mfcExtractDDX)**(string className)|
|[MessageMap](../script/messagemap.md) |**[mfcExtractMessageMap](#mfcExtractMessageMap)**(string className = "")|
|[CppDocument](../script/cppdocument.md) |**[openHeaderSource](#openHeaderSource)**()|
|QStringList |**[primitiveTypes](#primitiveTypes)**()|
|[QueryMatch](../script/querymatch.md) |**[queryClassDefinition](#queryClassDefinition)**(string className)|
|array&lt;[QueryMatch](../script/querymatch.md)> |**[queryFunctionCall](#queryFunctionCall)**(string functionName)|
|array&lt;[QueryMatch](../script/querymatch.md)> |**[queryFunctionCall](#queryFunctionCall)**(string functionName, array&lt;string> argumentCaptures)|
|[QueryMatch](../script/querymatch.md) |**[queryMember](#queryMember)**(string className, string memberName)|
|array&lt;[QueryMatch](../script/querymatch.md)> |**[queryMethodDeclaration](#queryMethodDeclaration)**(string className, string functionName)|
|array&lt;[QueryMatch](../script/querymatch.md)> |**[queryMethodDefinition](#queryMethodDefinition)**(string scope, string methodName)|
||**[removeInclude](#removeInclude)**(string include)|
|int |**[selectBlockEnd](#selectBlockEnd)**()|
|int |**[selectBlockStart](#selectBlockStart)**()|
|int |**[selectBlockUp](#selectBlockUp)**()|
||**[toggleSection](#toggleSection)**()|

Inherited methods: [CodeDocument methods](../script/codedocument.md#methods)

## Property Documentation

#### <a name="isHeader"></a>bool **isHeader**

Return true if the current document is a header.

## Method Documentation

#### <a name="addMember"></a>**addMember**(string member, string className, AccessSpecifier)

Adds a new member in a specific class under the specific access specifier.

If the class does not exist, log error can't find the class, but if the
specifier is valid but does not exist in the class, we will add that specifier in the end of the
class and add the member under it.
The specifier can take these values:

- `CppDocument.Public`
- `CppDocument.Protected`
- `CppDocument.Private`

#### <a name="addMethod"></a>**addMethod**(string declaration, string className, AccessSpecifier, string body)<br/>**addMethod**(string declaration, string className, AccessSpecifier)

Declares and defines a new method.
This method can be called on either the header or source file.
It will find the corresponding header/source file and add the declaration
to the header and the definition to the source.


#### <a name="addMethodDeclaration"></a>**addMethodDeclaration**(string method, string className, AccessSpecifier specifier)

Declares a new method in a specific class under the specific access specifier.

If the class does not exist, log error can't find the class, but if the
specifier is valid but does not exist in the class, we will add that specifier in the end of the
class and declare the method under it.
The specifier can take these values:

- `CppDocument.Public`
- `CppDocument.Protected`
- `CppDocument.Private`

#### <a name="addMethodDefinition"></a>**addMethodDefinition**(string method, string className)<br/>**addMethodDefinition**(string method, string className, string body)

Adds a new method definition for the method declared by the given `method` for
class `className` in the current file.
The provided `body` should not include the curly braces.

If no body is provided, it will default to an empty body.

#### <a name="changeBaseClass"></a>void **changeBaseClass**()

Convert Class Base

Also see: CppDocument::changeBaseClass(const QString &className, const QString &originalClassBaseName, const QString
&newClassBaseName)

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

Deletes the method/function at the current cursor position.
Overloads of the function will not be deleted!

Also see: CppDocument::deleteMethod(const QString& methodName, const QString& signature)

#### <a name="deleteMethod"></a>void **deleteMethod**(string methodName)

Deletes a method of the specified `methodName`, without matching a specific `signature`.
Therefore, all overloads of the function will be deleted.

Also see: CppDocument::deleteMethod(string methodName, string signature)

#### <a name="deleteMethod"></a>void **deleteMethod**(string method, string signature)

Delete the method or function with the specified `method` and optional `signature`.
The method definition/declaration will be deleted from the current file,
as well as the corresponding header/source file.
References to the method will not be deleted.

The `method` must be fully qualified, i.e. "<Namespaces>::<Class>::<Method>".

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
A block is defined by {} or () or [].
Does it `count` times.

#### <a name="gotoBlockStart"></a>int **gotoBlockStart**(int count)

Moves the cursor to the start of the block it's in, and returns the new cursor position.
A block is defined by {} or () or [].
Does it `count` times.

#### <a name="insertCodeInMethod"></a>**insertCodeInMethod**(string methodName, string code, Position insertAt)

Provides a fast way to add some code in an existing method definition. Does nothing if the method does not exist in
the current document.

This method will find a method in the current file with name matching with `methodName`. If the method exists in the
current document, then it will insert the supplied `code` either at the beginning of the method, or at the end of the
method, depending on the `insertAt` argument.

#### <a name="insertForwardDeclaration"></a>**insertForwardDeclaration**(string forwardDeclaration)

Inserts the forward declaration `forwardDeclaration` into the current file.
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

#### <a name="keywords"></a>QStringList **keywords**()

Returns a list of cpp keywords

See: Utils::cppKeywords()

#### <a name="mfcExtractDDX"></a>[DataExchange](../script/dataexchange.md) **mfcExtractDDX**(string className)

Extracts the DDX information from a MFC class.

The DDX information gives the mapping between the IDC and the member variables in the class.

#### <a name="mfcExtractMessageMap"></a>[MessageMap](../script/messagemap.md) **mfcExtractMessageMap**(string className = "")

Extracts information contained in the MFC MESSAGE_MAP.
The `className` parameter can be used to ensure the result matches to a specific class.

#### <a name="openHeaderSource"></a>[CppDocument](../script/cppdocument.md) **openHeaderSource**()

Opens the corresponding source or header files, the current document is the new file.
If no files have been found, it's a no-op.

#### <a name="primitiveTypes"></a>QStringList **primitiveTypes**()

Returns a list of cpp primitive types

See Utils::cppPrimitiveTypes()

#### <a name="queryClassDefinition"></a>[QueryMatch](../script/querymatch.md) **queryClassDefinition**(string className)

Returns the class or struct definition matching the given `className`.

The returned QueryMatch instance will have the following captures available:

- `name` - The name of the class or struct
- `base` - The list of base classes/structs, if any
- `body` - The body of the class or struct definition (including curly-braces)

#### <a name="queryFunctionCall"></a>array&lt;[QueryMatch](../script/querymatch.md)> **queryFunctionCall**(string functionName)

Returns the list of function calls to the function `functionName`, no matter how many arguments they were called with.

The returned QueryMatch instances will have the following captures available:

- `call` - The entire call expression
- `name` - The name of the function (the text will be equal to functionName)
- `argument-list` - The entire list of arguments, including the surroundg parentheses `()`
- `arguments` - Each argument provided to the function call, in order, excluding any comments

#### <a name="queryFunctionCall"></a>array&lt;[QueryMatch](../script/querymatch.md)> **queryFunctionCall**(string functionName, array&lt;string> argumentCaptures)

Returns the list of function calls to the function `functionName`.
Only calls that have the same number of arguments as `argumentCaptures` will be returned.

The `argumentCaptures` list is a list of names that will be used to capture the arguments of the function call.
E.g. `queryFunctionCall("foo", ["first", "second"])` will return a list of calls to `foo` with two arguments,
where the first argument will be captured in the `first` capture, and the second in the `second` capture.

The returned QueryMatch instances will have the following captures available:

- `call` - The entire call expression
- `name` - The name of the function (the text will be equal to functionName)
- `argument-list` - The entire list of arguments, including the surroundg parentheses `()`
- a capture for every argument in `argumentCaptures`

#### <a name="queryMember"></a>[QueryMatch](../script/querymatch.md) **queryMember**(string className, string memberName)

Finds the member definition inside a class or struct definition.
Returns a QueryMatch object containing the member definition if it exists.

The returned QueryMatch instance will have the following captures available:

- `member`: The full definition of the member
- `type`: The type of the member, without `const` or any reference/pointer specifiers (i.e. `&`/`*`)
- `name`: The name of the member (should be equal to memberName)

#### <a name="queryMethodDeclaration"></a>array&lt;[QueryMatch](../script/querymatch.md)> **queryMethodDeclaration**(string className, string functionName)

Finds the declaration of a method inside a class or struct definition.

Returns a list of QueryMatch objects containing the declaration.
A warning will be logged if no declarations are found.

The returned QueryMatch instances contain the following captures:

- `declaration`: The full declaration of the method
- `function`: The function declaration, without the return type
- `name`: The name of the function

#### <a name="queryMethodDefinition"></a>array&lt;[QueryMatch](../script/querymatch.md)> **queryMethodDefinition**(string scope, string methodName)

Returns the list of methods definitions matching the given name and scope.
`scope` may be either a class name, a namespace or empty.

Every QueryMatch returned by this function will have the following captures available:

- `scope` - The scope of the method (if any is provided)
- `name` - The name of the function
- `definition` - The entire method definition
- `parameter-list` - The list of parameters
- `parameters` - One capture per parameter, containing the type and name of the parameter, excluding comments!
- `body` - The body of the method (including curly-braces)

Please note that the return type is not available, as TreeSitter is not able to parse it easily.

#### <a name="removeInclude"></a>**removeInclude**(string include)

Remove `include` from the file. If the include is not in the file, do nothing (and returns true).

The `include` string should be either `<foo.h>` or `"foo.h"`, it will returns false otherwise.

#### <a name="selectBlockEnd"></a>int **selectBlockEnd**()

Selects the text from current cursor position to the end of the block, and returns the new cursor position.
A block is defined by {} or () or [].
Does it `count` times.

#### <a name="selectBlockStart"></a>int **selectBlockStart**()

Selects the text from current cursor position to the start of the block, and returns the new cursor position.
A block is defined by {} or () or [].
Does it `count` times.

#### <a name="selectBlockUp"></a>int **selectBlockUp**()

Selects the text of the block the cursor is in, and returns the new cursor position.
A block is defined by {} or () or [].
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
