# Getting involved

## Contributions

Please submit your contributions or issue reports from our GitHub space at <https://github.com/KDAB/Knut>

## Compilation

To compile Knut, you need:

- a C++20 compiler (recent compilers should work)
- Qt 6.2 at least

All 3rd-party dependencies are set up as git submodules, make sure to run:

```
git submodule update --init --recursive
```

### Qt Creator compilation error on Windows

On Windows, if you use Qt Creator, you will have a compilation error like that:

```
[140/402 21.9/sec] Update documentation
FAILED: CMakeFiles/docs C:/dev/knut/knut/build-debug/CMakeFiles/docs
cmd.exe /C "cd /D C:\dev\knut\knut\build-debug && C:\dev\knut\knut\build-debug\bin\cpp2doc.exe"
```

This is due to a bug in Qt Creator: [QTCREATORBUG-29936](https://bugreports.qt.io/browse/QTCREATORBUG-29963).

## Code contributions

In order to contribute code, make sure to read the following paragraphs.

### pre-commit

Make sure to install the pre-commit hooks. Those are installed using [`pre-commit`](https://pre-commit.com/):

```
pip install pre-commit
pre-commit install --hook-type commit-msg
```

At your first commit, it will download a bunch of necessary files, then all checks will be done at every commit, before hitting the CI.

Code formatting is handled as part of the pre-commit checks using `clang-format`.

### Includes order

Includes order follow this simple rule:

```cpp
// All Knut includes, without any empty lines, using ""

// All external includes, without empty lines, using <>
```

For example:

```cpp
#include "codedocument.h"
#include "astnode.h"
#include "treesitter/predicates.h"
#include "utils/string_helper.h"

#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <kdalgorithms.h>
#include <memory>
```

Includes are automatically sorted by `clang-format`, don't worry about that.

## Documentation

Knut is using `mkdocs` for generating its documentation. Make sure to properly document all new API you add to Knut. During compilation, the internal tool `cpp2doc` will update automatically the documentation for the classes you updated.

For full documentation visit [mkdocs.org](https://www.mkdocs.org/).

### Installation

```
pip install mkdocs
pip install mkdocs-material
pip install mkdocs-build-plantuml-plugin
```

### Commands

- `mkdocs serve` - Start the live-reloading docs server.
- `mkdocs build` - Build the documentation site.
- `mkdocs -h` - Print help message and exit.
