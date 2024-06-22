/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QToolBar>

namespace Gui {

class Toolbar : public QToolBar
{
    Q_OBJECT

public:
    explicit Toolbar(QWidget *parent = nullptr);

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void relayout();
};

}
