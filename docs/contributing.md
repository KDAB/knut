# Contributing

Feel free to contribute code, scripts, documentation...

## Development

If you want to collaborate, please checkout the `knut` repository on gerrit:

```
git clone ssh://codereview.kdab.com:29418/kdab/knut
```

The code layout is like this:

- `docs`: current documention
- `scripts`: available scripts known by knut
- `src`: source of the application
    - `core`: contains all the plumbing and objects exported to QML
    - `lsp`: client/server [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) implementation
- `tests`: unit-tests for the application
- `test_data`: data used by tests
- `tools`: specific tools written for knut
    - `spec2cpp`: convert the LSP specification into code, used by the lsp library

Happy hacking!

## Compilation

To compile knut, you need:
- a C++20 compiler (recent compilers should work)
- Qt 5.15

All 3rd-party are downloaded at configure time using cmake `FetchContent`.

## Documentation

The documention is done using [mkdocs.org](https://www.mkdocs.org/). To create the documentation, follow instructions:

### Installation

```
pip install mkdocs
pip install mkdocs-material
pip install mkdocs-localsearch
```

### Commands

- `mkdocs serve` - Start the live-reloading docs server.
- `mkdocs build` - Build the documentation site.
- `mkdocs -h` - Print help message and exit.
