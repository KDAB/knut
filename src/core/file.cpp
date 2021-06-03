#include "file.h"

#include <QFile>
#include <QTextStream>

namespace Core {

/*!
 * \qmltype File
 * \brief Singleton with methods to handle files.
 * \instantiates Core::File
 * \inqmlmodule Script
 * \since 4.0
 *
 * The `File` singleton implements most of the static methods from `QFile`, check
 * [QFile](https://doc.qt.io/qt-5/qfile.html) documentation.
 */

File::File(QObject *parent)
    : QObject(parent)
{
}

File::~File() { }

/*!
 * \qmlmethod bool File::copy( string fileName, string newName)
 */
bool File::copy(const QString &fileName, const QString &newName)
{
    return QFile::copy(fileName, newName);
}

/*!
 * \qmlmethod bool File::exists( string fileName)
 */
bool File::exists(const QString &fileName)
{
    return QFile::exists(fileName);
}

/*!
 * \qmlmethod bool File::remove( string fileName)
 */
bool File::remove(const QString &fileName)
{
    return QFile::remove(fileName);
}

/*!
 * \qmlmethod bool File::rename( string oldName, string newName)
 */
bool File::rename(const QString &oldName, const QString &newName)
{
    return QFile::rename(oldName, newName);
}

/*!
 * \qmlmethod bool File::touch( string fileName)
 */
bool File::touch(const QString &fileName)
{
    QFile file(fileName);
    return file.open(QFile::Append);
}

/*!
 * \qmlmethod string File::readAll( string fileName)
 */
QString File::readAll(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        return stream.readAll();
    }
    return {};
}

} // namespace Core
