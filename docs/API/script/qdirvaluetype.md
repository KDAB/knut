# QDirValueType

Wrapper around the `QDir` class. [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|string|**[absolutePath](#absolutePath)**|
|string|**[canonicalPath](#canonicalPath)**|
|int|**[count](#count)**|
|string|**[dirName](#dirName)**|
|bool|**[exists](#exists)**|
|bool|**[isAbsolute](#isAbsolute)**|
|bool|**[isReadable](#isReadable)**|
|bool|**[isRelative](#isRelative)**|
|bool|**[isRoot](#isRoot)**|
|string|**[path](#path)**|

## Methods

| | Name |
|-|-|
|string |**[at](#at)**(int pos)|
|bool |**[cd](#cd)**(string dirName)|
|bool |**[cdUp](#cdUp)**()|
|array&lt;string> |**[entryList](#entryList)**(int filters, int sort)|
|array&lt;string> |**[entryList](#entryList)**(string nameFilter, int filters, int sort)|
|array&lt;string> |**[entryList](#entryList)**(array&lt;string> nameFilters, int filters, int sort)|
|bool |**[fileExists](#fileExists)**(string name)|
|bool |**[makeAbsolute](#makeAbsolute)**()|
|bool |**[mkdir](#mkdir)**(string dirName)|
|bool |**[mkpath](#mkpath)**(string dirPath)|
|bool |**[remove](#remove)**(string fileName)|
|bool |**[removeRecursively](#removeRecursively)**()|
|bool |**[rename](#rename)**(string oldName, string newName)|
|bool |**[rmdir](#rmdir)**(string dirName)|
|bool |**[rmpath](#rmpath)**(string dirPath)|

## Detailed Description

The `QDirValueType` is a wrapper around the `QDir` C++ class, check [QDir](https://doc.qt.io/qt-6/qdir.html)
documentation. It can only be created using [Dir](dir.md) singleton.

## Property Documentation

#### <a name="absolutePath"></a>string **absolutePath**

#### <a name="canonicalPath"></a>string **canonicalPath**

#### <a name="count"></a>int **count**

#### <a name="dirName"></a>string **dirName**

#### <a name="exists"></a>bool **exists**

#### <a name="isAbsolute"></a>bool **isAbsolute**

#### <a name="isReadable"></a>bool **isReadable**

#### <a name="isRelative"></a>bool **isRelative**

#### <a name="isRoot"></a>bool **isRoot**

#### <a name="path"></a>string **path**

## Method Documentation

#### <a name="at"></a>string **at**(int pos)

#### <a name="cd"></a>bool **cd**(string dirName)

#### <a name="cdUp"></a>bool **cdUp**()

#### <a name="entryList"></a>array&lt;string> **entryList**(int filters, int sort)<br/>array&lt;string> **entryList**(string nameFilter, int filters, int sort)<br/>array&lt;string> **entryList**(array&lt;string> nameFilters, int filters, int sort)

`filters` is a combination of (default is `Dir.NoFilter`):

- `Dir.Dirs`
- `Dir.Files`
- `Dir.Drives`
- `Dir.NoSymLinks`
- `Dir.AllEntries`
- `Dir.TypeMask`
- `Dir.Readable`
- `Dir.Writable`
- `Dir.Executable`
- `Dir.PermissionMask`
- `Dir.Modified`
- `Dir.Hidden`
- `Dir.System`
- `Dir.AccessMask`
- `Dir.AllDirs`
- `Dir.CaseSensitive`
- `Dir.NoDot`
- `Dir.NoDotDot`
- `Dir.NoDotAndDotDot`
- `Dir.NoFilter`

`sort` is a combination of (default is `Dir.NoSort`):

- `Dir.Name`
- `Dir.Time`
- `Dir.Size`
- `Dir.Unsorted`
- `Dir.SortByMask`
- `Dir.DirsFirst`
- `Dir.Reversed`
- `Dir.IgnoreCase`
- `Dir.DirsLast`
- `Dir.LocaleAware`
- `Dir.Type`
- `Dir.NoSory`

#### <a name="fileExists"></a>bool **fileExists**(string name)

#### <a name="makeAbsolute"></a>bool **makeAbsolute**()

#### <a name="mkdir"></a>bool **mkdir**(string dirName)

#### <a name="mkpath"></a>bool **mkpath**(string dirPath)

#### <a name="remove"></a>bool **remove**(string fileName)

#### <a name="removeRecursively"></a>bool **removeRecursively**()

#### <a name="rename"></a>bool **rename**(string oldName, string newName)

#### <a name="rmdir"></a>bool **rmdir**(string dirName)

#### <a name="rmpath"></a>bool **rmpath**(string dirPath)
