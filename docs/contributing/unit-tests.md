# Unit tests

There are 2 different kinds of unit-tests: C++ and script. THey are located in the `tests` directory, with all test data in the `test_data` directory.

## C++ unit-tests

Tests written using the QtTest module.

### Utility classes

There are some utility classes you can use:

- `Test::testDataPath()`: returns the test data folder path
- `Test::FileTester`: create an original file based on the original one, and compare the result with an expected one

```cpp
Test::FileTester file(Test::testDataPath() + "/textdocument/foo_original.txt");
Core::TextDocument document;
document.load(file.fileName());
// do something with the document
QVERIFY(file.compare());
```

To use it, include `"common/test_utils.h".

If you use the `FileTester` with LSP, make sure to close the LSP server (the `KnutCore` object must be destroyed for that):

```cpp
Test::FileTester file(Test::testDataPath() + "/cpp-project/section_original.cpp");
{
    Core::KnutCore core;
    auto project = Core::Project::instance();
    //...
} // the core is destroyed before the FileTester
```

### Enabling or disabling tests

Some tests can't be run without `clangd` or with a version of `clangd` older than 13.0.
In this case, you can add some macro at the beginning of the tests:

- `CHECK_CLANGD` to skip the test if there are no `clangd`
- `CHECK_CLANGD_VERSION` to skip the test if there are no `clangd` or an obsolete one


## QML Unit-tests

Tests written using a QML scripts, they are launched by the `tst_knut.cpp` file:

```cpp
KNUT_TEST(settings)
KNUT_TEST(dir)
...
```

Each line will run a file from the `test_data` directory, for example the first one is looking for a `tst_settings.qml` file.

A QML test is using the module `Knut.Test` and the item `TestCase`. Each function will be called automatically and returns true or false, depending of the test results.

```qml
import Knut
import Knut.Test

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
