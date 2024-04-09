# Compilation

To compile Knut, you need:
- a C++20 compiler (recent compilers should work)
- Qt 6.2 at least

All 3rd-party dependencies are set up as git submodules.

## Qt Creator compilation error on Windows

On Windows, if you use Qt Creator, you will have a compilation error with KSyntaxHighlighting:

```
[1/183 103.8/sec] Generating index.katesyntax
FAILED: _deps/ksyntaxhighlighting-build/data/index.katesyntax C:/dev/knut/build-knut-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/_deps/ksyntaxhighlighting-build/data/index.katesyntax
cmd.exe /C "cd /D C:\dev\knut\build-knut-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\_deps\ksyntaxhighlighting-build\data && C:\dev\knut\build-knut-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\bin\katehighlightingindexer.exe C:/dev/knut/build-knut-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/_deps/ksyntaxhighlighting-build/data/index.katesyntax C:/dev/knut/build-knut-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/_deps/ksyntaxhighlighting-src/data/schema/language.xsd C:/dev/knut/build-knut-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/_deps/ksyntaxhighlighting-build/data/syntax-data.qrc"
```

The way to fix that is to open a console with Qt and Visual Studio setup, go to the Qt Creator build directory, and do a first compilation in command line with `ninja`.
Or setup the Qt environment before running Qt Creator.

## Contribution

If you want to contribute, make sure to install the pre-commit hooks. Those are installed using [pre-commit](https://pre-commit.com/):

```
pip install pre-commit
pre-commit install --hook-type commit-msg
```

At your first commit, it will download a bunch of necessary files, then all checks will be done at every commit, before hitting the CI.

> **Warning**: on Windows with gerrit, you need to edit the file `.git/hooks/commit-msg.legacy` and replace the first line `#!/bin/sh` with `#!/usr/bin/env bash`

# Usage

Knut is using [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) servers.
For C++, if you are using clangd, note that some tests are disabled for clangd version < 13.

# Documentation

For full documentation visit [mkdocs.org](https://www.mkdocs.org/).

## Installation

```
pip install mkdocs
pip install mkdocs-material
pip install mkdocs-build-plantuml-plugin
```

## Commands

- `mkdocs serve` - Start the live-reloading docs server.
- `mkdocs build` - Build the documentation site.
- `mkdocs -h` - Print help message and exit.
