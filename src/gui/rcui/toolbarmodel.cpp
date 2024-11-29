/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "toolbarmodel.h"
#include "rcviewer_global.h"

#include <QColor>

using namespace RcCore;

namespace RcUi {

ToolBarModel::ToolBarModel(const Data &data, int index, QObject *parent)
    : QAbstractListModel(parent)
    , m_items(data.toolBars.value(index).children)
{
}

int ToolBarModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant ToolBarModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        const auto &item = m_items.at(index.row());
        if (item.id.isEmpty())
            return tr("--------");
        return item.id;
    }

    if (role == LineRole) {
        const auto &item = m_items.at(index.row());
        return item.line;
    }

    return {};
}

QVariant ToolBarModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {tr("Id")};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}

} // namespace RcUi
