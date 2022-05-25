#pragma once

#include "core/scriptmanager.h"

#include <QAction>
#include <QObject>

#include <variant>
#include <vector>

class QShortcut;

namespace Gui {

class MainWindow;

class ShortcutManager : public QObject
{
    Q_OBJECT
public:
    explicit ShortcutManager(MainWindow *parent);

    void resetAll();
    QKeySequence resetShortcut(const QString &id);
    void setShortcut(const QString &id, const QKeySequence &shortcut);

    struct Shortcut
    {
        QString id;
        QString description;
        QString shortcut;
        bool isAction;
    };
    std::vector<Shortcut> shortcuts() const;

private:
    void initialize();

    struct Action
    {
        QAction *action;
        QKeySequence defaultShortcut;
    };
    using Script = Core::ScriptManager::Script;
    using Command = std::variant<Action, Script>;
    std::vector<Command> m_commands;
    QHash<QString, QShortcut *> m_scriptShortcuts;

    QString id(const Command &command) const;
    QString description(const Command &command) const;
    QKeySequence shortcut(const Command &command) const;
    QKeySequence defaultShortcut(const Command &command) const;
    void setShortcut(const Command &command, const QKeySequence &keySequence);
    void resetShortcut(const Command &command, bool updateSettings = true);
};

} // namespace Gui
