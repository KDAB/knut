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

class UiView : public QSplitter
{
    Q_OBJECT
public:
    explicit UiView(QWidget *parent = nullptr);

    void setUiDocument(Core::QtUiDocument *document);

private:
    void updateView();

    QTableView *m_tableView = nullptr;
    QMdiArea *m_previewArea = nullptr;
    Core::QtUiDocument *m_document = nullptr;
    QMdiSubWindow *m_previewWindow = nullptr;
};

} // namespace Gui
