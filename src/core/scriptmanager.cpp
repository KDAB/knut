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

#include <kdalgorithms.h>
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

void ScriptManager::runScriptInContext(const QString &fileName, const QueryMatch &context, bool async, bool log)
{
    if (log)
        spdlog::debug("==> Start script {} -- context: {}", fileName.toStdString(), context.toString().toStdString());
    ScriptRunner::EndScriptFunc logEndScript;
    if (log)
        logEndScript = [fileName]() {
            spdlog::debug("<== End script {}", fileName.toStdString());
        };

    if (async)
        QTimer::singleShot(0, this, [this, fileName, logEndScript, context = std::move(context)]() {
            doRunScript(fileName, logEndScript, context);
        });
    else
        doRunScript(fileName, logEndScript, context);
}

static QStringList readContextQueries(const QString &fileName, QTextStream &stream)
{
    QString line;
    QStringList contextQueries;
    QString contextQuery;
    bool inContextQuery = false;

    auto finishContextQuery = [&]() {
        if (!contextQuery.isEmpty()) {
            contextQueries.push_back(contextQuery.simplified());
            contextQuery.clear();
        } else {
            spdlog::warn("Encountered empty context query in {}", fileName.toStdString());
        }

        inContextQuery = false;
    };

    while ((line = stream.readLine()).startsWith("//")) {
        auto content = line.mid(2);
        if (content.simplified() == "CONTEXT QUERY") {
            if (inContextQuery) {
                // recover somewhat gracefully from a missing END CONTEXT QUERY
                spdlog::warn("Encountered another 'CONTEXT QUERY' without a preceding 'END CONTEXT QUERY' in {}",
                             fileName.toStdString());
                finishContextQuery();
            }
            inContextQuery = true;
        } else if (content.simplified() == "END CONTEXT QUERY") {
            finishContextQuery();
        } else if (inContextQuery) {
            contextQuery += content + '\n';
        }
    }
    if (inContextQuery) {
        spdlog::warn("Encountered 'CONTEXT QUERY' without a following 'END CONTEXT QUERY' in {}",
                     fileName.toStdString());
        finishContextQuery();
    }

    return contextQueries;
}

void ScriptManager::addScript(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QTextStream stream(&file);
    auto line = stream.readLine();
    const QString description = line.startsWith("//") ? line.mid(2).simplified() : "";

    const auto contextQueries = readContextQueries(fileName, stream);

    QFileInfo fi(fileName);

    Script script {fi.fileName(), fileName, description, contextQueries};
    emit aboutToAddScript(script, static_cast<int>(m_scriptList.size()));
    m_scriptList.push_back(std::move(script));
    emit scriptAdded(m_scriptList.back());
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
    QStringList filesInDir = scriptListFromDir(path);

    // Remove deleted scripts
    auto it = m_scriptList.begin();
    while (it != m_scriptList.end()) {
        const auto &script = *it;

        // Only remove the script if it was actually in the directory that had changes
        if (script.fileName.startsWith(path) && !filesInDir.contains(script.fileName)) {
            it = removeScript(it);
        } else {
            ++it;
        }
    }

    auto scriptToFileName = [](const auto &script) {
        return script.fileName;
    };
    auto currentFileNames = kdalgorithms::transformed(m_scriptList, scriptToFileName);
    // Add new scripts
    for (const auto &fileName : filesInDir) {
        if (!kdalgorithms::contains(currentFileNames, fileName)) {
            addScript(fileName);
        }
    }
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
        if (files.contains(it->fileName)) {
            it = removeScript(it);
        } else {
            ++it;
        }
    }
}

ScriptManager::ScriptList::iterator ScriptManager::removeScript(const ScriptList::iterator &iterator)
{
    auto script = *iterator;
    emit aboutToRemoveScript(script, static_cast<int>(iterator - m_scriptList.begin()));
    auto it = m_scriptList.erase(iterator);
    emit scriptRemoved(script);
    return it;
}

void ScriptManager::doRunScript(const QString &fileName, const std::function<void()> &endFunc,
                                const std::optional<QueryMatch> &context)
{
    auto result = m_runner->runScript(fileName, endFunc, context);
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
    QStringList directories;
    {
        LoggerDisabler ld(true);
        directories = Settings::instance()->value<QStringList>(Settings::ScriptPaths);
    }
    for (const auto &path : directories)
        addScriptsFromPath(path);
}

} // namespace Core
