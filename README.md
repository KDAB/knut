# Knut

[![GitHub branch status](https://img.shields.io/github/checks-status/KDAB/knut/main?logo=github)](https://github.com/KDAB/knut/actions)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSES/GPL-3.0-only.txt)
[![MkDocs](https://img.shields.io/badge/MkDocs-526CFE?logo=materialformkdocs&logoColor=fff)](https://kdab.github.io/knut/)

Knut is an automation tool for code transformation using scripts. The main use case is for migration, but it could be used elsewhere.

The scripts are written either in javascript or QML languages, the latter being able to display a user interface to customize some parts of the application.
The script API is available in this documentation, and ranges from normal text transformations (find and replace) to getting contextual information based on the file language (using [TreeSitter](https://tree-sitter.github.io/tree-sitter/)).

Knut can be used via a command line interface or a user interface.

## Supported programming languages

Knut supports multiple programming languages and file types to different degrees.

This table provides a basic overview of the current support.
Please refer to the subclasses of [Document](https://kdab.github.io/knut/API/knut/document.html) for details on the supported functions of each class.

<!-- NOTE: SYNC WITH docs/index.md! -->
|                            | 🌳 TreeSitter | 🔣 Code Items | 🧑‍💻 LSP | 🖥️ File Viewer |
|----------------------------|---------------|---------------|---------|---------------|
| C/C++                      | ✅            | ✅            | ✅      |               |
| C#                         | ✅            | ❌            | ❌      |               |
| JSON                       | ❌            | ✔️             | ❌      |               |
| [Qt Translate (.ts)][QtTs] | ❌            | ✔️             | ❌      |               |
| [Qt Qml][Qml]              | ✅            | ✅            | ❌      | ✅             |
| [Qt Ui files][QtUi]        | ❌            | ❌            | ❌      | ✅             |
| [Slint][Slint]             | ❌            | ❌            | ❌      | ✅             |
| [MFC rc files][MfcRc]      | ❌            | ✔️             | ❌      | ✔️             |

> ✅ Fully Supported
> ✔️  Partial Support
> ❌ Unsupported

[QtTs]: https://doc.qt.io/qt-6/linguist-ts-file-format.html
[QtUi]: https://doc.qt.io/qt-6/designer-ui-file-format.html
[Qml]: https://doc.qt.io/qt-6/qmlreference.html
[Slint]: https://slint.dev/
[MfcRc]: https://learn.microsoft.com/en-us/windows/win32/menurc/about-resource-files


## Requirements

Knut is using [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) servers for the GUI. It is not used for the scripts, so if you are using Knut with the command line interface, you don't need it.

For C++, if you are using clangd, note that some tests are disabled for clangd version < 13.

## Licensing

Knut is © Klarälvdalens Datakonsult AB (KDAB) and is licensed according to the terms of [GPL 3.0](LICENSES/GPL-3.0-only.txt).

Contact KDAB at <info@kdab.com> to inquire about licensing options.

Knut includes different 3rd party software, you can find the list and licenses for them in the [3RDPARTY](3RDPARTY.md) document.

## Building

To build Knut, you will need an up-to-date C++ & Qt toolchain.
This includes:

- A recent C++ compiler (GCC 11+, Clang 15+, MSVC 19.40+)
- [CMake](https://cmake.org) 3.15+
- An installation of [Qt 6](https://www.qt.io/download-open-source)
- (optional: [Ninja build system](https://ninja-build.org/))

On Windows, you may run into issues when building, especially if using QtCreator.
This is a known issue, and the steps to resolve it are detailed in the link to the Contributing Guide in the [Contributing](#contributing) section of this readme.

### Download Knut

To clone the code, simply run:

```bash
git clone https://github.com/KDAB/knut.git && cd knut
```

If you are a member of KDAB and have access to our private repositories, you may then simply run:

```bash
git submodule update --init --recursive
```

to download all dependencies.

If you are a contributor outside KDAB, you will need to download all submodules in the `3rdparty/` folder, without the private dependencies in the `3rdparty-kdab/` folder.

```bash
git submodule update --init --recursive -- 3rdparty/*
```

### Running CMake

After that you can build Knut with CMake via one of the presets.
E.g.:

```bash
cmake --preset=release
cmake --build --preset=release
```

Take a look at `CMakePreset.json` for a list of available presets.

### Running Knut

After building with CMake, run the knut binary from the bin folder within your build directory.

e.g.:

```bash
./build-release/bin/knut
```

## Documentation

Visit our comprehensive documentation here: [https://kdab.github.io/knut/index.html](https://kdab.github.io/knut/index.html)

## Contributing

For information about contributing to Knut, see the ["Contributing Guide"](https://kdab.github.io/knut/contributing/getting-involved.html) in our documentation.

## About KDAB

Knut is written and maintained by Klarälvdalens Datakonsult AB (KDAB).

The KDAB Group is the global No.1 software consultancy for Qt, C++ and OpenGL applications across desktop, embedded and mobile platforms.

The KDAB Group provides consulting and mentoring for developing Qt applications from scratch and in porting from all popular and legacy frameworks to Qt. We continue to help develop parts of Qt and are one of the major contributors to the Qt Project. We can give advanced or standard training anywhere around the globe on Qt as well as C++, OpenGL, 3D and more.

Please visit [https://www.kdab.com](https://www.kdab.com) to meet the people who write code like this

Stay up-to-date with KDAB product announcements:

- [KDAB Newsletter](https://news.kdab.com)
- [KDAB Blogs](https://www.kdab.com/category/blogs)
- [KDAB on Twitter](https://twitter.com/KDABQt)
