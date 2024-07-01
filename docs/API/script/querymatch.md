# QueryMatch

Contains all captures for a query match. [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|array&lt;[QueryCapture](../script/querycapture.md)>|**[captures](#captures)**|
|bool|**[isEmpty](#isEmpty)**|

## Methods

| | Name |
|-|-|
|[RangeMark](../script/rangemark.md) |**[get](#get)**(string name)|
|vector<[RangeMark](../script/rangemark.md)> |**[getAll](#getAll)**(string name)|
|vector<[RangeMark](../script/rangemark.md)> |**[getAllInRange](#getAllInRange)**(string name, [RangeMark](../script/rangemark.md) range)|
|[RangeMark](../script/rangemark.md) |**[getAllJoined](#getAllJoined)**(string name)|
|[RangeMark](../script/rangemark.md) |**[getInRange](#getInRange)**(string name, [RangeMark](../script/rangemark.md) range)|
|array&lt;[QueryMatch](../script/querymatch.md)> |**[queryIn](#queryIn)**(capture, query)|

## Detailed Description

The QueryMatch object allows you to get access to all the captures made by a [Tree-sitter
query](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries).

Some high-level functions on CodeDocument and its subclasses also return QueryMatch instances.
Usually these functions list which captures their matches will include.

!!! note
    If you expect a query will only return a single QueryMatch, you can uses Javascripts
    [destructuring assignment][destructuring] to easily get the right match:
    ``` javascript
    // Note the [] surrounding `match`
    let [match] = document.query("...");
    if (match) { // In case the query fails, match will be undefined.
        // ...
    }
    ```
 [destructuring]:
https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Destructuring_assignment?retiredLocale=de

## Property Documentation

#### <a name="captures"></a>array&lt;[QueryCapture](../script/querycapture.md)> **captures**

List of all the captures in the current document.

This allows you to get access to both the [range](./rangemark.md) and the name of the capture.

!!! note
     Usually you won't need to access the captures directly.
     Instead prefer to use the getter functions.

#### <a name="isEmpty"></a>bool **isEmpty**

Return true if the `QueryMatch` is empty.

## Method Documentation

#### <a name="get"></a>[RangeMark](../script/rangemark.md) **get**(string name)

Returns the range covered by the first capture with the given `name`.

This allows you to easily interact with a capture, if you know it will only cover a single node.
``` javascript
let [function] = document.query("...");

// Print the captured text
Message.log(match.get("parameter-list").text);
// Replace the captured text with something else
match.get("parameter-list").replace("(int myParameter)");
```

See the [RangeMark](rangemark.md) documentation for more information.

#### <a name="getAll"></a>vector<[RangeMark](../script/rangemark.md)> **getAll**(string name)

Returns all ranges that are covered by the captures of the given `name`

#### <a name="getAllInRange"></a>vector<[RangeMark](../script/rangemark.md)> **getAllInRange**(string name, [RangeMark](../script/rangemark.md) range)

Returns all ranges that are covered by the captures of the given `name` in the given `range`.

#### <a name="getAllJoined"></a>[RangeMark](../script/rangemark.md) **getAllJoined**(string name)

Returns a smallest range that contains all captures for the given `name`.

#### <a name="getInRange"></a>[RangeMark](../script/rangemark.md) **getInRange**(string name, [RangeMark](../script/rangemark.md) range)

Returns the range covered by the first capture with the given `name` in the given `range`.

#### <a name="queryIn"></a>array&lt;[QueryMatch](../script/querymatch.md)> **queryIn**(capture, query)

Executes the treesitter `query` on all nodes that were captured under the `capture` name.

This is useful if you want to query for nodes that might be nested arbitrarily deeply within a larger construct.

E.g. To search for all "return" statements within a function, no matter how deep they are nested:
``` javascript
let [function] = document.query(`
     (function_definition
         declarator: (
             ; Some query to find a specific function
         )
         body: (compound_statement) @body)
`);
let return_statements = function.queryIn("body", "(return_statement) @return");
```
