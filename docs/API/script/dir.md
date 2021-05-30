# Dir

Singleton with methods to handle directories. [More...](#detailed-description)

```qml
import Script 1.0
```

## Properties

- char **[separator](#separator)**
- string **[currentPath](#currentPath)**
- string **[currentScriptPath](#currentScriptPath)**
- string **[homePath](#homePath)**
- string **[rootPath](#rootPath)**
- string **[tempPath](#tempPath)**

## Methods

- string **[toNativeSeparators](#toNativeSeparators)**(string pathName)
- string **[fromNativeSeparators](#fromNativeSeparators)**(string pathName)
- bool **[isRelativePath](#isRelativePath)**(string path)
- QDirValueType **[current](#current)**()
- QDirValueType **[currentScript](#currentScript)**()
- QDirValueType **[home](#home)**()
- QDirValueType **[root](#root)**()
- QDirValueType **[temp](#temp)**()
- bool **[match](#match)**(array<string> filters, string fileName)
- bool **[match](#match)**(string filter, string fileName)
- string **[cleanPath](#cleanPath)**(string path)
- QDirValueType **[create](#create)**(string path)

## Detailed Description

The Dir singleton implements most of the static methods from \c QDir, check \c QDir documentation.

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

#### <a name="current"></a>QDirValueType **current**()

#### <a name="currentScript"></a>QDirValueType **currentScript**()

#### <a name="home"></a>QDirValueType **home**()

#### <a name="root"></a>QDirValueType **root**()

#### <a name="temp"></a>QDirValueType **temp**()

#### <a name="match"></a>bool **match**(array<string> filters, string fileName)<br/>bool **match**(string filter, string fileName)

#### <a name="cleanPath"></a>string **cleanPath**(string path)

#### <a name="create"></a>QDirValueType **create**(string path)
