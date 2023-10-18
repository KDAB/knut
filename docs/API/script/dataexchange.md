# DataExchange

DataExchange entries in a MFC C++ document. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.1</td></tr>
</table>

## Properties

| | Name |
|-|-|
|string|**[className](#className)**|
|list<[DataExchangeEntry](../script/dataexchangeentry.md)>|**[entries](#entries)**|
|[RangeMark](../script/rangemark.md)|**[range](#range)**|

## Detailed Description

The `DataExchange` object represents the data contained in the MFC `DoDataExchange` method.

## Property Documentation

#### <a name="className"></a>string **className**

The name of the class this data exchange belongs to.

#### <a name="entries"></a>list<[DataExchangeEntry](../script/dataexchangeentry.md)> **entries**

All entries found in the data exchange method as `DataExchangeEntry`.

#### <a name="range"></a>[RangeMark](../script/rangemark.md) **range**

The entire range of the `DoDataExchange` method.
