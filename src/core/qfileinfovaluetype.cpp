/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qfileinfovaluetype.h"

namespace Core {

/*!
 * \qmltype QFileInfoValueType
 * \brief Wrapper around the `QFileInfo` class.
 * \inqmlmodule Script
 * \ingroup Utilities/@last
 * \sa FileInfo
 *
 * The `QFileInfoValueType` is a wrapper around the `QFileInfo` C++ class, check
 * [QFileInfo](https://doc.qt.io/qt-6/qfileinfo.html) documentation. It can only be created using
 * [FileInfo](fileinfo.md].
 */

/*!
 * \qmlproperty bool QFileInfoValueType::exists
 */
/*!
 * \qmlproperty string QFileInfoValueType::filePath
 */
/*!
 * \qmlproperty string QFileInfoValueType::absoluteFilePath
 */
/*!
 * \qmlproperty string QFileInfoValueType::canonicalFilePath
 */
/*!
 * \qmlproperty string QFileInfoValueType::fileName
 */
/*!
 * \qmlproperty string QFileInfoValueType::baseName
 */
/*!
 * \qmlproperty string QFileInfoValueType::completeBaseName
 */
/*!
 * \qmlproperty string QFileInfoValueType::suffix
 */
/*!
 * \qmlproperty string QFileInfoValueType::bundleName
 */
/*!
 * \qmlproperty string QFileInfoValueType::completeSuffix
 */
/*!
 * \qmlproperty string QFileInfoValueType::path
 */
/*!
 * \qmlproperty string QFileInfoValueType::absolutePath
 */
/*!
 * \qmlproperty string QFileInfoValueType::canonicalPath
 */
/*!
 * \qmlproperty QDirValueType QFileInfoValueType::dir
 */
/*!
 * \qmlproperty QDirValueType QFileInfoValueType::absoluteDir
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isReadable
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isWritable
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isExecutable
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isHidden
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isRelative
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isAbsolute
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isFile
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isDir
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isSymLink
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isRoot
 */
/*!
 * \qmlproperty bool QFileInfoValueType::isBundle
 */
/*!
 * \qmlproperty string QFileInfoValueType::symLinkTarget
 */
/*!
 * \qmlproperty string QFileInfoValueType::owner
 */
/*!
 * \qmlproperty int QFileInfoValueType::ownerId
 */
/*!
 * \qmlproperty string QFileInfoValueType::group
 */
/*!
 * \qmlproperty int QFileInfoValueType::groupId
 */
/*!
 * \qmlproperty int QFileInfoValueType::size
 */
/*!
 * \qmlproperty Date QFileInfoValueType::created
 */
/*!
 * \qmlproperty Date QFileInfoValueType::lastModified
 */
/*!
 * \qmlproperty Date QFileInfoValueType::lastRead
 */

QFileInfoValueType::QFileInfoValueType() = default;

QFileInfoValueType::QFileInfoValueType(const QString &file)
    : m_fileInfoValue(file)
{
}

QString QFileInfoValueType::toString() const
{
    return QStringLiteral("QFileInfo(%1)").arg(m_fileInfoValue.filePath());
}

bool QFileInfoValueType::exists() const
{
    return m_fileInfoValue.exists();
}

QString QFileInfoValueType::filePath() const
{
    return m_fileInfoValue.filePath();
}

QString QFileInfoValueType::absoluteFilePath() const
{
    return m_fileInfoValue.absoluteFilePath();
}

QString QFileInfoValueType::canonicalFilePath() const
{
    return m_fileInfoValue.canonicalFilePath();
}

QString QFileInfoValueType::fileName() const
{
    return m_fileInfoValue.fileName();
}

QString QFileInfoValueType::baseName() const
{
    return m_fileInfoValue.baseName();
}

QString QFileInfoValueType::completeBaseName() const
{
    return m_fileInfoValue.completeBaseName();
}

QString QFileInfoValueType::suffix() const
{
    return m_fileInfoValue.suffix();
}

QString QFileInfoValueType::bundleName() const
{
    return m_fileInfoValue.bundleName();
}

QString QFileInfoValueType::completeSuffix() const
{
    return m_fileInfoValue.completeSuffix();
}

QString QFileInfoValueType::path() const
{
    return m_fileInfoValue.path();
}

QString QFileInfoValueType::absolutePath() const
{
    return m_fileInfoValue.absolutePath();
}

QString QFileInfoValueType::canonicalPath() const
{
    return m_fileInfoValue.canonicalPath();
}

QDirValueType QFileInfoValueType::dir() const
{
    return QDirValueType(m_fileInfoValue.dir());
}

QDirValueType QFileInfoValueType::absoluteDir() const
{
    return QDirValueType(m_fileInfoValue.absoluteDir());
}

bool QFileInfoValueType::isReadable() const
{
    return m_fileInfoValue.isReadable();
}

bool QFileInfoValueType::isWritable() const
{
    return m_fileInfoValue.isWritable();
}

bool QFileInfoValueType::isExecutable() const
{
    return m_fileInfoValue.isExecutable();
}

bool QFileInfoValueType::isHidden() const
{
    return m_fileInfoValue.isHidden();
}

bool QFileInfoValueType::isNativePath() const
{
    return m_fileInfoValue.isNativePath();
}

bool QFileInfoValueType::isRelative() const
{
    return m_fileInfoValue.isRelative();
}

bool QFileInfoValueType::isAbsolute() const
{
    return m_fileInfoValue.isAbsolute();
}

bool QFileInfoValueType::isFile() const
{
    return m_fileInfoValue.isFile();
}

bool QFileInfoValueType::isDir() const
{
    return m_fileInfoValue.isDir();
}

bool QFileInfoValueType::isSymLink() const
{
    return m_fileInfoValue.isSymLink();
}

bool QFileInfoValueType::isRoot() const
{
    return m_fileInfoValue.isRoot();
}

bool QFileInfoValueType::isBundle() const
{
    return m_fileInfoValue.isBundle();
}

QString QFileInfoValueType::symLinkTarget() const
{
    return m_fileInfoValue.symLinkTarget();
}

QString QFileInfoValueType::owner() const
{
    return m_fileInfoValue.owner();
}

uint QFileInfoValueType::ownerId() const
{
    return m_fileInfoValue.ownerId();
}

QString QFileInfoValueType::group() const
{
    return m_fileInfoValue.group();
}

uint QFileInfoValueType::groupId() const
{
    return m_fileInfoValue.groupId();
}

qint64 QFileInfoValueType::size() const
{
    return m_fileInfoValue.size();
}

QDateTime QFileInfoValueType::created() const
{
    return m_fileInfoValue.birthTime();
}

QDateTime QFileInfoValueType::lastModified() const
{
    return m_fileInfoValue.lastModified();
}

QDateTime QFileInfoValueType::lastRead() const
{
    return m_fileInfoValue.lastRead();
}

} // namespace Core
