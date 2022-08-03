#include "textdocument.h"
#include "textdocument_p.h"

#include "logger.h"
#include "mark.h"
#include "settings.h"
#include "string_utils.h"

#include <QFile>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QTextBlock>
#include <QTextStream>
#include <private/qwidgettextcontrol_p.h>

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
 * This read-only property holds the column of the cursor position.
 * Be careful the column is 1-based, so the column before the first character is 1.
 */
/*!
 * \qmlproperty int TextDocument::line
 * This read-only property holds the line of the cursor position.
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
/*!
 * \qmlproperty LineEnding TextDocument::lineEnding
 * This property holds the line ending for the document. It can be one of those choices:
 *
 * - `TextDocument.LFLineEnding`: '\n' character
 * - `TextDocument.CRLFLineEnding`: '\r\n' characters
 * - `TextDocument.NativeLineEnding`: LF on Linux and Mac, CRLF on Windows
 *
 * Native is the default for new documents.
 */
/*!
 * \qmlproperty string TextDocument::currentLine
 * This read-only property return the line under the current position.
 */
/*!
 * \qmlproperty string TextDocument::currentWord
 * This read-only property return the word under the current position.
 */

TextDocument::TextDocument(QObject *parent)
    : TextDocument(Document::Type::Text, parent)
{
}

TextDocument::~TextDocument()
{
    delete m_document;
}

TextDocument::TextDocument(Type type, QObject *parent)
    : Document(type, parent)
    , m_document(new QPlainTextEdit())
{
    m_document->hide();
    connect(m_document, &QPlainTextEdit::textChanged, this, &TextDocument::textChanged);
    connect(m_document, &QPlainTextEdit::selectionChanged, this, &TextDocument::selectionChanged);
    connect(m_document, &QPlainTextEdit::cursorPositionChanged, this, &TextDocument::positionChanged);
    connect(m_document->document(), &QTextDocument::contentsChange, this, [this]() {
        setHasChanged(true);
    });
    m_document->installEventFilter(this);
}

bool TextDocument::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(watched == m_document);

    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent == QKeySequence::MoveToNextChar)
            gotoNextChar();
        else if (keyEvent == QKeySequence::MoveToPreviousChar)
            gotoPreviousChar();
        else if (keyEvent == QKeySequence::SelectNextChar)
            selectNextChar();
        else if (keyEvent == QKeySequence::SelectPreviousChar)
            selectPreviousChar();
        else if (keyEvent == QKeySequence::SelectNextWord)
            selectNextWord();
        else if (keyEvent == QKeySequence::SelectPreviousWord)
            selectPreviousWord();
        else if (keyEvent == QKeySequence::SelectStartOfLine)
            selectStartOfLine();
        else if (keyEvent == QKeySequence::SelectEndOfLine)
            selectEndOfLine();
        else if (keyEvent == QKeySequence::SelectPreviousLine)
            selectPreviousLine();
        else if (keyEvent == QKeySequence::SelectNextLine)
            selectNextLine();
        else if (keyEvent == QKeySequence::MoveToNextWord)
            gotoNextWord();
        else if (keyEvent == QKeySequence::MoveToPreviousWord)
            gotoPreviousWord();
        else if (keyEvent == QKeySequence::MoveToNextLine)
            gotoNextLine();
        else if (keyEvent == QKeySequence::MoveToPreviousLine)
            gotoPreviousLine();
        else if (keyEvent == QKeySequence::MoveToStartOfLine)
            gotoStartOfLine();
        else if (keyEvent == QKeySequence::MoveToEndOfLine)
            gotoEndOfLine();
        else if (keyEvent == QKeySequence::MoveToStartOfDocument)
            gotoStartOfDocument();
        else if (keyEvent == QKeySequence::MoveToEndOfDocument)
            gotoEndOfDocument();
        else if (keyEvent == QKeySequence::MoveToNextPage)
            return false;
        else if (keyEvent == QKeySequence::MoveToPreviousPage)
            return false;
        else if ((keyEvent->key() == Qt::Key_Backtab))
            removeIndent();
        else if (keyEvent->key() == Qt::Key_Tab)
            indent();
        else if (keyEvent == QKeySequence::Undo)
            undo();
        else if (keyEvent == QKeySequence::Redo)
            redo();
        else if (keyEvent == QKeySequence::Cut)
            cut();
        else if (keyEvent == QKeySequence::Copy)
            copy();
        else if (keyEvent == QKeySequence::Paste)
            paste();
        else if (keyEvent == QKeySequence::Delete)
            m_document->textCursor().hasSelection() ? deleteSelection() : deleteNextCharacter();
        else if (keyEvent == QKeySequence::Backspace
                 || (keyEvent->key() == Qt::Key_Backspace
                     && !(keyEvent->modifiers() & ~Qt::ShiftModifier))) // test is coming from QTextWidgetControl
            m_document->textCursor().hasSelection() ? deleteSelection() : deletePreviousCharacter();
        else if (keyEvent == QKeySequence::InsertParagraphSeparator)
            insert("\n");
        else if (keyEvent == QKeySequence::InsertLineSeparator)
            insert(QString(QChar::LineSeparator));
        else if (keyEvent == QKeySequence::DeleteEndOfWord)
            deleteEndOfWord();
        else if (keyEvent == QKeySequence::DeleteStartOfWord)
            deleteStartOfWord();
        else if (keyEvent == QKeySequence::DeleteEndOfLine)
            deleteEndOfLine();
        else if (keyEvent == QKeySequence::SelectAll)
            selectAll();
        else if (!keyEvent->text().isEmpty()) {
            auto control = m_document->findChild<QWidgetTextControl *>();
            if (control->isAcceptableInput(keyEvent))
                insert(keyEvent->text());
        }

        return true;
    }
    return Document::eventFilter(watched, event);
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

    if (m_utf8Bom)
        file.write("\xef\xbb\xbf", 3);

    QString plainText = m_document->toPlainText();
    if (m_lineEnding == CRLFLineEnding)
        plainText.replace('\n', "\r\n");

    QTextStream stream(&file);
    stream << plainText;
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

    QByteArray data = file.readAll();
    detectFormat(data);
    QTextStream stream(data);
    const QString text = stream.readAll();

    QSignalBlocker sb(m_document->document());
    // This will replace '\r\n' with '\n'
    m_document->setPlainText(text);
    setHasChanged(false);

    return true;
}

