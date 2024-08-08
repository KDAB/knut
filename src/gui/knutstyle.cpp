/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "knutstyle.h"

#include <QComboBox>
#include <QDockWidget>
#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QStyleOptionComplex>
#include <QTabBar>
#include <QToolButton>
#include <QVariant>
#include <QWidget>

namespace Gui {

constexpr int LineMargin = 3;

class PainterPen
{
public:
    explicit PainterPen(QPainter *painter)
        : m_painter(painter)
        , m_oldPen(painter->pen())
    {
    }
    ~PainterPen() { m_painter->setPen(m_oldPen); }

private:
    QPainter *const m_painter;
    const QPen m_oldPen;
};

static bool isPanel(const QWidget *widget)
{
    widget = widget->parentWidget();
    while (widget) {
        if (widget->property("panelWidget").toBool())
            return true;
        widget = widget->parentWidget();
    }
    return false;
}

void KnutStyle::polish(QWidget *widget)
{
    if (isPanel(widget)) {
        widget->setSizePolicy(widget->sizePolicy().horizontalPolicy(), QSizePolicy::Preferred);
        if (auto toolButton = qobject_cast<QToolButton *>(widget))
            toolButton->setAttribute(Qt::WA_Hover);
    } else if (auto frame = qobject_cast<QFrame *>(widget)) {
        if (frame->frameShape() & QFrame::Panel)
            frame->setFrameShape(QFrame::NoFrame);
    } else if (auto tabBar = qobject_cast<QTabBar *>(widget)) {
        tabBar->setDrawBase(false);
    }

    if (widget->inherits("QLineEditIconButton")) {
        widget->installEventFilter(this);
    }

    QProxyStyle::polish(widget);
}

int KnutStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_DefaultFrameWidth:
        if (isPanel(widget))
            return 0;
        break;
    case PM_DockWidgetSeparatorExtent:
        return 1;
    case PM_TabBarTabVSpace:
        return 10;
    case PM_SplitterWidth:
        return 1;
    default:
        break;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

void KnutStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                              const QWidget *widget) const
{
    if (!isPanel(widget)) {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        return;
    }

    QRect rect = option->rect;

    switch (element) {
    case PE_PanelButtonTool: {
        bool pressed = option->state & State_Sunken || option->state & State_On;
        if (pressed) {
            painter->fillRect(rect, option->palette.base().color());
        } else if (option->state & State_Enabled && option->state & State_MouseOver) {
            painter->fillRect(rect, option->palette.highlight().color());
        }
    } break;
    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void KnutStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                            const QWidget *widget) const
{
    QRect rect = option->rect;

    switch (element) {
    case CE_ShapedFrame:
        if (const auto *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            if (f->frameShape == QFrame::HLine || f->frameShape == QFrame::VLine) {
                QPoint p1, p2;
                if (f->frameShape == QFrame::HLine) {
                    p1 = QPoint(rect.x() + LineMargin, rect.y() + rect.height() / 2);
                    p2 = QPoint(rect.x() + rect.width() - LineMargin, p1.y());
                } else {
                    p1 = QPoint(rect.x() + rect.width() / 2, rect.y() + LineMargin);
                    p2 = QPoint(p1.x(), rect.y() + rect.height() - LineMargin);
                }
                PainterPen pp(painter);
                const QColor line = option->palette.mid().color();
                painter->setPen(line);
                painter->drawLine(p1, p2);
                return;
            } else if (f->frameShape == QFrame::Box) {
                const QColor line = option->palette.window().color().darker(110);
                PainterPen pp(painter);
                painter->setPen(line);
                painter->drawRect(rect.adjusted(0, 0, -1, -1));
                return;
            }
        }
        QProxyStyle::drawControl(element, option, painter, widget);
        break;

    case CE_TabBarTabShape:
        if (const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            const bool selected = tab->state & State_Selected;
            const int tabOverlap = pixelMetric(PM_TabBarTabOverlap, option, widget);
            const bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            rect = option->rect.adjusted(0, 0, onlyOne ? 0 : tabOverlap, 0);
            const QColor background = option->palette.base().color();
            if (selected) {
                painter->fillRect(rect, background);
                return;
            }

            PainterPen pp(painter);
            painter->setPen(background);
            if (tab->position != QStyleOptionTab::Beginning
                && tab->selectedPosition != QStyleOptionTab::PreviousIsSelected)
                painter->drawLine(rect.topLeft(), rect.bottomLeft());
            if (tab->selectedPosition != QStyleOptionTab::NextIsSelected)
                painter->drawLine(rect.topRight(), rect.bottomRight());
        }
        return;
    case CE_Splitter: {
        painter->fillRect(rect, Qt::black);
        return;
    }
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

void KnutStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
                                   const QWidget *widget) const
{
    if (!isPanel(widget)) {
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        return;
    }

    QRect rect = option->rect;

    switch (control) {
    case CC_ComboBox: {
        PainterPen pp(painter);
        // Left/right lines
        const QColor line = option->palette.window().color().darker(110);
        painter->setPen(line);
        painter->drawLine(rect.left(), rect.top() + LineMargin + 1, rect.left(), rect.bottom() - LineMargin - 1);
        painter->drawLine(rect.right(), rect.top() + LineMargin + 1, rect.right(), rect.bottom() - LineMargin - 1);

        bool pressed = option->state & State_Sunken || option->state & State_On;
        if (pressed) {
            painter->fillRect(rect, option->palette.base().color());
        } else if (option->state & State_Enabled && option->state & State_MouseOver) {
            painter->fillRect(rect, option->palette.highlight().color());
        }

        // Draw arrow
        int menuButtonWidth = 12;
        const bool reverse = option->direction == Qt::RightToLeft;
        int left = !reverse ? rect.right() - menuButtonWidth : rect.left();
        int right = !reverse ? rect.right() : rect.left() + menuButtonWidth;
        QRect arrowRect((left + right) / 2 + (reverse ? 6 : -6), rect.center().y() - 3, 9, 9);

        if (option->state & State_On)
            arrowRect.translate(QProxyStyle::pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                                QProxyStyle::pixelMetric(PM_ButtonShiftVertical, option, widget));

        auto arrowOpt = *option;
        arrowOpt.rect = arrowRect;
        if (styleHint(SH_ComboBox_Popup, option, widget)) {
            arrowOpt.rect.translate(0, -3);
            drawPrimitive(PE_IndicatorArrowUp, &arrowOpt, painter, widget);
            arrowOpt.rect.translate(0, 6);
            drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, painter, widget);
        } else {
            drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, painter, widget);
        }
    } break;
    default:
        QProxyStyle::drawComplexControl(control, option, painter, widget);
    }
}

bool KnutStyle::eventFilter(QObject *watched, QEvent *event)
{
    if (watched->inherits("QLineEditIconButton") && event->type() == QEvent::Paint) {
        auto button = qobject_cast<QToolButton *>(watched);
        if (button->isChecked()) {
            QPainter painter(button);
            const QRect rect(0, 0, button->width(), button->height());
            painter.fillRect(rect, button->palette().highlight().color());
        }
    }
    return QProxyStyle::eventFilter(watched, event);
}

} // namespace Gui
