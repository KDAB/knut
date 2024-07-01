# Document

Base class for all documents [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|string|**[errorString](#errorString)**|
|bool|**[exists](#exists)**|
|string|**[fileName](#fileName)**|
|bool|**[hasChanged](#hasChanged)**|
|Type|**[type](#type)**|

## Methods

| | Name |
|-|-|
|bool |**[close](#close)**()|
|bool |**[load](#load)**(string fileName)|
|bool |**[save](#save)**()|
|bool |**[saveAs](#saveAs)**(string fileName)|

## Detailed Description

The `Document` class is the base class for all documents.
A document is a file loaded by Knut and that can be used in script (either to get data or to edit).

## Property Documentation

#### <a name="errorString"></a>string **errorString**

Returns the error string if an error occurred while loading the document, otherwise returns an empty string.

#### <a name="exists"></a>bool **exists**

Returns true if the document is a file on the disk, otherwise returns false.

#### <a name="fileName"></a>string **fileName**

Filename of the current document, changing it will load a new file **without changing the type**. It's better to use
a new `Document` to open a new file.

Changing the filename will:
- save the current document automatically
- load the new document
- put an error in `errorString` if it can't be loaded

#### <a name="hasChanged"></a>bool **hasChanged**

Returns true if the document has been edited, otherwise returns false.

#### <a name="type"></a>Type **type**

Returns the current type of the document, please note that the type is fixed once, and won't change. Available types
are:

- `Document.Text`
- `Document.Rc`

## Method Documentation

#### <a name="close"></a>bool **close**()

Close the current document. If the current document has some changes, save them
automatically.

#### <a name="load"></a>bool **load**(string fileName)

Load the document `fileName` **without changing the type**. If the current document has some changes, save them
automatically. In case of error put the error text in the `errorString` property.

#### <a name="save"></a>bool **save**()

Save the current document, in case of error put the error text in the `errorString` property.

#### <a name="saveAs"></a>bool **saveAs**(string fileName)

Save the current document as fileName, the previous file (if it exists) is not changed, and the current document
takes the new `fileName`. In case of error put the error text in the `errorString` property.
