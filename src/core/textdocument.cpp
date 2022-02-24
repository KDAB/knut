#include "textdocument.h"

#include "mark.h"
#include "string_utils.h"

#include <QFile>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextStream>

#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype TextRange
 * \brief Define a range of text in a text document
 * \inqmlmodule Script
 * \since 4.0
 * \sa TextDocument
 */
/*!
 * \qmlproperty int TextRange::start
 * Start position of the range.
 */
/*!
 * \qmlproperty int TextRange::end
 * End position of the range.
 */

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
            gotoNextCharacter();
        else if (keyEvent == QKeySequence::MoveToPreviousChar)
            gotoPreviousCharacter();
        else if (keyEvent == QKeySequence::SelectNextChar)
            selectNextCharacter();
        else if (keyEvent == QKeySequence::SelectPreviousChar)
            selectPreviousCharacter();
        else if (keyEvent == QKeySequence::SelectNextWord)
            selectNextWord();
        else if (keyEvent == QKeySequence::SelectPreviousWord)
            selectPreviousWord();
        else if (keyEvent == QKeySequence::SelectStartOfLine)
            selectLineStart();
        else if (keyEvent == QKeySequence::SelectEndOfLine)
            selectLineEnd();
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
            gotoLineStart();
        else if (keyEvent == QKeySequence::MoveToEndOfLine)
            gotoLineEnd();
        else if (keyEvent == QKeySequence::MoveToStartOfDocument)
            gotoDocumentStart();
        else if (keyEvent == QKeySequence::MoveToEndOfDocument)
            gotoDocumentEnd();
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
        else if (keyEvent == QKeySequence::DeleteEndOfWord)
            deleteEndOfWord();
        else if (keyEvent == QKeySequence::DeleteStartOfWord)
            deleteStartOfWord();
        else if (keyEvent == QKeySequence::DeleteEndOfLine)
            deleteEndOfLine();
        else if (!keyEvent->text().isEmpty())
            insert(keyEvent->text());

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
    const QTextCursor cursor = m_document->textCursor();
    return cursor.positionInBlock() + 1;
}

int TextDocument::line() const
{
    const QTextCursor cursor = m_document->textCursor();
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
    spdlog::trace("TextDocument::setPosition {}", newPosition);

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

QString TextDocument::text() const
{
    return m_document->toPlainText();
}

void TextDocument::setText(const QString &newText)
{
    spdlog::trace("TextDocument::setText");

    m_document->setPlainText(newText);
}

QString TextDocument::currentLine() const
{
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    return cursor.selectedText();
}

QString TextDocument::currentWord() const
{
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::StartOfWord);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    return cursor.selectedText();
}

