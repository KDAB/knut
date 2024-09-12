/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/qttsdocument.h"

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
    enum FindFlag {
        NoFindFlags = 0x0,
        FindBackward = 0X2,
        FindForward = 0x4,
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)
    Q_ENUM(FindFlag)

    explicit QtTsView(QWidget *parent = nullptr);

    void setTsDocument(Core::QtTsDocument *document);
    void setFindWidget(FindWidget *findWidget);

public slots:
    // Accessible from QML
    void search(const QString &searchText, int option = Gui::QtTsView::NoFindFlags);

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

Q_DECLARE_OPERATORS_FOR_FLAGS(Gui::QtTsView::FindFlags)