// This function is copied from TextFileFormat::detect from Qt Creator.
void TextDocument::detectFormat(const QByteArray &data)
{
    if (data.isEmpty())
        return;
    const int bytesRead = data.size();
    const auto buf = reinterpret_cast<const unsigned char *>(data.constData());
    // code taken from qtextstream
    if (bytesRead >= 3 && ((buf[0] == 0xef && buf[1] == 0xbb) && buf[2] == 0xbf))
        m_utf8Bom = true;

    // end code taken from qtextstream
    const int newLinePos = data.indexOf('\n');
    if (newLinePos == -1)
        setLineEnding(NativeLineEnding);
    else if (newLinePos == 0)
        setLineEnding(LFLineEnding);
    else
        setLineEnding(data.at(newLinePos - 1) == '\r' ? CRLFLineEnding : LFLineEnding);
}

int TextDocument::column() const
{
    LOG("TextDocument::column");
    const QTextCursor cursor = m_document->textCursor();
    LOG_RETURN("column", cursor.positionInBlock() + 1);
}

int TextDocument::line() const
{
    LOG("TextDocument::line");
    const QTextCursor cursor = m_document->textCursor();
    LOG_RETURN("line", cursor.blockNumber() + 1);
}

int TextDocument::lineCount() const
{
    LOG("TextDocument::lineCount");
    return m_document->document()->lineCount();
}

int TextDocument::position() const
{
    LOG("TextDocument::position");
    LOG_RETURN("pos", m_document->textCursor().position());
}

void TextDocument::setPosition(int newPosition)
{
    LOG("TextDocument::position", LOG_ARG("pos", newPosition));

    if (position() == newPosition)
        return;
    auto cursor = m_document->textCursor();
    cursor.setPosition(newPosition);
    m_document->setTextCursor(cursor);
    emit positionChanged();
}

void TextDocument::convertPosition(int pos, int *line, int *column) const
{
    Q_ASSERT(line && column);
    QTextBlock block = m_document->document()->findBlock(pos);
    if (!block.isValid()) {
        (*line) = -1;
        (*column) = -1;
    } else {
        // line and column are both 1-based
        (*line) = block.blockNumber() + 1;
        (*column) = pos - block.position() + 1;
    }
}

int TextDocument::position(QTextCursor::MoveOperation operation, int pos) const
{
    auto cursor = m_document->textCursor();

    if (pos != -1)
        cursor.setPosition(pos);

    if (operation == QTextCursor::NoMove)
        return cursor.position();

    cursor.movePosition(operation);
    return cursor.position();
}

QString TextDocument::text() const
{
    LOG("TextDocument::text");
    LOG_RETURN("text", m_document->toPlainText());
}

void TextDocument::setText(const QString &newText)
{
    LOG("TextDocument::text", LOG_ARG("text", newText));

    m_document->setPlainText(newText);
}

