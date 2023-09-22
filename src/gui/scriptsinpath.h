#pragma once

#include <core/scriptmanager.h>

#include <QAbstractTableModel>
#include <QString>

namespace Core {

class LspDocument;

} // namespace Core

namespace Gui {

class ScriptsInPath : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ScriptsInPath(QObject *parent = nullptr);

    enum Column : int {
        Path = 0,
        Name = 1,
        Description = 2,
    };
    Q_ENUM(Column)

    enum Role : int {
        ContextQueries = Qt::UserRole,
    };
    Q_ENUM(Role)

public slots:
    void aboutToAddScript(const Core::ScriptManager::Script &, int);
    void aboutToRemoveScript(const Core::ScriptManager::Script &, int);

    void scriptAdded(const Core::ScriptManager::Script &);
    void scriptRemoved(const Core::ScriptManager::Script &);

public:
    int columnCount(const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
    QVariant displayData(const Core::ScriptManager::Script &script, int column) const;

    QVariant columnHeaderDisplayData(int column) const;
};

} // namespace Gui
