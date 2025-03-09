/*
  This file is part of Knut.

SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

SPDX-License-Identifier: GPL-3.0-only

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "knuttabwidget.h"

#include <QEvent>
#include <QTabBar>
#include <QTimer>

namespace Gui {

KnutTabBar::KnutTabBar(QWidget *parent)
    : QTabBar(parent)
{
}

bool KnutTabBar::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Show:
    case QEvent::HoverLeave:
        setTabsClosable(false);
        break;
    case QEvent::HoverEnter:
        setTabsClosable(true);
        break;
    default:
        break;
    }
    return QTabBar::event(event);
}

KnutTabWidget::KnutTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    auto tabBar = new KnutTabBar(this);
    setTabBar(tabBar);
}

} // namespace Gui
