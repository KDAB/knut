# Dir

Singleton with methods to handle directories. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>knut 4.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|char|**[separator](#separator)**|
|string|**[currentPath](#currentPath)**|
|string|**[currentScriptPath](#currentScriptPath)**|
|string|**[homePath](#homePath)**|
|string|**[rootPath](#rootPath)**|
|string|**[tempPath](#tempPath)**|

## Methods

| | Name |
|-|-|
|string |**[toNativeSeparators](#toNativeSeparators)**(string pathName)|
|string |**[fromNativeSeparators](#fromNativeSeparators)**(string pathName)|
|bool |**[isRelativePath](#isRelativePath)**(string path)|
|[QDirValueType](../script/qdirvaluetype.md) |**[current](#current)**()|
|[QDirValueType](../script/qdirvaluetype.md) |**[currentScript](#currentScript)**()|
|[QDirValueType](../script/qdirvaluetype.md) |**[home](#home)**()|
|[QDirValueType](../script/qdirvaluetype.md) |**[root](#root)**()|
|[QDirValueType](../script/qdirvaluetype.md) |**[temp](#temp)**()|
|bool |**[match](#match)**(array<string> filters, string fileName)|
|bool |**[match](#match)**(string filter, string fileName)|
|string |**[cleanPath](#cleanPath)**(string path)|
|[QDirValueType](../script/qdirvaluetype.md) |**[create](#create)**(string path)|

## Detailed Description

The `Dir` singleton implements most of the static methods from `QDir`, check [QDir](https://doc.qt.io/qt-5/qdir.html)
documentation.

## Property Documentation

#### <a name="separator"></a>char **separator**

#### <a name="currentPath"></a>string **currentPath**

#### <a name="currentScriptPath"></a>string **currentScriptPath**

#### <a name="homePath"></a>string **homePath**

#### <a name="rootPath"></a>string **rootPath**

#### <a name="tempPath"></a>string **tempPath**

## Method Documentation

#### <a name="toNativeSeparators"></a>string **toNativeSeparators**(string pathName)

#### <a name="fromNativeSeparators"></a>string **fromNativeSeparators**(string pathName)

#### <a name="isRelativePath"></a>bool **isRelativePath**(string path)

#### <a name="current"></a>[QDirValueType](../script/qdirvaluetype.md) **current**()

#### <a name="currentScript"></a>[QDirValueType](../script/qdirvaluetype.md) **currentScript**()

#### <a name="home"></a>[QDirValueType](../script/qdirvaluetype.md) **home**()

#### <a name="root"></a>[QDirValueType](../script/qdirvaluetype.md) **root**()

#### <a name="temp"></a>[QDirValueType](../script/qdirvaluetype.md) **temp**()

#### <a name="match"></a>bool **match**(array<string> filters, string fileName)<br/>bool **match**(string filter, string fileName)

#### <a name="cleanPath"></a>string **cleanPath**(string path)

#### <a name="create"></a>[QDirValueType](../script/qdirvaluetype.md) **create**(string path)
