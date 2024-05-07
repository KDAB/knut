# LspDocument

Base document object for document using LSP. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.0</td></tr>
<tr><td>Inherits:</td><td><a href="TextDocument.html">TextDocument</a></td></tr>
</table>

## Properties


Inherited properties: [TextDocument properties](../script/textdocument.md#properties)

## Methods

| | Name |
|-|-|
|[Symbol](../script/symbol.md) |**[findSymbol](#findSymbol)**(string name, int options = TextDocument.NoFindFlags)|
||**[followSymbol](#followSymbol)**()|
|string |**[hover](#hover)**()|
|array<[QueryMatch](../script/querymatch.md)> |**[query](#query)**(string query)|
|[QueryMatch](../script/querymatch.md) |**[queryFirst](#queryFirst)**(string query)|
|array<[QueryMatch](../script/querymatch.md)> |**[queryInRange](#queryInRange)**([RangeMark](../script/rangemark.md) range, string query)|
|array<[TextLocation](../script/textlocation.md)> |**[references](#references)**()|
||**[selectSymbol](#selectSymbol)**(string name, int options = TextDocument.NoFindFlags)|
||**[switchDeclarationDefinition](#switchDeclarationDefinition)**()|
|[Symbol](../script/symbol.md) |**[symbolUnderCursor](#symbolUnderCursor)**()|
|array<[Symbol](../script/symbol.md)> |**[symbols](#symbols)**()|

Inherited methods: [TextDocument methods](../script/textdocument.md#methods)

## Method Documentation

#### <a name="findSymbol"></a>[Symbol](../script/symbol.md) **findSymbol**(string name, int options = TextDocument.NoFindFlags)

Finds a symbol based on its `name`, using different find `options`.

- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only fully qualified symbol
- `TextDocument.FindRegexp`: use a regexp

Note that the returned `Symbol` pointer is only valid until the document it originates
from is deconstructed.

#### <a name="followSymbol"></a>**followSymbol**()

Follows the symbol under the cursor.

- Go to the declaration, if the symbol under cursor is a use
- Go to the declaration, if the symbol under cursor is a function definition
- Go to the definition, if the symbol under cursor is a function declaration

#### <a name="hover"></a>string **hover**()

Returns information about the symbol at the current cursor position.
The result of this call is a plain string that may be formatted in Markdown.

#### <a name="query"></a>array<[QueryMatch](../script/querymatch.md)> **query**(string query)

Runs the given Tree-sitter `query` and returns the list of matches.

The query is using [Tree-sitter
queries](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries).

Also see: [Tree-sitter in Knut](../../getting-started/treesitter.md)

#### <a name="queryFirst"></a>[QueryMatch](../script/querymatch.md) **queryFirst**(string query)

Runs the given Tree-sitter `query` and returns the first match.
If no match can be found an empty match will be returned.

This can be a lot faster than `query` if you only need the first match.

The query is using [Tree-sitter
queries](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries).

Also see: [Tree-sitter in Knut](../../getting-started/treesitter.md)
 Core::QueryMatchList LspDocument::query(const QString &query)

#### <a name="queryInRange"></a>array<[QueryMatch](../script/querymatch.md)> **queryInRange**([RangeMark](../script/rangemark.md) range, string query)

Searches for the given `query`, but only in the provided `range`.


#### <a name="references"></a>array<[TextLocation](../script/textlocation.md)> **references**()

Returns the list of references at the given position.

#### <a name="selectSymbol"></a>**selectSymbol**(string name, int options = TextDocument.NoFindFlags)

Selects a symbol based on its `name`, using different find `options`.

- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only fully qualified symbol
- `TextDocument.FindRegexp`: use a regexp

If no symbols are found, do nothing.

#### <a name="switchDeclarationDefinition"></a>**switchDeclarationDefinition**()


!!! Warning "Experimental API"
    The API here is still experimental, and may change in follow-up release. Use it at your own risk.

Switches between the function declaration or definition.

#### <a name="symbolUnderCursor"></a>[Symbol](../script/symbol.md) **symbolUnderCursor**()

Returns the symbol that's at the current cursor position.

This function may return symbols that are not returned by the `symbols()` or `currentSymbol()` function,
as these only return high-level symbols, like classes and functions, but not symbols within functions.
`symbolUnderCursor()` can however return these Symbols.

#### <a name="symbols"></a>array<[Symbol](../script/symbol.md)> **symbols**()

Returns the list of symbols in the current document.

Note that the returned `Symbol` pointers are only valid until the document they
originate from is deconstructed.
