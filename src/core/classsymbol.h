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
    ClassSymbol(QObject *parent, const QueryMatch &match, Kind kind);

    // mutable for lazy initialization
    mutable std::optional<QVector<Symbol *>> m_members;

public:
    const QVector<Symbol *> &members() const;
    QString description() const override;
};
bool operator==(const ClassSymbol &left, const ClassSymbol &right);

} // namespace Core

Q_DECLARE_METATYPE(Core::ClassSymbol)
