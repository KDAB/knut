#include "scriptsinpath.h"

#include <core/scriptmanager.h>

#include <spdlog/spdlog.h>

using ScriptManager = Core::ScriptManager;

namespace Gui {

ScriptsInPath::ScriptsInPath(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(ScriptManager::instance(), &ScriptManager::aboutToAddScript, this, &ScriptsInPath::aboutToAddScript);
    connect(ScriptManager::instance(), &ScriptManager::aboutToRemoveScript, this, &ScriptsInPath::aboutToRemoveScript);

    connect(ScriptManager::instance(), &ScriptManager::scriptAdded, this, &ScriptsInPath::scriptAdded);
    connect(ScriptManager::instance(), &ScriptManager::scriptRemoved, this, &ScriptsInPath::scriptRemoved);
}

const ScriptManager::ScriptList &scriptList()
{
    return ScriptManager::instance()->scriptList();
}

void ScriptsInPath::aboutToAddScript(const Core::ScriptManager::Script &, int index)
{
    beginInsertRows({}, index, index);
}

void ScriptsInPath::scriptAdded(const Core::ScriptManager::Script &)
{
    endInsertRows();
}

void ScriptsInPath::aboutToRemoveScript(const Core::ScriptManager::Script &, int index)
{
    beginRemoveRows({}, index, index);
}
void ScriptsInPath::scriptRemoved(const Core::ScriptManager::Script &)
{
    endRemoveRows();
}

int ScriptsInPath::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 4;
}

int ScriptsInPath::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(scriptList().size());
}

QVariant ScriptsInPath::columnHeaderDisplayData(int column) const
{
    switch (column) {
    case 0:
        return tr("Path");
    case 1:
        return tr("Name");
    case 2:
        return tr("Description");
    case 3:
        return tr("Context Queries");
    default:
        spdlog::error("SuggestedScripts::columnHeaderDisplayData: column out of range: {}", column);
        return {};
    }
}

QVariant ScriptsInPath::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (role) {
    case Qt::DisplayRole:
        return columnHeaderDisplayData(section);
    default:
        return QAbstractTableModel::headerData(section, orientation, role);
    }
}

QVariant ScriptsInPath::data(const QModelIndex &index, int role) const
{
    auto row = index.row();

    const auto &scripts = scriptList();
    if (row < 0 || static_cast<size_t>(row) >= scripts.size()) {
        spdlog::error("SuggestedScripts::data: row out of range: {}", row);
        return {};
    }

    const auto &script = scripts.at(row);
    switch (role) {
    case Qt::DisplayRole:
        return displayData(script, index.column());
    case Qt::ToolTipRole:
        return script.description;
    case ScriptsInPath::Role::ContextQueries:
        return script.contextQueries;
    default:
        return QVariant {};
    }
}

QVariant ScriptsInPath::displayData(const ScriptManager::Script &script, int column) const
{
    switch (column) {
    case ScriptsInPath::Column::Path:
        return script.fileName;
    case ScriptsInPath::Column::Name:
        return script.name;
    case ScriptsInPath::Column::Description:
        return script.description;
    default:
        spdlog::error("SuggestedScripts::displayData: column out of range: {}", column);
        return {};
    }
}

} // namespace Gui
