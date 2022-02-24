#pragma once

#include "../symbol.h"

#include <QVector>

namespace Core {

class LspDocument;

class LspCache
{
public:
    explicit LspCache(LspDocument *document);

    void clear();

    const QVector<Core::Symbol> &symbols();

private:
    enum Flags {
        HasSymbols = 0x01,
    };

    LspDocument *const m_document;
    QVector<Core::Symbol> m_symbols;
    int m_flags = 0;
};

} // namespace Core
