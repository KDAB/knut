/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QSplitter>

class QTableView;
class QMdiArea;
class QMdiSubWindow;

namespace Core {
class QtUiDocument;
}

namespace Gui {

class QtUiView : public QSplitter
{
    Q_OBJECT
public:
    explicit QtUiView(QWidget *parent = nullptr);

    void setUiDocument(Core::QtUiDocument *document);

private:
    void updateView();

    QTableView *const m_tableView;
    QMdiArea *const m_previewArea;
    Core::QtUiDocument *m_document = nullptr;
    QMdiSubWindow *m_previewWindow = nullptr;
};

} // namespace Gui
