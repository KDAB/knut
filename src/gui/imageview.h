/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QGraphicsView>

namespace Core {
class ImageDocument;
}
namespace Gui {

class ImageView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageView(QWidget *parent = nullptr);

    void setImageDocument(Core::ImageDocument *document);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
};

} // namespace Gui
