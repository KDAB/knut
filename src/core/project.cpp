/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "project.h"
#include "cppdocument.h"
#include "csharpdocument.h"
#include "dartdocument.h"
#include "imagedocument.h"
#include "jsondocument.h"
#include "logger.h"
#include "lsp/client.h"
#include "project_p.h"
#include "qmldocument.h"
#include "qttsdocument.h"
#include "qtuidocument.h"
#include "rcdocument.h"
#include "rustdocument.h"
#include "settings.h"
#include "slintdocument.h"
#include "textdocument.h"
#include "utils/log.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMetaEnum>
#include <QProcess>
#include <QStandardPaths>
#include <algorithm>
#include <kdalgorithms.h>
#include <map>
#include <unordered_set>

namespace Core {

/*!
 * \qmltype Project
 * \brief Singleton for handling the current project.
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
    LOG(newRoot);
    const QDir dir(newRoot);
    if (m_root == dir.absolutePath())
        return true;

    if (!m_root.isEmpty())
        for (auto client : m_lspClients | std::views::values)
            client->closeProject(m_root);

    spdlog::info("{}: {}", FUNCTION_NAME, dir.absolutePath());

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

    LOG(type);

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

    LOG(extension, type);

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

    LOG(extensions, type);

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
    case Document::Type::CSharp:
        return new CSharpDocument();
    case Document::Type::Json:
        return new JsonDocument();
    case Document::Type::Rust:
        return new RustDocument();
    case Document::Type::Dart:
        return new DartDocument();
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
            spdlog::error("{}: {} - unknown document type", FUNCTION_NAME, fi.suffix());
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
    LOG(LOG_ARG("path", fileName));

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

    LOG(LOG_ARG("path", fileName));

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
    LOG();

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
    LOG();

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
    LOG(index);

    Q_ASSERT(index < m_documents.size());
    index = m_documents.size() - index - 1;
    const QString &fileName = m_documents.at(index)->fileName();

    LOG_RETURN("document", open(fileName));
}

/*!
 * \qmlmethod array<object> Project::findInFiles(const QString &pattern)
 * Search for a regex pattern in all files of the current project using ripgrep.
 * Returns a list of results (QVariantMaps) with the document name and position ("file", "line", "column").
 *
 * Example usage in QML:
 *
 * ```js
 * let findResults = Project.findInFiles("foo");
 * for (let result of findResults) {
 *     Message.log("Filename: " + result.file);
 *     Message.log("Line: " + result.line);
 *     Message.log("Column" + result.column);
 * }
 * ```
 *
 * Note: The method uses ripgrep (rg) for searching, which must be installed and accessible in PATH.
 * The `pattern` parameter should be a valid regular expression.
 */
QVariantList Project::findInFiles(const QString &pattern) const
{
    LOG(pattern);

    QVariantList result;

    const QString path = QStandardPaths::findExecutable("rg");
    if (path.isEmpty()) {
        spdlog::error("Ripgrep (rg) executable not found. Please ensure that ripgrep is installed and its location is "
                      "included in the PATH environment variable.");
        return result;
    }

    if (pattern.trimmed().isEmpty()) {
        return result;
    }
    if (m_root.isEmpty()) {
        return result;
    }

    QProcess process;

    const QStringList arguments {"--vimgrep", "-U", "--multiline-dotall", pattern, m_root};

    process.start(path, arguments);
    if (!process.waitForFinished()) {
        spdlog::error("The ripgrep process failed: {}", process.errorString());
        return result;
    }

    const QString output = process.readAllStandardOutput();

    const QString errorOutput = process.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        spdlog::error("Ripgrep error: {}", errorOutput);
    }

    const auto lines = output.split('\n', Qt::SkipEmptyParts);
    result.reserve(lines.count() * 3);
    for (const QString &line : lines) {
        QString currentLine = line;
        currentLine.replace('\\', '/');
        const auto parts = currentLine.split(':');

        QString filePath;
        int offset = 0;
        if (parts.size() > 2 && parts[0].length() == 1 && parts[1].startsWith('/')) {
            filePath = parts[0] + ':' + parts[1];
            offset = 2;
        } else {
            filePath = parts[0];
            offset = 1;
        }

        if (parts.size() > offset + 1) {
            QVariantMap matchResult;
            matchResult.insert("file", filePath);
            matchResult.insert("line", parts[offset].toInt());
            matchResult.insert("column", parts[offset + 1].toInt());
            result.append(matchResult);
        }
    }
    return result;
}

/*!
 * \qmlmethod bool Project::isFindInFilesAvailable()
 * Checks if the ripgrep (rg) command-line tool is available on the system.
 */
bool Project::isFindInFilesAvailable() const
{
    QString rgPath = QStandardPaths::findExecutable("rg");
    if (rgPath.isEmpty()) {
        return false;
    } else {
        return true;
    }
}

} // namespace Core
