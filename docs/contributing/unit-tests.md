# Unit tests

There are 2 different kinds of unit-tests: C++ and script. THey are located in the `tests` directory, with all test data in the `test_data` directory.

## C++ unit-tests

Tests written using the QtTest module. There are some utility classes you can use:

- `Test::testDataPath()`: returns the test data folder path
- `Test::LogSilencers`: silence logs during the tests
```cpp
// Silence multiple logs
auto ls = Test::LogSilencers {"cpp_client", "cpp_server", "cpp_messages"};

// Silence the default log
Test::LogSilencers ls;
```


- `Test::FileTester`: create an original file based on the original one, and compare the result with an expected one

```cpp
Test::FileTester file(Test::testDataPath() + "/textdocument/foo_original.txt");
Core::TextDocument document;
document.load(file.fileName());
// do something with the document
QVERIFY(file.compare());
```

To use it, include `"common/test_utils.h".

## QML Unit-tests

Tests written using a QML scripts, they are launched by the `tst_knut.cpp` file:

```cpp
KNUT_TEST(settings)
KNUT_TEST(dir)
...
```

Each line will run a file from the `test_data` directory, for example the first one is looking for a `tst_settings.qml` file.

A QML test is using the module `Script.Test` and the item `TestCase`. Each function will be called automatically and returns true or false, depending of the test results.

```qml
import Script 1.0
import Script.Test 1.0

TestCase {
    name: "FileInfo"

    function endsWith(str,pattern) {
        return str.substr(str.length-pattern.length,pattern.length) === pattern
    }

    function startsWith(str,pattern) {
        return str.substr(0,pattern.length) === pattern
    }
    //...
}
```
