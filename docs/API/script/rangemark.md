# RangeMark

Keeps track of a range within a text document. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.1</td></tr>
</table>

## Properties

| | Name |
|-|-|
|bool|**[isValid](#isValid)**|
|bool|**[start](#start)**|
|bool|**[text](#text)**|

## Methods

| | Name |
|-|-|
|[RangeMark](../script/rangemark.md) |**[join](#join)**([RangeMark](../script/rangemark.md) other)|
||**[remove](#remove)**()|
||**[replace](#replace)**(string text)|
||**[select](#select)**()|

## Detailed Description

The RangeMark object helps you track a logical range within a file.
It's `start` and `end` properties will change as text is deleted or inserted before or even within the range.

A RangeMark is always created by a [TextDocument](textdocument.md).

## Property Documentation

#### <a name="isValid"></a>bool **isValid**

This read-only property indicates if the RangeMark is valid.

#### <a name="start"></a>bool **start**

These read-only properties holds the start, end and length of the range. They will be updated
as the text of the TextDocument changes.

#### <a name="text"></a>bool **text**

This read-only property returns the text covered by the range.

## Method Documentation

#### <a name="join"></a>[RangeMark](../script/rangemark.md) **join**([RangeMark](../script/rangemark.md) other)

Joins the two `RangeMark` and creates a new one.

The new `RangeMark` is spaning from the minimum of the start to the maximum of the end.

#### <a name="remove"></a>**remove**()

Deletes the text defined by this range in the source document.

#### <a name="replace"></a>**replace**(string text)

Replaces the text defined by this range with the `text` string in the source document.

#### <a name="select"></a>**select**()

Selects the text defined by this range in the source document.
