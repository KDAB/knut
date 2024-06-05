# QtTsDocument

Provides access to the content of a Ts file (Qt linguist). [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|string|**[language](#language)**|
|array<QtUiMessage>|**[messages](#messages)**|

## Methods

| | Name |
|-|-|
||**[addMessage](#addMessage)**(string context, string location, string source, string translation)|
||**[setLanguage](#setLanguage)**(string lang)|

## Property Documentation

#### <a name="language"></a>string **language**

Return language name.

#### <a name="messages"></a>array<QtUiMessage> **messages**

List of all translations in the ts file.

## Method Documentation

#### <a name="addMessage"></a>**addMessage**(string context, string location, string source, string translation)

Add a new source text, its translation located in location within the given context.

#### <a name="setLanguage"></a>**setLanguage**(string lang)

Change language.
