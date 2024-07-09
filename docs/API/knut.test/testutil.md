# TestUtil

Provides utility methods useful for testing. [More...](#detailed-description)

```qml
import Knut.Test
```

## Methods

| | Name |
|-|-|
||**[compareDirectories](#compareDirectories)**(string current, string expected)|
||**[compareFiles](#compareFiles)**(string file, string expected, bool eolLF = true)|
||**[createTestProjectFrom](#createTestProjectFrom)**(string path)|
||**[removeTestProject](#removeTestProject)**(string path)|

## Detailed Description

This class is mainly used by the [TestCase](testcase.md) object, to extract some information on the script.

It should not be used in normal scripts.

## Method Documentation

#### <a name="compareDirectories"></a>**compareDirectories**(string current, string expected)

Compares the two directories recursively, and return true if they are the same.

#### <a name="compareFiles"></a>**compareFiles**(string file, string expected, bool eolLF = true)

Compares the content of the two files, and return true if they are the same.
If `eolLF` true, compareFiles will change the EOL of files to LF for comparison.

#### <a name="createTestProjectFrom"></a>**createTestProjectFrom**(string path)

Creates a new recursive copy of the directory at `path` with the name `path`_test_data.
Returns the newly created directory path.

#### <a name="removeTestProject"></a>**removeTestProject**(string path)

Removes the `path` from the disk.
