/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qdirvaluetype.h"

namespace Core {

/*!
 * \qmltype QDirValueType
 * \brief Wrapper around the `QDir` class.
 * \inqmlmodule Script
 * \ingroup Utilities/@last
 * \sa Dir
 *
 * The `QDirValueType` is a wrapper around the `QDir` C++ class, check [QDir](https://doc.qt.io/qt-6/qdir.html)
 * documentation. It can only be created using [Dir](dir.md) singleton.
 */

/*!
 * \qmlproperty string QDirValueType::path
 */
/*!
 * \qmlproperty string QDirValueType::absolutePath
 */
/*!
 * \qmlproperty string QDirValueType::canonicalPath
 */
/*!
 * \qmlproperty string QDirValueType::dirName
 */
/*!
 * \qmlproperty int QDirValueType::count
 */
/*!
 * \qmlproperty bool QDirValueType::isReadable
 */
/*!
 * \qmlproperty bool QDirValueType::exists
 */
/*!
 * \qmlproperty bool QDirValueType::isRoot
 */
/*!
 * \qmlproperty bool QDirValueType::isRelative
 */
/*!
 * \qmlproperty bool QDirValueType::isAbsolute
 */

QDirValueType::QDirValueType(const QString &path)
    : m_dirValue(path)
{
}

QDirValueType::QDirValueType(const QDir &dir)
    : m_dirValue(dir)
{
}

QString QDirValueType::toString() const
{
    return QStringLiteral("QDir(%1)").arg(m_dirValue.path());
}

QString QDirValueType::path() const
{
    return m_dirValue.path();
}

void QDirValueType::setPath(const QString &path)
{
    m_dirValue.setPath(path);
}

QString QDirValueType::absolutePath() const
{
    return m_dirValue.absolutePath();
}

QString QDirValueType::canonicalPath() const
{
    return m_dirValue.canonicalPath();
}

QString QDirValueType::dirName() const
{
    return m_dirValue.dirName();
}

qsizetype QDirValueType::count() const
{
    return m_dirValue.count();
}

bool QDirValueType::isReadable() const
{
    return m_dirValue.isReadable();
}

bool QDirValueType::exists() const
{
    return m_dirValue.exists();
}

bool QDirValueType::isRoot() const
{
    return m_dirValue.isRoot();
}

bool QDirValueType::isRelative() const
{
    return m_dirValue.isRelative();
}

bool QDirValueType::isAbsolute() const
{
    return m_dirValue.isAbsolute();
}

/*!
 * \qmlmethod bool QDirValueType::cd(string dirName)
 */
bool QDirValueType::cd(const QString &dirName)
{
    return m_dirValue.cd(dirName);
}

/*!
 * \qmlmethod bool QDirValueType::cdUp()
 */
bool QDirValueType::cdUp()
{
    return m_dirValue.cdUp();
}

/*!
 * \qmlmethod string QDirValueType::at(int pos)
 */
QString QDirValueType::at(int pos) const
{
    // We have to check here, as we don't want any errors
    if (pos >= 0 && pos <= count())
        return m_dirValue[pos];
    return QString();
}

/*!
 * \qmlmethod array<string> QDirValueType::entryList(int filters, int sort)
 * \qmlmethod array<string> QDirValueType::entryList(string nameFilter, int filters, int sort)
 * \qmlmethod array<string> QDirValueType::entryList(array<string> nameFilters, int filters, int sort)
 * `filters` is a combination of (default is `Dir.NoFilter`):
 *
 * - `Dir.Dirs`
 * - `Dir.Files`
 * - `Dir.Drives`
 * - `Dir.NoSymLinks`
 * - `Dir.AllEntries`
 * - `Dir.TypeMask`
 * - `Dir.Readable`
 * - `Dir.Writable`
 * - `Dir.Executable`
 * - `Dir.PermissionMask`
 * - `Dir.Modified`
 * - `Dir.Hidden`
 * - `Dir.System`
 * - `Dir.AccessMask`
 * - `Dir.AllDirs`
 * - `Dir.CaseSensitive`
 * - `Dir.NoDot`
 * - `Dir.NoDotDot`
 * - `Dir.NoDotAndDotDot`
 * - `Dir.NoFilter`
 *
 * `sort` is a combination of (default is `Dir.NoSort`):
 *
 * - `Dir.Name`
 * - `Dir.Time`
 * - `Dir.Size`
 * - `Dir.Unsorted`
 * - `Dir.SortByMask`
 * - `Dir.DirsFirst`
 * - `Dir.Reversed`
 * - `Dir.IgnoreCase`
 * - `Dir.DirsLast`
 * - `Dir.LocaleAware`
 * - `Dir.Type`
 * - `Dir.NoSory`
 */
QStringList QDirValueType::entryList(int filters, int sort) const
{
    return m_dirValue.entryList(static_cast<QDir::Filters>(filters), static_cast<QDir::SortFlags>(sort));
}

QStringList QDirValueType::entryList(const QString &nameFilter, int filters, int sort) const
{
    return m_dirValue.entryList({nameFilter}, static_cast<QDir::Filters>(filters), static_cast<QDir::SortFlags>(sort));
}

QStringList QDirValueType::entryList(const QStringList &nameFilters, int filters, int sort) const
{
    return m_dirValue.entryList(nameFilters, static_cast<QDir::Filters>(filters), static_cast<QDir::SortFlags>(sort));
}

/*!
 * \qmlmethod bool QDirValueType::mkdir(string dirName)
 */
bool QDirValueType::mkdir(const QString &dirName) const
{
    return m_dirValue.mkdir(dirName);
}

/*!
 * \qmlmethod bool QDirValueType::rmdir(string dirName)
 */
bool QDirValueType::rmdir(const QString &dirName) const
{
    return m_dirValue.rmdir(dirName);
}

/*!
 * \qmlmethod bool QDirValueType::mkpath(string dirPath)
 */
bool QDirValueType::mkpath(const QString &dirPath) const
{
    return m_dirValue.mkpath(dirPath);
}

/*!
 * \qmlmethod bool QDirValueType::rmpath(string dirPath)
 */
bool QDirValueType::rmpath(const QString &dirPath) const
{
    return m_dirValue.rmpath(dirPath);
}

/*!
 * \qmlmethod bool QDirValueType::removeRecursively()
 */
bool QDirValueType::removeRecursively()
{
    return m_dirValue.removeRecursively();
}

/*!
 * \qmlmethod bool QDirValueType::makeAbsolute()
 */
bool QDirValueType::makeAbsolute()
{
    return m_dirValue.makeAbsolute();
}

/*!
 * \qmlmethod bool QDirValueType::remove(string fileName)
 */
bool QDirValueType::remove(const QString &fileName)
{
    return m_dirValue.remove(fileName);
}

/*!
 * \qmlmethod bool QDirValueType::rename(string oldName, string newName)
 */
bool QDirValueType::rename(const QString &oldName, const QString &newName)
{
    return m_dirValue.rename(oldName, newName);
}

/*!
 * \qmlmethod bool QDirValueType::fileExists(string name)
 */
bool QDirValueType::fileExists(const QString &name) const
{
    return m_dirValue.exists(name);
}

} // namespace Core
