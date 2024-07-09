# TextDocument

Document object for text files. [More...](#detailed-description)

```qml
import Knut
```

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
|int|**[selectionEnd](#selectionEnd)**|
|int|**[selectionStart](#selectionStart)**|
|string|**[text](#text)**|

Inherited properties: [Document properties](../knut/document.md#properties)

## Methods

| | Name |
|-|-|
||**[columnAtPosition](#columnAtPosition)**(int position)|
||**[copy](#copy)**()|
|[Mark](../knut/mark.md) |**[createMark](#createMark)**(int pos = -1)|
|[RangeMark](../knut/rangemark.md) |**[createRangeMark](#createRangeMark)**()|
|[RangeMark](../knut/rangemark.md) |**[createRangeMark](#createRangeMark)**(int from, int to)|
||**[cut](#cut)**()|
||**[deleteEndOfLine](#deleteEndOfLine)**()|
||**[deleteEndOfWord](#deleteEndOfWord)**()|
||**[deleteLine](#deleteLine)**(int line = -1)|
||**[deleteNextCharacter](#deleteNextCharacter)**(int count = 1)|
||**[deletePreviousCharacter](#deletePreviousCharacter)**(int count = 1)|
||**[deleteRange](#deleteRange)**([TextRange](../knut/textrange.md) range)|
||**[deleteRegion](#deleteRegion)**(int from, int to)|
||**[deleteSelection](#deleteSelection)**()|
||**[deleteStartOfLine](#deleteStartOfLine)**()|
||**[deleteStartOfWord](#deleteStartOfWord)**()|
|bool |**[find](#find)**(string text, int options = TextDocument.NoFindFlags)|
|bool |**[findRegexp](#findRegexp)**(string regexp, int options = TextDocument.NoFindFlags)|
||**[gotoEndOfDocument](#gotoEndOfDocument)**()|
||**[gotoEndOfLine](#gotoEndOfLine)**()|
||**[gotoEndOfWord](#gotoEndOfWord)**()|
||**[gotoLine](#gotoLine)**(int line, int column = 1)|
||**[gotoMark](#gotoMark)**([Mark](../knut/mark.md) mark)|
||**[gotoNextChar](#gotoNextChar)**(int count = 1)|
||**[gotoNextLine](#gotoNextLine)**(int count = 1)|
||**[gotoNextWord](#gotoNextWord)**(int count = 1)|
||**[gotoPreviousChar](#gotoPreviousChar)**(int count = 1)|
||**[gotoPreviousLine](#gotoPreviousLine)**(int count = 1)|
||**[gotoPreviousWord](#gotoPreviousWord)**(int count = 1)|
||**[gotoStartOfDocument](#gotoStartOfDocument)**()|
||**[gotoStartOfLine](#gotoStartOfLine)**()|
||**[gotoStartOfWord](#gotoStartOfWord)**()|
|bool |**[hasSelection](#hasSelection)**()|
||**[indent](#indent)**(int count)|
||**[indentationAtPosition](#indentationAtPosition)**(int pos)|
||**[insert](#insert)**(string text)|
||**[insertAtLine](#insertAtLine)**(string text, int line = -1)|
||**[insertAtPosition](#insertAtPosition)**(string text, int pos)|
||**[lineAtPosition](#lineAtPosition)**(int position)|
|bool |**[match](#match)**(string regexp, int options = TextDocument.NoFindFlags)|
||**[paste](#paste)**()|
||**[positionAt](#positionAt)**(int line, int col)|
||**[redo](#redo)**(int count)|
||**[remove](#remove)**(int length)|
||**[removeIndent](#removeIndent)**(int count)|
||**[replace](#replace)**(int length, string text)|
||**[replace](#replace)**([TextRange](../knut/textrange.md) range, string text)|
||**[replace](#replace)**(int from, int to, string text)|
|bool |**[replaceAll](#replaceAll)**(string before, string after, int options = TextDocument.NoFindFlags)|
|bool |**[replaceAllInRange](#replaceAllInRange)**(string before, string after, [RangeMark](../knut/rangemark.md) range, int options = TextDocument.NoFindFlags)|
|bool |**[replaceAllRegexp](#replaceAllRegexp)**(string regexp, string after, int options = TextDocument.NoFindFlags)|
|bool |**[replaceAllRegexpInRange](#replaceAllRegexpInRange)**(string regexp, string after, [RangeMark](../knut/rangemark.md) range, int options = TextDocument.NoFindFlags)|
|bool |**[replaceOne](#replaceOne)**(string before, string after, int options = TextDocument.NoFindFlags)|
||**[selectAll](#selectAll)**()|
||**[selectEndOfLine](#selectEndOfLine)**()|
||**[selectEndOfWord](#selectEndOfWord)**()|
||**[selectNextChar](#selectNextChar)**(int count = 1)|
||**[selectNextLine](#selectNextLine)**(int count = 1)|
||**[selectNextWord](#selectNextWord)**(int count = 1)|
||**[selectPreviousChar](#selectPreviousChar)**(int count = 1)|
||**[selectPreviousLine](#selectPreviousLine)**(int count = 1)|
||**[selectPreviousWord](#selectPreviousWord)**(int count = 1)|
||**[selectRange](#selectRange)**([TextRange](../knut/textrange.md) range)|
||**[selectRangeMark](#selectRangeMark)**([RangeMark](../knut/rangemark.md) mark)|
||**[selectRegion](#selectRegion)**(int from, int to)|
||**[selectStartOfLine](#selectStartOfLine)**(int count = 1)|
||**[selectStartOfWord](#selectStartOfWord)**()|
||**[selectTo](#selectTo)**(int pos)|
||**[selectToMark](#selectToMark)**([Mark](../knut/mark.md) mark)|
||**[undo](#undo)**(int count)|
||**[unselect](#unselect)**()|

Inherited methods: [Document methods](../knut/document.md#methods)

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

#### <a name="selectionEnd"></a>int **selectionEnd**

This property holds the end of the selection or position if the cursor doesn't have a selection.

#### <a name="selectionStart"></a>int **selectionStart**

This property holds the start of the selection or position if the cursor doesn't have a selection.

#### <a name="text"></a>string **text**

This property holds the text of the document.

## Method Documentation

#### <a name="columnAtPosition"></a>**columnAtPosition**(int position)

Returns the column number for the given text cursor `position`. Or -1 if position is invalid

#### <a name="copy"></a>**copy**()

Copies the selected text.

#### <a name="createMark"></a>[Mark](../knut/mark.md) **createMark**(int pos = -1)

Creates a mark at the given position `pos`. If `pos` is -1, it will create a mark at the
current position.

#### <a name="createRangeMark"></a>[RangeMark](../knut/rangemark.md) **createRangeMark**()

Creates a range mark from the current selection.

Note: if there is no selection, the range mark will span an empty range!

#### <a name="createRangeMark"></a>[RangeMark](../knut/rangemark.md) **createRangeMark**(int from, int to)

Creates a range mark from `from` to `to`.

#### <a name="cut"></a>**cut**()

Cuts the selected text.

#### <a name="deleteEndOfLine"></a>**deleteEndOfLine**()

Deletes from the cursor position to the end of the line.

#### <a name="deleteEndOfWord"></a>**deleteEndOfWord**()

Deletes from the cursor position to the end of the word.

#### <a name="deleteLine"></a>**deleteLine**(int line = -1)

Remove a the line `line`. If `line` is -1, remove the current line. `line` is 1-based.

#### <a name="deleteNextCharacter"></a>**deleteNextCharacter**(int count = 1)

Deletes the next `count` characters.

#### <a name="deletePreviousCharacter"></a>**deletePreviousCharacter**(int count = 1)

Deletes the previous `count` characters.

#### <a name="deleteRange"></a>**deleteRange**([TextRange](../knut/textrange.md) range)

Deletes the range passed in parameter.

#### <a name="deleteRegion"></a>**deleteRegion**(int from, int to)

Deletes the text between `from` and `to` positions.

#### <a name="deleteSelection"></a>**deleteSelection**()

Deletes the current selection, does nothing if no text is selected.

#### <a name="deleteStartOfLine"></a>**deleteStartOfLine**()

Deletes from the cursor position to the start of the line.

#### <a name="deleteStartOfWord"></a>**deleteStartOfWord**()

Deletes from the cursor position to the start of the word.

#### <a name="find"></a>bool **find**(string text, int options = TextDocument.NoFindFlags)

Searches the string `text` in the editor. Options could be a combination of:

- `TextDocument.FindBackward`: search backward
- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only complete words
- `TextDocument.FindRegexp`: use a regexp, equivalent to calling `findRegexp`

Selects the match and returns `true` if a match is found.

#### <a name="findRegexp"></a>bool **findRegexp**(string regexp, int options = TextDocument.NoFindFlags)

Searches the string `regexp` in the editor using a regular expression. Options could be a combination of:

- `TextDocument.FindBackward`: search backward
- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only complete words

Selects the match and returns `true` if a match is found.

#### <a name="gotoEndOfDocument"></a>**gotoEndOfDocument**()

Goes to the document end.

#### <a name="gotoEndOfLine"></a>**gotoEndOfLine**()

Goes to the end of the line.

#### <a name="gotoEndOfWord"></a>**gotoEndOfWord**()

Goes to the end of the word under the cursor.

#### <a name="gotoLine"></a>**gotoLine**(int line, int column = 1)

Goes to the given `line` and `column` in the editor. Lines and columns are 1-based.

#### <a name="gotoMark"></a>**gotoMark**([Mark](../knut/mark.md) mark)

Goes to the given `mark`.

#### <a name="gotoNextChar"></a>**gotoNextChar**(int count = 1)

Goes to the next character, repeat the operation `count` times.

#### <a name="gotoNextLine"></a>**gotoNextLine**(int count = 1)

Goes to the next line, repeat the operation `count` times.

#### <a name="gotoNextWord"></a>**gotoNextWord**(int count = 1)

Goes to the next word, repeat the operation `count` times.

#### <a name="gotoPreviousChar"></a>**gotoPreviousChar**(int count = 1)

Goes to the previous character, repeat the operation `count` times.

#### <a name="gotoPreviousLine"></a>**gotoPreviousLine**(int count = 1)

Goes to the previous line, repeat the operation `count` times.

#### <a name="gotoPreviousWord"></a>**gotoPreviousWord**(int count = 1)

Goes to the previous word, repeat the operation `count` times.

#### <a name="gotoStartOfDocument"></a>**gotoStartOfDocument**()

Goes to the document start.

#### <a name="gotoStartOfLine"></a>**gotoStartOfLine**()

Goes to the start of the line.

#### <a name="gotoStartOfWord"></a>**gotoStartOfWord**()

Goes to the start of the word under the cursor.

#### <a name="hasSelection"></a>bool **hasSelection**()

Returns true if the editor has a selection.

#### <a name="indent"></a>**indent**(int count)

Indents the current line `count` times. If there's a selection, indent all lines in the selection.

#### <a name="indentationAtPosition"></a>**indentationAtPosition**(int pos)

Returns the indentation at the given position.

#### <a name="insert"></a>**insert**(string text)

Inserts the string `text` at the current position. If some text is selected it will be replaced.

#### <a name="insertAtLine"></a>**insertAtLine**(string text, int line = -1)

Inserts the string `text` at `line`. If `line` is -1, insert the text at the current position. `line` is 1-based.

#### <a name="insertAtPosition"></a>**insertAtPosition**(string text, int pos)

Inserts the string `text` at `pos`.

#### <a name="lineAtPosition"></a>**lineAtPosition**(int position)

Returns the line number for the given text cursor `position`. Or -1 if position is invalid

#### <a name="match"></a>bool **match**(string regexp, int options = TextDocument.NoFindFlags)

Searches the string `regexp` in the editor using a regular expression. Options could be a combination of:

- `TextDocument.FindBackward`: search backward
- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only complete words

Selects the match and returns the named group if a match is found.

#### <a name="paste"></a>**paste**()

Pastes text in the clipboard.

#### <a name="positionAt"></a>**positionAt**(int line, int col)

Returns the text cursor position for the given `line` number and `column` number. Or -1 if position was not found

#### <a name="redo"></a>**redo**(int count)

Redo `count` times the last actions.

#### <a name="remove"></a>**remove**(int length)

Remove `length` character from the current position.

#### <a name="removeIndent"></a>**removeIndent**(int count)

Indents the current line `count` times. If there's a selection, indent all lines in the selection.

#### <a name="replace"></a>**replace**(int length, string text)

Replaces `length` characters from the current position with the string `text`.

#### <a name="replace"></a>**replace**([TextRange](../knut/textrange.md) range, string text)

Replaces the text in the range `range` with the string `text`.

#### <a name="replace"></a>**replace**(int from, int to, string text)

Replaces the text from `from` to `to` with the string `text`.

#### <a name="replaceAll"></a>bool **replaceAll**(string before, string after, int options = TextDocument.NoFindFlags)

Replaces all occurrences of the string `before` with `after`. Options could be a combination of:

- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only complete words
- `TextDocument.FindRegexp`: use a regexp, equivalent to calling `findRegexp`
- `TextDocument.PreserveCase`: preserve case when replacing

If the option `TextEditor.PreserveCase` is used, it means:

- All upper-case occurrences are replaced with the upper-case new text.
- All lower-case occurrences are replaced with the lower-case new text.
- Capitalized occurrences are replaced with the capitalized new text.
- Other occurrences are replaced with the new text as entered. If an occurrence and the new text have the same prefix
or suffix, then the case of the prefix and/or suffix are preserved, and the other rules are applied on the rest of
the occurrence only.

Returns the number of changes done in the document.

#### <a name="replaceAllInRange"></a>bool **replaceAllInRange**(string before, string after, [RangeMark](../knut/rangemark.md) range, int options = TextDocument.NoFindFlags)

Replaces all occurrences of the string `before` with `after` in the given `range`. See the
options from `replaceAll`.

Returns the number of changes done in the document.

#### <a name="replaceAllRegexp"></a>bool **replaceAllRegexp**(string regexp, string after, int options = TextDocument.NoFindFlags)

Replaces all occurrences of the matches for the `regexp` with `after`. See the options from `replaceAll`.

The captures coming from the regexp can be used in the replacement text, using `\1`..`\n` or `$1`..`$n`.

Returns the number of changes done in the document.

#### <a name="replaceAllRegexpInRange"></a>bool **replaceAllRegexpInRange**(string regexp, string after, [RangeMark](../knut/rangemark.md) range, int options = TextDocument.NoFindFlags)

Replaces all occurrences of the matches for the `regexp` with `after` in the given `range`. See the options from `replaceAll`.

The captures coming from the regexp can be used in the replacement text, using `\1`..`\n` or `$1`..`$n`.

Returns the number of changes done in the document.

#### <a name="replaceOne"></a>bool **replaceOne**(string before, string after, int options = TextDocument.NoFindFlags)

Replaces one occurrence of the string `before` with `after`. Options could be a combination of:

- `TextDocument.FindCaseSensitively`: match case
- `TextDocument.FindWholeWords`: match only complete words
- `TextDocument.FindRegexp`: use a regexp, equivalent to calling `findRegexp`
- `TextDocument.PreserveCase`: preserve case when replacing

If the option `TextEditor.PreserveCase` is used, it means:

- All upper-case occurrences are replaced with the upper-case new text.
- All lower-case occurrences are replaced with the lower-case new text.
- Capitalized occurrences are replaced with the capitalized new text.
- Other occurrences are replaced with the new text as entered. If an occurrence and the new text have the same prefix
or suffix, then the case of the prefix and/or suffix are preserved, and the other rules are applied on the rest of
the occurrence only.

Returns true if a change occurs in the document..

#### <a name="selectAll"></a>**selectAll**()

Selects all the text.

#### <a name="selectEndOfLine"></a>**selectEndOfLine**()

Selects the text from the current position to the end of the line.

#### <a name="selectEndOfWord"></a>**selectEndOfWord**()

Selects the text from the current position to the end of the word.

#### <a name="selectNextChar"></a>**selectNextChar**(int count = 1)

Selects the next character, repeat the operation `count` times.

#### <a name="selectNextLine"></a>**selectNextLine**(int count = 1)

Selects the next line, repeat the operation `count` times.

#### <a name="selectNextWord"></a>**selectNextWord**(int count = 1)

Selects the next word, repeat the operation `count` times.

#### <a name="selectPreviousChar"></a>**selectPreviousChar**(int count = 1)

Selects the previous character, repeat the operation `count` times.

#### <a name="selectPreviousLine"></a>**selectPreviousLine**(int count = 1)

Selects the previous line, repeat the operation `count` times.

#### <a name="selectPreviousWord"></a>**selectPreviousWord**(int count = 1)

Selects the previous word, repeat the operation `count` times.

#### <a name="selectRange"></a>**selectRange**([TextRange](../knut/textrange.md) range)

Selects the range passed in parameter.

#### <a name="selectRangeMark"></a>**selectRangeMark**([RangeMark](../knut/rangemark.md) mark)

Selects the text defined by the range make `mark`.


#### <a name="selectRegion"></a>**selectRegion**(int from, int to)

Selects the text between `from` and `to` positions.

#### <a name="selectStartOfLine"></a>**selectStartOfLine**(int count = 1)

Selects the text from the current position to the start of the line.

#### <a name="selectStartOfWord"></a>**selectStartOfWord**()

Selects the text from the current position to the start of the word.

#### <a name="selectTo"></a>**selectTo**(int pos)

Selects the text from the current position to `pos`.

#### <a name="selectToMark"></a>**selectToMark**([Mark](../knut/mark.md) mark)

Selects the text from the cursor position to the `mark`.

#### <a name="undo"></a>**undo**(int count)

Undo `count` times the last actions.

#### <a name="unselect"></a>**unselect**()

Clears the current selection.
