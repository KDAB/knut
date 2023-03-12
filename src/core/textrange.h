#pragma once

#include <QObject>

namespace Core {

struct TextRange
{
    Q_GADGET
    Q_PROPERTY(int start MEMBER start CONSTANT)
    Q_PROPERTY(int end MEMBER end CONSTANT)

public:
    int start;
    int end;

    Q_INVOKABLE QString toString() const { return QString("{%1, %2}").arg(start).arg(end); }

    bool contains(int pos) const;
    bool contains(const TextRange &range) const;

    auto operator<=>(const TextRange &) const = default;
};

} // namespace Core

Q_DECLARE_METATYPE(Core::TextRange)
