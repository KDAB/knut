/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "file.h"
#include "logger.h"

#include <QFile>
#include <QTextStream>

namespace Core {

/*!
 * \qmltype File
 * \brief Singleton with methods to handle files.
 * \inqmlmodule Script
 * \ingroup Utilities
 *
 * The `File` singleton implements most of the static methods from `QFile`, check
 * [QFile](https://doc.qt.io/qt-6/qfile.html) documentation.
 */

File::File(QObject *parent)
    : QObject(parent)
{
}

File::~File() = default;

/*!
 * \qmlmethod bool File::copy(string fileName, string newName)
 */
bool File::copy(const QString &fileName, const QString &newName)
{
    LOG("File::copy", fileName, newName);
    return QFile::copy(fileName, newName);
}

/*!
 * \qmlmethod bool File::exists(string fileName)
 */
bool File::exists(const QString &fileName)
{
    LOG("File::exists", fileName);
    return QFile::exists(fileName);
}

/*!
 * \qmlmethod bool File::remove(string fileName)
 */
bool File::remove(const QString &fileName)
{
    LOG("File::remove", fileName);
    return QFile::remove(fileName);
}

/*!
 * \qmlmethod bool File::rename(string oldName, string newName)
 */
bool File::rename(const QString &oldName, const QString &newName)
{
    LOG("File::rename", oldName, newName);
    return QFile::rename(oldName, newName);
}

/*!
 * \qmlmethod bool File::touch(string fileName)
 */
bool File::touch(const QString &fileName)
{
    LOG("File::touch", fileName);
    QFile file(fileName);
    return file.open(QFile::Append);
}

/*!
 * \qmlmethod string File::readAll(string fileName)
 */
QString File::readAll(const QString &fileName)
{
    LOG("File::readAll", fileName);
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        return stream.readAll();
    }
    return {};
}

} // namespace Core
