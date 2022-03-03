#pragma once

#include <QProxyStyle>

namespace Gui {

class KnutStyle : public QProxyStyle
{
    Q_OBJECT
public:
    using QProxyStyle::QProxyStyle;

    void polish(QWidget *widget) override;

    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &csz,
                           const QWidget *widget) const override;

    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override;

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                       const QWidget *widget) const override;

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                     const QWidget *widget = nullptr) const override;
};

} // namespace Gui
