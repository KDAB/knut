#include "texteditor.h"

#include <QPainter>
#include <QTextBlock>

namespace Core {

// =====================================================================================================================

class Gutter : public QWidget
{
public:
    static constexpr int Padding = 6;
    static constexpr int Margin = 12;

    explicit Gutter(TextEditor *editor);

    QSize sizeHint() const override;
    int preferedWidth() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    TextEditor *textEditor() const;
};

// =====================================================================================================================

TextEditor::TextEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_gutter(new Gutter(this))
{
    connect(this, &TextEditor::blockCountChanged, this, &TextEditor::updateGutterWidth);
    connect(this, &TextEditor::updateRequest, this, &TextEditor::updateGutter);
    connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::updateCurrentLine);

    updateGutterWidth(0);
}

void TextEditor::updateGutterWidth(int /*newBlockCount*/)
{
    setViewportMargins(m_gutter->preferedWidth(), 0, 0, 0);
}

void TextEditor::updateGutter(const QRect &rect, int dy)
{
    if (dy)
        m_gutter->scroll(0, dy);
    else
        m_gutter->update(0, rect.y(), m_gutter->preferedWidth(), rect.height());

    if (rect.contains(viewport()->rect())) {
        updateGutterWidth(0);
    }
}

void TextEditor::updateCurrentLine()
{
    if (m_currentLine == textCursor().blockNumber())
        return;
    m_currentLine = textCursor().blockNumber();

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(palette().alternateBase());
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}

void TextEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_gutter->setGeometry(QRect(cr.left(), cr.top(), m_gutter->preferedWidth(), cr.height()));
}

// =====================================================================================================================

Gutter::Gutter(TextEditor *editor)
    : QWidget(editor)
{
}

QSize Gutter::sizeHint() const
{
    return {preferedWidth(), 0};
}

int Gutter::preferedWidth() const
{
    int count = textEditor()->blockCount();
    int digits = static_cast<int>(floor(log10(count) + 1));

    return Padding * 2 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits + Margin;
}

void Gutter::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    auto pal = textEditor()->palette();

    painter.fillRect(event->rect(), pal.alternateBase());

    QTextBlock block = textEditor()->firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(textEditor()->blockBoundingGeometry(block).translated(textEditor()->contentOffset()).top());
    int bottom = top + qRound(textEditor()->blockBoundingRect(block).height());

    painter.setPen(pal.color(QPalette::Disabled, QPalette::ButtonText));

    for (; block.isValid() && top <= event->rect().bottom(); ++blockNumber) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.drawText(0, top, width() - Padding - Margin, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(textEditor()->blockBoundingRect(block).height());
    }
}

TextEditor *Gutter::textEditor() const
{
    return qobject_cast<TextEditor *>(parentWidget());
}

}
