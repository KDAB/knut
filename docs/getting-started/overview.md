# Overview

## General overview

Knut is built to automate code modifications on a codebase.
For many repetitive tasks, Knut aims to automate around 90% of the work, with a bit of fix-up required afterwards.
Knut achieves this by providing high-level APIs to access and modify your code base, which can easily be scripted with JavaScript to run arbitrary transformations.
It is even possible to apply modifications on multiple files at once, for example the source and header files in a C++ project.

When running Knut, the first thing is to open a project, which is just the root directory for the codebase.
Once a project is open, Knut can open one or multiple files inside the project, and run transformations on those files.
Transformations are defined in script files (see [writing scripts](script.md)).

Each open file provides different API usable by the scripts, depending on the type of file:

- a text file has find/replace/navigation APIs (and more...)
- a C++ file adds block navigation, switch header/source or declaration/definition...
- a ui file has API to change the object name or class of items
- ...

![API usage by script](../assets/overview.svg)

## Running a script

### Setup

To run scripts you need to set Knut up accordingly:

- start the Knut user interface and go to `File`>`Options...`
- or start directly the settings dialog via the command line: `knut` --gui-setting`

This will display a dialog with the script Knut general settings.
![Knut settings](overview-settings.png)

You need at least one script path.
Script paths contain .js or .qml files that can be run by Knut (see [writing scripts](script.md)).

### Running

Once done, you can run a script inside your script paths:

- from the command line:
```
knut --run <path-to-script>
```

- from the user interface, using the `Script`>`Run Script...` menu

## Settings

Knut has 3 levels of settings:

- internal settings (can't be changed by the user)
- user settings: stored in `<user home>/knut.json`
- project settings: stored in `<project home>/knut.json`

Settings can be overridden, project settings have priority over user settings over internal settings. When a project is loaded, settings are saved in the project settings, otherwise in the user settings.
Script paths are merged between the user and project settings, which allows accessing both general scripts, and project-specific scripts.

Some settings (like Text Editor Behavior) are only per project.

### Internal settings

```json
{
    "lsp" : [
        {
            "type": "cpp_type",
            "program": "clangd",
            "arguments": []
        }
    ],
    "rc": {
        "dialog_flags": ["UpdateGeometry", "UseIdForPixmap"],
        "dialog_scalex": 1.5,
        "dialog_scaley": 1.65,
        "asset_flags": ["RemoveUnknown", "SplitToolBar", "ConvertToPng"],
        "asset_transparent_colors": ["Gray", "Magenta", "BottomLeftPixel"]
    },
    "mime_types": {
        "c": "cpp_type",
        "cpp": "cpp_type",
        "cxx": "cpp_type",
        "h": "cpp_type",
        "hpp": "cpp_type",
        "txt": "text_type",
        "json": "text_type",
        "rc": "rc_type",
        "rc2": "rc_type",
        "ui": "qtui_type",
        "png": "image_type",
        "bmp": "image_type",
        "jpg": "image_type",
        "ico": "image_type"
    },
    "text_editor": {
        "tab": {
            "insertSpaces": true,
            "tabSize": 4
        }
    }
}
```
