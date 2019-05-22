#include "textedit.h"
#include <QLabel>
#include <QLineEdit>
#include <QShortcut>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QTextCursor>

TextEdit::TextEdit(QWidget *parent)
    : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);
    mainLayout->addWidget(m_textEdit);

    // Ensure the selection is always visible
    auto palette = m_textEdit->palette();
    palette.setColor(QPalette::Highlight, palette.color(QPalette::Highlight));
    palette.setColor(QPalette::HighlightedText, palette.color(QPalette::HighlightedText));
    m_textEdit->setPalette(palette);

    m_searchWidget = new QWidget(this);
    mainLayout->addWidget(m_searchWidget);

    auto searchLayout = new QHBoxLayout(m_searchWidget);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel(QStringLiteral("Search:"), this);
    searchLayout->addWidget(label);

    m_searchText = new QLineEdit(this);
    m_searchText->setClearButtonEnabled(true);
    searchLayout->addWidget(m_searchText);

    m_searchPreviewButton = new QToolButton(this);
    m_searchPreviewButton->setText(QStringLiteral("<<"));
    searchLayout->addWidget(m_searchPreviewButton);

    m_searchNextButton = new QToolButton(this);
    m_searchNextButton->setText(QStringLiteral(">>"));
    searchLayout->addWidget(m_searchNextButton);

    connect(m_searchText, &QLineEdit::textChanged, this, &TextEdit::slotSearchLineEditChanged);
    connect(m_searchText, &QLineEdit::returnPressed, this, &TextEdit::slotSearchNextText);
    connect(m_searchPreviewButton, &QToolButton::clicked, this, &TextEdit::slotSearchPreviewText);
    connect(m_searchNextButton, &QToolButton::clicked, this, &TextEdit::slotSearchNextText);
    slotSearchLineEditChanged(QString());
    m_searchWidget->setVisible(false);

    QShortcut *findText = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), this);
    findText->setContext(Qt::WidgetWithChildrenShortcut);
    connect(findText, &QShortcut::activated, this, &TextEdit::slotSearchText);

    QShortcut *escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    escape->setContext(Qt::WidgetWithChildrenShortcut);
    connect(escape, &QShortcut::activated, this, &TextEdit::slotCloseSearchText);

    auto f = font();
    f.setFamily(QStringLiteral("Courier New"));
    f.setPointSize(10);
    setFont(f);
}

bool TextEdit::isReadOnly() const
{
    return m_textEdit->isReadOnly();
}

void TextEdit::setReadOnly(bool value)
{
    m_textEdit->setReadOnly(value);
}

QTextDocument *TextEdit::document() const
{
    return m_textEdit->document();
}

void TextEdit::setDocument(QTextDocument *document)
{
    m_textEdit->setDocument(document);
}

QTextCursor TextEdit::textCursor() const
{
    return m_textEdit->textCursor();
}

void TextEdit::setTextCursor(const QTextCursor &cursor)
{
    m_textEdit->setTextCursor(cursor);
}

QString TextEdit::text() const
{
    return m_textEdit->toPlainText();
}

void TextEdit::clear()
{
    m_textEdit->clear();
}

void TextEdit::setText(const QString &text)
{
    m_textEdit->setPlainText(text);
}

void TextEdit::slotSearchLineEditChanged(const QString &str)
{
    const bool enable = !str.isEmpty();
    m_searchPreviewButton->setEnabled(enable);
    m_searchNextButton->setEnabled(enable);
    if (enable) {
        slotSearchNextText();
    } else {
        // Clear the selection
        auto cursor = m_textEdit->textCursor();
        cursor.clearSelection();
        m_textEdit->setTextCursor(cursor);
    }
}

void TextEdit::slotSearchPreviewText()
{
    const QString searchString = m_searchText->text();
    m_textEdit->find(searchString, QTextDocument::FindBackward);
}

void TextEdit::slotSearchNextText()
{
    const QString searchString = m_searchText->text();
    m_textEdit->find(searchString);
}

void TextEdit::slotSearchText()
{
    if (m_textEdit->textCursor().hasSelection()) {
        const QString searchText = m_textEdit->textCursor().selectedText();
        m_searchText->setText(searchText);
    }
    m_searchWidget->setVisible(true);
    m_searchText->setFocus();
}

void TextEdit::slotCloseSearchText()
{
    m_searchWidget->setVisible(false);
}
