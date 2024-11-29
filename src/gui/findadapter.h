/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QAbstractItemView>

namespace Gui {

class FindAdapter
{
public:
    explicit FindAdapter(QAbstractItemView *view);

    void find(const QString &text, int options);
    void updateItemDelegate();

private:
    QModelIndexList searchModel(const QString &text, int options, const QModelIndex &parent = {}) const;

    QAbstractItemView *const m_view;
    QString m_highlightedText;
    int m_options = 0;
    QModelIndexList m_searchResults;
    int m_currentResultIndex = 0;
};

} // namespace Gui
