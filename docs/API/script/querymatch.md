# QueryMatch

Contains all matches for a query. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.1</td></tr>
</table>

## Properties

| | Name |
|-|-|
|array<[QueryCapture](../script/querycapture.md)>|**[captures](#captures)**|

## Methods

| | Name |
|-|-|
|[RangeMark](../script/rangemark.md) |**[get](#get)**(string name)|
|[RangeMark](../script/rangemark.md) |**[getAll](#getAll)**(string name)|
|[RangeMark](../script/rangemark.md) |**[getAllInRange](#getAllInRange)**(string name, [RangeMark](../script/rangemark.md) range)|
|[RangeMark](../script/rangemark.md) |**[getAllJoined](#getAllJoined)**(string name)|
|[RangeMark](../script/rangemark.md) |**[getInRange](#getInRange)**(string name, [RangeMark](../script/rangemark.md) range)|
|array<[QueryMatch](../script/querymatch.md)> |**[queryIn](#queryIn)**(capture, query)|

## Detailed Description

The QueryMatch object allows you to get access to all the captures made by the query.
The query is using the [TreeSitter](https://tree-sitter.github.io/tree-sitter/) query, you can find more information
on this page: [Pattern Matching with
Queries](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries).

## Property Documentation

#### <a name="captures"></a>array<[QueryCapture](../script/querycapture.md)> **captures**

List of all the captures in the current document.

## Method Documentation

#### <a name="get"></a>[RangeMark](../script/rangemark.md) **get**(string name)

Returns the first match for the query with the given `name`.

#### <a name="getAll"></a>[RangeMark](../script/rangemark.md) **getAll**(string name)

Returns all matches for the query with the given `name`

#### <a name="getAllInRange"></a>[RangeMark](../script/rangemark.md) **getAllInRange**(string name, [RangeMark](../script/rangemark.md) range)

Returns all matches for the query with the given `name` in the given `range`.

#### <a name="getAllJoined"></a>[RangeMark](../script/rangemark.md) **getAllJoined**(string name)

Returns a smallest range that contains all captures for the given `name`.

#### <a name="getInRange"></a>[RangeMark](../script/rangemark.md) **getInRange**(string name, [RangeMark](../script/rangemark.md) range)

Returns the first match for the query with the given `name` in the given `range`

#### <a name="queryIn"></a>array<[QueryMatch](../script/querymatch.md)> **queryIn**(capture, query)

Executes the treesitter `query` on all nodes that were captured under the `capture` name.

This is useful if you want to query for nodes that might be nested arbitrarily deeply within a larger construct.
E.g. searching for all "return" statements within a function, no matter how deep they are nested.
