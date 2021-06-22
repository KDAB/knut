#include "project.h"

#include "rcdocument.h"
#include "settings.h"
#include "textdocument.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFileSystemModel>

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
 * \qmlproperty string Project::fileModel
 * Qt model for the current root path. It is created on-demand.
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
    if (m_root == newRoot)
        return true;

    if (m_root.isEmpty()) {
        spdlog::info("Opening project {}", newRoot.toStdString());
    } else {
        spdlog::warn("Knut can't open a new project after loading one.");
        return false;
    }

    m_root = newRoot;
    Settings::instance()->loadProjectSettings(newRoot);
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
    if (fi.isRelative())
        fileName = m_root + '/' + fileName;

    auto findIt = std::find_if(m_documents.cbegin(), m_documents.cend(), [fileName](auto document) {
        return document->fileName() == fileName;
    });
    if (findIt != m_documents.cend())
        return *findIt;

    // TODO update once CppDocument exist
    static std::unordered_set<QString> textSuffix = {"txt", "cpp", "h"};

    Document *doc = nullptr;
    if (textSuffix.contains(fi.suffix()))
        doc = new TextDocument(this);
    else if (fi.suffix() == "rc")
        doc = new RcDocument(this);

    if (doc) {
        doc->load(fileName);
        m_documents.push_back(doc);
    } else {
        spdlog::critical("Document type is unmanaged in Knut: {}", fi.suffix().toStdString());
    }
    return doc;
}

QAbstractItemModel *Project::fileModel() const
{
    // Only create on-demand
    static QFileSystemModel *model = new QFileSystemModel(const_cast<Core::Project *>(this));
    model->setRootPath(m_root);
    return model;
}

} // namespace Core
