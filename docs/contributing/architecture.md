# Architecture

High-level architecture description.

## Modules

![architecture](../assets/architecture.svg){ align=right }

- **treesitter**: Tree-sitter C++ wrapper
    - Wraps C interface into C++ classes
    - Improved memory-safety by use of smart pointers
    - High-Level user access through `QueryMatch` implemented in **core**
- **lsp**: LSP client library
    - Complete LSP protocole definition
    - Client class to handle some of the messages
    - Use `clangd` for C/C++
- **rccore**: rc files parser and converter
    - Conversion to ui files
    - ⚠️ Not everything is parsed
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

The `Project` class is the root of everything, as it's the class used to create all documents. Then each document has a specific API to work with the type of document (for example, a CppDocument has a `switchDeclarationDefinition()` method).

![project classes](../assets/project-classes.svg)

All objects exported **must be declared** inside the `ScriptRunner` class constructor. This class, as the name suggest, is used to run the scripts. Also make sure to add the object properties at the end of the constructor, for example:
```cpp
addProperties<CppDocument>(m_properties);
```
This will store all property's names for this object, and is used when recording a script from the ui, to make a difference between a property call and a method call.

### Tree-sitter

Tree-sitter is used by knut to parse code and extract many higher-level structures (currently only C++).

In the Knut GUI, use the `C++`>`Tree-sitter Inspector` widget to explore the current Tree-sitter state.

!!! note
    Tree-sitter capabilities are implemented on the CodeDocument class.
    Most code-specific scripting functions implemented by Knut are based on Tree-sitter and we expect Tree-sitter bindings to be available for almost any language.

### LSP server

!!! warning
    For now, knut has only been tested with clangd LSP server, and require at least version 13.0.

When using a LSP server, different log files are created (next to the knut executable):

- *extension*_messages.log: contains all the different json messages exchanged between the client and the server
- *extension*_server.log: all the logs coming from the server

*extension* being the extension used for the language, typically `cpp` for C++ files.

The first one can be loaded directly in this online [lsp-viewer](https://lampepfl.github.io/lsp-viewer/).
