/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QObject>
#include <QStringList>
#include <QVariant>
#include <functional>
#include <nlohmann/json.hpp>
#include <vector>

class QFileSystemWatcher;
class QAbstractItemModel;

namespace Core {

class ScriptRunner;

/**
 * \brief Manager for scripts
 *
 * Manage all scripts available to the user, and run them.
 *
 * Scripts directory are watched using a QFileSystemWatcher, to update
 * the list of script in case one is added or deleted.
 */
class ScriptManager : public QObject
{
    Q_OBJECT

public:
    struct Script
    {
        QString name;
        QString fileName;
        QString description;
    };
    using ScriptList = std::vector<Script>;

    ~ScriptManager() override;

    static ScriptManager *instance();

    const ScriptList &scriptList() const;

    void addDirectory(const QString &path);
    void removeDirectory(const QString &path);

    QStringList directories() const;

    static QAbstractItemModel *model();

    void runScript(const QString &fileName, nlohmann::json &&data = nlohmann::json::object(), bool async = true,
                   bool log = true);

signals:
    void scriptFinished(const QVariant &result);

    // Added to allow models to call beginInsertRows, etc. correctly
    void aboutToAddScript(const Core::ScriptManager::Script &script, int index);
    void scriptAdded(const Core::ScriptManager::Script &script);

    // Added to allow models to emit abouToAddRow, etc. correctly
    void aboutToRemoveScript(const Core::ScriptManager::Script &script, int index);
    void scriptRemoved(const Core::ScriptManager::Script &script);

private:
    friend class KnutCore;
    explicit ScriptManager(QObject *parent = nullptr);

    void addScript(const QString &fileName);
    void addScriptsFromPath(const QString &path);
    void removeScriptsFromPath(const QString &path);

    void doRunScript(const QString &fileName, nlohmann::json &&data, const std::function<void()> &endFunc);

    void updateDirectories();
    void updateScriptDirectory(const QString &path);

    ScriptList::iterator removeScript(const ScriptList::iterator &iterator);

private:
    inline static ScriptManager *m_instance = nullptr;

    QFileSystemWatcher *const m_watcher;
    ScriptRunner *const m_runner;

    ScriptList m_scriptList;
    QStringList m_directories;
    QVariant m_result;
};

} // namespace Core
