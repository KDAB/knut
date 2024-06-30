# Dir

Singleton with methods to handle directories. [More...](#detailed-description)

```qml
import Script
```

## Properties

| | Name |
|-|-|
|string|**[currentPath](#currentPath)**|
|string|**[currentScriptPath](#currentScriptPath)**|
|string|**[homePath](#homePath)**|
|string|**[rootPath](#rootPath)**|
|char|**[separator](#separator)**|
|string|**[tempPath](#tempPath)**|

## Methods

| | Name |
|-|-|
|string |**[cleanPath](#cleanPath)**(string path)|
|[QDirValueType](../script/qdirvaluetype.md) |**[create](#create)**(string path)|
|[QDirValueType](../script/qdirvaluetype.md) |**[current](#current)**()|
|[QDirValueType](../script/qdirvaluetype.md) |**[currentScript](#currentScript)**()|
|string |**[fromNativeSeparators](#fromNativeSeparators)**(string pathName)|
|[QDirValueType](../script/qdirvaluetype.md) |**[home](#home)**()|
|bool |**[isAbsolutePath](#isAbsolutePath)**(string path)|
|bool |**[isRelativePath](#isRelativePath)**(string path)|
|bool |**[match](#match)**(array&lt;string> filters, string fileName)|
|bool |**[match](#match)**(string filter, string fileName)|
|[QDirValueType](../script/qdirvaluetype.md) |**[root](#root)**()|
|[QDirValueType](../script/qdirvaluetype.md) |**[temp](#temp)**()|
|string |**[toNativeSeparators](#toNativeSeparators)**(string pathName)|

## Detailed Description

The `Dir` singleton implements most of the static methods from `QDir`, check [QDir](https://doc.qt.io/qt-6/qdir.html)
documentation.

## Property Documentation

#### <a name="currentPath"></a>string **currentPath**

#### <a name="currentScriptPath"></a>string **currentScriptPath**

#### <a name="homePath"></a>string **homePath**

#### <a name="rootPath"></a>string **rootPath**

#### <a name="separator"></a>char **separator**

#### <a name="tempPath"></a>string **tempPath**

## Method Documentation

#### <a name="cleanPath"></a>string **cleanPath**(string path)

#### <a name="create"></a>[QDirValueType](../script/qdirvaluetype.md) **create**(string path)

#### <a name="current"></a>[QDirValueType](../script/qdirvaluetype.md) **current**()

#### <a name="currentScript"></a>[QDirValueType](../script/qdirvaluetype.md) **currentScript**()

#### <a name="fromNativeSeparators"></a>string **fromNativeSeparators**(string pathName)

#### <a name="home"></a>[QDirValueType](../script/qdirvaluetype.md) **home**()

#### <a name="isAbsolutePath"></a>bool **isAbsolutePath**(string path)

#### <a name="isRelativePath"></a>bool **isRelativePath**(string path)

#### <a name="match"></a>bool **match**(array&lt;string> filters, string fileName)<br/>bool **match**(string filter, string fileName)

#### <a name="root"></a>[QDirValueType](../script/qdirvaluetype.md) **root**()

#### <a name="temp"></a>[QDirValueType](../script/qdirvaluetype.md) **temp**()

#### <a name="toNativeSeparators"></a>string **toNativeSeparators**(string pathName)
