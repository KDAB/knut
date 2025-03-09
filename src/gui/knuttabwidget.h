/*
  This file is part of Knut.

SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

SPDX-License-Identifier: GPL-3.0-only

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QObject>
#include <QTabBar>
#include <QTabWidget>
#include <QWidget>

namespace Gui {

class KnutTabBar : public QTabBar
{
    Q_OBJECT
public:
    KnutTabBar(QWidget *parent);

protected:
    bool event(QEvent *event);
};

class KnutTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    KnutTabWidget(QWidget *parent);
};

} // namespace Gui
