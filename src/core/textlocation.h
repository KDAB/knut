#pragma once

#include <QObject>

#include "lsp/types.h"
#include "textrange.h"

namespace Core {

class LspDocument;

struct TextLocation
{
    Q_GADGET
    Q_PROPERTY(Core::LspDocument *document MEMBER document)
    Q_PROPERTY(Core::TextRange range MEMBER range)

public:
    LspDocument *document;
    TextRange range;

    Q_INVOKABLE QString toString() const;

    auto operator<=>(const TextLocation &) const = default;

    static QVector<TextLocation> fromLsp(const std::vector<Lsp::Location> &locations);
};

} // namespace Core

Q_DECLARE_METATYPE(Core::TextLocation)
