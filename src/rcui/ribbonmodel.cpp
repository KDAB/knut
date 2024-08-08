/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ribbonmodel.h"
#include "rcviewer_global.h"

#include <QColor>
#include <algorithm>

using namespace RcCore;

namespace RcUi {

RibbonModel::RibbonModel(const QList<Ribbon> &ribbons, QObject *parent)
    : QAbstractTableModel(parent)
    , m_ribbons(ribbons)
{
    std::ranges::sort(m_ribbons, [](const auto &left, const auto &right) {
        return left.id < right.id;
    });
}

int RibbonModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_ribbons.size();
}

int RibbonModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

QVariant RibbonModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        const auto &ribbon = m_ribbons.at(index.row());
        switch (index.column()) {
        case ID:
            return ribbon.id;
        case FileName:
            return ribbon.fileName;
        }
    }

    if (role == LineRole) {
        const auto &ribbon = m_ribbons.at(index.row());
        return ribbon.line;
    }

    return {};
}

QVariant RibbonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {tr("Id"), tr("FileName")};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}

} // namespace RcUi
