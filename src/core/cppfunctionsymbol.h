#pragma once

#include <QString>
#include <QVector>

#include "symbol.h"
#include "textrange.h"

namespace Core {

struct Argument
{
    Q_GADGET

    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(QString name MEMBER name)

public:
    QString type;
    QString name;

    static Argument fromHover(const QString &parameter);
};
bool operator==(const Argument &left, const Argument &right);

class CppFunctionSymbol : public Symbol
{
public:
    Q_OBJECT

    Q_PROPERTY(QString returnType READ returnType CONSTANT)
    Q_PROPERTY(QVector<Argument> arguments READ arguments CONSTANT)

protected:
    // necessary so the constructor can be accessed from the Symbol class.
    friend class Symbol;

    CppFunctionSymbol(QObject *parent, const QString &name, const QString &description, Kind kind, TextRange range,
                      TextRange selectionRange);

    mutable std::optional<QString> m_returnType;
    mutable std::optional<QVector<Argument>> m_arguments;

    // fallback heuristic, if `Hover` LSP call fails
    QString returnTypeFromDescription() const;
    QVector<Argument> argumentsFromDescription() const;

    std::optional<QString> returnTypeFromLSP() const;
    std::optional<QVector<Argument>> argumentsFromLSP() const;

    Argument splitNameAndType(const QString &parameter) const;

public:
    QString returnType() const;
    const QVector<Argument> &arguments() const;
};
bool operator==(const CppFunctionSymbol &left, const CppFunctionSymbol &right);

} // namespace Core

Q_DECLARE_METATYPE(Core::Argument)
Q_DECLARE_METATYPE(Core::CppFunctionSymbol)
Q_DECLARE_METATYPE(QVector<Core::Argument>)
