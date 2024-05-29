/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "toolbar.h"

#include <QAbstractScrollArea>
#include <QEvent>
#include <QScrollBar>
#include <QToolButton>

namespace Gui {

Toolbar::Toolbar(QWidget *parent)
    : QToolBar(parent)
{
    parent->installEventFilter(this);
    setAutoFillBackground(true);
    if (auto scroll = qobject_cast<QAbstractScrollArea *>(parentWidget())) {
        scroll->verticalScrollBar()->installEventFilter(this);
    }
}

bool Toolbar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget() && (event->type() == QEvent::Move || event->type() == QEvent::Resize)) {
        relayout();
    } else if (auto scroll = qobject_cast<QAbstractScrollArea *>(parentWidget());
               scroll && watched == scroll->verticalScrollBar()) {
        if (event->type() == QEvent::Show || event->type() == QEvent::Hide) {
            relayout();
        }
    }

    return QObject::eventFilter(watched, event);
}

void Toolbar::relayout()
{
    QSize size = {int(actions().size()) * iconSize().width(), 30};
    resize(size);
    adjustSize();
    int shift = 0;
    if (auto scroll = qobject_cast<QAbstractScrollArea *>(parentWidget())) {
        if (scroll->verticalScrollBar()->isVisible()) {
            shift += scroll->verticalScrollBar()->width();
        }
    }
    auto pos = QPoint {parentWidget()->width() - width() - shift, 0};
    move(pos);
}

}
