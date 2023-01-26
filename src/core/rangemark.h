#pragma once

#include <QObject>

#include "textdocument.h"

namespace Core {

class RangeMark : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int start READ start FINAL)
    Q_PROPERTY(int end READ end FINAL)
    Q_PROPERTY(bool isValid READ isValid FINAL)

public:
    explicit RangeMark(TextDocument *editor, int start, int end);
    virtual ~RangeMark() = default;

    bool isValid() const;

    int start() const;
    int end() const;
    int length() const;

    Q_INVOKABLE QString text() const;
    Q_INVOKABLE QString toString() const;

public slots:
    void select();

private:
    bool checkEditor() const;
    void update(int from, int charsRemoved, int charsAdded);

    void ensureInvariant();

    QPointer<TextDocument> m_editor;

    // We need to uphold the invariant
    // that m_start <= m_end
    //
    // Encoding with m_start + m_length would remove this requirement,
    // however, it would make update() more complicated.
    int m_start;
    // Note: m_end is exclusive
    int m_end;

    friend TextDocument;
};

} // namespace Core
