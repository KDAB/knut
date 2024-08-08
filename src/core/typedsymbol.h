#pragma once

#include "symbol.h"

namespace Core {

class TypedSymbol : public Symbol
{
    Q_OBJECT

    Q_PROPERTY(QString type READ type CONSTANT)

public:
    QString type() const;

private:
    friend class Symbol;
    TypedSymbol(QObject *parent, const QueryMatch &match, Kind kind);

    QString m_type;
};

}
