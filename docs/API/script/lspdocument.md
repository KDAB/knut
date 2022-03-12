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
|vector<[Symbol](../script/symbol.md)> |**[symbols](#symbols)**()|

Inherited methods: [TextDocument methods](../script/textdocument.md#methods)

## Method Documentation

#### <a name="findSymbol"></a>[Symbol](../script/symbol.md) **findSymbol**(string name, int options = TextDocument.NoFindFlags)

Find a symbol based on its `name`, using different find `options`.

- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only fully qualified symbol
- `TextDocument.FindRegexp`: use a regexp

#### <a name="symbols"></a>vector<[Symbol](../script/symbol.md)> **symbols**()

Returns the list of symbols in the current document.
