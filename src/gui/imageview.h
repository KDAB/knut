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
