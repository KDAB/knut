/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "scriptmodel.h"
#include "scriptmanager.h"
#include "utils/log.h"

using ScriptManager = Core::ScriptManager;

namespace Core {

ScriptModel::ScriptModel(ScriptManager *parent)
    : QAbstractTableModel(parent)
{
    Q_ASSERT(parent);

    connect(parent, &ScriptManager::aboutToAddScript, this, &ScriptModel::onAboutToAddScript);
    connect(parent, &ScriptManager::aboutToRemoveScript, this, &ScriptModel::onAboutToRemoveScript);

    connect(parent, &ScriptManager::scriptAdded, this, &ScriptModel::onScriptAdded);
    connect(parent, &ScriptManager::scriptRemoved, this, &ScriptModel::onScriptRemoved);
}

const ScriptManager::ScriptList &scriptList()
{
    return ScriptManager::instance()->scriptList();
}

void ScriptModel::onAboutToAddScript(const Core::ScriptManager::Script &, int index)
{
    beginInsertRows({}, index, index);
}

void ScriptModel::onScriptAdded(const Core::ScriptManager::Script &)
{
    endInsertRows();
}

void ScriptModel::onAboutToRemoveScript(const Core::ScriptManager::Script &, int index)
{
    beginRemoveRows({}, index, index);
}

void ScriptModel::onScriptRemoved(const Core::ScriptManager::Script &)
{
    endRemoveRows();
}

int ScriptModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

int ScriptModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(scriptList().size());
}

QVariant ScriptModel::columnHeaderDisplayData(int column) const
{
    switch (column) {
    case NameColumn:
        return tr("Name");
    case DescriptionColumn:
        return tr("Description");
    default:
        spdlog::error("SuggestedScripts::columnHeaderDisplayData: column out of range: {}", column);
        return {};
    }
}

QVariant ScriptModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QVariant ScriptModel::data(const QModelIndex &index, int role) const
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
    case PathRole:
        return script.fileName;
    default:
        return QVariant {};
    }
}

QVariant ScriptModel::displayData(const ScriptManager::Script &script, int column) const
{
    switch (column) {
    case NameColumn:
        return script.name;
    case DescriptionColumn:
        return script.description;
    default:
        spdlog::error("SuggestedScripts::displayData: column out of range: {}", column);
        return {};
    }
}

} // namespace Core
