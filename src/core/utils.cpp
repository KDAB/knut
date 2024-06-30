/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "utils.h"
#include "logger.h"
#include "scriptmanager.h"
#include "utils/log.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QTemporaryFile>

namespace Core {

/*!
 * \qmltype Utils
 * \brief Singleton with utility methods.
 * \inqmlmodule Script
 * \ingroup Utilities
 *
 * The `Utils` singleton implements some utility methods useful for scripts.
 */

QHash<QString, QString> Utils::m_globals;

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

Utils::~Utils() = default;

/*!
 * \qmlmethod string Utils::getEnv(string varName)
 * Returns the value of the environment variable `varName`.
 */
QString Utils::getEnv(const QString &varName)
{
    LOG("Utils::getEnv", varName);

    return QString::fromUtf8(qgetenv(varName.toLatin1()));
}

/*!
 * \qmlmethod string Utils::getGlobal(string varName)
 * Returns the value of the global `varName`. A global value is a value set by a script, and
 * persistent only in the current knut execution (it will disappear once closed).
 *
 * For persistent settings, see [Settings](settings.md).
 * \sa Settings
 */
QString Utils::getGlobal(const QString &varName)
{
    LOG("Utils::getGlobal", varName);

    return m_globals.value(varName);
}

/*!
 * \qmlmethod Utils::setGlobal(string varName, string value)
 * Sets the global value `varName` to `value`. A global value is a value set by a script, and
 * persistent only in the current Knut execution (it will disappear once closed).
 *
 * For persistent settings, see [Settings](settings.md).
 * \sa Settings
 */
void Utils::setGlobal(const QString &varName, const QString &value)
{
    LOG("Utils::setGlobal", varName, value);

    m_globals.insert(varName, value);
}

/*!
 * \qmlmethod Utils::addScriptPath(string path, bool projectOnly)
 * Adds the script directory `path` from another script.
 *
 * Could be useful to load multiple paths at once, by creating a *init.js* file like this:
 *
 * ```js
 * function main() {
 *     Utils.addScriptPath(Dir.currentScriptPath() + "/message")
 *     Utils.addScriptPath(Dir.currentScriptPath() + "/texteditor")
 *     Utils.addScriptPath(Dir.currentScriptPath() + "/dialog")
 *     Utils.addScriptPath(Dir.currentScriptPath() + "/cppeditor")
 * }
 * ```
 */
void Utils::addScriptPath(const QString &path)
{
    LOG("Utils::addScriptPath", path);

    ScriptManager::instance()->addDirectory(path);
}

/*!
 * \qmlmethod Utils::runScript(string path, bool log)
 * Runs the script given by `path`. If `log` is true, it will also log the run of the script.
 */
void Utils::runScript(const QString &path, bool log)
{
    LOG("Utils::runScript", LOG_ARG("path", path), log);

    // Run the script synchronously
    ScriptManager::instance()->runScript(path, false, log);
}

/*!
 * \qmlmethod Utils::sleep(int msecs)
 * Sleeps for `msecs` milliseconds.
 */
void Utils::sleep(int msecs)
{
    LOG("Utils::sleep", msecs);

    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < msecs) {
        qApp->processEvents(QEventLoop::AllEvents, 100);
    }
}

/*!
 * \qmlmethod string Utils::mktemp(string pattern)
 * Creates and returns the name of a temporary file based on a `pattern`.
 */
// This function is copied from Qt Creator.
QString Utils::mktemp(const QString &pattern)
{
    LOG("Utils::mktemp", pattern);

    QString tmp = pattern;
    if (tmp.isEmpty())
        tmp = "script_temp.XXXXXX";
    QFileInfo fi(tmp);
    if (!fi.isAbsolute()) {
        QString tempPattern = QDir::tempPath();
        if (!tempPattern.endsWith('/'))
            tempPattern += '/';
        tmp = tempPattern + tmp;
    }

    QTemporaryFile file(tmp);
    file.setAutoRemove(false);
    if (!file.open())
        return {};
    file.close();

    spdlog::debug("Utils::mktemp created {}", tmp);
    return file.fileName();
}

