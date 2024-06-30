/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "document.h"
#include "logger.h"
#include "utils/log.h"

#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QUrl>

namespace Core {

/*!
 * \qmltype Document
 * \brief Base class for all documents
 * \inqmlmodule Script
 *
 * The `Document` class is the base class for all documents.
 * A document is a file loaded by Knut and that can be used in script (either to get data or to edit).
 */

/*!
 * \qmlproperty string Document::fileName
 * Filename of the current document, changing it will load a new file **without changing the type**. It's better to use
 * a new `Document` to open a new file.
 *
 * Changing the filename will:
 * - save the current document automatically
 * - load the new document
 * - put an error in `errorString` if it can't be loaded
 * \sa Document::load
 */
/*!
 * \qmlproperty bool Document::exists
 * Returns true if the document is a file on the disk, otherwise returns false.
 */
/*!
 * \qmlproperty Type Document::type
 * Returns the current type of the document, please note that the type is fixed once, and won't change. Available types
 * are:
 *
 * - `Document.Text`
 * - `Document.Rc`
 */
/*!
 * \qmlproperty string Document::errorString
 * Returns the error string if an error occurred while loading the document, otherwise returns an empty string.
 */
/*!
 * \qmlproperty bool Document::hasChanged
 * Returns true if the document has been edited, otherwise returns false.
 */

Document::Document(Type type, QObject *parent)
    : QObject(parent)
    , m_type(type)
{
}

const QString &Document::fileName() const
{
    return m_fileName;
}

void Document::setFileName(const QString &newFileName)
{
    LOG("Document::setFileName", newFileName);
    if (m_fileName == newFileName)
        return;
    load(newFileName);
}

bool Document::exists() const
{
    return !m_fileName.isEmpty() && QFile::exists(m_fileName);
}

Document::Type Document::type() const
{
    return m_type;
}

const QString &Document::errorString() const
{
    return m_errorString;
}

void Document::setErrorString(const QString &error)
{
    if (m_errorString == error)
        return;
    m_errorString = error;
    emit errorStringChanged();
}

Document::ConflictResolution Document::resolveConflictsOnSave() const
{
    const QFileInfo fi(m_fileName);
    const auto result = QMessageBox::question(
        QApplication::activeWindow(), tr("File changed externally"),
        tr("%1\n\nThe file has unsaved changes inside this editor and has been changed externally.\n"
           "Do you want to overwrite the changes on the disk?")
            .arg(fi.fileName()));
    return result == QMessageBox::Yes ? Core::Document::OverwriteDiskChanges : Core::Document::KeepDiskChanges;
};

bool Document::hasChanged() const
{
    return m_hasChanged;
}

bool Document::hasChangedOnDisk() const
{
    if (!QFile::exists(m_fileName))
        return false;

    // Don't update if it was saved by Knut
    if (const QFileInfo fi(m_fileName); fi.lastModified() == m_lastModified)
        return false;
    return true;
}

void Document::reload()
{
    doLoad(m_fileName);
    const QFileInfo fi(m_fileName);
    m_lastModified = fi.lastModified();
    emit fileUpdated();
}

/*!
 * \qmlmethod bool Document::load(string fileName)
 * Load the document `fileName` **without changing the type**. If the current document has some changes, save them
 * automatically. In case of error put the error text in the `errorString` property.
 */
bool Document::load(const QString &fileName)
{
    LOG("Document::load", fileName);
    if (fileName.isEmpty()) {
        spdlog::warn("Document::load - fileName is empty");
        return false;
    }
    if (m_fileName == fileName)
        return true;

    close();
    const bool loadDone = doLoad(fileName);
    m_fileName = fileName;
    const QFileInfo fi(m_fileName);
    m_lastModified = fi.lastModified();

    didOpen();
    emit fileNameChanged();
    return loadDone;
}

/*!
 * \qmlmethod bool Document::save()
 * Save the current document, in case of error put the error text in the `errorString` property.
 */
bool Document::save()
{
    LOG("Document::save");
    return saveAs(m_fileName);
}

/*!
 * \qmlmethod bool Document::saveAs(string fileName)
 * Save the current document as fileName, the previous file (if it exists) is not changed, and the current document
 * takes the new `fileName`. In case of error put the error text in the `errorString` property.
 */
bool Document::saveAs(const QString &fileName)
{
    LOG("Document::saveAs", fileName);
    if (fileName.isEmpty()) {
        spdlog::error("Document::saveAs - fileName is empty");
        return false;
    }

    const bool isNewName = m_fileName != fileName;
    if (isNewName) {
        // We suppose that if the file exists, the user already agreed to overwrite it
        if (!m_fileName.isEmpty())
            didClose();
        m_fileName = fileName;
        emit fileNameChanged();
    } else {
        if (hasChangedOnDisk() && resolveConflictsOnSave() == KeepDiskChanges) {
            return false;
        }
    }

    const bool saveDone = doSave(m_fileName);
    if (saveDone) {
        setHasChanged(false);
        if (isNewName)
            didOpen();
        const QFileInfo fi(m_fileName);
        m_lastModified = fi.lastModified();
    }
    return saveDone;
}

/*!
 * \qmlmethod bool Document::close()
 * Close the current document. If the current document has some changes, save them
 * automatically.
 */
void Document::close()
{
    LOG("Document::close");
    if (m_fileName.isEmpty())
        return;
    if (m_hasChanged)
        save();
    didClose();
    m_fileName.clear();
}

void Document::setHasChanged(bool newHasChanged)
{
    if (m_hasChanged == newHasChanged)
        return;
    m_hasChanged = newHasChanged;
    emit hasChangedChanged();
}

} // namespace Core
