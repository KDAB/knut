# Knut

Knut is an automation tool for code transformation using scripts. The main use case is for migration, but it could be used elsewhere.

The scripts are written either in javascript or QML languages, the latter being able to display a user interface to customize some parts of the application.
The script API is available in this documentation, and ranges from normal text transformations (find and replace) to getting contextual information based on the file language (using [TreeSitter](https://tree-sitter.github.io/tree-sitter/).

Knut can be used via a command line interface or a user interface.

# Requirements

Knut is using [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) servers for the GUI. It is not used for the scripts, so if you are using Knut with the command line interface, you don't need it.

For C++, if you are using clangd, note that some tests are disabled for clangd version < 13.

# Licensing

Knut is © Klarälvdalens Datakonsult AB (KDAB) and is licensed according to the terms of [GPL 3.0](LICENSES/GPL-3.0-only.txt).

Contact KDAB at <info@kdab.com> to inquire about licensing options.

Knut includes different 3rd party software, you can find the list and licenses for them in the [3RDPARTY](3RDPARTY.md) document.

# About KDAB

Knut is written and maintained by Klarälvdalens Datakonsult AB (KDAB).

The KDAB Group is the global No.1 software consultancy for Qt, C++ and OpenGL applications across desktop, embedded and mobile platforms.

The KDAB Group provides consulting and mentoring for developing Qt applications from scratch and in porting from all popular and legacy frameworks to Qt. We continue to help develop parts of Qt and are one of the major contributors to the Qt Project. We can give advanced or standard training anywhere around the globe on Qt as well as C++, OpenGL, 3D and more.

Please visit [https://www.kdab.com](https://www.kdab.com) to meet the people who write code like this

Stay up-to-date with KDAB product announcements:

* [KDAB Newsletter](https://news.kdab.com)
* [KDAB Blogs](https://www.kdab.com/category/blogs)
* [KDAB on Twitter](https://twitter.com/KDABQt)
