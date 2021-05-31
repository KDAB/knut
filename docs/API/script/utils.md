# Utils

Singleton with utility methods. [More...](#detailed-description)

```qml
import Script 1.0
```

<table>
<tr><td>Since:</td><td>knut 4.0</td></tr>
</table>

## Methods

| | Name |
|-|-|
|string |**[getEnv](#getEnv)**(string varName)|
|string |**[getGlobal](#getGlobal)**(string varName)|
||**[setGlobal](#setGlobal)**(string varName, string value)|
||**[addScriptPath](#addScriptPath)**(string path, bool projectOnly)|
||**[runScript](#runScript)**(string path, bool log)|
||**[sleep](#sleep)**(int msecs)|
|string |**[mktemp](#mktemp)**(string pattern)|
|string |**[convertCase](#convertCase)**(string str, Case from, Case to)|

## Detailed Description

The `Utils` singleton implements some utility methods useful for scripts.

## Method Documentation

#### <a name="getEnv"></a>string **getEnv**(string varName)

Returns the value of the environment variable `varName`.

#### <a name="getGlobal"></a>string **getGlobal**(string varName)

Returns the value of the global `varName`. A global value is a value set by a script, and
persistent only in the current knut execution (it will disappear once closed).

For persistent settings, see [Settings](settings.md).

#### <a name="setGlobal"></a>**setGlobal**(string varName, string value)

Sets the global value `varName` to `value`. A global value is a value set by a script, and
persistent only in the current Qt Creator execution (it will disappear once closed).

For persistent settings, see [Settings](settings.md).

#### <a name="addScriptPath"></a>**addScriptPath**(string path, bool projectOnly)

Adds the script directory `path` from another script.

Could be useful to load multiple paths at once, by creating a *init.js* file like this:

```js
function main() {
    Utils.addScriptPath(Dir.currentScriptPath() + "/message")
    Utils.addScriptPath(Dir.currentScriptPath() + "/texteditor")
    Utils.addScriptPath(Dir.currentScriptPath() + "/dialog")
    Utils.addScriptPath(Dir.currentScriptPath() + "/cppeditor")
}
```

#### <a name="runScript"></a>**runScript**(string path, bool log)

Runs the script given by `path`. If `log` is true, it will also log the run of the script.

#### <a name="sleep"></a>**sleep**(int msecs)

Sleeps for `msecs` milliseconds.

#### <a name="mktemp"></a>string **mktemp**(string pattern)

Creates and returns the name of a temporory file based on a `pattern`.
This function is copied from UtilsJsExtension::mktemp from Qt Creator.

#### <a name="convertCase"></a>string **convertCase**(string str, Case from, Case to)

Converts and returns the string `str` with a different case pattern: from `from` to `to`.

The different cases are:

- `Utils.CamelCase`: "toCamelCase",
- `Utils.PascalCase`: "ToPascalCase",
- `Utils.SnakeCase`: "to_snake_case",
- `Utils.UpperCase`: "TO_UPPER_CASE",
- `Utils.KebabCase`: "to-kebab-case",
- `Utils.TitleCase`: "To Title Case".
