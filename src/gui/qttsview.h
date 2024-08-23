/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/qttsdocument.h"
#include "findinterface.h"

#include <QSortFilterProxyModel>
#include <QWidget>

class QTableView;
class QLineEdit;

namespace Gui {

class QtTsProxy;
class SearchableTableView;

class QtTsView : public QWidget, public FindInterface
{
    Q_OBJECT

public:
    explicit QtTsView(QWidget *parent = nullptr);

    void setTsDocument(Core::QtTsDocument *document);

    void find(const QString &text, int options) override;
    void cancelFind() override;

private:
    void updateView();

    SearchableTableView *const m_tableView;
    QLineEdit *const m_searchLineEdit;
    Core::QtTsDocument *m_document = nullptr;
    QtTsProxy *const m_contentProxyModel;
    QAbstractItemModel *m_contentModel = nullptr;
};

} // namespace Gui
