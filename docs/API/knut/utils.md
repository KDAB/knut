# Utils

Singleton with utility methods. [More...](#detailed-description)

```qml
import Knut
```

## Methods

| | Name |
|-|-|
||**[addScriptPath](#addScriptPath)**(string path, bool projectOnly)|
|string |**[convertCase](#convertCase)**(string str, Case from, Case to)|
|string |**[copyToClipboard](#copyToClipboard)**(string text)|
|string |**[cppKeywords](#cppKeywords)**()|
|string |**[cppPrimitiveTypes](#cppPrimitiveTypes)**()|
|string |**[getEnv](#getEnv)**(string varName)|
|string |**[getGlobal](#getGlobal)**(string varName)|
|string |**[mktemp](#mktemp)**(string pattern)|
||**[runScript](#runScript)**(string path, bool log)|
||**[setGlobal](#setGlobal)**(string varName, string value)|
||**[sleep](#sleep)**(int msecs)|

## Detailed Description

The `Utils` singleton implements some utility methods useful for scripts.

## Method Documentation

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

#### <a name="convertCase"></a>string **convertCase**(string str, Case from, Case to)

Converts and returns the string `str` with a different case pattern: from `from` to `to`.

The different cases are:

- `Utils.CamelCase`: "toCamelCase",
- `Utils.PascalCase`: "ToPascalCase",
- `Utils.SnakeCase`: "to_snake_case",
- `Utils.UpperCase`: "TO_UPPER_CASE",
- `Utils.KebabCase`: "to-kebab-case",
- `Utils.TitleCase`: "To Title Case".

#### <a name="copyToClipboard"></a>string **copyToClipboard**(string text)

Copy the text to the clipboard

#### <a name="cppKeywords"></a>string **cppKeywords**()

Returns a list of cpp keywords.

#### <a name="cppPrimitiveTypes"></a>string **cppPrimitiveTypes**()

Returns a list of cpp primitive types

#### <a name="getEnv"></a>string **getEnv**(string varName)

Returns the value of the environment variable `varName`.

#### <a name="getGlobal"></a>string **getGlobal**(string varName)

Returns the value of the global `varName`. A global value is a value set by a script, and
persistent only in the current knut execution (it will disappear once closed).

For persistent settings, see [Settings](settings.md).

#### <a name="mktemp"></a>string **mktemp**(string pattern)

Creates and returns the name of a temporary file based on a `pattern`.

#### <a name="runScript"></a>**runScript**(string path, bool log)

Runs the script given by `path`. If `log` is true, it will also log the run of the script.

#### <a name="setGlobal"></a>**setGlobal**(string varName, string value)

Sets the global value `varName` to `value`. A global value is a value set by a script, and
persistent only in the current Knut execution (it will disappear once closed).

For persistent settings, see [Settings](settings.md).

#### <a name="sleep"></a>**sleep**(int msecs)

Sleeps for `msecs` milliseconds.
