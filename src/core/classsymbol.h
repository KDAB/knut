#pragma once

#include "symbol.h"

#include <QString>
#include <QVector>

namespace Core {

class ClassSymbol : public Symbol
{
    Q_OBJECT

    Q_PROPERTY(QVector<Symbol *> members READ members CONSTANT)

    QVector<Symbol *> findMembers() const;

protected:
    friend class Symbol;
    ClassSymbol(QObject *parent, const QString &name, const QString &description, Kind kind, TextRange range,
                TextRange selectionRange);

    // mutable for lazy initialization
    mutable std::optional<QVector<Symbol *>> m_members;

public:
    const QVector<Symbol *> &members() const;
};
bool operator==(const ClassSymbol &left, const ClassSymbol &right);

} // namespace Core

Q_DECLARE_METATYPE(Core::ClassSymbol)
