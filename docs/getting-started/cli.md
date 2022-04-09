# Command Line Interface

## Using command line options

You can start knut by passing directly some options to the command line, using it in a headless way:
```
knut [option]
```

All availables options are documented here:

| Options | Description |
|-|-|
|-s, --script `<file>` | Run given script `<file>` then exit|
|-r, --root `<directory>` | Root `<directory>` of the project|
|-i, --input `<file>` | Open document `<file>` on startup|
|-l, --line `<line>` | Line value to set the current cursor position in the passed file|
|-c, --column `<column>` | Column value to set the current cursor position in the passed file|
|--gui | Open Knut user interface|
|--gui-script | Open the run script dialog|
|--gui-settings | Open the settings dialog|

Wihtout any options, knut will start the uer interface.

## IDE integration

Using the command line interface, one can integrate with existing IDE.

### Qt Creator

To add knut to Qt Creator, go to the menu `Tools`>`External`>`Configure...`, create a new tool, and fill the different fileds for a new tool.

![Tool settings in Qt Creator](cli-qtcreator.png)

Particularly:

- in Arguments, you can use different variables, like:
    - `--root %{ActiveProject:NativePath}`
    - `--input %{CurrentDocument:FilePath}`
    - `--line %{CurrentDocument:Row}`
    - `--column %{CurrentDocument:Column}`
- in Workding directory: `%{ActiveProject:NativePath}`

For example, to open the knut user interface using the same project and same file as Qt Creator, you will pass in Arguments:
```
--gui --root %{ActiveProject:NativePath} --input %{CurrentDocument:FilePath} --line %{CurrentDocument:Row} --column %{CurrentDocument:Column}
```
