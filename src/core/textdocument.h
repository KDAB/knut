#pragma once

#include "document.h"

#include <memory>

class QPlainTextEdit;

namespace Core {

class TextDocument : public Document
{
    Q_OBJECT
    Q_PROPERTY(int column READ column)
    Q_PROPERTY(int line READ line)
    Q_PROPERTY(int lineCount READ lineCount)
    Q_PROPERTY(int position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString selectedText READ selectedText NOTIFY selectionChanged)
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

    explicit TextDocument(QObject *parent = nullptr);
    ~TextDocument();

    int column() const;
    int line() const;
    int lineCount() const;

    int position() const;
    void setPosition(int newPosition);

    QString text() const;
    void setText(const QString &newText);

    QString selectedText() const;

    LineEnding lineEnding() const;
    void setLineEnding(LineEnding newLineEnding);

    bool hasUtf8Bom() const;

signals:
    void positionChanged();
    void textChanged();
    void selectionChanged();
    void lineEndingChanged();

protected:
    explicit TextDocument(Type type, QObject *parent = nullptr);

    bool doSave(const QString &fileName) override;
    bool doLoad(const QString &fileName) override;

    void detectFormat(const QByteArray &data);

private:
    // TODO: use a QTextDocument maybe, to avoid creating a widget
    // The QPlainTextEdit has a nicer API, so it's slightly easier with that now
    std::unique_ptr<QPlainTextEdit> m_document;
    LineEnding m_lineEnding = NativeLineEnding;
    bool m_utf8Bom = false;
};

} // namespace Core
