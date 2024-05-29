/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "rccore/rcfile.h"

#include <QAbstractListModel>

namespace RcUi {

class IncludeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Columns {
        FileName,
    };

public:
    explicit IncludeModel(const RcCore::RcFile &rcFile, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    const QVector<RcCore::Data::Include> m_includes;
};

} // namespace RcUi
