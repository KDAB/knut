/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/qtuidocument.h"
#include "core/textdocument.h"

#include <QModelIndex>
#include <QSplitter>

class QTableView;
class QMdiArea;
class QMdiSubWindow;

namespace Gui {

class FindWidget;

class QtUiView : public QSplitter
{
    Q_OBJECT

public:
    explicit QtUiView(QWidget *parent = nullptr);

    void setFindWidget(FindWidget *findWidget);
    void setUiDocument(Core::QtUiDocument *document);

public slots:
    // Accessible from QML
    void search(const QString &searchText, int option = Core::TextDocument::NoFindFlags);

private:
    QModelIndexList searchModel(const QString &searchText, const QAbstractItemModel *model) const;
    void updateView();

    QTableView *const m_tableView;
    QMdiArea *const m_previewArea;
    Core::QtUiDocument *m_document = nullptr;
    QMdiSubWindow *m_previewWindow = nullptr;
    // Search feature
    QModelIndexList m_currentSearchResult;
    QString m_currentSearchText;
    int m_currentSearchIndex = 0;
    bool m_initialSearchProcessed = false;
};

} // namespace Gui
