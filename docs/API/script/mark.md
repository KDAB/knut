# Mark

Keeps track of a position in a text document. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut Script 1.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|bool|**[isValid](#isValid)**|
|bool|**[line](#line)**|

## Methods

| | Name |
|-|-|
||**[restore](#restore)**()|

## Detailed Description

The Mark object helps you track a logical possition in a file.
Its line and column properties will change as text are deleted or inserted before the mark.

## Property Documentation

#### <a name="isValid"></a>bool **isValid**

This read-only property indicates if the mark is valid.

#### <a name="line"></a>bool **line**

Those read-only properties holds the position of the mark. They will be updated if text is
deleted or inserted.

## Method Documentation

#### <a name="restore"></a>**restore**()

Returns the cursor position in the editor to the position saved by this object.
