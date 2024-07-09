/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "functionsymbol.h"
#include "codedocument.h"

#include <QRegularExpression>
#include <kdalgorithms.h>

namespace Core {

/*!
 * \qmltype FunctionArgument
 * \brief Represents an argument to be passed to the function
 * \inqmlmodule Knut
 * \ingroup CodeDocument
 * \todo
 * \sa FunctionSymbol
 */

/*!
 * \qmlproperty string FunctionArgument::type
 * Returns the type of this argument.
 */
/*!
 * \qmlproperty string FunctionArgument::name
 * Returns the name of this argument.
 */

/*!
 * \qmltype FunctionSymbol
 * \brief Represents a function or a method in the current file
 * \inqmlmodule Knut
 * \ingroup CodeDocument
 * \todo
 */

/*!
 * \qmlproperty string FunctionSymbol::returnType
 * Returns the return type of this function.
 */
/*!
 * \qmlproperty vector<FunctionArgument> FunctionSymbol::arguments
 * Returns the list of arguments being passed to this function.
 */
/*!
 * \qmlproperty TextRange FunctionSymbol::range
 * The range enclosing this function, not including leading/trailing
 * whitespace but everything else like comments.
 */

FunctionSymbol::FunctionSymbol(QObject *parent, const QueryMatch &match, Kind kind)
    : Symbol(parent, match, kind)
{
}

QString FunctionSymbol::description() const
{
    return signature();
}

QString FunctionSymbol::signature() const
{
    const auto args = arguments();
    auto toType = [](const FunctionArgument &arg) {
        return arg.type;
    };
    auto argumentTypes = kdalgorithms::transformed<QStringList>(args, toType);

    return QString("%1 (%2)").arg(returnType(), argumentTypes.join(", "));
}

QString FunctionSymbol::returnTypeFromQueryMatch() const
{
    return m_queryMatch.getAllJoined("return").text();
}

QString FunctionSymbol::returnType() const
{
    if (!m_returnType.has_value()) {
        m_returnType = std::make_optional(returnTypeFromQueryMatch());
    }

    return m_returnType.value();
}
const QList<FunctionArgument> &FunctionSymbol::arguments() const
{
    if (!m_arguments.has_value()) {
        m_arguments = std::make_optional(argumentsFromQueryMatch());
    }

    return m_arguments.value();
}

QList<FunctionArgument> FunctionSymbol::argumentsFromQueryMatch() const
{
    auto arguments = m_queryMatch.getAll("parameter");
    auto to_function_arg = [this](const RangeMark &argument) {
        auto result = document()->queryInRange(argument, "(identifier) @name");
        auto nameRange = result.isEmpty() ? RangeMark() : result.first().get("name");
        auto name = nameRange.text().simplified();
        auto type = argument.textExcept(nameRange).simplified();

        return FunctionArgument {.type = type, .name = name};
    };

    return kdalgorithms::transformed<QList<FunctionArgument>>(arguments, to_function_arg);
}

bool operator==(const FunctionSymbol &left, const FunctionSymbol &right)
{
    return left.Symbol::operator==(right) && left.returnType() == right.returnType()
        && left.arguments() == right.arguments();
}

bool operator==(const FunctionArgument &left, const FunctionArgument &right)
{
    return ((left.type == right.type) && (left.name == right.name));
}

} // namespace Core
