/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QProxyStyle>

namespace Gui {

class KnutStyle : public QProxyStyle
{
    Q_OBJECT

public:
    using QProxyStyle::QProxyStyle;

    void polish(QWidget *widget) override;

    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override;

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                       const QWidget *widget) const override;

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                     const QWidget *widget = nullptr) const override;

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
                            const QWidget *widget = nullptr) const override;

    bool eventFilter(QObject *watched, QEvent *event) override;
};

} // namespace Gui
