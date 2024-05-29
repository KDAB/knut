/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "document.h"
#include "mark.h"
#include "textrange.h"

#include <QPointer>
#include <QRegularExpressionMatch>
#include <QTextCursor>
#include <QTextDocument>
#include <memory>

class QPlainTextEdit;

namespace Core {

class RangeMark;

class TextDocument : public Document
{
    Q_OBJECT
    Q_PROPERTY(int column READ column NOTIFY positionChanged)
    Q_PROPERTY(int line READ line NOTIFY positionChanged)
    Q_PROPERTY(int lineCount READ lineCount NOTIFY textChanged)
    Q_PROPERTY(int position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart NOTIFY selectionChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd NOTIFY selectionChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString selectedText READ selectedText NOTIFY selectionChanged)
    Q_PROPERTY(QString currentLine READ currentLine NOTIFY positionChanged)
    Q_PROPERTY(QString currentWord READ currentWord NOTIFY positionChanged)
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
    Q_ENUM(FindFlag)

    explicit TextDocument(QObject *parent = nullptr);
    ~TextDocument() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

    int column() const;
    int line() const;
    int lineCount() const;

    int position() const;
    int selectionStart() const;
    int selectionEnd() const;

    QString text() const;
    QString currentLine() const;
    QString currentWord() const;

    QString selectedText() const;

    LineEnding lineEnding() const;

    bool hasUtf8Bom() const;

    QPlainTextEdit *textEdit() const;

    QString tab() const;

public slots:
    void setPosition(int newPosition);
    void setText(const QString &newText);
    void setLineEnding(Core::TextDocument::LineEnding newLineEnding);

    int lineAtPosition(int position);
    int columnAtPosition(int position);
    int positionAt(int line, int column);

    void undo(int count = 1);
    void redo(int count = 1);

    // Goto methods, to move around the document
    void gotoLine(int line, int column = 1);
    void gotoStartOfLine();
    void gotoEndOfLine();
    void gotoStartOfWord();
    void gotoEndOfWord();
    void gotoPreviousLine(int count = 1);
    void gotoNextLine(int count = 1);
    void gotoPreviousChar(int count = 1);
    void gotoNextChar(int count = 1);
    void gotoPreviousWord(int count = 1);
    void gotoNextWord(int count = 1);
    void gotoStartOfDocument();
    void gotoEndOfDocument();

    // Selection methods
    void unselect();
    bool hasSelection();
    void selectAll();

    void selectTo(int pos);
    void selectStartOfLine();
    void selectEndOfLine();
    void selectStartOfWord();
    void selectEndOfWord();
    void selectPreviousLine(int count = 1);
    void selectNextLine(int count = 1);
    void selectPreviousChar(int count = 1);
    void selectNextChar(int count = 1);
    void selectPreviousWord(int count = 1);
    void selectNextWord(int count = 1);
    void selectRegion(int from, int to);
    void selectRange(const Core::TextRange &range);

    // Copy & paste
    void copy();
    void paste();
    void cut();

    // Text handling
    void remove(int length);
    void insert(const QString &text);
    void insertAtLine(const QString &text, int line = -1);
    void insertAtPosition(const QString &text, int pos);
    void replace(int length, const QString &text);
    void replace(int from, int to, const QString &text);
    void replace(const Core::TextRange &range, const QString &text);

    // Deletion
    void deleteLine(int line = -1);
    void deleteSelection();
    void deleteEndOfLine();
    void deleteStartOfLine();
    void deleteEndOfWord();
    void deleteStartOfWord();
    void deletePreviousCharacter(int count = 1);
    void deleteNextCharacter(int count = 1);
    void deleteRegion(int from, int to);
    void deleteRange(const Core::TextRange &range);

    // Mark
    Core::Mark createMark(int pos = -1);
    void gotoMark(const Core::Mark &mark);
    void selectToMark(const Core::Mark &mark);

    // RangeMark
    Core::RangeMark createRangeMark(int from, int to);
    Core::RangeMark createRangeMark();
    void selectRangeMark(const Core::RangeMark &mark);

    // Find
    bool find(const QString &text, int options = NoFindFlags);
    bool findRegexp(const QString &regexp, int options = NoFindFlags);
    QString match(QString expr, int options = NoFindFlags);

    // Replace
    bool replaceOne(const QString &before, const QString &after, int options = NoFindFlags);
    int replaceAll(const QString &before, const QString &after, int options = NoFindFlags);
    int replaceAllInRange(const QString &before, const QString &after, const Core::RangeMark &range,
                          int options = NoFindFlags);
    int replaceAllRegexpInRange(const QString &regexp, const QString &after, const Core::RangeMark &range,
                                int options = NoFindFlags);
    int replaceAllRegexp(const QString &regexp, const QString &after, int options = NoFindFlags);

    // Indentation
    void indent(int count = 1);
    void removeIndent(int count = 1);
    QString indentationAtPosition(int pos);

signals:
    void positionChanged();
    void textChanged();
    void selectionChanged();
    void lineEndingChanged();

protected:
    explicit TextDocument(Type type, QObject *parent = nullptr);

    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

    friend MarkPrivate;
    void convertPosition(int pos, int *line, int *column) const;
    int position(QTextCursor::MoveOperation operation, int pos) const;

    int replaceAll(const QString &before, const QString &after, int options,
                   const std::function<bool(QTextCursor)> &filterAcceptsCursor);
    int replaceAllRegexp(const QString &regexp, const QString &after, int options,
                         const std::function<bool(QTextCursor)> &filterAcceptsCursor);

private:
    void detectFormat(const QByteArray &data);

    void movePosition(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor,
                      int count = 1);

    auto selectRegexpMatch(
        QString regexp, int options,
        std::function<bool(const QRegularExpression &, const QRegularExpressionMatch &, const QTextCursor &)>
            selectionFunction = [](const auto &, const auto &, const auto &) {
                return true;
            }) -> std::optional<std::pair<QRegularExpressionMatch, QTextCursor>>;

    // TODO: use a QTextDocument maybe, to avoid creating a widget
    // The QPlainTextEdit has a nicer API, so it's slightly easier with that now
    QPointer<QPlainTextEdit> m_document;
    LineEnding m_lineEnding = NativeLineEnding;
    bool m_utf8Bom = false;
};

} // namespace Core

Q_DECLARE_OPERATORS_FOR_FLAGS(Core::TextDocument::FindFlags)
