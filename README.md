# Compilation

To compile Knut, you need:
- a C++20 compiler (recent compilers should work)
- Qt 5.15

All 3rd-party are downloaded at configure time using Cmake `FetchContent`.

## Contribution

If you want to contribute, make sure to install the pre-commit hooks. Those are installed using [pre-commit](https://pre-commit.com/):

```
pip install pre-commit
pre-commit install
```

At your first commit, it will download a bunch of necessary files, then all checks will be done at every commit, before hitting the CI.

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
