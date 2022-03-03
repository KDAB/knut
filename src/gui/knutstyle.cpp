#include "knutstyle.h"

#include <QDockWidget>
#include <QPainter>
#include <QStyleOptionComplex>
#include <QTabBar>
#include <QToolButton>
#include <QVariant>
#include <QWidget>

namespace Gui {

static bool isPanel(const QWidget *widget)
{
    while (widget) {
        if (auto dock = qobject_cast<QDockWidget *>(widget->parentWidget()))
            return dock->titleBarWidget() == widget;
        widget = widget->parentWidget();
    }
    return false;
}

void KnutStyle::polish(QWidget *widget)
{
    if (isPanel(widget)) {
        if (auto toolButton = qobject_cast<QToolButton *>(widget)) {
            widget->setAttribute(Qt::WA_Hover);
            widget->setSizePolicy(widget->sizePolicy().horizontalPolicy(), QSizePolicy::Preferred);
        }
    }
    if (auto frame = qobject_cast<QFrame *>(widget)) {
        if (frame->frameShape() == QFrame::Box)
            frame->setFrameShape(QFrame::NoFrame);
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
            const QColor shade = option->palette.base().color();
            painter->fillRect(rect, shade);
        } else if (option->state & State_Enabled && option->state & State_MouseOver) {
            painter->fillRect(rect, option->palette.highlight().color());
        } else {
            painter->fillRect(rect, option->palette.window().color());
        }
    } break;
    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

} // namespace Gui
