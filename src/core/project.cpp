#include "project.h"

#include "rcdocument.h"
#include "settings.h"
#include "textdocument.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

#include <spdlog/spdlog.h>

#include <algorithm>
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

Project::Project(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;
}

Project::~Project()
{
    m_instance = nullptr;
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
        spdlog::info("Opening project {}", dir.absolutePath().toStdString());
    } else {
        spdlog::warn("Knut can't open a new project after loading one.");
        return false;
    }

    m_root = dir.absolutePath();
    Settings::instance()->loadProjectSettings(m_root);
    emit rootChanged();
    return true;
}

/*!
 * \qmlmethod array<string> Project::allFiles()
 * Returns all files in the current project.
 */
QStringList Project::allFiles() const
{
    if (m_root.isEmpty())
        return {};

    QDir dir(m_root);
    QDirIterator it(m_root, QDirIterator::Subdirectories);
    QStringList result;
    while (it.hasNext()) {
        it.next();
        auto fi = it.fileInfo();
        if (fi.isFile())
            result.push_back(dir.relativeFilePath(fi.absoluteFilePath()));
    }
    std::sort(result.begin(), result.end());
    return result;
}

/*!
 * \qmlmethod array<string> Project::allFilesWithExtension( string extension)
 * Returns all files with the `extension` given in the current project.
 */
QStringList Project::allFilesWithExtension(const QString &extension)
{
    if (m_root.isEmpty())
        return {};

    QDir dir(m_root);
    QDirIterator it(m_root, QDirIterator::Subdirectories);
    QStringList result;
    while (it.hasNext()) {
        it.next();
        auto fi = it.fileInfo();
        if (fi.isFile() && fi.suffix() == extension)
            result.push_back(dir.relativeFilePath(fi.absoluteFilePath()));
    }
    std::sort(result.begin(), result.end());
    return result;
}

/*!
 * \qmlmethod Document open(string fileName)
 * Opens a document for the given `fileName`. If the document already exists, returns the same instance, a document
 * can't be open twice. If the fileName is relative, use the root path as the base.
 */
Document *Project::open(QString fileName)
{
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

    static std::unordered_set<QString> TextSuffix = {"txt", "cpp", "h"};
    static std::unordered_set<QString> RcSuffix = {"rc"};

    if (findIt != m_documents.cend()) {
        doc = *findIt;
    } else {
        if (TextSuffix.contains(fi.suffix()))
            doc = new TextDocument(this);
        else if (RcSuffix.contains(fi.suffix()))
            doc = new RcDocument(this);

        if (doc) {
            doc->load(fileName);
            m_documents.push_back(doc);
        } else {
            spdlog::error("Document type is unmanaged in Knut: {}", fi.suffix().toStdString());
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

} // namespace Core
