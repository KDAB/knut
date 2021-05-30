# Utils

Singleton with utility methods. [More...](#detailed-description)

```qml
import Script 1.0
```

## Methods

- string **[getEnv](#getEnv)**(string varName)
- string **[getGlobal](#getGlobal)**(string varName)
- **[setGlobal](#setGlobal)**(string varName, string value)
- **[addScriptPath](#addScriptPath)**(string path, bool projectOnly)
- **[runScript](#runScript)**(string path, bool log)
- **[sleep](#sleep)**(int msecs)
- string **[mktemp](#mktemp)**(string pattern)
- string **[convertCase](#convertCase)**(string str, Case from, Case to)

## Detailed Description

The Utilss singleton implements some Utilsity methods useful for scripts.

## Method Documentation

#### <a name="getEnv"></a>string **getEnv**(string varName)

Returns the value of the environment variable \a varName.

#### <a name="getGlobal"></a>string **getGlobal**(string varName)

Returns the value of the global \a varName. A global value is a value set by a script, and
persistent only in the current Qt Creator execution (it will disappear once closed).

For persistent settings, see Settings.

#### <a name="setGlobal"></a>**setGlobal**(string varName, string value)

Sets the global value \a varName to \a value. A global value is a value set by a script, and
persistent only in the current Qt Creator execution (it will disappear once closed).

For persistent settings, see Settings.

#### <a name="addScriptPath"></a>**addScriptPath**(string path, bool projectOnly)

Adds the script directory \a path from another script.

Could be useful to load multiple paths at once, by creating a \c {init.js} file like this:
function main() {
Utils.addScriptPath(Dir.currentScriptPath() + "/message")
Utils.addScriptPath(Dir.currentScriptPath() + "/texteditor")
Utils.addScriptPath(Dir.currentScriptPath() + "/dialog")
Utils.addScriptPath(Dir.currentScriptPath() + "/cppeditor")


#### <a name="runScript"></a>**runScript**(string path, bool log)

Runs the script given by \a path. If \a log is true, it will also log the run on the Script
Output.

#### <a name="sleep"></a>**sleep**(int msecs)

Sleeps for \a msecs milliseconds.

#### <a name="mktemp"></a>string **mktemp**(string pattern)

Creates and returns the name of a temporory file based on a \a pattern.
This function is copied from UtilssJsExtension::mktemp

#### <a name="convertCase"></a>string **convertCase**(string str, Case from, Case to)

Converts and returns the string \a str with a different case pattern: from \a from to \a to.

The different cases are:
