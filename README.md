# Compilation

To compile Knut, you need:
- a C++20 compiler (recent compilers should work)
- Qt 5.15

All 3rd-party are downloaded at configure time using Cmake `FetchContent`.

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

