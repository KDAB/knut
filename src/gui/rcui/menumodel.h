/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "rccore/data.h"

#include <QAbstractItemModel>

namespace RcUi {

class MenuModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Columns {
        Title,
        Checked,
        ID,
        Shortcut,
        ToolTip,
        StatusTip,
        ColumnCount,
    };

public:
    explicit MenuModel(const RcCore::Data &data, int index, QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    struct Item
    {
        Item *parent = nullptr;
        int index = 0;
        QString title;
        QString id;
        QString shortcut;
        QString tooltip;
        QString statusTip;
        int line;
        bool isChecked;
        QList<Item> children;
    };

    void createTree(Item &item, const QList<RcCore::MenuItem> &children, const RcCore::Data &data);
    Item *indexToItem(const QModelIndex &index) const;

    Item m_root;
};

} // namespace RcUi
