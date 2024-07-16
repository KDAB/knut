/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QWidget>

class QScrollBar;

namespace Gui {

class ToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBar(QWidget *parent = nullptr);

    void setView(QWidget *view);

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void relayout();
    QScrollBar *getVerticallScrollBar();

    QWidget *m_view = nullptr;
};

}
