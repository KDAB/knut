# CodeDocument

Base document object for any code that Knut can parse. [More...](#detailed-description)

```qml
import Script
```

## Properties


Inherited properties: [TextDocument properties](../script/textdocument.md#properties)

## Methods

| | Name |
|-|-|
|[Symbol](../script/symbol.md) |**[findSymbol](#findSymbol)**(string name, int options = TextDocument.NoFindFlags)|
|string |**[hover](#hover)**()|
|array&lt;[QueryMatch](../script/querymatch.md)> |**[query](#query)**(string query)|
|[QueryMatch](../script/querymatch.md) |**[queryFirst](#queryFirst)**(string query)|
|array&lt;[QueryMatch](../script/querymatch.md)> |**[queryInRange](#queryInRange)**([RangeMark](../script/rangemark.md) range, string query)|
||**[selectSymbol](#selectSymbol)**(string name, int options = TextDocument.NoFindFlags)|
|[Symbol](../script/symbol.md) |**[symbolUnderCursor](#symbolUnderCursor)**()|
|array&lt;[Symbol](../script/symbol.md)> |**[symbols](#symbols)**()|

Inherited methods: [TextDocument methods](../script/textdocument.md#methods)

## Detailed Description

Knut uses Tree-sitter to parse the code and provide additional information about it.
For a better user experience, the Knut GUI also uses a Language server (LSP), if available.
For each language that Knut can work with, this class should be subclassed to provide language-specific
functionality.

This class provides the language-independent basis of integration with Tree-sitter and the LSP.

## Method Documentation

#### <a name="findSymbol"></a>[Symbol](../script/symbol.md) **findSymbol**(string name, int options = TextDocument.NoFindFlags)

Finds a symbol based on its `name`, using different find `options`.

- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only fully qualified symbol
- `TextDocument.FindRegexp`: use a regexp

Note that the returned `Symbol` pointer is only valid until the document it originates
from is deconstructed.

#### <a name="hover"></a>string **hover**()

Returns information about the symbol at the current cursor position.
The result of this call is a plain string that may be formatted in Markdown.

#### <a name="query"></a>array&lt;[QueryMatch](../script/querymatch.md)> **query**(string query)

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
 Core::QueryMatchList CodeDocument::query(const QString &query)

#### <a name="queryInRange"></a>array&lt;[QueryMatch](../script/querymatch.md)> **queryInRange**([RangeMark](../script/rangemark.md) range, string query)

Searches for the given `query`, but only in the provided `range`.


#### <a name="selectSymbol"></a>**selectSymbol**(string name, int options = TextDocument.NoFindFlags)

Selects a symbol based on its `name`, using different find `options`.

- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only fully qualified symbol
- `TextDocument.FindRegexp`: use a regexp

If no symbols are found, do nothing.

#### <a name="symbolUnderCursor"></a>[Symbol](../script/symbol.md) **symbolUnderCursor**()

Returns the symbol that's at the current cursor position.

This function may return symbols that are not returned by the `symbols()` or `currentSymbol()` function,
as these only return high-level symbols, like classes and functions, but not symbols within functions.
`symbolUnderCursor()` can however return these Symbols.

#### <a name="symbols"></a>array&lt;[Symbol](../script/symbol.md)> **symbols**()

Returns the list of symbols in the current document.

Note that the returned `Symbol` pointers are only valid until the document they
originate from is deconstructed.
