#pragma once

#include "document.h"

#include <QPointer>
#include <QTextCursor>
#include <QTextDocument>

#include <memory>

class QPlainTextEdit;

namespace Lsp {
class Client;
}

namespace Core {

class Mark;

class TextDocument : public Document
{
    Q_OBJECT
    Q_PROPERTY(int column READ column)
    Q_PROPERTY(int line READ line)
    Q_PROPERTY(int lineCount READ lineCount)
    Q_PROPERTY(int position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString selectedText READ selectedText NOTIFY selectionChanged)
    Q_PROPERTY(QString currentLine READ currentLine)
    Q_PROPERTY(QString currentWord READ currentWord)
    Q_PROPERTY(LineEnding lineEnding READ lineEnding WRITE setLineEnding NOTIFY lineEndingChanged)

public:
    enum LineEnding {
        LFLineEnding,
        CRLFLineEnding,
        NativeLineEnding =
#if defined(Q_OS_WIN)
            CRLFLineEnding,
#else
            LFLineEnding,
#endif
    };
    Q_ENUM(LineEnding)

    enum FindFlag {
        NoFindFlags = 0x0,
        FindBackward = QTextDocument::FindBackward,
        FindCaseSensitively = QTextDocument::FindCaseSensitively,
        FindWholeWords = QTextDocument::FindWholeWords,
        FindRegexp = 0x08,
        PreserveCase = 0x10
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    explicit TextDocument(QObject *parent = nullptr);
    ~TextDocument();

    int column() const;
    int line() const;
    int lineCount() const;

    int position() const;
    void setPosition(int newPosition);
    void convertPosition(int pos, int *line, int *column) const;

    QString text() const;
    void setText(const QString &newText);
    QString currentLine() const;
    QString currentWord() const;

    QString selectedText() const;

    LineEnding lineEnding() const;
    void setLineEnding(LineEnding newLineEnding);

    bool hasUtf8Bom() const;

    QPlainTextEdit *textEdit() const;

    void setLspClient(Lsp::Client *client);

public slots:
    void undo();
    void redo();

    // Goto methods, to move around the document
    void gotoLine(int line, int column = 1);
    void gotoLineStart();
    void gotoLineEnd();
    void gotoWordStart();
    void gotoWordEnd();
    void gotoPreviousLine(int count = 1);
    void gotoNextLine(int count = 1);
    void gotoPreviousCharacter(int count = 1);
    void gotoNextCharacter(int count = 1);
    void gotoPreviousWord(int count = 1);
    void gotoNextWord(int count = 1);
    void gotoDocumentStart();
    void gotoDocumentEnd();

    // Selection methods
    void unselect();
    bool hasSelection();
    void selectAll();

    void selectTo(int pos);
    void selectLineStart();
    void selectLineEnd();
    void selectWordStart();
    void selectWordEnd();
    void selectPreviousLine(int count = 1);
    void selectNextLine(int count = 1);
    void selectPreviousCharacter(int count = 1);
    void selectNextCharacter(int count = 1);
    void selectPreviousWord(int count = 1);
    void selectNextWord(int count = 1);

    // Copy & paste
    void copy();
    void paste();
    void cut();

    // Text handling
    void remove(int length);
    void insert(const QString &text);
    void replace(int length, const QString &text);

    // Deletion
    void deleteSelection();
    void deleteRegion(int from, int to);
    void deleteEndOfLine();
    void deleteStartOfLine();
    void deleteEndOfWord();
    void deleteStartOfWord();
    void deletePreviousCharacter(int count);
    void deleteNextCharacter(int count);

    // Mark
    Core::Mark *createMark(int pos = -1);
    void gotoMark(Core::Mark *mark);
    void selectToMark(Core::Mark *mark);

    // Find
    bool find(const QString &text, int options = NoFindFlags);
    bool findRegexp(const QString &regexp, int options = NoFindFlags);

    // Replace
    int replaceAll(const QString &searchText, const QString &replaceText, int options = NoFindFlags);

signals:
    void positionChanged();
    void textChanged();
    void selectionChanged();
    void lineEndingChanged();

protected:
    explicit TextDocument(Type type, QObject *parent = nullptr);

    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

    void didOpen() override;
    void didClose() override;

    void detectFormat(const QByteArray &data);

    void movePosition(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor,
                      int count = 1);

    Lsp::Client *client() const;

private:
    friend Mark;

    // TODO: use a QTextDocument maybe, to avoid creating a widget
    // The QPlainTextEdit has a nicer API, so it's slightly easier with that now
    QPointer<QPlainTextEdit> m_document;
    LineEnding m_lineEnding = NativeLineEnding;
    bool m_utf8Bom = false;
    QPointer<Lsp::Client> m_lspClient;
};

} // namespace Core

Q_DECLARE_OPERATORS_FOR_FLAGS(Core::TextDocument::FindFlags)
