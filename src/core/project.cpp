#include "project.h"

#include "cppdocument.h"
#include "rcdocument.h"
#include "settings.h"
#include "textdocument.h"
#include "uidocument.h"

#include "lsp/client.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMetaEnum>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <map>
#include <unordered_set>

namespace Core {

/*!
 * \qmltype Project
 * \brief Singleton for handling the current project.
 * \instantiates Core::Project
 * \inqmlmodule Script
 * \since 4.0
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

    const auto documents = findChildren<Document *>();
    for (auto document : documents)
        document->close();

    for (auto clients : m_lspClients)
        clients.second->shutdown();
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
    QDir dir(newRoot);
    if (m_root == dir.absolutePath())
        return true;

    if (m_root.isEmpty()) {
        spdlog::info("Project::setRoot {}", dir.absolutePath().toStdString());
    } else {
        spdlog::error("Project::setRoot - can't open a new project");
        return false;
    }

    m_root = dir.absolutePath();
    Settings::instance()->loadProjectSettings(m_root);
    for (auto clients : m_lspClients)
        clients.second->openProject(m_root);

    emit rootChanged();
    return true;
}

/*!
 * \qmlmethod array<string> Project::allFiles( PathType type = RelativeToRoot)
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

    spdlog::trace("Project::allFiles");

    QDir dir(m_root);
    QDirIterator it(m_root, QDirIterator::Subdirectories);
    QStringList result;
    while (it.hasNext()) {
        it.next();
        auto fi = it.fileInfo();
        if (fi.isFile())
            result.push_back(type == FullPath ? fi.absoluteFilePath() : dir.relativeFilePath(fi.absoluteFilePath()));
    }
    std::sort(result.begin(), result.end());
    return result;
}

/*!
 * \qmlmethod array<string> Project::allFilesWithExtension( string extension, PathType type = RelativeToRoot)
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

    spdlog::trace("Project::allFilesWithExtension {}", extension.toStdString());

    QDir dir(m_root);
    QDirIterator it(m_root, QDirIterator::Subdirectories);
    QStringList result;
    while (it.hasNext()) {
        it.next();
        auto fi = it.fileInfo();
        if (fi.isFile() && fi.suffix() == extension)
            result.push_back(type == FullPath ? fi.absoluteFilePath() : dir.relativeFilePath(fi.absoluteFilePath()));
    }
    std::sort(result.begin(), result.end());
    return result;
}

/*!
 * \qmlmethod array<string> Project::allFilesWithExtensions( array<string> extensions, PathType type = RelativeToRoot)
 * Returns all files with an extenstion from `extensions` in the current project.
 * `type` defines the type of path, and can be one of those values:
 *
 * - `Project.FullPath`
 * - `Project.RelativeToRoot`
 */
QStringList Project::allFilesWithExtensions(const QStringList &extensions, PathType type)
{
    if (m_root.isEmpty())
        return {};

    spdlog::trace("Project::allFilesWithExtensions {}", extensions.join('|').toStdString());

    QDir dir(m_root);
    QDirIterator it(m_root, QDirIterator::Subdirectories);
    QStringList result;
    while (it.hasNext()) {
        it.next();
        auto fi = it.fileInfo();
        if (fi.isFile() && extensions.contains(fi.suffix(), Qt::CaseInsensitive))
            result.push_back(type == FullPath ? fi.absoluteFilePath() : dir.relativeFilePath(fi.absoluteFilePath()));
    }
    std::sort(result.begin(), result.end());
    return result;
}

static Document *createDocument(const QString &suffix)
{
    static const char MimeTypes[] = "/mime_types";
    static const auto mimeTypes = Settings::instance()->value<std::map<std::string, Document::Type>>(MimeTypes);

    auto it = mimeTypes.find(suffix.toStdString());
    if (it == mimeTypes.end())
        return nullptr;

    switch (it->second) {
    case Document::Type::Cpp:
        return new CppDocument();
    case Document::Type::Text:
        return new TextDocument();
    case Document::Type::Rc:
        return new RcDocument();
    case Document::Type::Ui:
        return new UiDocument();
    }
    Q_UNREACHABLE();
    return nullptr;
}

Lsp::Client *Project::getClient(Document::Type type)
{
    static const char Lsp[] = "/lsp";
    static auto lspServers = Settings::instance()->value<std::vector<Settings::LspServer>>(Lsp);

    auto cit = m_lspClients.find(type);
    if (cit != m_lspClients.end())
        return cit->second;

    auto sit = std::find_if(lspServers.cbegin(), lspServers.cend(), [type](const Settings::LspServer &server) {
        return server.type == type;
    });
    if (sit == lspServers.cend())
        return nullptr;
    QString language(QMetaEnum::fromType<Document::Type>().key(static_cast<int>(type)));
    auto client = new Lsp::Client(language.toLower().toStdString(), sit->program, sit->arguments, this);
    if (client->initialize(m_root)) {
        m_lspClients[type] = client;
        return client;
    }
    return nullptr;
}

const QVector<Document *> &Project::documents() const
{
    return m_documents;
}

/*!
 * \qmlmethod Document Project::open(string fileName)
 * Opens a document for the given `fileName`. If the document already exists, returns the same instance, a document
 * can't be open twice. If the fileName is relative, use the root path as the base.
 */
Document *Project::open(QString fileName)
{
    spdlog::trace("Project::open {}", fileName.toStdString());

    QFileInfo fi(fileName);
    if (!fi.exists() && fi.isRelative())
        fileName = m_root + '/' + fileName;
    else
        fileName = fi.absoluteFilePath();

    if (m_current && m_current->fileName() == fileName)
        return m_current;

    auto findIt = std::find_if(m_documents.cbegin(), m_documents.cend(), [fileName](auto document) {
        return document->fileName() == fileName;
    });

    Document *doc = nullptr;

    if (findIt != m_documents.cend()) {
        doc = *findIt;
    } else {
        doc = createDocument(fi.suffix());
        if (doc) {
            if (auto textDocument = qobject_cast<TextDocument *>(doc))
                textDocument->setLspClient(getClient(doc->type()));
            doc->setParent(this);
            doc->load(fileName);
            m_documents.push_back(doc);
            Q_EMIT documentsChanged();
        } else {
            spdlog::error("Project::open {} - unknow document type", fi.suffix().toStdString());
            return nullptr;
        }
    }

    m_current = doc;
    emit currentDocumentChanged();

    return doc;
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
    spdlog::trace("Project::saveAllDocuments");

    for (auto d : std::as_const(m_documents)) {
        if (d->hasChanged()) {
            d->save();
        }
    }
}

} // namespace Core
