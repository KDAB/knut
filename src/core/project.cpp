/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "project.h"
#include "cppdocument.h"
#include "imagedocument.h"
#include "jsondocument.h"
#include "logger.h"
#include "lsp/client.h"
#include "project_p.h"
#include "qmldocument.h"
#include "qttsdocument.h"
#include "qtuidocument.h"
#include "rcdocument.h"
#include "settings.h"
#include "slintdocument.h"
#include "textdocument.h"
#include "utils/log.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMetaEnum>
#include <algorithm>
#include <kdalgorithms.h>
#include <map>
#include <unordered_set>

namespace Core {

/*!
 * \qmltype Project
 * \brief Singleton for handling the current project.
 * \inqmlmodule Script
 * The `Project` object is not meant to open multiple projects, but only open one.
 */

/*!
 * \qmlproperty string Project::root
 * Current root path of the project, this can be set only once.
 */

/*!
 * \qmlproperty string Project::currentDocument
 * Current document opened in the project.
 */

/*!
 * \qmlproperty array<string> Project::documents
 * Return all documents opened in the project.
 */

Project::Project(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;
}

Project::~Project()
{
    m_instance = nullptr;

    closeAll();

    for (auto client : m_lspClients | std::views::values)
        client->shutdown();
}

Project *Project::instance()
{
    Q_ASSERT(m_instance);
    return m_instance;
}

const QString &Project::root() const
{
    return m_root;
}

bool Project::setRoot(const QString &newRoot)
{
    LOG("Project::setRoot", newRoot);
    const QDir dir(newRoot);
    if (m_root == dir.absolutePath())
        return true;

    if (m_root.isEmpty()) {
        spdlog::info("Project::setRoot {}", dir.absolutePath());
    } else {
        spdlog::error("Project::setRoot - can't open a new project");
        return false;
    }

    m_root = dir.absolutePath();
    Settings::instance()->loadProjectSettings(m_root);
    for (auto client : m_lspClients | std::views::values)
        client->openProject(m_root);

    emit rootChanged();
    return true;
}

/*!
 * \qmlmethod array<string> Project::allFiles(PathType type = RelativeToRoot)
 * Returns all files in the current project.
 * `type` defines the type of path, and can be one of those values:
 *
 * - `Project.FullPath`
 * - `Project.RelativeToRoot`
 */
QStringList Project::allFiles(PathType type) const
{
    if (m_root.isEmpty())
        return {};

    LOG("Project::allFiles", type);

    QDir dir(m_root);
    QDirIterator it(m_root, QDirIterator::Subdirectories);
    QStringList result;
    while (it.hasNext()) {
        it.next();
        auto fi = it.fileInfo();
        if (fi.isFile())
            result.push_back(type == FullPath ? fi.absoluteFilePath() : dir.relativeFilePath(fi.absoluteFilePath()));
    }
    std::ranges::sort(result);
    return result;
}

/*!
 * \qmlmethod array<string> Project::allFilesWithExtension(string extension, PathType type = RelativeToRoot)
 * Returns all files with the `extension` given in the current project.
 * `type` defines the type of path, and can be one of those values:
 *
 * - `Project.FullPath`
 * - `Project.RelativeToRoot`
 */
QStringList Project::allFilesWithExtension(const QString &extension, PathType type)
{
    if (m_root.isEmpty())
        return {};

    LOG("Project::allFilesWithExtension", extension, type);

    QDir dir(m_root);
    QDirIterator it(m_root, QDirIterator::Subdirectories);
    QStringList result;
    while (it.hasNext()) {
        it.next();
        auto fi = it.fileInfo();
        if (fi.isFile() && fi.suffix() == extension)
            result.push_back(type == FullPath ? fi.absoluteFilePath() : dir.relativeFilePath(fi.absoluteFilePath()));
    }
    std::ranges::sort(result);
    return result;
}

/*!
 * \qmlmethod array<string> Project::allFilesWithExtensions(array<string> extensions, PathType type = RelativeToRoot)
 * Returns all files with an extension from `extensions` in the current project.
 * `type` defines the type of path, and can be one of those values:
 *
 * - `Project.FullPath`
 * - `Project.RelativeToRoot`
 */
QStringList Project::allFilesWithExtensions(const QStringList &extensions, PathType type)
{
    if (m_root.isEmpty())
        return {};

    LOG("Project::allFilesWithExtensions", extensions, type);

    QDir dir(m_root);
    QDirIterator it(m_root, QDirIterator::Subdirectories);
    QStringList result;
    while (it.hasNext()) {
        it.next();
        auto fi = it.fileInfo();
        if (fi.isFile() && extensions.contains(fi.suffix(), Qt::CaseInsensitive))
            result.push_back(type == FullPath ? fi.absoluteFilePath() : dir.relativeFilePath(fi.absoluteFilePath()));
    }
    std::ranges::sort(result);
    return result;
}

static Document *createDocument(const QString &suffix)
{
    static const auto mimeTypes =
        Settings::instance()->value<std::map<std::string, Document::Type>>(Settings::MimeTypes);

    auto it = mimeTypes.find(suffix.toStdString());
    if (it == mimeTypes.end()) {
        // No mime found, so, just open it as text
        return new TextDocument();
    }

    switch (it->second) {
    case Document::Type::Cpp:
        return new CppDocument();
    case Document::Type::Text:
        return new TextDocument();
    case Document::Type::Rc:
        return new RcDocument();
    case Document::Type::QtUi:
        return new QtUiDocument();
    case Document::Type::Image:
        return new ImageDocument();
    case Document::Type::Slint:
        return new SlintDocument();
    case Document::Type::QtTs:
        return new QtTsDocument();
    case Document::Type::Qml:
        return new QmlDocument();
    case Document::Type::Json:
        return new JsonDocument();
    default:
        return new TextDocument();
    }
    Q_UNREACHABLE();
    return nullptr;
}

Lsp::Client *Project::getClient(Document::Type type)
{
    // Check if we use LSP
    if (!Settings::instance()->hasLsp())
        return nullptr;

    static auto lspServers = Settings::instance()->value<std::vector<LspServer>>(Settings::LspServers);

    auto cit = m_lspClients.find(type);
    if (cit != m_lspClients.end())
        return cit->second;

    auto sit = kdalgorithms::find_if(lspServers, [type](const LspServer &server) {
        return server.type == type;
    });
    if (!sit)
        return nullptr;
    QString language(QMetaEnum::fromType<Document::Type>().key(static_cast<int>(type)));
    auto client = new Lsp::Client(language.toLower().toStdString(), sit->program, sit->arguments, this);
    if (client->initialize(m_root)) {
        m_lspClients[type] = client;
        return client;
    }
    return nullptr;
}

const QList<Document *> &Project::documents() const
{
    return m_documents;
}

Document *Project::getDocument(QString fileName, bool moveToBack)
{
    QFileInfo fi(fileName);
    if (!fi.exists() && fi.isRelative())
        fileName = m_root + '/' + fileName;
    else
        fileName = fi.absoluteFilePath();

    auto findIt = std::ranges::find_if(m_documents, [fileName](auto document) {
        return document->fileName() == fileName;
    });

    Document *doc = nullptr;

    if (findIt != m_documents.end()) {
        doc = *findIt;
        if (moveToBack)
            std::rotate(findIt, findIt + 1, m_documents.end());
    } else {
        doc = createDocument(fi.suffix());
        if (doc) {
            if (auto codeDocument = qobject_cast<CodeDocument *>(doc))
                codeDocument->setLspClient(getClient(doc->type()));
            doc->setParent(this);
            doc->load(fileName);
            m_documents.push_back(doc);
            emit documentsChanged();
        } else {
            spdlog::error("Project::open {} - unknown document type", fi.suffix());
            return nullptr;
        }
    }
    return doc;
}

/*!
 * \qmlmethod Document Project::get(string fileName)
 * Gets the document for the given `fileName`. If the document is not opened yet, open it. If the document
 * is already opened, returns the same instance, a document can't be open twice. If the fileName is relative, use the
 * root path as the base.
 *
 * If the document does not exist, creates a new document (but don't save it yet).
 *
 * !!! note
 *     This command does not change the current document.
 */
Document *Project::get(const QString &fileName)
{
    LOG("Project::get", LOG_ARG("path", fileName));

    LOG_RETURN("document", getDocument(fileName, false));
}

/*!
 * \qmlmethod Document Project::open(string fileName)
 * Opens or creates a document for the given `fileName` and make it current. If the document is already opened, returns
 * the same instance, a document can't be open twice. If the fileName is relative, use the root path as the base.
 *
 *  If the document does not exist, creates a new document (but don't save it yet).
 */
Document *Project::open(const QString &fileName)
{
    if (m_current && m_current->fileName() == fileName)
        return m_current;

    LOG("Project::open", LOG_ARG("path", fileName));

    m_current = getDocument(fileName, true);
    emit currentDocumentChanged(m_current);

    LOG_RETURN("document", m_current);
}

/*!
 * \qmlmethod Project::closeAll()
 * Close all documents. If the document has some changes, save the changes.
 */
void Project::closeAll()
{
    LOG("Project::closeAll");
    for (auto d : std::as_const(m_documents))
        d->close();
}

Core::Document *Project::currentDocument() const
{
    return m_current;
}

/*!
 * \qmlmethod Project::saveAllDocuments()
 * Save all Documents opened in project.
 */
void Project::saveAllDocuments()
{
    LOG("Project::saveAllDocuments");

    for (auto d : std::as_const(m_documents)) {
        if (d->hasChanged()) {
            d->save();
        }
    }
}

/*!
 * \qmlmethod Project::openPrevious(int index = 1)
 * Open a previously opened document. `index` is the position of this document in the last opened document.
 *
 * `document.openPrevious(1)` (the default) opens the last document, like Ctrl+Tab in any editors.
 */
Document *Project::openPrevious(int index)
{
    LOG("Project::openPrevious", index);

    Q_ASSERT(index < m_documents.size());
    index = m_documents.size() - index - 1;
    const QString &fileName = m_documents.at(index)->fileName();

    LOG_RETURN("document", open(fileName));
}

} // namespace Core