QString TextDocument::currentLine() const
{
    LOG("TextDocument::currentLine");
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    LOG_RETURN("text", cursor.selectedText());
}

QString TextDocument::currentWord() const
{
    LOG("TextDocument::currentWord");
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::StartOfWord);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    LOG_RETURN("text", cursor.selectedText());
}

QString TextDocument::selectedText() const
{
    LOG("TextDocument::selectedText");
    // Replace \u2029 with \n
    QString text = m_document->textCursor().selectedText().replace(QChar(8233), "\n");
    LOG_RETURN("text", text);
}

TextDocument::LineEnding TextDocument::lineEnding() const
{
    return m_lineEnding;
}

bool TextDocument::hasUtf8Bom() const
{
    return m_utf8Bom;
}

QPlainTextEdit *TextDocument::textEdit() const
{
    return m_document;
}

/**
 * \brief Returns the string when pressing on the tab key
 */
QString TextDocument::tab() const
{
    auto settings = Settings::instance()->value<TabSettings>(Settings::Tab);
    if (settings.insertSpaces)
        return QString(settings.tabSize, ' ');
    return QStringLiteral("\t");
}

/*!
 * \qmlmethod TextDocument::undo(int count)
 * Undo `count` times the last actions.
 */
void TextDocument::undo(int count)
{
    LOG_AND_MERGE("TextDocument::undo", count);
    while (count != 0) {
        m_document->undo();
        --count;
    }
}

/*!
 * \qmlmethod TextDocument::redo(int count)
 * Redo `count` times the last actions.
 */
void TextDocument::redo(int count)
{
    LOG_AND_MERGE("TextDocument::redo", count);
    while (count != 0) {
        m_document->redo();
        --count;
    }
}

void TextDocument::movePosition(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode, int count)
{
    auto cursor = m_document->textCursor();
    cursor.movePosition(operation, mode, count);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::gotoLine(int line, int column = 1)
 * Goes to the given `line` and `column` in the editor. Lines and columns are 1-based.
 * \sa TextDocument::line
 * \sa TextDocument::column
 */
void TextDocument::gotoLine(int line, int column)
{
    LOG("TextDocument::gotoLine", LOG_ARG("line", line), LOG_ARG("column", column));

    // Internally, columns are 0-based, while 1-based on the API
    column = column - 1;
    const int blockNumber = qMin(line, m_document->document()->blockCount()) - 1;
    const QTextBlock &block = m_document->document()->findBlockByNumber(blockNumber);
    if (block.isValid()) {
        QTextCursor cursor(block);
        if (column > 0)
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);

        m_document->setTextCursor(cursor);
    }
}

/*!
 * \qmlmethod TextDocument::gotoStartOfLine()
 * Goes to the start of the line.
 */
void TextDocument::gotoStartOfLine()
{
    LOG("TextDocument::gotoStartOfLine");
    movePosition(QTextCursor::StartOfLine);
}

/*!
 * \qmlmethod TextDocument::gotoEndOfLine()
 * Goes to the end of the line.
 */
void TextDocument::gotoEndOfLine()
{
    LOG("TextDocument::gotoEndOfLine");
    movePosition(QTextCursor::EndOfLine);
}

/*!
 * \qmlmethod TextDocument::gotoStartOfWord()
 * Goes to the start of the word under the cursor.
 */
void TextDocument::gotoStartOfWord()
{
    LOG("TextDocument::gotoStartOfWord");
    movePosition(QTextCursor::StartOfWord);
}

/*!
 * \qmlmethod TextDocument::gotoEndOfWord()
 * Goes to the end of the word under the cursor.
 */
void TextDocument::gotoEndOfWord()
{
    LOG("TextDocument::gotoEndOfWord");
    movePosition(QTextCursor::EndOfWord);
}

/*!
 * \qmlmethod TextDocument::gotoNextLine(int count = 1)
 * Goes to the next line, repeat the operation `count` times.
 */
