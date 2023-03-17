# TestUtil

Provides utility methods useful for testing. [More...](#detailed-description)

```qml
import Script.Test 1.0
```

<table>
<tr><td>Since:</td><td>Knut 1.0</td></tr>
</table>

## Methods

| | Name |
|-|-|
||**[compareFiles](#compareFiles)**(string file, string expected, bool eolLF = true)|

## Detailed Description

This class is mainly used by the [TestCase](testcase.md) object, to extract some information on the script.

It should not be used in normal scripts.

## Method Documentation

#### <a name="compareFiles"></a>**compareFiles**(string file, string expected, bool eolLF = true)

Compares the content of the two files, and return true if they are the same.
If `eolLF` true, compareFiles will change the EOL of files to LF for comparison.
