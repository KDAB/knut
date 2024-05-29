#pragma once

#include <QObject>
#include <memory>

namespace Core {

class TextDocument;

// Mark is shared_ptr to a MarkPrivate.
// This way we can ensure that a Mark is easy to copy and move
// around, whilst still ensuring the QObject that listens to the
// changes of the TextDocument is correctly deleted both from QML and C++.
class Mark
{
    Q_GADGET

    Q_PROPERTY(int line READ line FINAL)
    Q_PROPERTY(int column READ column FINAL)
    Q_PROPERTY(int position READ position FINAL)
    Q_PROPERTY(bool isValid READ isValid CONSTANT)
    Q_PROPERTY(TextDocument *document READ document CONSTANT)

public:
    // Default constructor is required for Q_DECLARE_METATYPE
    Mark() = default;

    explicit Mark(TextDocument *editor, int pos);

    bool isValid() const;

    int position() const;
    int line() const;
    int column() const;

    Q_INVOKABLE QString toString() const;

    TextDocument *document() const;

    // Extracted into a static function, so it can be reused by RangeMark
    static void updateMark(int &mark, int from, int charsRemoved, int charsAdded);

    Q_INVOKABLE void restore() const;

private:
    std::shared_ptr<class MarkPrivate> d = nullptr;

    friend TextDocument;
};

bool operator==(const Mark &left, const Mark &right);

} // namespace Core

Q_DECLARE_METATYPE(Core::Mark)
