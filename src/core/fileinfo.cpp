#include "fileinfo.h"

#include <QFileInfo>

namespace Core {

/*!
 * \qmltype FileInfo
 * \brief Singleton with methods to handle file information.
 * \instantiates Core::FileInfo
 * \inqmlmodule Script
 * \since 4.0
 *
 * The `FileInfo` singleton implements most of the static methods from `QFileInfo`, check
 * [QFileInfo](https://doc.qt.io/qt-5/qfileinfo.html) documentation.
 */

FileInfo::FileInfo(QObject *parent)
    : QObject(parent)
{
}

FileInfo::~FileInfo() { }

/*!
 * \qmlmethod bool FileInfo::exists( string file)
 */
bool FileInfo::exists(const QString &file)
{
    return QFileInfo::exists(file);
}

/*!
 * \qmlmethod QFileInfoValueType FileInfo::create( string file)
 */
QFileInfoValueType FileInfo::create(const QString &file)
{
    return QFileInfoValueType(file);
}

}
