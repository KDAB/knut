/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/qtuidocument.h"

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
    enum FindFlag {
        NoFindFlags = 0x0,
        FindBackward = 0X2,
        FindForward = 0x4,
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)
    Q_ENUM(FindFlag)

    explicit QtUiView(QWidget *parent = nullptr);

    void setFindWidget(FindWidget *findWidget);
    void setUiDocument(Core::QtUiDocument *document);

public slots:
    // Accessible from QML
    void search(const QString &searchText, int option = QtUiView::NoFindFlags);

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

Q_DECLARE_OPERATORS_FOR_FLAGS(Gui::QtUiView::FindFlags)
