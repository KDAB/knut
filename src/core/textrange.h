#pragma once

#include <QObject>

namespace Core {

struct TextRange
{
    Q_GADGET
    Q_PROPERTY(int start MEMBER start)
    Q_PROPERTY(int end MEMBER end)

public:
    int start;
    int end;

    auto operator<=>(const TextRange &) const = default;
};

} // namespace Core

Q_DECLARE_METATYPE(Core::TextRange)
