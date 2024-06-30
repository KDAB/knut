# QtTsDocument

Provides access to the content of a Ts file (Qt linguist). [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|string|**[language](#language)**|
|array&lt;QtUiMessage>|**[messages](#messages)**|
|string|**[sourceLanguage](#sourceLanguage)**|

## Methods

| | Name |
|-|-|
||**[addMessage](#addMessage)**(string context, string location, string source, string translation)|
||**[setLanguage](#setLanguage)**(string lang)|
||**[setSourceLanguage](#setSourceLanguage)**(string lang)|

## Property Documentation

#### <a name="language"></a>string **language**

Return language name.

#### <a name="messages"></a>array&lt;QtUiMessage> **messages**

List of all translations in the ts file.

#### <a name="sourceLanguage"></a>string **sourceLanguage**

Return source language name.

## Method Documentation

#### <a name="addMessage"></a>**addMessage**(string context, string location, string source, string translation)

Add a new source text, its translation located in location within the given context.

#### <a name="setLanguage"></a>**setLanguage**(string lang)

Change language.

#### <a name="setSourceLanguage"></a>**setSourceLanguage**(string lang)

Change source language.
