# Architecture

High-level architecture description.

## Modules

![architecture](../assets/architecture.svg){ align=right }

- **lsp**: LSP client library
    - Complete LSP protocole definition
    - Client class to handle some of the messages
    - Use `clangd` for C/C++
- **rccore**: rc files parser and converter
    - Conversion to ui files
    - /!\ Not everything is parsed
- **rcui**: widgets for viewing a rcfile
- **core**: script engine and all script objects
    - The engine is based on QML
    - Manage project and documents
- **gui**: ui on top of the core
    - IDE-like
    - All dialogs available from command line
- **knut**: the application itself, just a `main.cpp` file

## Details

### Startup

`KnutCore` is the central object in the application (module **core**), and is created in the `main.cpp`:

- create the application singletons: [`Project`](../API/script/project.md), [`Settings`](../API/script/settings.md) and `ScriptManager`
- read parameters from the command line

`KnutMain` is a subclass (module **gui**) that adds the gui specific parameters.

### Project

The `Project` class is the root of everything, as it's the class used to create all documents.

![project classes](../assets/project-classes.svg)

All objects exported **must be declared** inside the `ScriptRunner` class constructor. This class, as the name suggest, is used to run the scripts.

### LSP server

!!! warning
    For now, knut has only been tested with clangd LSP server.

When using a LSP server, different log files are created (next to the knut executable):

- *extension*_messages.log: contains all the different json messages exchanged between the client and the server
- *extension*_server.log: all the logs coming from the server

*extension* being the extension used for the language, typically `cpp` for C++ files.

The first one can be loaded directly in this online [lsp-viewer](https://lampepfl.github.io/lsp-viewer/).
