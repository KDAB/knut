# LspDocument

Base document object for document using LSP. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 4.0</td></tr>
<tr><td>Inherits:</td><td><a href="TextDocument.html">TextDocument</a></td></tr>
</table>

## Properties


Inherited properties: [TextDocument properties](../script/textdocument.md#properties)

## Methods

| | Name |
|-|-|
|[Symbol](../script/symbol.md) |**[findSymbol](#findSymbol)**(string name, int options = TextDocument.NoFindFlags)|
||**[followSymbol](#followSymbol)**()|
||**[selectSymbol](#selectSymbol)**(string name, int options = TextDocument.NoFindFlags)|
||**[switchDeclarationDefinition](#switchDeclarationDefinition)**()|
|vector<[Symbol](../script/symbol.md)> |**[symbols](#symbols)**()|

Inherited methods: [TextDocument methods](../script/textdocument.md#methods)

## Method Documentation

#### <a name="findSymbol"></a>[Symbol](../script/symbol.md) **findSymbol**(string name, int options = TextDocument.NoFindFlags)

Find a symbol based on its `name`, using different find `options`.

- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only fully qualified symbol
- `TextDocument.FindRegexp`: use a regexp

#### <a name="followSymbol"></a>**followSymbol**()

Follow the symbol under the cursor.

- Go to the declaration, if the symbol under cursor is a use
- Go to the declaration, if the symbol under cursor is a function definition
- Go to the definition, if the symbol under cursor is a function declaration

#### <a name="selectSymbol"></a>**selectSymbol**(string name, int options = TextDocument.NoFindFlags)

Select a symbol based on its `name`, using different find `options`.

- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only fully qualified symbol
- `TextDocument.FindRegexp`: use a regexp

If no symbols are found, do nothing.

#### <a name="switchDeclarationDefinition"></a>**switchDeclarationDefinition**()

Switch between the function declaration or definition.

#### <a name="symbols"></a>vector<[Symbol](../script/symbol.md)> **symbols**()

Returns the list of symbols in the current document.
