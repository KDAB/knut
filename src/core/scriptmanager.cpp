#include "scriptmanager.h"

#include "scriptrunner.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTimer>

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Core {

ScriptManager::ScriptManager(QObject *parent)
    : QObject(parent)
    , m_watcher(new QFileSystemWatcher(this))
    , m_runner(new ScriptRunner(this))
{
    m_instance = this;

    m_logger = spdlog::get("script");
    if (!m_logger) {
        m_logger = spdlog::stdout_color_mt("script");
        m_logger->set_level(spdlog::level::debug);
    }

    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &ScriptManager::updateScriptDirectory);
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

ScriptManager::ScriptList ScriptManager::scriptList() const
{
    return m_scriptList;
}

void ScriptManager::addDirectory(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists())
        return;

    // Check that the path does not already exist as global
    if (m_directories.contains(path))
        return;

    m_directories.append(path);
    addScriptsFromPath(path);
}

void ScriptManager::removeDirectory(const QString &path)
{
    m_directories.removeAll(path);
    removeScriptsFromPath(path);
}

void ScriptManager::runScript(const QString &fileName, bool async, bool log)
{
    m_logger->info("==> Start script {}", fileName.toStdString());
    ScriptRunner::EndScriptFunc logEndScript;
    if (log)
        logEndScript = [this, fileName]() {
            m_logger->info("<== End script {}", fileName.toStdString());
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
    QFileInfo fi(fileName);

    if (!fi.isReadable())
        return;

    Script script;
    script.fileName = fileName;
    script.name = fi.fileName();
    m_scriptList.push_back(script);
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
    m_watcher->addPath(path);

    const QStringList files = scriptListFromDir(path);
    for (const auto &fileName : files)
        addScript(fileName);
}

void ScriptManager::removeScriptsFromPath(const QString &path)
{
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
            m_logger->error("{}({}): {}", QDir::toNativeSeparators(error.url().toLocalFile()).toStdString(),
                            error.line(), error.description().toStdString());
    } else {
        if (result.isValid())
            m_logger->debug("Script result is {}", result.toString().toStdString());
    }
    emit scriptFinished(result);
}

}
