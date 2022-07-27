#include "cppfunctionsymbol.h"

namespace Core {

/*!
 * \qmltype CppFunction
 * \brief Represents a function or a method in the current file
 * \instantiates Core::CppFunction
 * \inqmlmodule Script
 * \since 4.0
 * \todo
 */

/*!
 * \qmlproperty string CppFunction::returnType
 * Returns the return type of this function.
 */

/*!
 * \qmlproperty vector<Argument> CppFunction::arguments
 * Returns the list of arguments being passed to this function.
 */

/*!
 * \qmlproperty TextRange CppFunction::range
 * The range enclosing this function, not including leading/trailing
 * whitespace but everything else like comments.
 */

/*!
 * \qmltype Argument
 * \brief Represents an argument to be passed to the function
 * \instantiates Core::Argument
 * \inqmlmodule Script
 * \since 4.0
 * \todo
 */

/*!
 * \qmlproperty string Argument::type
 * Returns the type of this argument.
 */

/*!
 * \qmlproperty string Argument::name
 * Returns the name of this argument.
 */

CppFunctionSymbol::CppFunctionSymbol(QObject *parent, const QString &name, const QString &description, Kind kind,
                                     TextRange range, TextRange selectionRange)
    : Symbol(parent, name, description, kind, range, selectionRange)
    , m_returnType {}
    , m_arguments {}
{
    QString desc = description;
    // TODO: Add logic to handle type-qualifiers.
    // For now, discard type-qualifier, if found any.
    if (desc.startsWith("static "))
        desc.remove(0, 7);
    desc.chop((desc.length() - desc.lastIndexOf(')') - 1));

    m_returnType = desc.left(desc.indexOf('(')).trimmed();
    int argStart = desc.indexOf('(') + 1;
    QString args = desc.mid(argStart, desc.length() - argStart - 1);

    QStringList argsList = args.split(',', Qt::SkipEmptyParts);

    for (const auto &arg : qAsConst(argsList)) {
        m_arguments.push_back(Argument {.type = arg.trimmed(), .name = ""});
    }
}
QString CppFunctionSymbol::returnType() const
{
    return m_returnType;
}
const QVector<Argument> &CppFunctionSymbol::arguments() const
{
    return m_arguments;
}

bool operator==(const Argument &left, const Argument &right)
{
    return ((left.type == right.type) && (left.name == right.name));
}

bool operator==(const CppFunctionSymbol &left, const CppFunctionSymbol &right)
{
    return left.Symbol::operator==(right) && left.returnType() == right.returnType()
        && left.arguments() == right.arguments();
}

} // namespace Core
