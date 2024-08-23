/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/qttsdocument.h"
#include "core/textdocument.h"

#include <QSortFilterProxyModel>
#include <QWidget>

class QTableView;
class QLineEdit;

namespace Gui {
class FindWidget;
class QtTsProxy;

class QtTsView : public QWidget
{
    Q_OBJECT

public:
    explicit QtTsView(QWidget *parent = nullptr);

    void setTsDocument(Core::QtTsDocument *document);
    void setFindWidget(FindWidget *findWidget);

public slots:
    // Accessible from QML
    void search(const QString &searchText, int option = Core::TextDocument::NoFindFlags);

private:
    QModelIndexList searchModel(const QString &searchText, const QAbstractItemModel *model) const;
    void updateView();

    QTableView *const m_tableView;
    QLineEdit *const m_searchLineEdit;
    Core::QtTsDocument *m_document = nullptr;
    QtTsProxy *const m_contentProxyModel;
    QAbstractItemModel *m_contentModel = nullptr;
    // Search feature
    QModelIndexList m_currentSearchResult;
    QString m_currentSearchText;
    int m_currentSearchIndex = 0;
    bool m_initialSearchProcessed = false;
};

} // namespace Gui
