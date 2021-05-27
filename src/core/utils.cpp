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
 * \since Script 1.0
 *
 * The Utilss singleton implements some Utilsity methods useful for scripts.
 */

QHash<QString, QString> Utils::m_globals;

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

Utils::~Utils() { }

/*!
 * \qmlmethod string Utils::getEnv( string varName)
 * Returns the value of the environment variable \a varName.
 */
QString Utils::getEnv(const QString &varName) const
{
    return QString::fromUtf8(qgetenv(varName.toLatin1()));
}

/*!
 * \qmlmethod string Utils::getGlobal( string varName)
 * Returns the value of the global \a varName. A global value is a value set by a script, and
 * persistent only in the current Qt Creator execution (it will disappear once closed).
 *
 * For persistent settings, see Settings.
 */
QString Utils::getGlobal(const QString &varName) const
{
    return m_globals.value(varName);
}

/*!
 * \qmlmethod Utils::setGlobal( string varName, string value)
 * Sets the global value \a varName to \a value. A global value is a value set by a script, and
 * persistent only in the current Qt Creator execution (it will disappear once closed).
 *
 * For persistent settings, see Settings.
 */
void Utils::setGlobal(const QString &varName, const QString &value)
{
    m_globals.insert(varName, value);
}

/*!
 * \qmlmethod Utils::addScriptPath( string path, bool projectOnly)
 * Adds the script directory \a path from another script.
 *
 * Could be useful to load multiple paths at once, by creating a \c {init.js} file like this:
 * \code
 * function main() {
 *     Utils.addScriptPath(Dir.currentScriptPath() + "/message")
 *     Utils.addScriptPath(Dir.currentScriptPath() + "/texteditor")
 *     Utils.addScriptPath(Dir.currentScriptPath() + "/dialog")
 *     Utils.addScriptPath(Dir.currentScriptPath() + "/cppeditor")
 * }
 * \endcode
 */
void Utils::addScriptPath(const QString &path) const
{
    ScriptManager::instance()->addDirectory(path);
}

/*!
 * \qmlmethod Utils::runScript( string path, bool log)
 * Runs the script given by \a path. If \a log is true, it will also log the run on the Script
 * Output.
 */
void Utils::runScript(const QString &path, bool log) const
{
    // Run the script synchronously
    ScriptManager::instance()->runScript(path, false, log);
}

/*!
 * \qmlmethod Utils::sleep( int msecs)
 * Sleeps for \a msecs milliseconds.
 */
void Utils::sleep(int msecs) const
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < msecs) {
        qApp->processEvents(QEventLoop::AllEvents, 100);
    }
}

/*!
 * \qmlmethod string Utils::mktemp( string pattern)
 * Creates and returns the name of a temporory file based on a \a pattern.
 * This function is copied from UtilssJsExtension::mktemp
 */
QString Utils::mktemp(const QString &pattern) const
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
 * Converts and returns the string \a str with a different case pattern: from \a from to \a to.
 *
 * The different cases are:
 * \list
 * \li Utils.CamelCase: "toCamelCase",
 * \li Utils.PascalCase: "ToPascalCase",
 * \li Utils.SnakeCase: "to_snake_case",
 * \li Utils.UpperCase: "TO_UPPER_CASE",
 * \li Utils.KebabCase: "to-kebab-case",
 * \li Utils.TitleCase: "To Title Case".
 * \endlist
 */
QString Utils::convertCase(const QString &str, Case from, Case to)
{
    return Core::convertCase(str, static_cast<::Core::Case>(from), static_cast<::Core::Case>(to));
}

}
