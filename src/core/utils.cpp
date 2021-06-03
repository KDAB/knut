#include "utils.h"

#include "scriptmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QTemporaryFile>

namespace Core {

/*!
 * \qmltype Utils
 * \brief Singleton with utility methods.
 * \instantiates Core::Utils
 * \inqmlmodule Script
 * \since 4.0
 *
 * The `Utils` singleton implements some utility methods useful for scripts.
 */

QHash<QString, QString> Utils::m_globals;

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

Utils::~Utils() { }

/*!
 * \qmlmethod string Utils::getEnv( string varName)
 * Returns the value of the environment variable `varName`.
 */
QString Utils::getEnv(const QString &varName)
{
    return QString::fromUtf8(qgetenv(varName.toLatin1()));
}

/*!
 * \qmlmethod string Utils::getGlobal( string varName)
 * Returns the value of the global `varName`. A global value is a value set by a script, and
 * persistent only in the current knut execution (it will disappear once closed).
 *
 * For persistent settings, see [Settings](settings.md).
 * \sa Settings
 */
QString Utils::getGlobal(const QString &varName)
{
    return m_globals.value(varName);
}

/*!
 * \qmlmethod Utils::setGlobal( string varName, string value)
 * Sets the global value `varName` to `value`. A global value is a value set by a script, and
 * persistent only in the current Qt Creator execution (it will disappear once closed).
 *
 * For persistent settings, see [Settings](settings.md).
 * \sa Settings
 */
void Utils::setGlobal(const QString &varName, const QString &value)
{
    m_globals.insert(varName, value);
}

/*!
 * \qmlmethod Utils::addScriptPath( string path, bool projectOnly)
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
    ScriptManager::instance()->addDirectory(path);
}

/*!
 * \qmlmethod Utils::runScript( string path, bool log)
 * Runs the script given by `path`. If `log` is true, it will also log the run of the script.
 */
void Utils::runScript(const QString &path, bool log)
{
    // Run the script synchronously
    ScriptManager::instance()->runScript(path, false, log);
}

/*!
 * \qmlmethod Utils::sleep( int msecs)
 * Sleeps for `msecs` milliseconds.
 */
void Utils::sleep(int msecs)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < msecs) {
        qApp->processEvents(QEventLoop::AllEvents, 100);
    }
}

/*!
 * \qmlmethod string Utils::mktemp( string pattern)
 * Creates and returns the name of a temporory file based on a `pattern`.
 * This function is copied from UtilsJsExtension::mktemp from Qt Creator.
 */
QString Utils::mktemp(const QString &pattern)
{
    QString tmp = pattern;
    if (tmp.isEmpty())
        tmp = QStringLiteral("script_temp.XXXXXX");
    QFileInfo fi(tmp);
    if (!fi.isAbsolute()) {
        QString tempPattern = QDir::tempPath();
        if (!tempPattern.endsWith(QLatin1Char('/')))
            tempPattern += QLatin1Char('/');
        tmp = tempPattern + tmp;
    }

    QTemporaryFile file(tmp);
    file.setAutoRemove(false);
    if (!file.open())
        return {};
    file.close();
    return file.fileName();
}

/*!
 * \qmlmethod string Utils::convertCase( string str, Case from, Case to)
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
    return Core::convertCase(str, static_cast<::Core::Case>(from), static_cast<::Core::Case>(to));
}

} // namespace Core
