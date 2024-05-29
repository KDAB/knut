#pragma once

#include "lsp/types.h"
#include "textrange.h"

#include <QObject>

namespace Core {

class CodeDocument;

struct TextLocation
{
    Q_GADGET
    Q_PROPERTY(Core::CodeDocument *document MEMBER document CONSTANT)
    Q_PROPERTY(Core::TextRange range MEMBER range CONSTANT)

public:
    CodeDocument *document;
    TextRange range;

    Q_INVOKABLE QString toString() const;

    auto operator<=>(const TextLocation &) const = default;

    static QVector<TextLocation> fromLsp(const std::vector<Lsp::Location> &locations);
};

using TextLocationList = QList<Core::TextLocation>;

} // namespace Core

Q_DECLARE_METATYPE(Core::TextLocation)
