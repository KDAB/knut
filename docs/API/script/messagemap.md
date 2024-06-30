# MessageMap

Message map in a MFC C++ document [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|string|**[className](#className)**|
|list<[MessageMapEntry](../script/messagemapentry.md)>|**[entries](#entries)**|
|bool|**[isValid](#isValid)**|
|[RangeMark](../script/rangemark.md)|**[range](#range)**|
|string|**[superClass](#superClass)**|

## Methods

| | Name |
|-|-|
|[MessageMapEntry](../script/messagemapentry.md) |**[get](#get)**(string name)|
|list<[MessageMapEntry](../script/messagemapentry.md)> |**[getAll](#getAll)**(string name)|

## Detailed Description

The `MessageMap` object represents the data contained in the MFC MessageMap.

## Property Documentation

#### <a name="className"></a>string **className**

The name of the class this message map belongs to.

#### <a name="entries"></a>list<[MessageMapEntry](../script/messagemapentry.md)> **entries**

All entries found in the MessageMap as `MessageMapEntry`.

#### <a name="isValid"></a>bool **isValid**

Whether the MessageMap refers to an actual message map.
If no MessageMap could be found, this will be false.

#### <a name="range"></a>[RangeMark](../script/rangemark.md) **range**

The entire range of the Message Map.
Can be used to select & delete the entire message map.

#### <a name="superClass"></a>string **superClass**

The name of the super class this class inherits from.

## Method Documentation

#### <a name="get"></a>[MessageMapEntry](../script/messagemapentry.md) **get**(string name)

Gets the first entry with the given `name`.
If no entry could be found, isValid will be false on the resulting MessageMapEntry.

#### <a name="getAll"></a>list<[MessageMapEntry](../script/messagemapentry.md)> **getAll**(string name)

Gets all entries with the given `name`.
