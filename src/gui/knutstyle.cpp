#include "knutstyle.h"

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
        if (widget->property("panel").toBool())
            return true;
        widget = widget->parentWidget();
    }
    return false;
}

void KnutStyle::polish(QWidget *widget)
{
    if (isPanel(widget)) {
        if (qobject_cast<QToolButton *>(widget))
            widget->setAttribute(Qt::WA_Hover);
    }
    if (auto tabBar = qobject_cast<QTabBar *>(widget))
        tabBar->setDrawBase(false);
}

QSize KnutStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &csz,
                                  const QWidget *widget) const
{
    return QProxyStyle::sizeFromContents(ct, opt, csz, widget);
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

void KnutStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                            const QWidget *widget) const
{
    switch (element) {
    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QStyleOptionTab newOption = *tab;
            newOption.rect.adjust(0, 0, 0, 2);
            QProxyStyle::drawControl(element, &newOption, painter, widget);
        }
        break;
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

} // namespace Gui
