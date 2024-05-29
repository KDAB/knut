/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "shortcutmanager.h"
#include "guisettings.h"
#include "mainwindow.h"

#include <QShortcut>
#include <QTimer>

namespace Gui {

//=============================================================================
// Utility method
//=============================================================================
QString ShortcutManager::id(const Command &command) const
{
    if (std::holds_alternative<Action>(command))
        return std::get<Action>(command).action->objectName().mid(6);
    else
        return std::get<Script>(command).name;
}

QString ShortcutManager::description(const Command &command) const
{
    if (std::holds_alternative<Action>(command))
        return std::get<Action>(command).action->text().remove('&');
    else
        return std::get<Script>(command).description;
}

QKeySequence ShortcutManager::shortcut(const Command &command) const
{
    if (std::holds_alternative<Action>(command)) {
        return std::get<Action>(command).action->shortcut();
    } else {
        const QString &key = std::get<Script>(command).fileName;
        if (auto shortcut = m_scriptShortcuts.value(key); shortcut)
            return shortcut->key();
        return {};
    }
}

QKeySequence ShortcutManager::defaultShortcut(const Command &command) const
{
    if (std::holds_alternative<Action>(command))
        return std::get<Action>(command).defaultShortcut;
    else
        return {};
}

void ShortcutManager::setShortcut(const Command &command, const QKeySequence &keySequence)
{
    if (std::holds_alternative<Action>(command)) {
        std::get<Action>(command).action->setShortcut(keySequence);
    } else {
        const QString &key = std::get<Script>(command).fileName;
        auto shortcut = m_scriptShortcuts.value(key);
        if (!shortcut) {
            shortcut = new QShortcut(parent());
            auto runScript = [key]() {
                Core::ScriptManager::instance()->runScript(key, true);
            };
            connect(shortcut, &QShortcut::activated, Core::ScriptManager::instance(), runScript);
            m_scriptShortcuts[key] = shortcut;
        }
        shortcut->setKey(keySequence);
    }

    if (keySequence == defaultShortcut(command))
        GuiSettings::instance()->removeShortcut(id(command));
    else
        GuiSettings::instance()->setShortcut(id(command), keySequence.toString(QKeySequence::NativeText));
}

void ShortcutManager::resetShortcut(const Command &command, bool updateSettings)
{
    if (std::holds_alternative<Action>(command)) {
        auto action = std::get<Action>(command);
        action.action->setShortcut(action.defaultShortcut);
    } else {
        delete m_scriptShortcuts.take(std::get<Script>(command).fileName);
    }

    if (updateSettings)
        GuiSettings::instance()->removeShortcut(id(command));
}

//=============================================================================
// ShortcutManager
//=============================================================================
ShortcutManager::ShortcutManager(MainWindow *parent)
    : QObject {parent}
{
    // Defer initialization, so actions are created
    QTimer::singleShot(0, this, &ShortcutManager::initialize);

    auto addScript = [this](const Script &script) {
        m_commands.emplace_back(script);
    };
    connect(Core::ScriptManager::instance(), &Core::ScriptManager::scriptAdded, this, addScript);
    auto removeScript = [this](const Script &script) {
        std::erase_if(m_commands, [this, &script](const auto &command) {
            return ShortcutManager::id(command) == script.name;
        });
        delete m_scriptShortcuts.take(script.fileName);
    };
    connect(Core::ScriptManager::instance(), &Core::ScriptManager::scriptRemoved, this, removeScript);
}

void ShortcutManager::resetAll()
{
    for (const auto &command : std::as_const(m_commands))
        resetShortcut(command, false);
    GuiSettings::instance()->removeAllShortcuts();
}

QKeySequence ShortcutManager::resetShortcut(const QString &id)
{
    auto it = std::ranges::find_if(m_commands, [this, &id](const auto &command) {
        return ShortcutManager::id(command) == id;
    });
    if (it != m_commands.end()) {
        resetShortcut(*it);
        return shortcut(*it);
    }
    return {};
}

void ShortcutManager::setShortcut(const QString &id, const QKeySequence &shortcut)
{
    auto it = std::ranges::find_if(m_commands, [this, &id](const auto &command) {
        return ShortcutManager::id(command) == id;
    });
    if (it != m_commands.end())
        setShortcut(*it, shortcut);
}

std::vector<ShortcutManager::Shortcut> ShortcutManager::shortcuts() const
{
    std::vector<ShortcutManager::Shortcut> results;
    results.reserve(m_commands.size());
    for (const auto &command : m_commands)
        results.push_back({id(command), description(command), shortcut(command).toString(QKeySequence::NativeText),
                           std::holds_alternative<Action>(command)});
    return results;
}

void ShortcutManager::initialize()
{
    m_commands.clear();
    // Actions from the mainwindow
    const auto &actions = qobject_cast<MainWindow *>(parent())->menuActions();
    m_commands.reserve(actions.size());
    for (auto action : actions)
        m_commands.emplace_back(Action {action, action->shortcut()});

    // Scripts
    const auto &scripts = Core::ScriptManager::instance()->scriptList();
    for (const auto &script : scripts)
        m_commands.emplace_back(script);

    // Restore shortcuts
    auto shortcuts = GuiSettings::instance()->shortcuts();
    if (shortcuts.isEmpty())
        return;
    for (auto &command : m_commands) {
        if (auto shortcut = shortcuts.value(id(command)); !shortcut.isEmpty())
            setShortcut(command, QKeySequence(shortcut));
    }
}

} // namespace Gui
