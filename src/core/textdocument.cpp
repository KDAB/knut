/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "textdocument.h"
#include "logger.h"
#include "mark.h"
#include "rangemark.h"
#include "settings.h"
#include "textdocument_p.h"
#include "texteditor.h"
#include "utils/log.h"
#include "utils/string_helper.h"

#include <QFile>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QTextBlock>
#include <QTextStream>
#include <private/qwidgettextcontrol_p.h>

namespace Core {

static std::optional<std::pair<QRegularExpressionMatch, QTextCursor>>
matchInBlock(const QTextBlock &block, const QRegularExpression &expr, int offset, int options)
{
    QString text = block.text();
    // Open Question: Why is this replacement necessary?
    text.replace(QChar::Nbsp, u' ');
    QRegularExpressionMatch match;

    if (options & TextDocument::FindBackward) {
        // For backwards search use offset - 1 because the cursor is positioned between characters,
        // so don't include the character for backward search.
        --offset;
    }

    if (offset >= 0 && offset <= text.size()) {
        const auto matchStart = (options & TextDocument::FindBackward) ? text.lastIndexOf(expr, offset, &match)
                                                                       : text.indexOf(expr, offset, &match);
        if (matchStart == -1)
            return {};

        QTextCursor cursor(block);
        if (options & TextDocument::FindBackward) {
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, matchStart + match.capturedLength());
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, match.capturedLength());
        } else {
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, matchStart);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, match.capturedLength());
        }

        return std::make_pair(match, cursor);
    }
    return {};
}

/*!
 * \qmltype TextDocument
 * \brief Document object for text files.
 * \ingroup TextDocument/@first
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
 * \qmlproperty int TextDocument::selectionStart
 * This property holds the start of the selection or position if the cursor doesn't have a selection.
 */
/*!
 * \qmlproperty int TextDocument::selectionEnd
 * This property holds the end of the selection or position if the cursor doesn't have a selection.
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
    , m_document(new TextEditor)
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
            indent(-1);
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
        spdlog::error("{} - Can't save file {}: {}", FUNCTION_NAME, fileName, errorString());
        return false;
    }

    if (m_utf8Bom)
        file.write("\xef\xbb\xbf", 3);

    QString plainText = m_document->toPlainText();
    if (m_lineEnding == CRLFLineEnding)
        plainText.replace('\n', "\r\n");

    QTextStream stream(&file);
    stream.setEncoding(static_cast<QStringConverter::Encoding>(DEFAULT_VALUE(TextDocument::Encoding, Encoding)));
    stream << plainText;
    return true;
}

bool TextDocument::doLoad(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        setErrorString(file.errorString());
        spdlog::warn("{} - Can't load file {}: {}", FUNCTION_NAME, fileName, errorString());
        return false;
    }

    QByteArray data = file.readAll();
    detectFormat(data);
    QTextStream stream(data);
    stream.setEncoding(static_cast<QStringConverter::Encoding>(DEFAULT_VALUE(TextDocument::Encoding, Encoding)));
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
    LOG();
    const QTextCursor cursor = m_document->textCursor();
    LOG_RETURN("column", cursor.positionInBlock() + 1);
}

int TextDocument::line() const
{
    LOG();
    const QTextCursor cursor = m_document->textCursor();
    LOG_RETURN("line", cursor.blockNumber() + 1);
}

int TextDocument::lineCount() const
{
    LOG();
    return m_document->document()->lineCount();
}

int TextDocument::position() const
{
    LOG();
    LOG_RETURN("pos", m_document->textCursor().position());
}

int TextDocument::selectionStart() const
{
    LOG();
    LOG_RETURN("pos", m_document->textCursor().selectionStart());
}

int TextDocument::selectionEnd() const
{
    LOG();
    LOG_RETURN("pos", m_document->textCursor().selectionEnd());
}

void TextDocument::setPosition(int newPosition)
{
    LOG(LOG_ARG("pos", newPosition));

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
    const QTextBlock block = m_document->document()->findBlock(pos);
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

/*!
 * \qmlmethod TextDocument::lineAtPosition(int position)
 * Returns the line number for the given text cursor `position`. Or -1 if position is invalid
 */
int TextDocument::lineAtPosition(int position)
{
    LOG(LOG_ARG("position", position));
    int line = -1;
    int col = -1;
    convertPosition(position, &line, &col);
    return line;
}

/*!
 * \qmlmethod TextDocument::columnAtPosition(int position)
 * Returns the column number for the given text cursor `position`. Or -1 if position is invalid
 */
