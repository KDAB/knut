# File

Singleton with methods to handle files. [More...](#detailed-description)

```qml
import Script
```

## Methods

| | Name |
|-|-|
|bool |**[copy](#copy)**(string fileName, string newName)|
|bool |**[exists](#exists)**(string fileName)|
|string |**[readAll](#readAll)**(string fileName)|
|bool |**[remove](#remove)**(string fileName)|
|bool |**[rename](#rename)**(string oldName, string newName)|
|bool |**[touch](#touch)**(string fileName)|

## Detailed Description

The `File` singleton implements most of the static methods from `QFile`, check
[QFile](https://doc.qt.io/qt-6/qfile.html) documentation.

## Method Documentation

#### <a name="copy"></a>bool **copy**(string fileName, string newName)

#### <a name="exists"></a>bool **exists**(string fileName)

#### <a name="readAll"></a>string **readAll**(string fileName)

#### <a name="remove"></a>bool **remove**(string fileName)

#### <a name="rename"></a>bool **rename**(string oldName, string newName)

#### <a name="touch"></a>bool **touch**(string fileName)
