#pragma once

#include <QObject>
#include <QPointer>

namespace Core {

class TextDocument;

class Mark : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int line READ line FINAL)
    Q_PROPERTY(int column READ column FINAL)
    Q_PROPERTY(int position READ position FINAL)
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

public:
    explicit Mark(TextDocument *editor, int pos);

    bool isValid() const;

    int position() const;
    int line() const;
    int column() const;

public slots:
    void restore();

private:
    bool checkEditor() const;
    void update(int from, int charsRemoved, int charsAdded);

private:
    friend TextDocument;
    QPointer<TextDocument> m_editor;
    int m_pos = -1;
    QString m_fileName;
};

} // namespace Core
