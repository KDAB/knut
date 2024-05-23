# Project

Singleton for handling the current project. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|string|**[currentDocument](#currentDocument)**|
|array<string>|**[documents](#documents)**|
|string|**[root](#root)**|

## Methods

| | Name |
|-|-|
|array<string> |**[allFiles](#allFiles)**(PathType type = RelativeToRoot)|
|array<string> |**[allFilesWithExtension](#allFilesWithExtension)**(string extension, PathType type = RelativeToRoot)|
|array<string> |**[allFilesWithExtensions](#allFilesWithExtensions)**(array<string> extensions, PathType type = RelativeToRoot)|
||**[closeAll](#closeAll)**()|
|[Document](../script/document.md) |**[get](#get)**(string fileName)|
|[Document](../script/document.md) |**[open](#open)**(string fileName)|
||**[openPrevious](#openPrevious)**(int index)|
||**[saveAllDocuments](#saveAllDocuments)**()|

## Detailed Description

The `Project` object is not meant to open multiple projects, but only open one.

## Property Documentation

#### <a name="currentDocument"></a>string **currentDocument**

Current document opened in the project.

#### <a name="documents"></a>array<string> **documents**

Return all documents opened in the project.

#### <a name="root"></a>string **root**

Current root path of the project, this can be set only once.

## Method Documentation

#### <a name="allFiles"></a>array<string> **allFiles**(PathType type = RelativeToRoot)

Returns all files in the current project.
`type` defines the type of path, and can be one of those values:

- `Project.FullPath`
- `Project.RelativeToRoot`

#### <a name="allFilesWithExtension"></a>array<string> **allFilesWithExtension**(string extension, PathType type = RelativeToRoot)

Returns all files with the `extension` given in the current project.
`type` defines the type of path, and can be one of those values:

- `Project.FullPath`
- `Project.RelativeToRoot`

#### <a name="allFilesWithExtensions"></a>array<string> **allFilesWithExtensions**(array<string> extensions, PathType type = RelativeToRoot)

Returns all files with an extension from `extensions` in the current project.
`type` defines the type of path, and can be one of those values:

- `Project.FullPath`
- `Project.RelativeToRoot`

#### <a name="closeAll"></a>**closeAll**()

Close all documents. If the document has some changes, save the changes.

#### <a name="get"></a>[Document](../script/document.md) **get**(string fileName)

Get the document for the given `fileName`. If the document is not opened yet, open it. If the document already
exists, returns the same instance, a document can't be open twice. If the fileName is relative, use the root path as
the base.

#### <a name="open"></a>[Document](../script/document.md) **open**(string fileName)

Opens a document for the given `fileName`. If the document already exists, returns the same instance, a document
can't be open twice. If the fileName is relative, use the root path as the base.

#### <a name="openPrevious"></a>**openPrevious**(int index)

Open a previously opened document. `index` is the position of this document in the last opened document.

`document.openPrevious(1)` (the default) opens the last document, like Ctrl+Tab in any editors.

#### <a name="saveAllDocuments"></a>**saveAllDocuments**()

Save all Documents opened in project.
