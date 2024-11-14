/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "stringmodel.h"
#include "rcviewer_global.h"

#include <algorithm>

using namespace RcCore;

namespace RcUi {

StringModel::StringModel(const Data &data, QObject *parent)
    : QAbstractTableModel(parent)
    , m_strings(data.strings.values())
{
    std::ranges::sort(m_strings, [](const auto &left, const auto &right) {
        return left.id < right.id;
    });
}

int StringModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_strings.size();
}

int StringModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

QVariant StringModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        const auto &asset = m_strings.at(index.row());
        switch (index.column()) {
        case ID:
            return asset.id;
        case Text:
            return asset.text;
        }
    }

    if (role == LineRole) {
        const auto &asset = m_strings.at(index.row());
        return asset.line;
    }

    return {};
}

QVariant StringModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList headers = {tr("Id"), tr("Text")};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers.value(section);
    }
    return {};
}

} // namespace RcUi
