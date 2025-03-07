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
    , m_hideCloseTimer(new QTimer(this))
{
    // Human moment delay before hiding close buttons
    m_hideCloseTimer->setInterval(2000);
    connect(m_hideCloseTimer, &QTimer::timeout, this, [this]() {
        setTabsClosable(false);
    });
}

bool KnutTabBar::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::HoverEnter:
        m_hideCloseTimer->stop();
        setTabsClosable(true);
        break;
    case QEvent::HoverLeave:
        m_hideCloseTimer->start();
        break;
    case QEvent::Show:
        setTabsClosable(false);
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
