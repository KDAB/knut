#include "textview.h"

#include "guisettings.h"

#include "core/logger.h"
#include "core/lspdocument.h"
#include "core/mark.h"
#include "core/textdocument.h"

#include <QEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QRubberBand>
#include <QTextDocument>
#include <QToolTip>
#include <QVBoxLayout>

namespace Gui {

constexpr int MarkWidth = 6;
constexpr int MarkWidth2 = MarkWidth / 2;

class MarkRect : public QRubberBand
{
public:
    explicit MarkRect(QWidget *parent)
        : QRubberBand(QRubberBand::Line, parent)
    {
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.translate(0.5, 0);
        const QColor &color = palette().highlight().color();
        p.setPen(color);
        p.setBrush(color);

        const QRect r = rect();
        QPolygon p1({r.topLeft(), r.topRight(), {r.center().x(), r.top() + MarkWidth2 + 1}});
        p.drawPolygon(p1);

        QPolygon p2({r.bottomLeft(), r.bottomRight(), {r.center().x(), r.bottom() - MarkWidth2 - 1}});
        p.drawPolygon(p2);

        p.setOpacity(0.7);
        p.drawLine(p1.at(2), p2.at(2));
    }
};

TextView::TextView(QWidget *parent)
    : QWidget {parent}
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void TextView::setTextDocument(Core::TextDocument *document)
{
    m_document = document;
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});

    auto *textEdit = m_document->textEdit();
    layout->addWidget(textEdit);
    textEdit->setVisible(true);
    textEdit->installEventFilter(this);
    setFocusProxy(textEdit);
    connect(textEdit->document(), &QTextDocument::contentsChanged, this, &TextView::updateMarkRect);
    GuiSettings::setupDocumentTextEdit(textEdit, document->fileName());
}

void TextView::toggleMark()
{
    Q_ASSERT(m_document);

    const bool newMark = !m_mark || m_mark->position() != m_document->position();

    m_mark.reset();
    delete m_markRect;
    m_markRect = nullptr;

    if (newMark) {
        m_mark = m_document->createMark();
        m_markRect = new MarkRect(m_document->textEdit());
        m_markRect->show();
        updateMarkRect();
    }
}

void TextView::gotoMark()
{
    Q_ASSERT(m_document);
    if (m_mark.has_value()) {
        m_document->gotoMark(m_mark.value());
    }
}

void TextView::selectToMark()
{
    Q_ASSERT(m_document);
    if (m_mark.has_value()) {
        m_document->selectToMark(m_mark.value());
    }
}

bool TextView::hasMark() const
{
    return m_mark.has_value();
}

bool TextView::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    if (event->type() == QEvent::Paint)
        updateMarkRect();
    if (event->type() == QEvent::ToolTip) {
        if (const auto *lspdocument = qobject_cast<Core::LspDocument *>(m_document)) {
            if (const auto *helpEvent = dynamic_cast<QHelpEvent *>(event)) {
                auto cursor = lspdocument->textEdit()->cursorForPosition(helpEvent->pos());

                // Make the textEdit a guarded pointer, as it might have been destroyed once the hover
                // callback returns.
                QPointer<QPlainTextEdit> textEdit(lspdocument->textEdit());
                QPoint position(helpEvent->globalPos());

                // Hover spams the log if it doesn't find anything.
                // In our case, that's not a problem, so just disable the log.
                Core::LoggerDisabler ld;

                lspdocument->hover(cursor.position(), [textEdit, position](const auto &hoverText) {
                    if (!textEdit.isNull() && textEdit->isVisible()) {
                        QToolTip::showText(position, hoverText, textEdit);
                    }
                });
                return true;
            }
        }
    }
    return false;
}

void TextView::updateMarkRect()
{
    if (m_markRect) {
        auto textCursor = m_document->textEdit()->textCursor();
        textCursor.setPosition(m_mark->position());
        auto rect = m_document->textEdit()->cursorRect(textCursor);
        rect.adjust(-MarkWidth2, -MarkWidth2, MarkWidth2, MarkWidth2);
        m_markRect->setGeometry(rect);
    }
}

} // namespace Gui
