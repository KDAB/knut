#pragma once

#include <QObject>
#include <QPointer>

namespace Core {

class TextDocument;

class RangeMarkPrivate : public QObject
{
    Q_OBJECT

public:
    // Unfortunately this needs to be public, as otherwise std::make_shared can't access it
    explicit RangeMarkPrivate(TextDocument *editor, int start, int end);

private:
    void ensureInvariant();

    bool isValid() const;
    bool checkEditor() const;

    void update(int from, int charsRemoved, int charsAdded);

    QPointer<TextDocument> m_editor;

    // We need to uphold the invariant
    // that m_start <= m_end
    //
    // Encoding with m_start + m_length would remove this requirement,
    // however, it would make update() more complicated.
    int m_start;
    // Note: m_end is exclusive
    int m_end;

    friend class RangeMark;
    friend class AstNode;
};

} // namespace Core
