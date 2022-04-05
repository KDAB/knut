#include "scriptmanager.h"

#include "logger.h"
#include "scriptrunner.h"
#include "settings.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTextStream>
#include <QTimer>

#include <spdlog/spdlog.h>

namespace Core {

ScriptManager::ScriptManager(QObject *parent)
    : QObject(parent)
    , m_watcher(new QFileSystemWatcher(this))
    , m_runner(new ScriptRunner(this))
{
    m_instance = this;

    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &ScriptManager::updateScriptDirectory);
    connect(Settings::instance(), &Settings::settingsLoaded, this, &ScriptManager::updateDirectories);
    updateDirectories();
}

ScriptManager::~ScriptManager()
{
    m_instance = nullptr;
}

ScriptManager *ScriptManager::instance()
{
    Q_ASSERT(m_instance);
    return m_instance;
}

const ScriptManager::ScriptList &ScriptManager::scriptList() const
{
    return m_scriptList;
}

void ScriptManager::addDirectory(const QString &path)
{
    addScriptsFromPath(path);
    Settings::instance()->addScriptPath(path);
}

void ScriptManager::removeDirectory(const QString &path)
{
    removeScriptsFromPath(path);
    Settings::instance()->removeScriptPath(path);
}

QStringList ScriptManager::directories() const
{
    return m_directories;
}

void ScriptManager::runScript(const QString &fileName, bool async, bool log)
{
    if (log)
        spdlog::debug("==> Start script {}", fileName.toStdString());
    ScriptRunner::EndScriptFunc logEndScript;
    if (log)
        logEndScript = [fileName]() {
            spdlog::debug("<== End script {}", fileName.toStdString());
        };

    if (async)
        QTimer::singleShot(0, this, [this, fileName, logEndScript]() {
            doRunScript(fileName, logEndScript);
        });
    else
        doRunScript(fileName, logEndScript);
}

void ScriptManager::addScript(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QTextStream stream(&file);
    const QString &line = stream.readLine();
    const QString description = line.startsWith("//") ? line.mid(2).simplified() : "";
    QFileInfo fi(fileName);
    m_scriptList.push_back(Script {fi.fileName(), fileName, description});
}

static QStringList scriptListFromDir(const QString &path)
{
    const QStringList filter {"*.js", "*.qml"};
    QDirIterator it(path, filter, QDir::Files);
    QStringList files;
    files.reserve(filter.count());
    while (it.hasNext())
        files << it.next();
    return files;
}

void ScriptManager::updateScriptDirectory(const QString &path)
{
    QStringList files = scriptListFromDir(path);

    // Remove deleted scripts
    auto it = m_scriptList.begin();
    while (it != m_scriptList.end()) {
        if (files.contains(it->fileName)) {
            files.removeAll(it->fileName);
            ++it;
        } else {
            it = m_scriptList.erase(it);
        }
    }

    // Add new scripts
    for (const auto &fileName : files)
        addScript(fileName);
}

void ScriptManager::addScriptsFromPath(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists())
        return;
    Q_ASSERT(path == fi.absoluteFilePath());

    // Check that the path does not already exist
    if (m_directories.contains(path))
        return;

    m_directories.append(path);
    m_watcher->addPath(path);

    const QStringList files = scriptListFromDir(path);
    for (const auto &fileName : files)
        addScript(fileName);
}

void ScriptManager::removeScriptsFromPath(const QString &path)
{
    m_directories.removeAll(path);

    if (m_watcher->directories().contains(path))
        m_watcher->removePath(path);

    const QStringList files = scriptListFromDir(path);
    auto it = m_scriptList.begin();
    while (it != m_scriptList.end()) {
        if (files.contains(it->fileName))
            it = m_scriptList.erase(it);
        else
            ++it;
    }
}

void ScriptManager::doRunScript(const QString &fileName, std::function<void()> endFunc)
{
    auto result = m_runner->runScript(fileName, endFunc);
    if (m_runner->hasError()) {
        const auto errors = m_runner->errors();
        for (const auto &error : errors)
            spdlog::error("{}({}): {}", error.url().toLocalFile().toStdString(), error.line(),
                          error.description().toStdString());
    } else {
        if (result.isValid())
            spdlog::info("Script result is {}", result.toString().toStdString());
    }
    emit scriptFinished(result);
}

void ScriptManager::updateDirectories()
{
    LoggerDisabler ld(true);
    const auto directories = Settings::instance()->value<QStringList>(Settings::ScriptPaths);
    for (const auto &path : directories)
        addScriptsFromPath(path);
}

} // namespace Core
