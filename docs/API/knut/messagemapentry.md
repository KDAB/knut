# MessageMapEntry

Refers to a single entry within the `MessageMap` [More...](#detailed-description)

```qml
import Knut
```

## Properties

| | Name |
|-|-|
|bool|**[isValid](#isValid)**|
|string|**[name](#name)**|
|list<[RangeMark](../knut/rangemark.md)>|**[parameters](#parameters)**|
|[RangeMark](../knut/rangemark.md)|**[range](#range)**|

## Detailed Description

A MessageMapEntry is a single entry within the `MessageMap`.
It contains the name of the message as well as the list of parameters.

## Property Documentation

#### <a name="isValid"></a>bool **isValid**

Whether the struct refers to a real entry.
Possible reasons why a MessageMapEntry might not be valid:
- The entry was default-constructed.
- The document this entry refers to no longer exists.

Please note that an entry that has been deleted may still be valid, but all its ranges will be empty.

#### <a name="name"></a>string **name**

The name of the entry.

#### <a name="parameters"></a>list<[RangeMark](../knut/rangemark.md)> **parameters**

A list of `RangeMark` instances referring to each parameter of the entry.

#### <a name="range"></a>[RangeMark](../knut/rangemark.md) **range**

The range of the entire message call, including parameters.
Use this to select & delete this message from the MESSAGE_MAP entirely.