int TextDocument::columnAtPosition(int position)
{
    LOG(LOG_ARG("position", position));
    int line = -1;
    int col = -1;
    convertPosition(position, &line, &col);
    return col;
}

/*!
 * \qmlmethod TextDocument::positionAt(int line, int col)
 * Returns the text cursor position for the given `line` number and `column` number. Or -1 if position was not found
 */
int TextDocument::positionAt(int line, int column)
{
    LOG(LOG_ARG("line", line), LOG_ARG("column", column));
    const QTextBlock block = m_document->document()->findBlockByLineNumber(line - 1);
    if (!block.isValid()) {
        return -1;
    } else {
        return block.position() + column - 1;
    }
}

QString TextDocument::text() const
{
    LOG();
    return m_document->toPlainText();
}

void TextDocument::setText(const QString &newText)
{
    LOG(LOG_ARG("text", newText));

    m_document->setPlainText(newText);
}

QString TextDocument::currentLine() const
{
    LOG();
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    LOG_RETURN("text", cursor.selectedText());
}

QString TextDocument::currentWord() const
{
    LOG();
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::StartOfWord);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    LOG_RETURN("text", cursor.selectedText());
}

QString TextDocument::selectedText() const
{
    LOG();
    // Replace \u2029 with \n
    const QString text = m_document->textCursor().selectedText().replace(QChar(8233), "\n");
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
    const auto settings = Settings::instance()->value<TabSettings>(Settings::Tab);
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
    LOG_AND_MERGE(count);
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
    LOG_AND_MERGE(count);
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
    LOG(LOG_ARG("line", line), LOG_ARG("column", column));

    gotoLineInTextEdit(m_document, line, column);
}

void gotoLineInTextEdit(QPlainTextEdit *textEdit, int line, int column)
{
    // Internally, columns are 0-based, while 1-based on the API
    column = column - 1;
    const int blockNumber = qMin(line, textEdit->document()->blockCount()) - 1;
    const QTextBlock &block = textEdit->document()->findBlockByNumber(blockNumber);
    if (block.isValid()) {
        QTextCursor cursor(block);
        if (column > 0)
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);

        textEdit->setTextCursor(cursor);
    }
}

/*!
 * \qmlmethod TextDocument::gotoStartOfLine()
 * Goes to the start of the line.
 */
void TextDocument::gotoStartOfLine()
{
    LOG();
    movePosition(QTextCursor::StartOfLine);
}

/*!
 * \qmlmethod TextDocument::gotoEndOfLine()
 * Goes to the end of the line.
 */
void TextDocument::gotoEndOfLine()
{
    LOG();
    movePosition(QTextCursor::EndOfLine);
}

/*!
 * \qmlmethod TextDocument::gotoStartOfWord()
 * Goes to the start of the word under the cursor.
 */
void TextDocument::gotoStartOfWord()
{
    LOG();
    movePosition(QTextCursor::StartOfWord);
}

/*!
 * \qmlmethod TextDocument::gotoEndOfWord()
 * Goes to the end of the word under the cursor.
 */
void TextDocument::gotoEndOfWord()
{
    LOG();
    movePosition(QTextCursor::EndOfWord);
}

/*!
 * \qmlmethod TextDocument::gotoNextLine(int count = 1)
 * Goes to the next line, repeat the operation `count` times.
 */
