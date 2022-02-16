# Getting started

## Development

If you want to collaborate, please checkout the `knut` repository on gerrit:

```
git clone ssh://codereview.kdab.com:29418/kdab/knut
```

The code layout is like this:

- `docs`: current documention
- `scripts`: available scripts known by knut
- `src`: source of the application
- `tests`: unit-tests for the application
- `test_data`: data used by tests
- `tools`: specific tools written for knut

## Compilation

To compile Knut, you need:

- a C++20 compiler (recent compilers should work)
- Qt 5.15
- `clang-format` in the path (needed by KSyntaxHighlighting)

All 3rd-party are downloaded at configure time using cmake `FetchContent`.


## Usage

Knut is using [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) servers.

For C++, the default LSP server is clangd. The recommanded version is clangd >= 13, some tests are disabled otherwise.


## Documentation

The documention is done using [mkdocs.org](https://www.mkdocs.org/). To create the documentation, follow instructions:

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
