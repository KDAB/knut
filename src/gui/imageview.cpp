#include "imageview.h"

#include "core/imagedocument.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QShortcut>
#include <QWheelEvent>

namespace Gui {

ImageView::ImageView(QWidget *parent)
    : QGraphicsView(parent)
{
    setScene(new QGraphicsScene(this));

    auto zoomIn = new QShortcut(QKeySequence("Ctrl++"), this);
    zoomIn->setContext(Qt::WidgetShortcut);
    connect(zoomIn, &QShortcut::activated, this, [this]() {
        scale(2, 2);
    });

    auto zoomOut = new QShortcut(QKeySequence("Ctrl+-"), this);
    zoomOut->setContext(Qt::WidgetShortcut);
    connect(zoomOut, &QShortcut::activated, this, [this]() {
        scale(.5, .5);
    });

    auto zoomReset = new QShortcut(QKeySequence("Ctrl+0"), this);
    zoomReset->setContext(Qt::WidgetShortcut);
    connect(zoomReset, &QShortcut::activated, this, [this]() {
        setTransform({});
    });
}

void ImageView::setImageDocument(Core::ImageDocument *document)
{
    scene()->addPixmap(QPixmap::fromImage(document->image()));
}

void ImageView::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0)
        scale(2, 2);
    else
        scale(.5, .5);

    QGraphicsView::wheelEvent(event);
}

void ImageView::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect)
    painter->resetTransform();
    painter->fillRect(0, 0, width(), height(), QBrush(QImage(":/gui/tile.png")));
}

} // namespace Gui
