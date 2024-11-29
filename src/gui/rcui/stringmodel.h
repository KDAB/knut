/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "rccore/data.h"

#include <QAbstractTableModel>
#include <QList>

namespace RcUi {

class StringModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        ID,
        Text,
    };

public:
    explicit StringModel(const RcCore::Data &data, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QList<RcCore::String> m_strings;
};

} // namespace RcUi
