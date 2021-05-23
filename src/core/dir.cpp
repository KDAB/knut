#include "dir.h"

#include <QVariant>

namespace Core {

/*!
 * \qmltype Dir
 * \brief Singleton with methods to handle directories.
 * \instantiates Core::Dir
 * \inqmlmodule Script
 * \since Script 1.0
 *
 * The Dir singleton implements most of the static methods from \c QDir, check \c QDir documentation.
 */

/*!
 * \qmlproperty char Dir::separator
 */
/*!
 * \qmlproperty string Dir::currentPath
 */
/*!
 * \qmlproperty QDirValueType Dir::current
 */
/*!
 * \qmlproperty string Dir::currentScriptPath
 */
/*!
 * \qmlproperty QDirValueType Dir::currentScript
 */
/*!
 * \qmlproperty string Dir::homePath
 */
/*!
 * \qmlproperty QDirValueType Dir::home
 */
/*!
 * \qmlproperty string Dir::rootPath
 */
/*!
 * \qmlproperty QDirValueType Dir::root
 */
/*!
 * \qmlproperty string Dir::tempPath
 */
/*!
 * \qmlproperty QDirValueType Dir::temp
 */

Dir::Dir(QObject *parent)
    : QObject(parent)
{
}

Dir::~Dir() { }

/*!
 * \qmlmethod string Dir::toNativeSeparators( string pathName)
 */
QString Dir::toNativeSeparators(const QString &pathName) const
{
    return QDir::toNativeSeparators(pathName);
}

/*!
 * \qmlmethod string Dir::fromNativeSeparators( string pathName)
 */
QString Dir::fromNativeSeparators(const QString &pathName) const
{
    return QDir::fromNativeSeparators(pathName);
}

/*!
 * \qmlmethod bool Dir::isRelativePath( string path)
 */
bool Dir::isRelativePath(const QString &path) const
{
    return QDir::isRelativePath(path);
}

QChar Dir::separator() const
{
    return QDir::separator();
}

bool Dir::setCurrentPath(const QString &path)
{
    if (path != QDir::currentPath()) {
        if (QDir::setCurrent(path)) {
            emit currentPathChanged(path);
            return true;
        }
    }
    return false;
}

QString Dir::currentPath() const
{
    return QDir::currentPath();
}

QString Dir::currentScriptPath() const
{
    return property("scriptPath").toString();
}

QString Dir::homePath() const
{
    return QDir::homePath();
}

QString Dir::rootPath() const
{
    return QDir::rootPath();
}

QString Dir::tempPath() const
{
    return QDir::tempPath();
}

/*!
 * \qmlmethod bool Dir::match( array<string> filters, string fileName)
 * \qmlmethod bool Dir::match( string filter, string fileName)
 */
bool Dir::match(const QStringList &filters, const QString &fileName) const
{
    return QDir::match(filters, fileName);
}

bool Dir::match(const QString &filter, const QString &fileName) const
{
    return QDir::match(filter, fileName);
}

/*!
 * \qmlmethod string Dir::cleanPath( string path)
 */
QString Dir::cleanPath(const QString &path) const
{
    return QDir::cleanPath(path);
}

/*!
 * \qmlmethod QDirValueType Dir::create( string path)
 */
QDirValueType Dir::create(const QString &path) const
{
    return QDirValueType(path);
}

}
