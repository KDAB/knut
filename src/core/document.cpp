#include "document.h"

#include <QFile>

#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype Document
 * \brief Base class for all documents
 * \instantiates Core::Document
 * \inqmlmodule Script
 * \since 4.0
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
 * - `Document.Text`
 */
/*!
 * \qmlproperty string Document::errorString
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
    if (m_fileName == newFileName)
        return;
    m_fileName = newFileName;
    emit fileNameChanged();
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

bool Document::hasChanged() const
{
    return m_hasChanged;
}

/*!
 * \qmlmethod bool Document::load(string fileName)
 * Load the document `fileName` **without changing the type**. If the current document has some changes, save them
 * automatically. In case of error put the error text in the `errorString` property.
 */
bool Document::load(const QString &fileName)
{
    spdlog::trace("Loading document {}", fileName.toStdString());
    if (fileName.isEmpty()) {
        spdlog::warn("Can't load document: fileName is empty");
        return false;
    }
    if (m_fileName == fileName)
        return true;
    if (m_hasChanged)
        save();
    m_fileName = fileName;
    emit fileNameChanged();
    return doLoad(m_fileName);
}

/*!
 * \qmlmethod bool Document::save()
 * Save the current document, in case of error put the error text in the `errorString` property.
 */
bool Document::save()
{
    spdlog::trace("Saving document {}", m_fileName.toStdString());
    if (m_fileName.isEmpty()) {
        spdlog::warn("Can't save document: fileName is empty");
        return false;
    }
    bool saveDone = doSave(m_fileName);
    if (saveDone)
        setHasChanged(false);
    return saveDone;
}

/*!
 * \qmlmethod bool Document::saveAs( string fileName)
 * Save the current document as fileName, the previous file (if it exists) is not changed, and the current document
 * takes the new `fileName`. In case of error put the error text in the `errorString` property.
 */
bool Document::saveAs(const QString &fileName)
{
    spdlog::trace("Saving document {} as {}", m_fileName.toStdString(), fileName.toStdString());
    if (m_fileName == fileName)
        return doSave(m_fileName);
    m_fileName = fileName;
    emit fileNameChanged();
    return save();
}

void Document::setHasChanged(bool newHasChanged)
{
    if (m_hasChanged == newHasChanged)
        return;
    m_hasChanged = newHasChanged;
    emit hasChangedChanged();
}

} // namespace Core
