#include "cppfunction.h"

namespace Core {

/*!
 * \qmltype CppFunction
 * \brief Represents a function or a method in the current file
 * \instantiates Core::CppFunction
 * \inqmlmodule Script
 * \todo
 */

/*!
 * \qmlproperty string CppFunction::name
 * Returns the fully qualified name of this function.
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
 * \qmlproperty bool CppFunction::isNull
 * This property returns `true` if the function is null.
 */

/*!
 * \qmltype Argument
 * \brief Represents an argument to be passed to the function
 * \instantiates Core::Argument
 * \inqmlmodule Script
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

bool operator==(const Argument &left, const Argument &right)
{
    return ((left.type == right.type) && (left.name == right.name));
}

bool operator==(const CppFunction &left, const CppFunction &right)
{
    return ((left.name == right.name) && (left.returnType == right.returnType) && (left.arguments == right.arguments)
            && (left.range == right.range));
}

bool CppFunction::isNull() const
{
    return name.isNull();
}

} // namespace Core