void TextDocument::gotoNextLine(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoPreviousLine(int count = 1)
 * Goes to the previous line, repeat the operation `count` times.
 */
void TextDocument::gotoPreviousLine(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoPreviousChar(int count = 1)
 * Goes to the previous character, repeat the operation `count` times.
 */
void TextDocument::gotoPreviousChar(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoNextChar(int count = 1)
 * Goes to the next character, repeat the operation `count` times.
 */
void TextDocument::gotoNextChar(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoPreviousWord(int count = 1)
 * Goes to the previous word, repeat the operation `count` times.
 */
void TextDocument::gotoPreviousWord(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoNextWord(int count = 1)
 * Goes to the next word, repeat the operation `count` times.
 */
void TextDocument::gotoNextWord(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoStartOfDocument()
 * Goes to the document start.
 */
void TextDocument::gotoStartOfDocument()
{
    LOG();
    movePosition(QTextCursor::Start);
}

/*!
 * \qmlmethod TextDocument::gotoEndOfDocument()
 * Goes to the document end.
 */
void TextDocument::gotoEndOfDocument()
{
    LOG();
    movePosition(QTextCursor::End);
}

/*!
 * \qmlmethod TextDocument::unselect()
 * Clears the current selection.
 */
void TextDocument::unselect()
{
    LOG();
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
    LOG();
    return m_document->textCursor().hasSelection();
}

/*!
 * \qmlmethod TextDocument::selectAll()
 * Selects all the text.
 */
void TextDocument::selectAll()
{
    LOG();
    m_document->selectAll();
}

/*!
 * \qmlmethod TextDocument::selectTo(int pos)
 * Selects the text from the current position to `pos`.
 */
void TextDocument::selectTo(int pos)
{
    LOG(LOG_ARG("pos", pos));
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
    LOG();
    movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectEndOfLine()
 * Selects the text from the current position to the end of the line.
 */
void TextDocument::selectEndOfLine()
{
    LOG();
    movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectStartOfWord()
 * Selects the text from the current position to the start of the word.
 */
void TextDocument::selectStartOfWord()
{
    LOG();
    movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectEndOfWord()
 * Selects the text from the current position to the end of the word.
 */
void TextDocument::selectEndOfWord()
{
    LOG();
    movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectNextLine(int count = 1)
 * Selects the next line, repeat the operation `count` times.
 */
void TextDocument::selectNextLine(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectPreviousLine(int count = 1)
 * Selects the previous line, repeat the operation `count` times.
 */
void TextDocument::selectPreviousLine(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::Up, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectPreviousChar(int count = 1)
 * Selects the previous character, repeat the operation `count` times.
 */
void TextDocument::selectPreviousChar(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectNextChar(int count = 1)
 * Selects the next character, repeat the operation `count` times.
 */
void TextDocument::selectNextChar(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectPreviousWord(int count = 1)
 * Selects the previous word, repeat the operation `count` times.
 */
void TextDocument::selectPreviousWord(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectNextWord(int count = 1)
 * Selects the next word, repeat the operation `count` times.
 */
void TextDocument::selectNextWord(int count)
{
    LOG_AND_MERGE(count);
    movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectRegion(int from, int to)
 * Selects the text between `from` and `to` positions.
 */
void TextDocument::selectRegion(int from, int to)
{
    LOG(from, to);
    QTextCursor cursor(m_document->document());
    cursor.setPosition(from, QTextCursor::MoveAnchor);
    cursor.setPosition(to, QTextCursor::KeepAnchor);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::selectRange(RangeMark range)
 * Selects the range passed in parameter.
 */
void TextDocument::selectRange(const RangeMark &range)
{
    LOG(range);
    if (range.document() != this) {
        spdlog::error("{}: Can't use a range mark from another editor.", FUNCTION_NAME);
        return;
    }
    range.select();
}

/*!
 * \qmlmethod TextDocument::copy()
 * Copies the selected text.
 */
void TextDocument::copy()
{
    LOG();
    m_document->copy();
}

/*!
 * \qmlmethod TextDocument::paste()
 * Pastes text in the clipboard.
 */
void TextDocument::paste()
{
    LOG();
    m_document->paste();
}

/*!
 * \qmlmethod TextDocument::cut()
 * Cuts the selected text.
 */
void TextDocument::cut()
{
    LOG();
    m_document->cut();
}

/*!
 * \qmlmethod TextDocument::remove(int length)
 * Remove `length` character from the current position.
 */
void TextDocument::remove(int length)
{
    LOG(length);
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
    LOG_AND_MERGE(LOG_ARG("text", text));
    m_document->insertPlainText(text);
}

/*!
 * \qmlmethod TextDocument::insertAtLine(string text, int line = -1)
 * Inserts the string `text` at `line`. If `line` is -1, insert the text at the current position. `line` is 1-based.
 */
void TextDocument::insertAtLine(const QString &text, int line)
{
    if (line == -1)
        LOG(LOG_ARG("text", text));
    else
        LOG(LOG_ARG("text", text), LOG_ARG("line", line));

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
 * \qmlmethod TextDocument::insertAtPosition(string text, int pos)
 * Inserts the string `text` at `pos`.
 */
void TextDocument::insertAtPosition(const QString &text, int pos)
{
    LOG(text, pos);
    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(pos);
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.insertText(text);
    cursor.endEditBlock();
}

/*!
 * \qmlmethod TextDocument::replace(int length, string text)
 * Replaces `length` characters from the current position with the string `text`.
 */
void TextDocument::replace(int length, const QString &text)
{
    LOG(length, text);
    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(cursor.position() + length, QTextCursor::KeepAnchor);
    cursor.insertText(text);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::replace(int from, int to, string text)
 * Replaces the text from `from` to `to` with the string `text`.
 */
void TextDocument::replace(int from, int to, const QString &text)
{
    LOG(from, to, text);
    QTextCursor cursor(m_document->document());
    cursor.setPosition(from);
    cursor.setPosition(to, QTextCursor::KeepAnchor);
    cursor.insertText(text);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::replace(RangeMark range, string text)
 * Replaces the text in the range `range` with the string `text`.
 */
void TextDocument::replace(const RangeMark &range, const QString &text)
{
    LOG(range, text);
    replace(range.start(), range.end(), text);
}

/*!
 * \qmlmethod TextDocument::deleteLine(int line = -1)
 * Remove a the line `line`. If `line` is -1, remove the current line. `line` is 1-based.
 */
void TextDocument::deleteLine(int line)
{
    if (line == -1)
        LOG();
    else
        LOG(LOG_ARG("line", line));

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
    LOG();
    m_document->textCursor().removeSelectedText();
}

/*!
 * \qmlmethod TextDocument::deleteRegion(int from, int to)
 * Deletes the text between `from` and `to` positions.
 */
void TextDocument::deleteRegion(int from, int to)
{
    LOG(from, to);
    QTextCursor cursor(m_document->document());
    cursor.setPosition(from);
    cursor.setPosition(to, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deleteRange(RangeMark range)
 * Deletes the range passed in parameter.
 */
void TextDocument::deleteRange(const RangeMark &range)
{
    LOG(range);
    if (range.document() != this) {
        spdlog::error("{}: Can't use a range mark from another editor.", FUNCTION_NAME);
        return;
    }
    range.remove();
}

/*!
 * \qmlmethod TextDocument::deleteEndOfLine()
 * Deletes from the cursor position to the end of the line.
 */
void TextDocument::deleteEndOfLine()
{
    LOG();
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
    LOG();
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
    LOG();
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
    LOG();
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
    LOG_AND_MERGE(count);
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
    LOG_AND_MERGE(count);
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, count);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod Mark TextDocument::createMark(int pos = -1)
 * Creates a mark at the given position `pos`. If `pos` is -1, it will create a mark at the
 * current position.
 * \sa Mark
 */
Mark TextDocument::createMark(int pos)
{
    LOG(LOG_ARG("pos", pos));
    if (pos < 0)
        pos = position();
    LOG_RETURN("mark", Mark(this, pos));
}

/*!
 * \qmlmethod TextDocument::gotoMark(Mark mark)
 * Goes to the given `mark`.
 */
void TextDocument::gotoMark(const Mark &mark)
{
    LOG(LOG_ARG("mark", mark));
    if (mark.document() != this) {
        spdlog::error("{}: Can't use a mark from another editor.", FUNCTION_NAME);
        return;
    }

    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(mark.position());
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::selectToMark(Mark mark)
 * Selects the text from the cursor position to the `mark`.
 */
void TextDocument::selectToMark(const Mark &mark)
{
    LOG(LOG_ARG("mark", mark));
    if (mark.document() != this) {
        spdlog::error("{}: Can't use a mark from another editor.", FUNCTION_NAME);
        return;
    }

    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(mark.position(), QTextCursor::KeepAnchor);
    m_document->setTextCursor(cursor);
}

/**
 * \qmlmethod RangeMark TextDocument::createRangeMark(int from, int to)
 *
 * Creates a range mark from `from` to `to`.
 * \sa RangeMark
 */
Core::RangeMark TextDocument::createRangeMark(int from, int to)
{
    LOG(LOG_ARG("start", from), LOG_ARG("end", to));
    LOG_RETURN("rangeMark", Core::RangeMark(this, from, to));
}

/**
 * \qmlmethod RangeMark TextDocument::createRangeMark()
 *
 * Creates a range mark from the current selection.
 *
 * Note: if there is no selection, the range mark will span an empty range!
 * \sa RangeMark
 */
Core::RangeMark TextDocument::createRangeMark()
{
    LOG();
    const auto cursor = m_document->textCursor();
    const int start = cursor.selectionStart();
    const int end = cursor.selectionEnd();

    if (start == end)
        spdlog::warn("{}: Creating a range mark with an empty range.", FUNCTION_NAME);

    LOG_RETURN("rangeMark", createRangeMark(start, end));
}

/*!
 * \qmlmethod bool TextDocument::find(string text, FindFlags options = TextDocument.NoFindFlags)
 * Searches the string `text` in the editor. Options could be a combination of:
 *
 * - `TextDocument.FindBackward`: search backward
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only complete words
 * - `TextDocument.FindRegexp`: use a regexp, equivalent to calling `findRegexp`
 *
 * Selects the match and returns `true` if a match is found.
 */
bool TextDocument::find(const QString &text, FindFlags options)
{
    LOG(LOG_ARG("text", text), options);
    if (options & FindRegexp)
        return findRegexp(text, options);
    else if (options & FindWholeWords)
        return findRegexp(QRegularExpression::escape(text), options);
    else
        return m_document->find(text, static_cast<QTextDocument::FindFlags>(static_cast<int>(options)));
}

/*!
 * \qmlmethod bool TextDocument::findRegexp(string regexp, FindFlags options = TextDocument.NoFindFlags)
 * Searches the string `regexp` in the editor using a regular expression. Options could be a combination of:
 *
 * - `TextDocument.FindBackward`: search backward
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only complete words
 *
 * Selects the match and returns `true` if a match is found.
 */
bool TextDocument::findRegexp(const QString &regexp, FindFlags options)
{
    LOG(LOG_ARG("text", regexp), options);

    const auto found = selectRegexpMatch(regexp, options);
    return found.has_value();
}

auto TextDocument::selectRegexpMatch(
    QString regexp, int options,
    const std::function<bool(const QRegularExpression &, const QRegularExpressionMatch &, const QTextCursor &)>
        &selectionFunction) -> std::optional<std::pair<QRegularExpressionMatch, QTextCursor>>
{
    unselect();

    if (options & FindWholeWords) {
        if (!regexp.startsWith("\\b"))
            regexp = "\\b" + regexp;
        if (!regexp.endsWith("\\b"))
            regexp += "\\b";
    }

    QRegularExpression expression(regexp);
    if (options & (TextDocument::FindCaseSensitively | TextDocument::PreserveCase))
        expression.setPatternOptions(expression.patternOptions() & ~QRegularExpression::CaseInsensitiveOption);
    else
        expression.setPatternOptions(expression.patternOptions() | QRegularExpression::CaseInsensitiveOption);

    const QTextCursor startCursor = m_document->textCursor();
    QTextBlock block = startCursor.block();
    int blockOffset = startCursor.positionInBlock();

    while (block.isValid()) {
        const auto found = matchInBlock(block, expression, blockOffset, options);
        if (found.has_value()) {
            const auto &[match, newCursor] = *found;
            if (selectionFunction(expression, match, newCursor)) {
                m_document->setTextCursor(newCursor);
                return found;
            }

            // The previous match has been discarded, continue searching
            blockOffset = newCursor.positionInBlock();
        } else {
            if (options & FindBackward) {
                block = block.previous();
                // Use `block.text().size()` here, instead of `block.length()`, as the later includes the trailing
                // newline.
                blockOffset = block.text().size();
            } else {
                block = block.next();
                blockOffset = 0;
            }
        }
    }

    return {};
}

/*!
 * \qmlmethod bool TextDocument::match(string regexp, FindFlags options = TextDocument.NoFindFlags)
 * Searches the string `regexp` in the editor using a regular expression. Options could be a combination of:
 *
 * - `TextDocument.FindBackward`: search backward
 * - `TextDocument.FindCaseSensitively`: match case
 * - `TextDocument.FindWholeWords`: match only complete words
 *
 * Selects the match and returns the named group if a match is found.
 */
QString TextDocument::match(const QString &regexp, FindFlags options)
{
    LOG(regexp, options);

    QString captureGroup;
    const auto result = selectRegexpMatch(
        regexp, options, [&captureGroup](const auto &expression, const auto &match, const auto &cursor) {
            Q_UNUSED(cursor)
            for (const auto &name : expression.namedCaptureGroups()) {
                if (!name.isEmpty() && match.hasCaptured(name)) {
                    captureGroup = name;
                    return true;
                }
            }
            return false;
        });

    LOG_RETURN("group", result.has_value() ? captureGroup : QString(""));
}

/*!
 * \qmlmethod bool TextDocument::replaceOne(string before, string after, FindFlags options = TextDocument.NoFindFlags)
 * Replaces one occurrence of the string `before` with `after`. Options could be a combination of:
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
bool TextDocument::replaceOne(const QString &before, const QString &after, FindFlags options)
{
    LOG(LOG_ARG("text", before), after, options);

    auto cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_document->setTextCursor(cursor);

    const bool usesRegExp = options & FindRegexp;
    const bool preserveCase = options & PreserveCase;

    const auto regexp = Utils::createRegularExpression(before, options, usesRegExp);
    if (find(before, options)) {
        cursor.beginEditBlock();
        const auto found = m_document->textCursor();
        cursor.setPosition(found.selectionStart());
        cursor.setPosition(found.selectionEnd(), QTextCursor::KeepAnchor);
        QString afterText = after;
        if (usesRegExp) {
            const QRegularExpressionMatch match = regexp.match(selectedText());
            afterText = Utils::expandRegExpReplacement(after, match.capturedTexts());
        } else if (preserveCase) {
            afterText = Utils::matchCaseReplacement(cursor.selectedText(), after);
        }
        cursor.insertText(afterText);
        cursor.endEditBlock();
        return true;
    }

    return false;
}

/*!
 * \qmlmethod bool TextDocument::replaceAll(string before, string after, FindFlags options = TextDocument.NoFindFlags)
 * Replaces all occurrences of the string `before` with `after`. Options could be a combination of:
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
int TextDocument::replaceAll(const QString &before, const QString &after, FindFlags options /* = NoFindFlags */)
{
    LOG(LOG_ARG("text", before), after, options);
    return replaceAll(before, after, options, [](auto) {
        return true;
    });
}

// clang-format off
/*!
 * \qmlmethod bool TextDocument::replaceAllInRange(string before, string after, RangeMark range, FindFlags options = TextDocument.NoFindFlags)
 * Replaces all occurrences of the string `before` with `after` in the given `range`. See the
 * options from `replaceAll`.
 *
 * Returns the number of changes done in the document.
 */
// clang-format on
int TextDocument::replaceAllInRange(const QString &before, const QString &after, const Core::RangeMark &range,
                                    FindFlags options)
{
    LOG(LOG_ARG("text", before), after, range, options);
    if (!range.isValid()) {
        spdlog::warn("{}: Invalid range!", FUNCTION_NAME);
        return 0;
    }
    if (range.document() != this) {
        spdlog::warn("{}: Range is not from this document!", FUNCTION_NAME);
        return 0;
    }

    return replaceAll(before, after, options, [&range](auto cursor) {
        return range.start() <= cursor.selectionStart()
            && cursor.selectionEnd()
            <= range.end(); // Use <= here, as the selection may be equal to the range, both values are exclusive.
    });
}

int TextDocument::replaceAll(const QString &before, const QString &after, FindFlags options,
                             const std::function<bool(QTextCursor)> &filterAcceptsCursor)
{
    const bool backwards = options & FindBackward;
    const bool usesRegExp = options & FindRegexp;
    const bool preserveCase = options & PreserveCase;

    int count = 0;
    auto cursor = m_document->textCursor();
    cursor.movePosition(backwards ? QTextCursor::End : QTextCursor::Start);
    m_document->setTextCursor(cursor);
    cursor.beginEditBlock();

    const auto regexp = Utils::createRegularExpression(before, options, usesRegExp);
    while (find(before, options)) {
        const auto found = m_document->textCursor();
        cursor.setPosition(found.selectionStart());
        cursor.setPosition(found.selectionEnd(), QTextCursor::KeepAnchor);
        if (!filterAcceptsCursor(cursor)) {
            // Result filtered, so do not replace.
            continue;
        }
        QString afterText = after;
        if (usesRegExp) {
            QRegularExpressionMatch match = regexp.match(selectedText());
            afterText = Utils::expandRegExpReplacement(after, match.capturedTexts());
        } else if (preserveCase) {
            afterText = Utils::matchCaseReplacement(cursor.selectedText(), after);
        }
        cursor.insertText(afterText);
        ++count;
    }

    cursor.endEditBlock();
    return count;
}

// clang-format off
/*!
 * \qmlmethod bool TextDocument::replaceAllRegexp(string regexp, string after, FindFlags options = TextDocument.NoFindFlags)
 * Replaces all occurrences of the matches for the `regexp` with `after`. See the options from
 * `replaceAll`.
 *
 * The captures coming from the regexp can be used in the replacement text, using `\1`..`\n` or `$1`..`$n`.
 *
 * Returns the number of changes done in the document.
 */
// clang-format on
int TextDocument::replaceAllRegexp(const QString &regexp, const QString &after, FindFlags options /* = NoFindFlags */)
{
    LOG(LOG_ARG("text", regexp), after, options);
    return replaceAllRegexp(regexp, after, options, [](auto) {
        return true;
    });
}

// clang-format off
/*!
 * \qmlmethod bool TextDocument::replaceAllRegexpInRange(string regexp, string after, RangeMark range, FindFlags options = TextDocument.NoFindFlags)
 * Replaces all occurrences of the matches for the `regexp` with `after` in the given `range`. See the options from `replaceAll`.
 *
 * The captures coming from the regexp can be used in the replacement text, using `\1`..`\n` or `$1`..`$n`.
 *
 * Returns the number of changes done in the document.
 */
// clang-format on
int TextDocument::replaceAllRegexpInRange(const QString &regexp, const QString &after, const RangeMark &range,
                                          FindFlags options /* = NoFindFlags*/)
{
    LOG(LOG_ARG("text", regexp), after, range, options);
    if (!range.isValid()) {
        spdlog::warn("{}: Invalid range!", FUNCTION_NAME);
        return 0;
    }
    if (range.document() != this) {
        spdlog::warn("{}: Range is not from this document!", FUNCTION_NAME);
        return 0;
    }

    return replaceAllRegexp(regexp, after, options, [&range](auto cursor) {
        return range.start() <= cursor.selectionStart()
            && cursor.selectionEnd()
            <= range.end(); // Use <= here, as the selection may be equal to the range, both values are exclusive.
    });
}

int TextDocument::replaceAllRegexp(const QString &regexp, const QString &after, FindFlags options,
                                   const std::function<bool(QTextCursor)> &filterAcceptsCursor)
{
    return replaceAll(regexp, after, options | FindRegexp, filterAcceptsCursor);
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

static QString indentToString(int indentSize, const TabSettings &settings)
{
    return settings.insertSpaces ? QString(indentSize * settings.tabSize, ' ') : QString(indentSize, '\t');
}

static int indentOneLine(QTextCursor &cursor, int tabCount, const TabSettings &settings, bool relative)
{
    QString text = cursor.selectedText();
    cursor.removeSelectedText();

    const int oldSize = text.size();
    const int firstChar = firstNonSpace(text);
    const int startColumn = columnAt(text, firstChar, settings.tabSize);
    const int currentIndent = startColumn / settings.tabSize;
    const int indentSize = qMax(relative ? (currentIndent + tabCount) : tabCount, 0);

    text.remove(0, firstChar);
    text = indentToString(indentSize, settings) + text;

    cursor.insertText(text);
    return text.size() - oldSize;
}

static void indentBlocksInTextEdit(QPlainTextEdit *textEdit, int blockStart, int blockEnd, int tabCount, bool relative)
{
    const auto settings = DEFAULT_VALUE(Core::TabSettings, Tab);
    QTextCursor cursor = textEdit->textCursor();

    // Make sure we don't move the cursor outside the first line it started on.
    const int minStart = textEdit->document()->findBlock(cursor.selectionStart()).position();
    int newStart = cursor.selectionStart();
    int newEnd = cursor.selectionEnd();

    // Move the position to the beginning of the first line
    cursor.setPosition(textEdit->document()->findBlockByNumber(blockStart).position());

    cursor.beginEditBlock();
    // Iterate through all line, and change the indentation
    for (int block = blockStart; block <= blockEnd; ++block) {
        cursor.select(QTextCursor::LineUnderCursor);
        // We need to update the new selection if we're doing modifications at or before the selection.
        const auto updateStart = cursor.selectionStart() <= newStart;
        const auto updateEnd = cursor.selectionStart() <= newEnd;

        const int delta = indentOneLine(cursor, tabCount, settings, relative);

        // update the position of the selection, depending on how much text was added/removed
        if (updateStart) {
            newStart += delta;
        }
        if (updateEnd) {
            newEnd += delta;
        }
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
    }
    cursor.endEditBlock();

    // Restore the selection, adjusted for the inserted/removed indentation
    cursor.setPosition(qMax(minStart, newStart));
    cursor.setPosition(qMax(minStart, newEnd), QTextCursor::KeepAnchor);

    textEdit->setTextCursor(cursor);
}

void indentTextInTextEdit(QPlainTextEdit *textEdit, int tabCount, bool relative)
{
    QTextCursor cursor = textEdit->textCursor();
    const int blockStart = textEdit->document()->findBlock(cursor.selectionStart()).blockNumber();
    const int blockEnd = textEdit->document()->findBlock(cursor.selectionEnd()).blockNumber();

    indentBlocksInTextEdit(textEdit, blockStart, blockEnd, tabCount, relative);
}

/*!
 * \qmlmethod TextDocument::indent(int count)
 * Indents the current line `count` times. If there's a selection, indent all lines in the selection.
 *
 * The `count` can be negative to reduce the existing indentation.
 *
 * See also: [`setIndentation`](#setIndentation).
 */
void TextDocument::indent(int count)
{
    LOG_AND_MERGE(count);
    indentTextInTextEdit(m_document, count);
}

/*!
 * \qmlmethod TextDocument::indentLine(int count, int line)
 * Indents the `line` `count` times.
 *
 * See also: [`indent`](#indent)
 */
void TextDocument::indentLine(int count, int line)
{
    LOG(LOG_ARG("count", count), LOG_ARG("line", line));

    indentBlocksInTextEdit(m_document, line - 1, line - 1, count, true);
}

/*!
 * \qmlmethod TextDocument::setIndentation(int indent)
 * Sets the absolute indentation of the current line to `indent` indentations.
 * If there's a selection, sets the indentation of all lines in the selection.
 *
 * For relative indentation, see [`indent`](#indent) and [`indentLine`](#indentLine).
 */
void TextDocument::setIndentation(int indent)
{
    LOG(LOG_ARG("indent", indent));

    indentTextInTextEdit(m_document, indent, false);
}

/*!
 * \qmlmethod TextDocument::setIndentationAtLine(int indent, int line)
 * Sets the absolute indentation of the `line` to `indent` indentations.
 *
 * See also: [`setIndentation`](#setIndentation)
 */
void TextDocument::setIndentationAtLine(int indent, int line)
{
    LOG(LOG_ARG("indent", indent), LOG_ARG("line", line));

    indentBlocksInTextEdit(m_document, line - 1, line - 1, indent, false);
}

void TextDocument::setLineEnding(LineEnding newLineEnding)
{
    LOG(newLineEnding);
    if (m_lineEnding == newLineEnding)
        return;
    setHasChanged(true);
    m_lineEnding = newLineEnding;
    emit lineEndingChanged();
}

/*!
 * \qmlmethod string TextDocument::indentTextAtPosition(int pos)
 * Returns the indentation text at the given position.
 *
 * Note: To get the level of indentation, use [`indentationAtPosition`](#indentationAtPosition).
 */
QString TextDocument::indentTextAtPosition(int pos) const
{
    LOG(LOG_ARG("position", pos));

    auto cursor = m_document->textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::StartOfLine);
    const QString line = cursor.block().text();
    static QRegularExpression nonSpaces("\\S");
    return line.left(line.indexOf(nonSpaces));
}

/*!
 * \qmlmethod string TextDocument::indentTextAtLine(int line = -1)
 * Returns the indentation text at the given line.
 *
 * If `line` is -1 it will return the indentation at the current line.
 * If `line` is larger than the number of lines in the document, it will return an empty string
 *
 * Note: To get the level of indentation, use [`indentationAtLine`](#indentationAtLine).
 */
QString TextDocument::indentTextAtLine(int line /* = -1 */) const
{
    LOG(LOG_ARG("line", line));

    if (line <= 0) {
        line = this->line();
    }

    // API-wise the line numbers are 1-based, but internally they are 0-based
    auto blockNumber = line - 1;

    const QTextBlock &block = m_document->document()->findBlockByNumber(blockNumber);
    if (block.isValid()) {
        return indentTextAtPosition(block.position());
    }
    return 0;
}

/*!
 * \qmlmethod int TextDocument::indentationAtPosition(int pos)
 * Returns the indentation level at the given position.
 */
int TextDocument::indentationAtPosition(int pos) const
{
    LOG(LOG_ARG("position", pos));

    const auto indentText = indentTextAtPosition(pos);
    const auto settings = DEFAULT_VALUE(Core::TabSettings, Tab);
    return columnAt(indentText, indentText.size(), settings.tabSize) / settings.tabSize;
}

/*!
 * \qmlmethod int TextDocument::indentationAtLine(int line = -1)
 * Returns the indentation level at the given line.
 *
 * If `line` is -1 it will return the indentation at the current line.
 * If `line` is larger than the number of lines in the document, it will return 0
 */
int TextDocument::indentationAtLine(int line /* = -1 */) const
{
    LOG(LOG_ARG("line", line));

    const auto indentText = indentTextAtLine(line);
    const auto settings = DEFAULT_VALUE(Core::TabSettings, Tab);
    return columnAt(indentText, indentText.size(), settings.tabSize) / settings.tabSize;
}

} // namespace Core