void TextDocument::gotoNextLine(int count)
{
    LOG_AND_MERGE("TextDocument::gotoNextLine", count);
    movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoPreviousLine(int count = 1)
 * Goes to the previous line, repeat the operation `count` times.
 */
void TextDocument::gotoPreviousLine(int count)
{
    LOG_AND_MERGE("TextDocument::gotoPreviousLine", count);
    movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoPreviousChar(int count = 1)
 * Goes to the previous character, repeat the operation `count` times.
 */
void TextDocument::gotoPreviousChar(int count)
{
    LOG_AND_MERGE("TextDocument::gotoPreviousChar", count);
    movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoNextChar(int count = 1)
 * Goes to the next character, repeat the operation `count` times.
 */
void TextDocument::gotoNextChar(int count)
{
    LOG_AND_MERGE("TextDocument::gotoNextChar", count);
    movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoPreviousWord(int count = 1)
 * Goes to the previous word, repeat the operation `count` times.
 */
void TextDocument::gotoPreviousWord(int count)
{
    LOG_AND_MERGE("TextDocument::gotoPreviousWord", count);
    movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoNextWord(int count = 1)
 * Goes to the next word, repeat the operation `count` times.
 */
void TextDocument::gotoNextWord(int count)
{
    LOG_AND_MERGE("TextDocument::gotoNextWord", count);
    movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoStartOfDocument()
 * Goes to the document start.
 */
void TextDocument::gotoStartOfDocument()
{
    LOG("TextDocument::gotoStartOfDocument");
    movePosition(QTextCursor::Start);
}

/*!
 * \qmlmethod TextDocument::gotoEndOfDocument()
 * Goes to the document end.
 */
void TextDocument::gotoEndOfDocument()
{
    LOG("TextDocument::gotoEndOfDocument");
    movePosition(QTextCursor::End);
}

/*!
 * \qmlmethod TextDocument::unselect()
 * Clears the current selection.
 */
void TextDocument::unselect()
{
    LOG("TextDocument::unselect");
    QTextCursor cursor = m_document->textCursor();
    cursor.clearSelection();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod bool TextDocument::hasSelection()
 * Returns true if the editor has a selection.
 */
bool TextDocument::hasSelection()
{
    LOG("TextDocument::hasSelection");
    return m_document->textCursor().hasSelection();
}

/*!
 * \qmlmethod TextDocument::selectAll()
 * Selects all the text.
 */
void TextDocument::selectAll()
{
    LOG("TextDocument::selectAll");
    m_document->selectAll();
}

/*!
 * \qmlmethod TextDocument::selectTo(int pos)
 * Selects the text from the current position to `pos`.
 */
void TextDocument::selectTo(int pos)
{
    LOG("TextDocument::selectTo", LOG_ARG("pos", pos));
    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(pos, QTextCursor::KeepAnchor);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::selectStartOfLine(int count = 1)
 * Selects the text from the current position to the start of the line.
 */
void TextDocument::selectStartOfLine()
{
    LOG("TextDocument::selectStartOfLine");
    movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectEndOfLine()
 * Selects the text from the current position to the end of the line.
 */
void TextDocument::selectEndOfLine()
{
    LOG("TextDocument::selectEndOfLine");
    movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectStartOfWord()
 * Selects the text from the current position to the start of the word.
 */
void TextDocument::selectStartOfWord()
{
    LOG("TextDocument::selectStartOfWord");
    movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectEndOfWord()
 * Selects the text from the current position to the end of the word.
 */
void TextDocument::selectEndOfWord()
{
    LOG("TextDocument::selectEndOfWord");
    movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectNextLine(int count = 1)
 * Selects the next line, repeat the operation `count` times.
 */
void TextDocument::selectNextLine(int count)
{
    LOG_AND_MERGE("TextDocument::selectNextLine", count);
    movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectPreviousLine(int count = 1)
 * Selectes the previous line, repeat the operation `count` times.
 */
void TextDocument::selectPreviousLine(int count)
{
    LOG_AND_MERGE("TextDocument::selectPreviousLine", count);
    movePosition(QTextCursor::Up, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectPreviousChar(int count = 1)
 * Selects the previous character, repeat the operation `count` times.
 */
void TextDocument::selectPreviousChar(int count)
{
    LOG_AND_MERGE("TextDocument::selectPreviousChar", count);
    movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectNextChar(int count = 1)
 * Selects the next character, repeat the operation `count` times.
 */
void TextDocument::selectNextChar(int count)
{
    LOG_AND_MERGE("TextDocument::selectNextChar", count);
    movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectPreviousWord(int count = 1)
 * Selects the previous word, repeat the operation `count` times.
 */
void TextDocument::selectPreviousWord(int count)
{
    LOG_AND_MERGE("TextDocument::selectPreviousWord {}", count);
    movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectNextWord(int count = 1)
 * Selects the next word, repeat the operation `count` times.
 */
void TextDocument::selectNextWord(int count)
{
    LOG_AND_MERGE("TextDocument::selectNextWord", count);
    movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectRegion(int from, int to)
 * Selects the text between `from` and `to` positions.
 */
void TextDocument::selectRegion(int from, int to)
{
    LOG("TextDocument::selectRegion", from, to);
    QTextCursor cursor(m_document->document());
    cursor.setPosition(from, QTextCursor::MoveAnchor);
    cursor.setPosition(to, QTextCursor::KeepAnchor);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::selectRange(TextRange range)
 * Selects the range passed in parameter.
 */
void TextDocument::selectRange(const TextRange &range)
{
    LOG("TextDocument::selectRange", range);
    selectRegion(range.start, range.end);
}

/*!
 * \qmlmethod TextDocument::copy()
 * Copies the selected text.
 */
void TextDocument::copy()
{
    LOG("TextDocument::copy");
    m_document->copy();
}

/*!
 * \qmlmethod TextDocument::paste()
 * Pastes text in the clipboard.
 */
void TextDocument::paste()
{
    LOG("TextDocument::paste");
    m_document->paste();
}

/*!
 * \qmlmethod TextDocument::cut()
 * Cuts the selected text.
 */
void TextDocument::cut()
{
    LOG("TextDocument::cut");
    m_document->cut();
}

/*!
 * \qmlmethod TextDocument::remove(int length)
 * Remove `length` character from the current position.
 */
void TextDocument::remove(int length)
{
    LOG("TextDocument::remove", length);
    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(cursor.position() + length, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::insert(string text)
 * Inserts the string `text` at the current position. If some text is selected it will be replaced.
 */
void TextDocument::insert(const QString &text)
{
    LOG_AND_MERGE("TextDocument::insert", LOG_ARG("text", text));
    m_document->insertPlainText(text);
}

/*!
 * \qmlmethod TextDocument::insertAtLine(string text, int line = -1)
 * Inserts the string `text` at `line`. If `line` is -1, insert the text at the current position. `line` is 1-based.
 */
void TextDocument::insertAtLine(const QString &text, int line)
{
    if (line == -1)
        LOG("TextDocument::insertAtLine", LOG_ARG("text", text));
    else
        LOG("TextDocument::insertAtLine", LOG_ARG("text", text), LOG_ARG("line", line));

    QTextCursor cursor = m_document->textCursor();
    if (line > 0) {
        const int blockNumber = qMin(line, m_document->document()->blockCount()) - 1;
        const QTextBlock &block = m_document->document()->findBlockByNumber(blockNumber);
        if (block.isValid())
            cursor = QTextCursor(block);
    }
    cursor.insertText(text);
}

/*!
 * \qmlmethod TextDocument::replace(int length, string text)
 * Replaces `length` characters from the current position with the string `text`.
 */
void TextDocument::replace(int length, const QString &text)
{
    LOG("TextDocument::replace", length, text);
    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(cursor.position() + length, QTextCursor::KeepAnchor);
    cursor.insertText(text);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::replace(int length, string text)
 * Replaces the text from `from` to `to` with the string `text`.
 */
void TextDocument::replace(int from, int to, const QString &text)
{
    LOG("TextDocument::replace", from, to, text);
    QTextCursor cursor(m_document->document());
    cursor.setPosition(from);
    cursor.setPosition(to, QTextCursor::KeepAnchor);
    cursor.insertText(text);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::replace(TextRange range, string text)
 * Replaces the text in the range `range` with the string `text`.
 */
void TextDocument::replace(const TextRange &range, const QString &text)
{
    LOG("TextDocument::replace", range, text);
    QTextCursor cursor(m_document->document());
    cursor.setPosition(range.start);
    cursor.setPosition(range.end, QTextCursor::KeepAnchor);
    cursor.insertText(text);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deleteLine(int line = -1)
 * Remove a the line `line`. If `line` is -1, remove the current line. `line` is 1-based.
 */
void TextDocument::deleteLine(int line)
{
    if (line == -1)
        LOG("TextDocument::removeLine");
    else
        LOG("TextDocument::removeLine", LOG_ARG("line", line));

    QTextCursor cursor = m_document->textCursor();
    if (line > 0) {
        const int blockNumber = qMin(line, m_document->document()->blockCount()) - 1;
        const QTextBlock &block = m_document->document()->findBlockByNumber(blockNumber);
        if (block.isValid())
            cursor = QTextCursor(block);
    } else {
        cursor.movePosition(QTextCursor::StartOfLine);
    }
    cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

/*!
 * \qmlmethod TextDocument::deleteSelection()
 * Deletes the current selection, does nothing if no text is selected.
 */
void TextDocument::deleteSelection()
{
    LOG("TextDocument::deleteSelection");
    m_document->textCursor().removeSelectedText();
}

/*!
 * \qmlmethod TextDocument::deleteRegion(int from, int to)
 * Deletes the text between `from` and `to` positions.
 */
void TextDocument::deleteRegion(int from, int to)
{
    LOG("TextDocument::deleteRegion", from, to);
    QTextCursor cursor(m_document->document());
    cursor.setPosition(from);
    cursor.setPosition(to, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deleteRange(TextRange range)
 * Deletes the range passed in parameter.
 */
void TextDocument::deleteRange(const TextRange &range)
{
    LOG("TextDocument::deleteRange", range);
    QTextCursor cursor(m_document->document());
    cursor.setPosition(range.start, QTextCursor::MoveAnchor);
    cursor.setPosition(range.end, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deleteEndOfLine()
 * Deletes from the cursor position to the end of the line.
 */
void TextDocument::deleteEndOfLine()
{
    LOG("TextDocument::deleteEndOfLine");
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deleteStartOfLine()
 * Deletes from the cursor position to the start of the line.
 */
void TextDocument::deleteStartOfLine()
{
    LOG("TextDocument::deleteStartOfLine");
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deleteEndOfWord()
 * Deletes from the cursor position to the end of the word.
 */
void TextDocument::deleteEndOfWord()
{
    LOG("TextDocument::deleteEndOfWord");
    QTextCursor cursor = m_document->textCursor();
    if (!cursor.hasSelection())
        cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deleteStartOfWord()
 * Deletes from the cursor position to the start of the word.
 */
void TextDocument::deleteStartOfWord()
{
    LOG("TextDocument::deleteStartOfWord");
    QTextCursor cursor = m_document->textCursor();
    if (!cursor.hasSelection())
        cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deletePreviousCharacter(int count = 1)
 * Deletes the previous `count` characters.
 */
void TextDocument::deletePreviousCharacter(int count)
{
    LOG_AND_MERGE("TextDocument::deletePreviousCharacter", count);
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, count);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deleteNextCharacter(int count = 1)
 * Deletes the next `count` characters.
 */
void TextDocument::deleteNextCharacter(int count)
{
    LOG_AND_MERGE("TextDocument::deleteNextCharacter", count);
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, count);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod Mark TextDocument::createMark(int pos = -1)
 * Create a mark at the given position `pos`. If `pos` is -1, it will create a mark at the
 * current position.
 * \sa Mark
 */
Mark *TextDocument::createMark(int pos)
{
    LOG("TextDocument::createMark", LOG_ARG("pos", pos));
    if (pos == -1)
        pos = position();
    LOG_RETURN("mark", new Mark(this, pos));
}

/*!
 * \qmlmethod TextDocument::gotoMark(Mark mark)
 * Go to the given `mark`.
 */
void TextDocument::gotoMark(Mark *mark)
{
    if (!mark)
        return;
    LOG("TextDocument::gotoMark", LOG_ARG("mark", mark));
    if (mark->m_editor != this) {
        spdlog::error("Can't use a mark from another editor.");
        return;
    }

    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(mark->position());
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::selectToMark(Mark mark)
 * Select the text from the cursor position to the `mark`.
 */
void TextDocument::selectToMark(Mark *mark)
{
    if (!mark)
        return;
    LOG("TextDocument::selectToMark", LOG_ARG("mark", mark));
    if (mark->m_editor != this) {
        spdlog::error("Can't use a mark from another editor.");
        return;
    }

    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(mark->position(), QTextCursor::KeepAnchor);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod bool TextDocument::find(string text, int options = TextDocument.NoFindFlags)
 * Searches the string `text` in the editor. Options could be a combination of:
 *
 * - `TextDocument.FindBackward`: search backward
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only complete words
 * - `TextDocument.FindRegexp`: use a regexp, equivalent to calling `findRegexp`
 *
 * Selects the match and returns `true` if a match is found.
 */
bool TextDocument::find(const QString &text, int options)
{
    LOG("TextDocument::find", LOG_ARG("text", text), options);
    if (options & FindRegexp)
        return findRegexp(text, options);
    else
        return m_document->find(text, static_cast<QTextDocument::FindFlags>(options));
}

/*!
 * \qmlmethod bool TextDocument::findRegexp(string regexp, int options = TextDocument.NoFindFlags)
 * Searches the string `regexp` in the editor using a regular expression. Options could be a combination of:
 *
 * - `TextDocument.FindBackward`: search backward
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only complete words
 *
 * Selects the match and returns `true` if a match is found.
 */
bool TextDocument::findRegexp(const QString &regexp, int options)
{
    LOG("TextDocument::findRegexp", regexp, options);
    auto regularExpression = createRegularExpression(regexp, options | TextDocument::FindRegexp);

    auto cursor = m_document->textCursor();
    auto text = m_document->toPlainText();

    QRegularExpressionMatch match;
    if (options & TextDocument::FindBackward) {
        int pos;
        if (cursor.hasSelection())
            pos = cursor.selectionStart();
        else
            pos = qMax(0, cursor.position() - 1);

        // Find last match
        Q_UNUSED(text.sliced(0, pos).lastIndexOf(regularExpression, &match))
    } else {
        match = regularExpression.match(text, cursor.position());
    }

    if (!match.hasMatch())
        return false;

    cursor.setPosition(match.capturedStart(), QTextCursor::MoveAnchor);
    cursor.setPosition(match.capturedEnd(), QTextCursor::KeepAnchor);

    m_document->setTextCursor(cursor);
    return true;
}

/*!
 * \qmlmethod bool TextDocument::replaceOne(string before, string after, int options = TextDocument.NoFindFlags)
 * Replace one occurence of the string `before` with `after`. Options could be a combination of:
 *
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only complete words
 * - `TextDocument.FindRegexp`: use a regexp, equivalent to calling `findRegexp`
 * - `TextDocument.PreserveCase`: preserve case when replacing
 *
 * If the option `TextEditor.PreserveCase` is used, it means:
 *
 * - All upper-case occurrences are replaced with the upper-case new text.
 * - All lower-case occurrences are replaced with the lower-case new text.
 * - Capitalized occurrences are replaced with the capitalized new text.
 * - Other occurrences are replaced with the new text as entered. If an occurrence and the new text have the same prefix
 * or suffix, then the case of the prefix and/or suffix are preserved, and the other rules are applied on the rest of
 * the occurrence only.
 *
 * Returns true if a change occurs in the document..
 */
bool TextDocument::replaceOne(const QString &before, const QString &after, int options)
{
    LOG("TextDocument::replaceOne", LOG_ARG("text", before), after, options);

    auto cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_document->setTextCursor(cursor);

    const bool usesRegExp = options & FindRegexp;
    const bool preserveCase = options & PreserveCase;

    auto regexp = createRegularExpression(before, options);
    if (find(before, options)) {
        cursor.beginEditBlock();
        auto found = m_document->textCursor();
        cursor.setPosition(found.selectionStart());
        cursor.setPosition(found.selectionEnd(), QTextCursor::KeepAnchor);
        QString afterText = after;
        if (usesRegExp) {
            QRegularExpressionMatch match = regexp.match(selectedText());
            afterText = expandRegExpReplacement(after, match.capturedTexts());
        } else if (preserveCase) {
            afterText = matchCaseReplacement(cursor.selectedText(), after);
        }
        cursor.insertText(afterText);
        cursor.endEditBlock();
        return true;
    }

    return false;
}

/*!
 * \qmlmethod bool TextDocument::replaceAll(string before, string after, int options = TextDocument.NoFindFlags)
 * Replace all occurences of the string `before` with `after`. Options could be a combination of:
 *
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only complete words
 * - `TextDocument.FindRegexp`: use a regexp, equivalent to calling `findRegexp`
 * - `TextDocument.PreserveCase`: preserve case when replacing
 *
 * If the option `TextEditor.PreserveCase` is used, it means:
 *
 * - All upper-case occurrences are replaced with the upper-case new text.
 * - All lower-case occurrences are replaced with the lower-case new text.
 * - Capitalized occurrences are replaced with the capitalized new text.
 * - Other occurrences are replaced with the new text as entered. If an occurrence and the new text have the same prefix
 * or suffix, then the case of the prefix and/or suffix are preserved, and the other rules are applied on the rest of
 * the occurrence only.
 *
 * Returns the number of changes done in the document.
 */
int TextDocument::replaceAll(const QString &before, const QString &after, int options)
{
    LOG("TextDocument::replaceAll", LOG_ARG("text", before), after, options);

    const bool backwards = options & FindBackward;
    const bool usesRegExp = options & FindRegexp;
    const bool preserveCase = options & PreserveCase;

    int count = 0;
    auto cursor = m_document->textCursor();
    cursor.movePosition(backwards ? QTextCursor::End : QTextCursor::Start);
    m_document->setTextCursor(cursor);
    cursor.beginEditBlock();

    auto regexp = createRegularExpression(before, options);
    while (find(before, options)) {
        auto found = m_document->textCursor();
        cursor.setPosition(found.selectionStart());
        cursor.setPosition(found.selectionEnd(), QTextCursor::KeepAnchor);
        QString afterText = after;
        if (usesRegExp) {
            QRegularExpressionMatch match = regexp.match(selectedText());
            afterText = expandRegExpReplacement(after, match.capturedTexts());
        } else if (preserveCase) {
            afterText = matchCaseReplacement(cursor.selectedText(), after);
        }
        cursor.insertText(afterText);
        ++count;
    }

    cursor.endEditBlock();
    return count;
}

/*!
 * \qmlmethod bool TextDocument::replaceAllRegexp(string regexp, string after, int options = TextDocument.NoFindFlags)
 * Replace all occurences of the matches for the `regexp` with `after`. See the options from `replaceAll`.
 *
 * The captures coming from the regexp can be used in the replacement text, using `\1`..`\n` or `$1`..`$n`.
 *
 * Returns the number of changes done in the document.
 */
int TextDocument::replaceAllRegexp(const QString &regexp, const QString &after, int options)
{
    LOG("TextDocument::replaceAllRegexp", regexp, after, options);
    return replaceAll(regexp, after, options | FindRegexp);
}

static int columnAt(const QString &text, int position, int tabSize)
{
    int column = 0;
    for (int i = 0; i < position; ++i) {
        if (text.at(i) == '\t')
            column = column - (column % tabSize) + tabSize;
        else
            ++column;
    }
    return column;
}

static int firstNonSpace(const QString &text)
{
    int i = 0;
    while (i < text.size()) {
        if (!text.at(i).isSpace())
            return i;
        ++i;
    }
    return i;
}

static int indentOneLine(QTextCursor &cursor, int tabCount, const TabSettings &settings)
{
    QString text = cursor.selectedText();
    cursor.removeSelectedText();

    const int oldSize = text.size();
    const int firstChar = firstNonSpace(text);
    const int startColumn = columnAt(text, firstChar, settings.tabSize);
    const int indentSize = qMax(startColumn / settings.tabSize + tabCount, 0);

    text.remove(0, firstChar);
    if (settings.insertSpaces)
        text = QString(indentSize * settings.tabSize, ' ') + text;
    else
        text = QString(indentSize, '\t') + text;

    cursor.insertText(text);
    return text.size() - oldSize;
}

void TextDocument::doIndent(int tabCount)
{
    const auto settings = Core::Settings::instance()->value<Core::TabSettings>(Core::Settings::Tab);

    QTextCursor cursor = m_document->textCursor();
    const bool hasSelection = cursor.hasSelection();
    const int lineStart = m_document->document()->findBlock(cursor.selectionStart()).blockNumber();
    const int lineEnd = m_document->document()->findBlock(cursor.selectionEnd()).blockNumber();

    // Move the position to the beginning of the first line
    int startPosition = cursor.position();
    cursor.setPosition(cursor.selectionStart());

    cursor.beginEditBlock();
    if (hasSelection) {
        startPosition = cursor.position();
        // Iterate through all line, and change the indentation
        for (int line = lineStart; line <= lineEnd; ++line) {
            cursor.select(QTextCursor::LineUnderCursor);
            const int delta = indentOneLine(cursor, tabCount, settings);
            if (line == lineStart)
                startPosition += delta;
            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
        }
        const int endPosition = cursor.position();
        // Select whole the lines indented
        cursor.setPosition(startPosition);
        cursor.setPosition(endPosition - 1, QTextCursor::KeepAnchor);
    } else {
        cursor.select(QTextCursor::LineUnderCursor);
        startPosition += indentOneLine(cursor, tabCount, settings);
        const int finalLine = m_document->document()->findBlock(startPosition).blockNumber();
        if (finalLine != lineStart)
            gotoLine(lineStart + 1);
        else
            cursor.setPosition(startPosition);
    }
    cursor.endEditBlock();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::indent(int count)
 * Indents the current line `count` times. If there's a selection, indent all lines in the selection.
 */
void TextDocument::indent(int count)
{
    LOG_AND_MERGE("TextDocument::indent", count);
    while (count != 0) {
        doIndent(1);
        --count;
    }
}

/*!
 * \qmlmethod TextDocument::removeIndent(int count)
 * Indents the current line `count` times. If there's a selection, indent all lines in the selection.
 */
void TextDocument::removeIndent(int count)
{
    LOG_AND_MERGE("TextDocument::removeIndent{}", count);
    while (count != 0) {
        doIndent(-1);
        --count;
    }
}

void TextDocument::setLineEnding(LineEnding newLineEnding)
{
    LOG("TextDocument::setLineEnding", newLineEnding);
    if (m_lineEnding == newLineEnding)
        return;
    setHasChanged(true);
    m_lineEnding = newLineEnding;
    emit lineEndingChanged();
}

} // namespace Core
