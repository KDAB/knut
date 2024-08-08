/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "toolbar.h"
#include "textview.h"

#include <QAbstractScrollArea>
#include <QAction>
#include <QEvent>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QStyle>
#include <QToolButton>

namespace Gui {

ToolBar::ToolBar(QWidget *parent)
    : QWidget(parent)
{
    setProperty("panelWidget", true);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    setAutoFillBackground(true);
}

void ToolBar::setView(QWidget *view)
{
    if (m_view == view)
        return;

    // Remove all existing buttons and event filters
    const auto toolButtons = findChildren<QToolButton *>();
    for (auto child : toolButtons) {
        delete child;
    }
    if (m_view) {
        m_view->removeEventFilter(this);
        if (auto scroll = getVerticallScrollBar()) {
            scroll->removeEventFilter(this);
        }
    }

    // Add view buttons and event filters
    m_view = view;
    if (view) {
        const auto actions = view->actions();
        for (auto action : actions) {
            auto button = new QToolButton(this);
            button->setDefaultAction(action);
            layout()->addWidget(button);
        }
        view->installEventFilter(this);
        if (auto scroll = getVerticallScrollBar()) {
            scroll->installEventFilter(this);
        }
        // We need this hack otherwise it will draw a white line in place of the toolbar
        // I couldn't find out where this line is coming from...
        if (actions.isEmpty())
            layout()->setContentsMargins({});
        else
            layout()->setContentsMargins({1, 0, style()->pixelMetric(QStyle::PM_ScrollBarExtent), 0});
    }
    relayout();
}

bool ToolBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_view && (event->type() == QEvent::Move || event->type() == QEvent::Resize)) {
        relayout();
    } else if (watched == getVerticallScrollBar() && (event->type() == QEvent::Show || event->type() == QEvent::Hide)) {
        relayout();
    }

    return QObject::eventFilter(watched, event);
}

void ToolBar::relayout()
{
    adjustSize();
    int shift = 0;
    if (auto scroll = getVerticallScrollBar()) {
        if (scroll->isVisible()) {
            shift += scroll->width();
        }
    }
    auto pos = QPoint {m_view->width() - width() - shift, 0};
    move(pos);
}

QScrollBar *ToolBar::getVerticallScrollBar()
{
    if (auto textView = qobject_cast<TextView *>(m_view)) {
        auto scrollView = textView->findChild<QPlainTextEdit *>({}, Qt::FindDirectChildrenOnly);
        return scrollView->verticalScrollBar();
    }
    return nullptr;
}

} // namespace Gui
