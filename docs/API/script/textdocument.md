# TextDocument

Document object for text files. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 4.0</td></tr>
<tr><td>Inherits:</td><td><a href="Document.html">Document</a></td></tr>
</table>

## Properties

| | Name |
|-|-|
|int|**[column](#column)**|
|int|**[line](#line)**|
|int|**[lineCount](#lineCount)**|
|LineEnding|**[lineEnding](#lineEnding)**|
|int|**[position](#position)**|
|string|**[selectedText](#selectedText)**|
|string|**[text](#text)**|

Inherited properties: [Document properties](../script/document.md#properties)

## Methods


Inherited methods: [Document methods](../script/document.md#methods)

## Property Documentation

#### <a name="column"></a>int **column**

This read-only property hold the column of the cursor position.
Be careful the column is 1-based, so the column before the first character is 1.

#### <a name="line"></a>int **line**

This read-only property hold the line of the cursor position.
Be careful the line is 1-based, so the first line of the document is 1,

#### <a name="lineCount"></a>int **lineCount**

This read-only property holds the number of lines in the document.

#### <a name="lineEnding"></a>LineEnding **lineEnding**

This property holds the line ending for the document. It can be one of those choices:

- `TextDocument.LFLineEnding`: '\n' character
- `TextDocument.CRLFLineEnding`: '\r\n' characters
- `TextDocument.NativeLineEnding`: LF on Linux and Mac, CRLF on Windows

Native is the default for new documents.

#### <a name="position"></a>int **position**

This property holds the absolute position of the cursor inside the text document.

#### <a name="selectedText"></a>string **selectedText**

This property holds the selected text of the document.

#### <a name="text"></a>string **text**

This property holds the text of the document.
