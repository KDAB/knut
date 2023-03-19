#pragma once

#include "document.h"
#include "symbol.h"
#include "textrange.h"

#include <QString>
#include <QVector>

namespace Core {

struct FunctionArgument
{
    Q_GADGET

    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(QString name MEMBER name)

public:
    QString type;
    QString name;

    static FunctionArgument fromHover(const QString &parameter, Document::Type type);
};
bool operator==(const FunctionArgument &left, const FunctionArgument &right);

class FunctionSymbol : public Symbol
{
public:
    Q_OBJECT

    Q_PROPERTY(QString returnType READ returnType CONSTANT)
    Q_PROPERTY(QVector<Core::FunctionArgument> arguments READ arguments CONSTANT)

protected:
    // necessary so the constructor can be accessed from the Symbol class.
    friend class Symbol;

    FunctionSymbol(QObject *parent, const QString &name, const QString &description, Kind kind, TextRange range,
                   TextRange selectionRange);

    mutable std::optional<QString> m_returnType;
    mutable std::optional<QVector<FunctionArgument>> m_arguments;

    // fallback heuristic, if `Hover` LSP call fails
    QString returnTypeFromDescription() const;
    QVector<FunctionArgument> argumentsFromDescription() const;

    std::optional<QString> returnTypeFromLSP() const;
    std::optional<QVector<FunctionArgument>> argumentsFromLSP() const;

    FunctionArgument splitNameAndType(const QString &parameter) const;

public:
    QString returnType() const;
    const QVector<FunctionArgument> &arguments() const;
};
bool operator==(const FunctionSymbol &left, const FunctionSymbol &right);

} // namespace Core

Q_DECLARE_METATYPE(Core::FunctionArgument)
Q_DECLARE_METATYPE(Core::FunctionSymbol)
Q_DECLARE_METATYPE(QVector<Core::FunctionArgument>)