/*!
 * \qmlmethod string Utils::convertCase(string str, Case from, Case to)
 * Converts and returns the string `str` with a different case pattern: from `from` to `to`.
 *
 * The different cases are:
 *
 * - `Utils.CamelCase`: "toCamelCase",
 * - `Utils.PascalCase`: "ToPascalCase",
 * - `Utils.SnakeCase`: "to_snake_case",
 * - `Utils.UpperCase`: "TO_UPPER_CASE",
 * - `Utils.KebabCase`: "to-kebab-case",
 * - `Utils.TitleCase`: "To Title Case".
 */
QString Utils::convertCase(const QString &str, Case from, Case to)
{
    LOG("Utils::convertCase", str, from, to);
    return ::Utils::convertCase(str, static_cast<::Utils::Case>(from), static_cast<::Utils::Case>(to));
}

/*!
 * \qmlmethod string Utils::copyToClipboard(string text)
 * Copy the text to the clipboard
 */
void Utils::copyToClipboard(const QString &text)
{
    LOG("Utils::copyToClipboard", text);
    auto clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

/*!
 * \qmlmethod string Utils::cppKeywords()
 * Returns a list of cpp keywords.
 */
// Source: https://en.cppreference.com/w/cpp/keyword
// As of August 2022
//
// Added a 'readonly' (C++ attribute) keyword:
// Source: https://learn.microsoft.com/en-us/cpp/windows/attributes/readonly-cpp?view=msvc-170
// Prohibits assignment to a data member.
// Applies to Interface method.
// The readonly C++ attribute has the same functionality as the readonly MIDL attribute.
QStringList Utils::cppKeywords()
{
    LOG("Utils::cppKeywords");
    return QStringList {"alignas",
                        "alignof",
                        "and",
                        "and_eq",
                        "asm",
                        "atomic_cancel",
                        "atomic_commit",
                        "atomic_noexcept",
                        "auto",
                        "bitand",
                        "bitor",
                        "bool",
                        "break",
                        "case",
                        "catch",
                        "class",
                        "compl",
                        "concept",
                        "const",
                        "consteval",
                        "constexpr",
                        "constinit",
                        "const_cast",
                        "continue",
                        "co_await",
                        "co_return",
                        "co_yield",
                        "decltype",
                        "default",
                        "delete",
                        "do",
                        "dynamic_cast",
                        "else",
                        "enum",
                        "explicit",
                        "export",
                        "extern",
                        "false",
                        "for",
                        "friend",
                        "goto",
                        "if",
                        "inline",
                        "mutable",
                        "namespace",
                        "new",
                        "noexcept",
                        "not",
                        "not_eq",
                        "nullptr",
                        "operator",
                        "or",
                        "or_eq",
                        "private",
                        "protected",
                        "public",
                        "readonly",
                        "reflexpr",
                        "register",
                        "reinterpret_cast",
                        "requires",
                        "return",
                        "signed",
                        "sizeof",
                        "static",
                        "static_assert",
                        "static_cast",
                        "struct",
                        "switch",
                        "synchronized",
                        "template",
                        "this",
                        "thread_local",
                        "throw",
                        "true",
                        "try",
                        "typedef",
                        "typeid",
                        "typename",
                        "union",
                        "unsigned",
                        "using",
                        "virtual",
                        "volatile",
                        "while",
                        "xor",
                        "xor_eq"};
}

/*!
 * \qmlmethod string Utils::cppPrimitiveTypes()
 * Returns a list of cpp primitive types
 */
QStringList Utils::cppPrimitiveTypes()
{
    LOG("Utils::cppPrimitiveTypes");
    return QStringList {"int",  "long",    "short",    "void",     "float",  "double",
                        "char", "char8_t", "char16_t", "char32_t", "wchar_t"};
}

} // namespace Core
