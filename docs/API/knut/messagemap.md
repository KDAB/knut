# MessageMap

Message map in a MFC C++ document [More...](#detailed-description)

```qml
import Knut
```

## Properties

| | Name |
|-|-|
|string|**[className](#className)**|
|list<[MessageMapEntry](../knut/messagemapentry.md)>|**[entries](#entries)**|
|bool|**[isValid](#isValid)**|
|[RangeMark](../knut/rangemark.md)|**[range](#range)**|
|string|**[superClass](#superClass)**|

## Methods

| | Name |
|-|-|
|[MessageMapEntry](../knut/messagemapentry.md) |**[get](#get)**(string name)|
|list<[MessageMapEntry](../knut/messagemapentry.md)> |**[getAll](#getAll)**(string name)|

## Detailed Description

The `MessageMap` object represents the data contained in the MFC MessageMap.

## Property Documentation

#### <a name="className"></a>string **className**

The name of the class this message map belongs to.

#### <a name="entries"></a>list<[MessageMapEntry](../knut/messagemapentry.md)> **entries**

All entries found in the MessageMap as `MessageMapEntry`.

#### <a name="isValid"></a>bool **isValid**

Whether the MessageMap refers to an actual message map.
If no MessageMap could be found, this will be false.

#### <a name="range"></a>[RangeMark](../knut/rangemark.md) **range**

The entire range of the Message Map.
Can be used to select & delete the entire message map.

#### <a name="superClass"></a>string **superClass**

The name of the super class this class inherits from.

## Method Documentation

#### <a name="get"></a>[MessageMapEntry](../knut/messagemapentry.md) **get**(string name)

Gets the first entry with the given `name`.
If no entry could be found, isValid will be false on the resulting MessageMapEntry.

#### <a name="getAll"></a>list<[MessageMapEntry](../knut/messagemapentry.md)> **getAll**(string name)

Gets all entries with the given `name`.
