/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QTableWidget>

namespace Gui {

class SearchableTableView : public QTableView
{
    Q_OBJECT

public:
    explicit SearchableTableView(QWidget *parent = nullptr);
    ~SearchableTableView() override;

    void find(const QString &text, int options);

private:
    QModelIndexList searchModel(const QString &text, int options) const;

    QString m_highlightedText;
    int m_options = 0;
    QModelIndexList m_searchResults;
    int m_currentResultIndex = 0;
};

} // namespace Gui
