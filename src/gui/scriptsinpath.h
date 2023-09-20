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

public slots:
    void aboutToAddScript(const Core::ScriptManager::Script &, int);
    void aboutToRemoveScript(const Core::ScriptManager::Script &, int);

    void scriptAdded(const Core::ScriptManager::Script &);
    void scriptRemoved(const Core::ScriptManager::Script &);

public:
    int columnCount(const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
    QVariant displayData(const Core::ScriptManager::Script &script, int column) const;
};

} // namespace Gui
