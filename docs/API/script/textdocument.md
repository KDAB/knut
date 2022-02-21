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
|string|**[currentLine](#currentLine)**|
|string|**[currentWord](#currentWord)**|
|int|**[line](#line)**|
|int|**[lineCount](#lineCount)**|
|LineEnding|**[lineEnding](#lineEnding)**|
|int|**[position](#position)**|
|string|**[selectedText](#selectedText)**|
|string|**[text](#text)**|

Inherited properties: [Document properties](../script/document.md#properties)

## Methods

| | Name |
|-|-|
||**[copy](#copy)**()|
|[Mark](../script/mark.md) |**[createMark](#createMark)**(int pos = -1)|
||**[cut](#cut)**()|
||**[deleteEndOfLine](#deleteEndOfLine)**()|
||**[deleteEndOfWord](#deleteEndOfWord)**()|
||**[deleteNextCharacter](#deleteNextCharacter)**(int count)|
||**[deletePreviousCharacter](#deletePreviousCharacter)**(int count)|
||**[deleteRegion](#deleteRegion)**(int from, int to)|
||**[deleteSelection](#deleteSelection)**()|
||**[deleteStartOfLine](#deleteStartOfLine)**()|
||**[deleteStartOfWord](#deleteStartOfWord)**()|
|bool |**[find](#find)**(string text, int options = NoFindFlags)|
|bool |**[findRegexp](#findRegexp)**(string regexp, int options = NoFindFlags)|
||**[gotoDocumentEnd](#gotoDocumentEnd)**()|
||**[gotoDocumentStart](#gotoDocumentStart)**()|
||**[gotoLine](#gotoLine)**(int line, int column = 1)|
||**[gotoLineEnd](#gotoLineEnd)**()|
||**[gotoLineStart](#gotoLineStart)**()|
||**[gotoMark](#gotoMark)**([Mark](../script/mark.md) mark)|
||**[gotoNextCharacter](#gotoNextCharacter)**(int count = 1)|
||**[gotoNextLine](#gotoNextLine)**(int count = 1)|
||**[gotoNextWord](#gotoNextWord)**(int count = 1)|
||**[gotoPreviousCharacter](#gotoPreviousCharacter)**(int count = 1)|
||**[gotoPreviousLine](#gotoPreviousLine)**(int count = 1)|
||**[gotoPreviousWord](#gotoPreviousWord)**(int count = 1)|
||**[gotoWordEnd](#gotoWordEnd)**()|
||**[gotoWordStart](#gotoWordStart)**()|
|bool |**[hasSelection](#hasSelection)**()|
||**[paste](#paste)**()|
||**[redo](#redo)**()|
|bool |**[replaceAll](#replaceAll)**(string before, string after, int options = NoFindFlags)|
|bool |**[replaceAllRegexp](#replaceAllRegexp)**(string regexp, string after, int options = NoFindFlags)|
||**[selectAll](#selectAll)**()|
||**[selectLineEnd](#selectLineEnd)**()|
||**[selectLineStart](#selectLineStart)**(int count = 1)|
||**[selectNextCharacter](#selectNextCharacter)**(int count = 1)|
||**[selectNextLine](#selectNextLine)**(int count = 1)|
||**[selectNextWord](#selectNextWord)**(int count = 1)|
||**[selectPreviousCharacter](#selectPreviousCharacter)**(int count = 1)|
||**[selectPreviousLine](#selectPreviousLine)**(int count = 1)|
||**[selectPreviousWord](#selectPreviousWord)**(int count = 1)|
||**[selectRange](#selectRange)**([TextRange](../script/textrange.md) range)|
||**[selectTo](#selectTo)**(int pos)|
||**[selectToMark](#selectToMark)**([Mark](../script/mark.md) mark)|
||**[selectWordEnd](#selectWordEnd)**()|
||**[selectWordStart](#selectWordStart)**()|
||**[undo](#undo)**()|
||**[unselect](#unselect)**()|

Inherited methods: [Document methods](../script/document.md#methods)

## Property Documentation

#### <a name="column"></a>int **column**

This read-only property holds the column of the cursor position.
Be careful the column is 1-based, so the column before the first character is 1.

#### <a name="currentLine"></a>string **currentLine**

This read-only property return the line under the current position.

#### <a name="currentWord"></a>string **currentWord**

This read-only property return the word under the current position.

#### <a name="line"></a>int **line**

This read-only property holds the line of the cursor position.
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

## Method Documentation

#### <a name="copy"></a>**copy**()

Copies the selected text.

#### <a name="createMark"></a>[Mark](../script/mark.md) **createMark**(int pos = -1)

Create a mark at the given position `pos`. If `pos` is -1, it will create a mark at the
current position.

#### <a name="cut"></a>**cut**()

Cuts the selected text.

#### <a name="deleteEndOfLine"></a>**deleteEndOfLine**()

Deletes from the cursor position to the end of the line.

#### <a name="deleteEndOfWord"></a>**deleteEndOfWord**()

Deletes from the cursor position to the end of the word.

#### <a name="deleteNextCharacter"></a>**deleteNextCharacter**(int count)

Deletes the next `count` characters.

#### <a name="deletePreviousCharacter"></a>**deletePreviousCharacter**(int count)

Deletes the previous `count` characters.

#### <a name="deleteRegion"></a>**deleteRegion**(int from, int to)

Deletes the text between `from` and `to` positions.

#### <a name="deleteSelection"></a>**deleteSelection**()

Deletes the current selection, does nothing if no text is selected.

#### <a name="deleteStartOfLine"></a>**deleteStartOfLine**()

Deletes from the cursor position to the start of the line.

#### <a name="deleteStartOfWord"></a>**deleteStartOfWord**()

Deletes from the cursor position to the start of the word.

#### <a name="find"></a>bool **find**(string text, int options = NoFindFlags)

Searches the string `text` in the editor. Options could be a combination of:

- `TextEditor.FindBackward`: search backward
- `TextEditor.FindCaseSensitively`: match case
- `TextEditor.FindWholeWords`: match only complete words
- `TextEditor.FindRegexp`: use a regexp, equivalent to calling `findRegexp`

Selects the match and returns `true` if a match is found.

#### <a name="findRegexp"></a>bool **findRegexp**(string regexp, int options = NoFindFlags)

Searches the string `regexp` in the editor using a regular expression. Options could be a combination of:

- `TextEditor.FindBackward`: search backward
- `TextEditor.FindCaseSensitively`: match case
- `TextEditor.FindWholeWords`: match only complete words

Selects the match and returns `true` if a match is found.

#### <a name="gotoDocumentEnd"></a>**gotoDocumentEnd**()

Goes to the document end.

#### <a name="gotoDocumentStart"></a>**gotoDocumentStart**()

Goes to the document start.

#### <a name="gotoLine"></a>**gotoLine**(int line, int column = 1)

Goes to the given `line` and `column` in the editor. Lines and columns are 1-based.

#### <a name="gotoLineEnd"></a>**gotoLineEnd**()

Goes to the end of the line.

#### <a name="gotoLineStart"></a>**gotoLineStart**()

Goes to the start of the line.

#### <a name="gotoMark"></a>**gotoMark**([Mark](../script/mark.md) mark)

Go to the given `mark`.

#### <a name="gotoNextCharacter"></a>**gotoNextCharacter**(int count = 1)

Goes to the next character, repeat the operation `count` times.

#### <a name="gotoNextLine"></a>**gotoNextLine**(int count = 1)

Goes to the next line, repeat the operation `count` times.

#### <a name="gotoNextWord"></a>**gotoNextWord**(int count = 1)

Goes to the next word, repeat the operation `count` times.

#### <a name="gotoPreviousCharacter"></a>**gotoPreviousCharacter**(int count = 1)

Goes to the previous character, repeat the operation `count` times.

#### <a name="gotoPreviousLine"></a>**gotoPreviousLine**(int count = 1)

Goes to the previous line, repeat the operation `count` times.

#### <a name="gotoPreviousWord"></a>**gotoPreviousWord**(int count = 1)

Goes to the previous word, repeat the operation `count` times.

#### <a name="gotoWordEnd"></a>**gotoWordEnd**()

Goes to the end of the word under the cursor.

#### <a name="gotoWordStart"></a>**gotoWordStart**()

Goes to the start of the word under the cursor.

#### <a name="hasSelection"></a>bool **hasSelection**()

Returns true if the editor has a selection.

#### <a name="paste"></a>**paste**()

Pastes text in the clipboard.

#### <a name="redo"></a>**redo**()

Redo the last action.

#### <a name="replaceAll"></a>bool **replaceAll**(string before, string after, int options = NoFindFlags)

Replace all occurences of the string `before` with `after`. Options could be a combination of:

- `TextEditor.FindCaseSensitively`: match case
- `TextEditor.FindWholeWords`: match only complete words
- `TextEditor.FindRegexp`: use a regexp, equivalent to calling `findRegexp`
- `TextEditor.PreserveCase`: preserve case when replacing

If the option `TextEditor.PreserveCase` is used, it means:

- All upper-case occurrences are replaced with the upper-case new text.
- All lower-case occurrences are replaced with the lower-case new text.
- Capitalized occurrences are replaced with the capitalized new text.
- Other occurrences are replaced with the new text as entered. If an occurrence and the new text have the same prefix
or suffix, then the case of the prefix and/or suffix are preserved, and the other rules are applied on the rest of
the occurrence only.

Returns the number of changes done in the document.

#### <a name="replaceAllRegexp"></a>bool **replaceAllRegexp**(string regexp, string after, int options = NoFindFlags)

Replace all occurences of the matches for the `regexp` with `after`. See the options from `replaceAll`.

The captures coming from the regexp can be used in the replacement text, using `\1`..`\n` or `$1`..`$n`.

Returns the number of changes done in the document.

#### <a name="selectAll"></a>**selectAll**()

Selects all the text.

#### <a name="selectLineEnd"></a>**selectLineEnd**()

Selects the text from the current position to the end of the line.

#### <a name="selectLineStart"></a>**selectLineStart**(int count = 1)

Selects the text from the current position to the start of the line.

#### <a name="selectNextCharacter"></a>**selectNextCharacter**(int count = 1)

Selects the next character, repeat the operation `count` times.

#### <a name="selectNextLine"></a>**selectNextLine**(int count = 1)

Selects the next line, repeat the operation `count` times.

#### <a name="selectNextWord"></a>**selectNextWord**(int count = 1)

Selects the next word, repeat the operation `count` times.

#### <a name="selectPreviousCharacter"></a>**selectPreviousCharacter**(int count = 1)

Selects the previous character, repeat the operation `count` times.

#### <a name="selectPreviousLine"></a>**selectPreviousLine**(int count = 1)

Selectes the previous line, repeat the operation `count` times.

#### <a name="selectPreviousWord"></a>**selectPreviousWord**(int count = 1)

Selects the previous word, repeat the operation `count` times.

#### <a name="selectRange"></a>**selectRange**([TextRange](../script/textrange.md) range)

Selects the range passed in parameter.

#### <a name="selectTo"></a>**selectTo**(int pos)

Selects the text from the current position to `pos`.

#### <a name="selectToMark"></a>**selectToMark**([Mark](../script/mark.md) mark)

Select the text from the cursor position to the `mark`.

#### <a name="selectWordEnd"></a>**selectWordEnd**()

Selects the text from the current position to the end of the word.

#### <a name="selectWordStart"></a>**selectWordStart**()

Selects the text from the current position to the start of the word.

#### <a name="undo"></a>**undo**()

Undo the last action.

#### <a name="unselect"></a>**unselect**()

Clears the current selection.
