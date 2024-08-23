/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QModelIndex>
#include <QWidget>
#include <memory>

class QSortFilterProxyModel;
class QAbstractItemModel;
class QTreeView;
class QPlainTextEdit;

namespace RcCore {
struct Data;
struct RcFile;
}

namespace RcUi {

namespace Ui {
    class RcFileView;
}

class RcFileView : public QWidget
{
    Q_OBJECT

public:
    explicit RcFileView(QWidget *parent = nullptr);
    ~RcFileView() override;

    void setRcFile(const RcCore::RcFile &rcFile);

    QPlainTextEdit *textEdit() const;

signals:
    void languageChanged(const QString &language);

private:
    enum class View { Content, Data };
    struct ViewSearchData
    {
        // Content View
        QModelIndexList contentCurrentSearchResult;
        QString contentCurrentSearchText;
        int contentCurrentSearchIndex = 0;
        // Data View
        QModelIndexList dataCurrentSearchResult;
        QString dataCurrentSearchText;
        int dataCurrentSearchIndex = 0;
    };

    struct ViewData
    {
        QTreeView *treeView;
        QString searchText;
        int offset = 0;
        QModelIndexList currentSearchResult;
        int currentSearchIndex = 0;
    };

    void changeDataItem(const QModelIndex &current);
    void changeContentItem(const QModelIndex &current);
    void setData(int type, int index);
    void updateDialogProperty(int index);
    void previewData(const QModelIndex &index);
    void slotContextMenu(QTreeView *treeView, const QPoint &pos);

    // Text view search.
    void highlightLine(int line);
    void slotSearchText(const QString &text);
    void slotSearchNext();
    void slotSearchPrevious();
    // Views search (Content or Data view).
    void searchView(View view);
    void viewFindPrevious(View view);
    void viewFindNext(View view);

    const RcCore::Data &data() const;
    // Dearch content or data views model.
    QModelIndexList searchModel(const View &view) const;
    // Convenience functions (avoid code repetition)
    bool hasMatch(const QModelIndex &index, const QString &searchText) const;
    ViewData viewData(View view);

private:
    std::unique_ptr<Ui::RcFileView> ui;
    const RcCore::RcFile *m_rcFile = nullptr;
    QSortFilterProxyModel *const m_dataProxyModel;
    QSortFilterProxyModel *const m_contentProxyModel;
    QAbstractItemModel *m_contentModel = nullptr;
    // Search data members for both content and data views
    ViewSearchData m_viewSearchData;
};

} // namespace RcUi
