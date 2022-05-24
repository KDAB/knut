#pragma once

#include <QAction>
#include <QObject>

#include <vector>

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

    struct Command
    {
        QAction *action;
        QKeySequence defaultShortcut;
    };
    std::vector<Command> m_commands;

    QString id(const Command &) const;
    QString description(const Command &) const;
    QKeySequence shortcut(const Command &) const;
    void setShortcut(const Command &, const QKeySequence &);
    void resetShortcut(const Command &);
};

} // namespace Gui
