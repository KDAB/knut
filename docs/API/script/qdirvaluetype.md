# QDirValueType

Wrapper around the \c QDir class. [More...](#detailed-description)

```qml
import Script 1.0
```

## Properties

- string **[path](#path)**
- string **[absolutePath](#absolutePath)**
- string **[canonicalPath](#canonicalPath)**
- string **[dirName](#dirName)**
- int **[count](#count)**
- bool **[isReadable](#isReadable)**
- bool **[exists](#exists)**
- bool **[isRoot](#isRoot)**
- bool **[isRelative](#isRelative)**
- bool **[isAbsolute](#isAbsolute)**

## Methods

- bool **[cd](#cd)**(string dirName)
- bool **[cdUp](#cdUp)**()
- string **[at](#at)**(int pos)
- array<string> **[entryList](#entryList)**(int filters, int sort)
- array<string> **[entryList](#entryList)**(string nameFilter, int filters, int sort)
- array<string> **[entryList](#entryList)**(array<string> nameFilters, int filters, int sort)
- bool **[mkdir](#mkdir)**(string dirName)
- bool **[rmdir](#rmdir)**(string dirName)
- bool **[mkpath](#mkpath)**(string dirPath)
- bool **[rmpath](#rmpath)**(string dirPath)
- bool **[removeRecursively](#removeRecursively)**()
- bool **[makeAbsolute](#makeAbsolute)**()
- bool **[remove](#remove)**(string fileName)
- bool **[rename](#rename)**(string oldName, string newName)
- bool **[fileExists](#fileExists)**(string name)

## Detailed Description

The QDirValueType is a wrapper around the \c QDir C++ class, check \c QDir documentation.
It can only be created using \c Dir.

## Property Documentation

#### <a name="path"></a>string **path**

#### <a name="absolutePath"></a>string **absolutePath**

#### <a name="canonicalPath"></a>string **canonicalPath**

#### <a name="dirName"></a>string **dirName**

#### <a name="count"></a>int **count**

#### <a name="isReadable"></a>bool **isReadable**

#### <a name="exists"></a>bool **exists**

#### <a name="isRoot"></a>bool **isRoot**

#### <a name="isRelative"></a>bool **isRelative**

#### <a name="isAbsolute"></a>bool **isAbsolute**

## Method Documentation

#### <a name="cd"></a>bool **cd**(string dirName)

#### <a name="cdUp"></a>bool **cdUp**()

#### <a name="at"></a>string **at**(int pos)

#### <a name="entryList"></a>array<string> **entryList**(int filters, int sort)<br/>array<string> **entryList**(string nameFilter, int filters, int sort)<br/>array<string> **entryList**(array<string> nameFilters, int filters, int sort)

#### <a name="mkdir"></a>bool **mkdir**(string dirName)

#### <a name="rmdir"></a>bool **rmdir**(string dirName)

#### <a name="mkpath"></a>bool **mkpath**(string dirPath)

#### <a name="rmpath"></a>bool **rmpath**(string dirPath)

#### <a name="removeRecursively"></a>bool **removeRecursively**()

#### <a name="makeAbsolute"></a>bool **makeAbsolute**()

#### <a name="remove"></a>bool **remove**(string fileName)

#### <a name="rename"></a>bool **rename**(string oldName, string newName)

#### <a name="fileExists"></a>bool **fileExists**(string name)
