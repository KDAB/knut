# Knut

Knut is an automation tool for code transformation using scripts. The main use case is for migration, but it could be used elsewhere.

The scripts are written either in javascript or QML languages, the latter being able to display a user interface to customize some parts of the application.
The script API is available in this documentation, and ranges from normal text transformations (find and replace) to getting contextual information based on the file language (using [TreeSitter](https://tree-sitter.github.io/tree-sitter/)).

Knut can be used via a command line interface or a user interface.

## Supported programming languages

Knut supports multiple programming languages and file types to different degrees.

This table provides a basic overview of the current support.
Please refer to the subclasses of [Document](https://kdab.github.io/knut/API/knut/document.html) for details on the supported functions of each class.

<!-- NOTE: SYNC WITH README.md! -->
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

# Licensing

Knut is © Klarälvdalens Datakonsult AB (KDAB) and is licensed according to the terms of [GPL 3.0](https://www.gnu.org/licenses/gpl-3.0.en.html).

Contact KDAB at <info@kdab.com> to inquire about licensing options.

# About KDAB

Knut is written and maintained by Klarälvdalens Datakonsult AB (KDAB).

The KDAB Group is the global No.1 software consultancy for Qt, C++ and OpenGL applications across desktop, embedded and mobile platforms.

The KDAB Group provides consulting and mentoring for developing Qt applications from scratch and in porting from all popular and legacy frameworks to Qt. We continue to help develop parts of Qt and are one of the major contributors to the Qt Project. We can give advanced or standard training anywhere around the globe on Qt as well as C++, OpenGL, 3D and more.

Please visit [https://www.kdab.com](https://www.kdab.com) to meet the people who write code like this.

Stay up-to-date with KDAB product announcements:

* [KDAB Newsletter](https://news.kdab.com)
* [KDAB Blogs](https://www.kdab.com/category/blogs)
* [KDAB on Twitter](https://twitter.com/KDABQt)
* [KDAB on YouTube](https://www.youtube.com/@KDABtv)
