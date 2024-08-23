/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/qtuidocument.h"
#include "findinterface.h"

#include <QModelIndex>
#include <QSplitter>

class QTableView;
class QMdiArea;
class QMdiSubWindow;

namespace Gui {

class SearchableTableView;

class QtUiView : public QSplitter, public FindInterface
{
    Q_OBJECT

public:
    explicit QtUiView(QWidget *parent = nullptr);

    void setUiDocument(Core::QtUiDocument *document);

    void find(const QString &text, int options) override;
    void cancelFind() override;

private:
    void updateView();

    SearchableTableView *const m_tableView;
    QMdiArea *const m_previewArea;
    Core::QtUiDocument *m_document = nullptr;
    QMdiSubWindow *m_previewWindow = nullptr;
};

} // namespace Gui
