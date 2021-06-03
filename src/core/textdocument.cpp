#include "textdocument.h"

#include <QFile>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextStream>

#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype TextDocument
 * \brief Document object for text files.
 * \instantiates Core::TextDocument
 * \inqmlmodule Script
 * \since 4.0
 * \inherits Document
 */

/*!
 * \qmlproperty int TextDocument::column
 * This read-only property hold the column of the cursor position.
 * Be careful the column is 1-based, so the column before the first character is 1.
 */
/*!
 * \qmlproperty int TextDocument::line
 * This read-only property hold the line of the cursor position.
 * Be careful the line is 1-based, so the first line of the document is 1,
 */
/*!
 * \qmlproperty int TextDocument::lineCount
 * This read-only property holds the number of lines in the document.
 */
/*!
 * \qmlproperty int TextDocument::position
 * This property holds the absolute position of the cursor inside the text document.
 */
/*!
 * \qmlproperty string TextDocument::text
 * This property holds the text of the document.
 */
/*!
 * \qmlproperty string TextDocument::selectedText
 * This property holds the selected text of the document.
 */

TextDocument::TextDocument(QObject *parent)
    : TextDocument(Document::Type::Text, parent)
{
}

TextDocument::~TextDocument() { }

TextDocument::TextDocument(Type type, QObject *parent)
    : Document(type, parent)
    , m_document(new QPlainTextEdit())
{
    m_document->hide();
    connect(m_document.get(), &QPlainTextEdit::textChanged, this, &TextDocument::textChanged);
    connect(m_document.get(), &QPlainTextEdit::selectionChanged, this, &TextDocument::selectionChanged);
    connect(m_document.get(), &QPlainTextEdit::cursorPositionChanged, this, &TextDocument::positionChanged);
}

bool TextDocument::doSave(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        setErrorString(file.errorString());
        spdlog::error("Can't save file {}: {}", fileName.toStdString(), errorString().toStdString());
        return false;
    }

    QTextStream stream(&file);
    stream << m_document->toPlainText();
    return true;
}

bool TextDocument::doLoad(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        setErrorString(file.errorString());
        spdlog::warn("Can't load file {}: {}", fileName.toStdString(), errorString().toStdString());
        return false;
    }

    QTextStream stream(&file);
    setText(stream.readAll());
    return true;
}

int TextDocument::column() const
{
    const auto cursor = m_document->textCursor();
    return cursor.position() - cursor.block().position();
}

int TextDocument::line() const
{
    const auto cursor = m_document->textCursor();
    return cursor.blockNumber() + 1;
}

int TextDocument::lineCount() const
{
    return m_document->document()->lineCount();
}

int TextDocument::position() const
{
    return m_document->textCursor().position();
}

void TextDocument::setPosition(int newPosition)
{
    if (position() == newPosition)
        return;
    auto cursor = m_document->textCursor();
    cursor.setPosition(newPosition);
    m_document->setTextCursor(cursor);
    emit positionChanged();
}

QString TextDocument::text() const
{
    return m_document->toPlainText();
}

void TextDocument::setText(const QString &newText)
{
    setHasChanged(true);
    m_document->setPlainText(newText);
}

QString TextDocument::selectedText() const
{
    return m_document->textCursor().selectedText();
}

} // namespace Core
