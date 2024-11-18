/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "../findinterface.h"

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

namespace Gui {
class FindAdapter;
}

namespace RcUi {

namespace Ui {
    class RcFileView;
}

class RcFileView : public QWidget, public Gui::FindInterface
{
    Q_OBJECT

public:
    explicit RcFileView(QWidget *parent = nullptr);
    ~RcFileView() override;

    void setRcFile(const RcCore::RcFile &rcFile);

    QPlainTextEdit *textEdit() const;

    void find(const QString &text, int options) override;

signals:
    void languageChanged(const QString &language);

private:
    void changeDataItem(const QModelIndex &current);
    void changeContentItem(const QModelIndex &current);
    void setData(int type, int index);
    void updateDialogProperty(int index);
    void previewData(const QModelIndex &index);
    void slotContextMenu(QTreeView *treeView, const QPoint &pos);

    void highlightLine(int line);
    void slotSearchText(const QString &text);
    void slotSearchNext();
    void slotSearchPrevious();

    const RcCore::Data &data() const;

private:
    std::unique_ptr<Ui::RcFileView> ui;
    const RcCore::RcFile *m_rcFile = nullptr;
    QSortFilterProxyModel *const m_dataProxyModel;
    QSortFilterProxyModel *const m_contentProxyModel;
    QAbstractItemModel *m_contentModel = nullptr;
    Gui::FindAdapter *m_findAdapter = nullptr;
};

} // namespace RcUi
