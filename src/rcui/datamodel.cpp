/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "datamodel.h"
#include "rccore/rcfile.h"
#include "rcviewer_global.h"

using namespace RcCore;

namespace {
const char *DataTypeStr[] = {
    "Dialogs", "Menus", "ToolBars", "Accelerators", "Assets", "Icons", "Strings", "Includes",
};
}

namespace RcUi {

DataModel::DataModel(const RcCore::RcFile &rcFile, QString language, QObject *parent)
    : QAbstractItemModel(parent)
    , m_rcFile(rcFile)
    , m_language(std::move(language))
{
    Q_ASSERT(m_rcFile.data.contains(m_language));
}

QModelIndex DataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column, static_cast<quintptr>(NoData));

    return createIndex(row, column, static_cast<quintptr>(parent.row()));
}

QModelIndex DataModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return {};

    int data = static_cast<int>(child.internalId());
    if (data == NoData)
        return {};

    return createIndex(data, 0, static_cast<quintptr>(NoData));
}

int DataModel::rowCount(const QModelIndex &parent) const
{
    if (!m_rcFile.isValid)
        return 0;
    if (!parent.isValid())
        return static_cast<int>(std::size(DataTypeStr));

    const int dataType = static_cast<int>(parent.internalId());
    if (dataType == NoData) {
        const auto &data = m_rcFile.data.value(m_language);
        switch (parent.row()) {
        case DialogData:
            return data.dialogs.size();
        case MenuData:
            return data.menus.size();
        case ToolBarData:
            return data.toolBars.size();
        case AcceleratorData:
            return data.acceleratorTables.size();
        case AssetData:
        case IconData:
        case StringData:
        case IncludeData:
            return 0;
        }
    }
    return 0;
}

int DataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (!m_rcFile.isValid)
        return 0;
    return 1;
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

    const int dataType = static_cast<int>(index.internalId());
    const auto &data = m_rcFile.data.value(m_language);

    if (role == Qt::DisplayRole) {
        if (dataType == NoData)
            return QLatin1String(DataTypeStr[index.row()]);

        switch (dataType) {
        case DialogData:
            return data.dialogs.value(index.row()).id;
        case MenuData:
            return data.menus.value(index.row()).id;
        case ToolBarData:
            return data.toolBars.value(index.row()).id;
        case AcceleratorData:
            return data.acceleratorTables.value(index.row()).id;
        }
        return {};
    }

    if (role == LineRole) {
        switch (dataType) {
        case DialogData:
            return data.dialogs.value(index.row()).line;
        case MenuData:
            return data.menus.value(index.row()).line;
        case ToolBarData:
            return data.toolBars.value(index.row()).line;
        case AcceleratorData:
            return data.acceleratorTables.value(index.row()).line;
        case AssetData:
        case IconData:
        case StringData:
        case IncludeData:
        case NoData:
            return -1;
        }
        return -1;
    }

    if (role == EmptyRole && dataType == NoData) {
        switch (index.row()) {
        case DialogData:
            return data.dialogs.isEmpty();
        case MenuData:
            return data.menus.isEmpty();
        case ToolBarData:
            return data.toolBars.isEmpty();
        case AcceleratorData:
            return data.acceleratorTables.isEmpty();
        case AssetData:
            return data.assets.isEmpty();
        case IconData:
            return data.icons.isEmpty();
        case StringData:
            return data.strings.isEmpty();
        case IncludeData:
            return m_rcFile.includes.isEmpty();
        case NoData:
            break;
        }
    }

    if (role == TypeRole) {
        if (dataType == NoData)
            return index.row();
        return index.parent().row();
    }

    if (role == IndexRole) {
        if (dataType == NoData)
            return -1;
        return index.row();
    }
    return {};
}

void DataModel::setLanguage(const QString &language)
{
    beginResetModel();
    m_language = language;
    endResetModel();
}

} // namespace RcUi
