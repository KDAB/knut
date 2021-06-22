# Project

Singleton for handling the current project. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>Knut 4.0</td></tr>
</table>

## Properties

| | Name |
|-|-|
|string|**[fileModel](#fileModel)**|
|string|**[root](#root)**|

## Methods

| | Name |
|-|-|
|array<string> |**[allFiles](#allFiles)**()|
|array<string> |**[allFilesWithExtension](#allFilesWithExtension)**(string extension)|

## Detailed Description

The `Project` object is not meant to open multiple projects, but only open one.

## Property Documentation

#### <a name="fileModel"></a>string **fileModel**

Qt model for the current root path. It is created on-demand.

#### <a name="root"></a>string **root**

Current root path of the project, this can be set only once.

## Method Documentation

#### <a name="allFiles"></a>array<string> **allFiles**()

Returns all files in the current project.

#### <a name="allFilesWithExtension"></a>array<string> **allFilesWithExtension**(string extension)

Returns all files with the `extension` given in the current project.