QString TextDocument::selectedText() const
{
    // Replace \u2029 with \n
    return m_document->textCursor().selectedText().replace(QChar(8233), "\n");
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

/*!
 * \qmlmethod TextDocument::undo()
 * Undo the last action.
 */
void TextDocument::undo()
{
    spdlog::trace("TextDocument::undo");
    m_document->undo();
}

/*!
 * \qmlmethod TextDocument::redo()
 * Redo the last action.
 */
void TextDocument::redo()
{
    spdlog::trace("TextDocument::redo");
    m_document->redo();
}

void TextDocument::movePosition(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode, int count)
{
    for (int i = 0; i < count; ++i) {
        m_document->moveCursor(operation, mode);
        m_document->setTextCursor(m_document->textCursor());
    }
}

/*!
 * \qmlmethod TextDocument::gotoLine( int line, int column = 1)
 * Goes to the given `line` and `column` in the editor. Lines and columns are 1-based.
 * \sa TextDocument::line
 * \sa TextDocument::column
 */
void TextDocument::gotoLine(int line, int column)
{
    spdlog::trace("TextDocument::gotoLine {} col:{}", line, column);

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
 * \qmlmethod TextDocument::gotoLineStart()
 * Goes to the start of the line.
 */
void TextDocument::gotoLineStart()
{
    spdlog::trace("TextDocument::gotoLineStart");
    movePosition(QTextCursor::StartOfLine);
}

/*!
 * \qmlmethod TextDocument::gotoLineEnd()
 * Goes to the end of the line.
 */
void TextDocument::gotoLineEnd()
{
    spdlog::trace("TextDocument::gotoLineEnd");
    movePosition(QTextCursor::EndOfLine);
}

/*!
 * \qmlmethod TextDocument::gotoWordStart()
 * Goes to the start of the word under the cursor.
 */
void TextDocument::gotoWordStart()
{
    spdlog::trace("TextDocument::gotoWordStart");
    movePosition(QTextCursor::StartOfWord);
}

/*!
 * \qmlmethod TextDocument::gotoWordEnd()
 * Goes to the end of the word under the cursor.
 */
void TextDocument::gotoWordEnd()
{
    spdlog::trace("TextDocument::gotoWordEnd");
    movePosition(QTextCursor::EndOfWord);
}

/*!
 * \qmlmethod TextDocument::gotoNextLine( int count = 1)
 * Goes to the next line, repeat the operation `count` times.
 */
void TextDocument::gotoNextLine(int count)
{
    spdlog::trace("TextDocument::gotoNextLine");
    movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoPreviousLine( int count = 1)
 * Goes to the previous line, repeat the operation `count` times.
 */
void TextDocument::gotoPreviousLine(int count)
{
    spdlog::trace("TextDocument::gotoPreviousLine");
    movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoPreviousCharacter( int count = 1)
 * Goes to the previous character, repeat the operation `count` times.
 */
void TextDocument::gotoPreviousCharacter(int count)
{
    spdlog::trace("TextDocument::gotoPreviousCharacter");
    movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoNextCharacter( int count = 1)
 * Goes to the next character, repeat the operation `count` times.
 */
void TextDocument::gotoNextCharacter(int count)
{
    spdlog::trace("TextDocument::gotoNextCharacter");
    movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoPreviousWord( int count = 1)
 * Goes to the previous word, repeat the operation `count` times.
 */
void TextDocument::gotoPreviousWord(int count)
{
    spdlog::trace("TextDocument::gotoPreviousWord");
    movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoNextWord( int count = 1)
 * Goes to the next word, repeat the operation `count` times.
 */
void TextDocument::gotoNextWord(int count)
{
    spdlog::trace("TextDocument::gotoNextWord");
    movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor, count);
}

/*!
 * \qmlmethod TextDocument::gotoDocumentStart()
 * Goes to the document start.
 */
void TextDocument::gotoDocumentStart()
{
    spdlog::trace("TextDocument::gotoDocumentStart");
    movePosition(QTextCursor::Start);
}

/*!
 * \qmlmethod TextDocument::gotoDocumentEnd()
 * Goes to the document end.
 */
void TextDocument::gotoDocumentEnd()
{
    spdlog::trace("TextDocument::gotoDocumentEnd");
    movePosition(QTextCursor::End);
}

/*!
 * \qmlmethod TextDocument::unselect()
 * Clears the current selection.
 */
void TextDocument::unselect()
{
    spdlog::trace("TextDocument::unselect");
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
    spdlog::trace("TextDocument::hasSelection");
    return m_document->textCursor().hasSelection();
}

/*!
 * \qmlmethod TextDocument::selectAll()
 * Selects all the text.
 */
void TextDocument::selectAll()
{
    spdlog::trace("TextDocument::selectAll");
    m_document->selectAll();
}

/*!
 * \qmlmethod TextDocument::selectTo( int pos)
 * Selects the text from the current position to `pos`.
 */
void TextDocument::selectTo(int pos)
{
    spdlog::trace("TextDocument::selectTo {}", pos);
    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(pos, QTextCursor::KeepAnchor);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::selectLineStart( int count = 1)
 * Selects the text from the current position to the start of the line.
 */
void TextDocument::selectLineStart()
{
    spdlog::trace("TextDocument::selectLineStart");
    movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectLineEnd()
 * Selects the text from the current position to the end of the line.
 */
void TextDocument::selectLineEnd()
{
    spdlog::trace("TextDocument::selectLineEnd");
    movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectWordStart()
 * Selects the text from the current position to the start of the word.
 */
void TextDocument::selectWordStart()
{
    spdlog::trace("TextDocument::selectWordStart");
    movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectWordEnd()
 * Selects the text from the current position to the end of the word.
 */
void TextDocument::selectWordEnd()
{
    spdlog::trace("TextDocument::selectWordEnd");
    movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
}

/*!
 * \qmlmethod TextDocument::selectNextLine( int count = 1)
 * Selects the next line, repeat the operation `count` times.
 */
void TextDocument::selectNextLine(int count)
{
    spdlog::trace("TextDocument::selectNextLine {}", count);
    movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectPreviousLine( int count = 1)
 * Selectes the previous line, repeat the operation `count` times.
 */
void TextDocument::selectPreviousLine(int count)
{
    spdlog::trace("TextDocument::selectPreviousLine {}", count);
    movePosition(QTextCursor::Up, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectPreviousCharacter( int count = 1)
 * Selects the previous character, repeat the operation `count` times.
 */
void TextDocument::selectPreviousCharacter(int count)
{
    spdlog::trace("TextDocument::selectPreviousCharacter {}", count);
    movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectNextCharacter( int count = 1)
 * Selects the next character, repeat the operation `count` times.
 */
void TextDocument::selectNextCharacter(int count)
{
    spdlog::trace("TextDocument::selectNextCharacter {}", count);
    movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectPreviousWord( int count = 1)
 * Selects the previous word, repeat the operation `count` times.
 */
void TextDocument::selectPreviousWord(int count)
{
    spdlog::trace("TextDocument::selectPreviousWord {}", count);
    movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectNextWord( int count = 1)
 * Selects the next word, repeat the operation `count` times.
 */
void TextDocument::selectNextWord(int count)
{
    spdlog::trace("TextDocument::selectNextWord {}", count);
    movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor, count);
}

/*!
 * \qmlmethod TextDocument::selectRange( TextRange range)
 * Selects the range passed in parameter.
 */
void TextDocument::selectRange(const TextRange &range)
{
    spdlog::trace("TextDocument::selectRange {} - {}", range.start, range.end);
    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(range.start, QTextCursor::MoveAnchor);
    cursor.setPosition(range.end, QTextCursor::KeepAnchor);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::copy()
 * Copies the selected text.
 */
void TextDocument::copy()
{
    spdlog::trace("TextDocument::copy");
    m_document->copy();
}

/*!
 * \qmlmethod TextDocument::paste()
 * Pastes text in the clipboard.
 */
void TextDocument::paste()
{
    spdlog::trace("TextDocument::paste");
    m_document->paste();
}

/*!
 * \qmlmethod TextDocument::cut()
 * Cuts the selected text.
 */
void TextDocument::cut()
{
    spdlog::trace("TextDocument::cut");
    m_document->cut();
}

/*!
 * \qmlmethod TextEditor::remove( int length)
 * Remove `length` character from the current position.
 */
void TextDocument::remove(int length)
{
    spdlog::trace("TextDocument::remove {}", length);
    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(cursor.position() + length, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextEditor::insert( string text)
 * Inserts the string `text` at the current position. If some text is selected it will be replaced.
 */
void TextDocument::insert(const QString &text)
{
    spdlog::trace("TextDocument::insert {}", text.toStdString());
    m_document->insertPlainText(text);
}

/*!
 * \qmlmethod TextEditor::replace( int length, string text)
 * Replaces `length` characters from the current position with the string `text`.
 */
void TextDocument::replace(int length, const QString &text)
{
    spdlog::trace("TextDocument::replace {} chars by {}", length, text.toStdString());
    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(cursor.position() + length, QTextCursor::KeepAnchor);
    cursor.insertText(text);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deleteSelection()
 * Deletes the current selection, does nothing if no text is selected.
 */
void TextDocument::deleteSelection()
{
    spdlog::trace("TextDocument::deleteSelection");
    m_document->textCursor().removeSelectedText();
}

/*!
 * \qmlmethod TextDocument::deleteRegion( int from, int to)
 * Deletes the text between `from` and `to` positions.
 */
void TextDocument::deleteRegion(int from, int to)
{
    spdlog::trace("TextDocument::deleteRegion from {} to {}", from, to);
    QTextCursor cursor(m_document->document());
    cursor.setPosition(from);
    cursor.setPosition(to, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::deleteEndOfLine()
 * Deletes from the cursor position to the end of the line.
 */
void TextDocument::deleteEndOfLine()
{
    spdlog::trace("TextDocument::deleteEndOfLine");
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
    spdlog::trace("TextDocument::deleteStartOfLine");
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
    spdlog::trace("TextDocument::deleteEndOfWord");
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
    spdlog::trace("TextDocument::deleteStartOfWord");
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
    spdlog::trace("TextDocument::deletePreviousCharacter {}", count);
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
    spdlog::trace("TextDocument::deleteNextCharacter {}", count);
    QTextCursor cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, count);
    cursor.removeSelectedText();
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod Mark TextDocument::createMark( int pos = -1)
 * Create a mark at the given position `pos`. If `pos` is -1, it will create a mark at the
 * current position.
 * \sa Mark
 */
Mark *TextDocument::createMark(int pos)
{
    spdlog::trace("TextDocument::createMark {}", pos);
    if (pos == -1)
        pos = position();
    return new Mark(this, pos);
}

/*!
 * \qmlmethod TextDocument::gotoMark( Mark mark)
 * Go to the given `mark`.
 */
void TextDocument::gotoMark(Mark *mark)
{
    spdlog::trace("TextDocument::gotoMark {}", mark->position());
    if (mark->m_editor != this) {
        spdlog::error("Can't use a mark from another editor.");
        return;
    }

    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(mark->position());
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod TextDocument::selectToMark( Mark mark)
 * Select the text from the cursor position to the `mark`.
 */
void TextDocument::selectToMark(Mark *mark)
{
    spdlog::trace("TextDocument::selectToMark {}", mark->position());
    if (mark->m_editor != this) {
        spdlog::error("Can't use a mark from another editor.");
        return;
    }

    QTextCursor cursor = m_document->textCursor();
    cursor.setPosition(mark->position(), QTextCursor::KeepAnchor);
    m_document->setTextCursor(cursor);
}

/*!
 * \qmlmethod bool TextDocument::find( string text, int options = NoFindFlags)
 * Searches the string `text` in the editor. Options could be a combination of:
 *
 * - `TextEditor.FindBackward`: search backward
 * - `TextEditor.FindCaseSensitively`: match case
 * - `TextEditor.FindWholeWords`: match only complete words
 * - `TextEditor.FindRegexp`: use a regexp, equivalent to calling `findRegexp`
 *
 * Selects the match and returns `true` if a match is found.
 */
bool TextDocument::find(const QString &text, int options)
{
    spdlog::trace("TextDocument::find {} - {}", text.toStdString(), options);
    if (options & FindRegexp)
        return findRegexp(text, options);
    else
        return m_document->find(text, static_cast<QTextDocument::FindFlags>(options));
}

static QRegularExpression createRegularExpression(const QString &txt, int flags)
{
    return QRegularExpression((flags & TextDocument::FindRegexp) ? txt : QRegularExpression::escape(txt),
                              (flags & TextDocument::PreserveCase) ? QRegularExpression::NoPatternOption
                                                                   : QRegularExpression::CaseInsensitiveOption);
}

/*!
 * \qmlmethod bool TextDocument::findRegexp( string regexp, int options = NoFindFlags)
 * Searches the string `regexp` in the editor using a regular expression. Options could be a combination of:
 *
 * - `TextEditor.FindBackward`: search backward
 * - `TextEditor.FindCaseSensitively`: match case
 * - `TextEditor.FindWholeWords`: match only complete words
 *
 * Selects the match and returns `true` if a match is found.
 */
bool TextDocument::findRegexp(const QString &regexp, int options)
{
    spdlog::trace("TextDocument::findRegexp {} - {}", regexp.toStdString(), options);
    auto regularExpression = createRegularExpression(regexp, options | TextDocument::FindRegexp);
    return m_document->find(regularExpression, static_cast<QTextDocument::FindFlags>(options));
}

/*!
 * \qmlmethod bool TextDocument::replaceAll( string before, string after, int options = NoFindFlags)
 * Replace all occurences of the string `before` with `after`. Options could be a combination of:
 *
 * - `TextEditor.FindCaseSensitively`: match case
 * - `TextEditor.FindWholeWords`: match only complete words
 * - `TextEditor.FindRegexp`: use a regexp, equivalent to calling `findRegexp`
 * - `TextEditor.PreserveCase`: preserve case when replacing
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
    spdlog::trace("TextDocument::replaceAll {} by {} - {}", before.toStdString(), after.toStdString(), options);

    int count = 0;
    auto cursor = m_document->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_document->setTextCursor(cursor);
    cursor.beginEditBlock();

    const bool usesRegExp = options & FindRegexp;
    const bool preserveCase = options & PreserveCase;

    auto regexp = createRegularExpression(before, options);

    while (m_document->find(regexp, static_cast<QTextDocument::FindFlags>(options))) {
        auto found = m_document->textCursor();
        cursor.setPosition(found.selectionStart());
        cursor.setPosition(found.selectionEnd(), QTextCursor::KeepAnchor);
        QString afterText = after;
        if (usesRegExp) {
            QRegularExpressionMatch match = regexp.match(found.selectedText());
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
 * \qmlmethod bool TextDocument::replaceAllRegexp( string regexp, string after, int options = NoFindFlags)
 * Replace all occurences of the matches for the `regexp` with `after`. See the options from `replaceAll`.
 *
 * The captures coming from the regexp can be used in the replacement text, using `\1`..`\n` or `$1`..`$n`.
 *
 * Returns the number of changes done in the document.
 */
int TextDocument::replaceAllRegexp(const QString &regexp, const QString &after, int options)
{
    spdlog::trace("TextDocument::replaceAllRegexp {} by {} - {}", regexp.toStdString(), after.toStdString(), options);
    return replaceAll(regexp, after, options | FindRegexp);
}

void TextDocument::setLineEnding(LineEnding newLineEnding)
{
    if (m_lineEnding == newLineEnding)
        return;
    setHasChanged(true);
    m_lineEnding = newLineEnding;
    emit lineEndingChanged();
}

} // namespace Core
