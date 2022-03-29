#pragma once

#include <QWidget>

class QRubberBand;

namespace Core {
class TextDocument;
class Mark;
}
namespace Gui {

class TextView : public QWidget
{
    Q_OBJECT
public:
    explicit TextView(QWidget *parent = nullptr);

    void setTextDocument(Core::TextDocument *document);

    void toggleMark();
    void gotoMark();
    void selectToMark();
    bool hasMark() const;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void updateMarkRect();

    Core::TextDocument *m_document;
    Core::Mark *m_mark = nullptr;
    QRubberBand *m_markRect = nullptr;
};

} // namespace Gui
