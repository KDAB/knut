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

    Q_INVOKABLE QString toString() const;

    // Extracted into a static function, so it can be reused by RangeMark
    static void updateMark(int &mark, int from, int charsRemoved, int charsAdded);

public slots:
    void restore();

private:
    bool checkEditor() const;
    void update(int from, int charsRemoved, int charsAdded);

private:
    friend TextDocument;
    QPointer<TextDocument> m_editor;
    int m_pos = -1;
};

} // namespace Core
