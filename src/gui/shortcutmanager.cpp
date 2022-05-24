#include "shortcutmanager.h"

#include "guisettings.h"
#include "mainwindow.h"

#include <QTimer>

namespace Gui {

//=============================================================================
// Utility method
//=============================================================================
QString ShortcutManager::id(const ShortcutManager::Command &command) const
{
    return command.action->objectName().mid(6);
}
QString ShortcutManager::description(const ShortcutManager::Command &command) const
{
    return command.action->text().remove('&');
}
QKeySequence ShortcutManager::shortcut(const ShortcutManager::Command &command) const
{
    return command.action->shortcut();
}
void ShortcutManager::setShortcut(const ShortcutManager::Command &command, const QKeySequence &shortcut)
{
    command.action->setShortcut(shortcut);
    if (shortcut == command.defaultShortcut)
        GuiSettings::instance()->removeShortcut(id(command));
    else
        GuiSettings::instance()->setShortcut(id(command), shortcut.toString(QKeySequence::NativeText));
}
void ShortcutManager::resetShortcut(const ShortcutManager::Command &command)
{
    command.action->setShortcut(command.defaultShortcut);
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
}

void ShortcutManager::resetAll()
{
    for (const auto &command : m_commands) {
        command.action->setShortcut(command.defaultShortcut);
    }
    GuiSettings::instance()->removeAllShortcuts();
}

QKeySequence ShortcutManager::resetShortcut(const QString &id)
{
    auto it = std::ranges::find_if(m_commands, [this, &id](const auto &command) {
        return ShortcutManager::id(command) == id;
    });
    if (it != m_commands.end()) {
        resetShortcut(*it);
        return it->defaultShortcut;
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
        results.push_back(
            {id(command), description(command), shortcut(command).toString(QKeySequence::NativeText), true});
    return results;
}

void ShortcutManager::initialize()
{
    const auto &actions = qobject_cast<MainWindow *>(parent())->menuActions();
    m_commands.reserve(actions.size());
    for (auto action : actions)
        m_commands.push_back({action, action->shortcut()});

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
