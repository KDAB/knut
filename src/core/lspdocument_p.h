#pragma once

#include "symbol.h"

#include <QVector>

namespace Core {

class LspDocument;

class LspCache
{
public:
    explicit LspCache(LspDocument *document);

    void clear();

    const QVector<Core::Symbol *> &symbols();

    const Core::Symbol *inferSymbol(const QString &hoverText, TextRange range);

private:
    enum Flags {
        HasSymbols = 0x01,
    };

    const Core::Symbol *inferVariable(const QStringList &lines, TextRange range, Symbol::Kind kind);
    const Core::Symbol *inferMethod(const QStringList &lines, TextRange range, Symbol::Kind kind);
    const Core::Symbol *inferGenericSymbol(QStringList lines, TextRange range);

    LspDocument *const m_document;
    QVector<Core::Symbol *> m_symbols;
    QVector<const Core::Symbol *> m_inferredSymbols;
    int m_flags = 0;
};

} // namespace Core
