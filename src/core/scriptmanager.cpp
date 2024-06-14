/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "scriptmanager.h"
#include "logger.h"
#include "scriptmodel.h"
#include "scriptrunner.h"
#include "settings.h"
#include "utils/log.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTextStream>
#include <QTimer>
#include <kdalgorithms.h>

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

QAbstractItemModel *ScriptManager::model()
{
    static auto *model = new ScriptModel(instance());
    return model;
}

void ScriptManager::runScript(const QString &fileName, bool async, bool log)
{
    if (log)
        spdlog::debug("==> Start script {}", fileName);
    auto endScriptCallback = [this, log, fileName]() {
        if (log)
            spdlog::debug("<== End script {}", fileName);
        emit scriptFinished(m_result);
    };

    if (async)
        QTimer::singleShot(0, this, [this, fileName, endScriptCallback]() {
            doRunScript(fileName, endScriptCallback);
        });
    else
        doRunScript(fileName, endScriptCallback);
}

void ScriptManager::addScript(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QTextStream stream(&file);
    const auto line = stream.readLine();
    const QString description = line.startsWith("//") ? line.mid(2).simplified() : "";

    const QFileInfo fi(fileName);

    Script script {fi.fileName(), fileName, description};
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
    const QStringList filesInDir = scriptListFromDir(path);

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

void ScriptManager::doRunScript(const QString &fileName, const std::function<void()> &endFunc)
{
    m_result = m_runner->runScript(fileName, endFunc);
    if (m_runner->hasError()) {
        const auto errors = m_runner->errors();
        for (const auto &error : errors)
            spdlog::error("{}({}): {}", error.url().toLocalFile(), error.line(), error.description());
    } else {
        if (m_result.isValid())
            spdlog::info("Script result is {}", m_result.toString());
    }
}

void ScriptManager::updateDirectories()
{
    const QStringList directories = []() {
        LoggerDisabler ld(true);
        return Settings::instance()->value<QStringList>(Settings::ScriptPaths);
    }();
    for (const auto &path : directories)
        addScriptsFromPath(path);
}

} // namespace Core
