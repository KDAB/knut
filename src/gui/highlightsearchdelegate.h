/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QItemDelegate>

namespace Gui {

class HighlightSearchDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit HighlightSearchDelegate(QObject *parent = nullptr);

    void setSearchText(const QString &searchText, int offset = 0);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QString m_searchText;
    int m_offset;
};

} // namespace Gui
