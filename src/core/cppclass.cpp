#include "cppclass.h"

namespace Core {

/*!
 * \qmltype CppClass
 * \brief Represents a class in the current file
 * \instantiates Core::CppClass
 * \inqmlmodule Script
 * \since 4.0
 * \todo
 */

/*!
 * \qmlproperty string CppClass::name
 * Returns the name of this class.
 */

/*!
 * \qmlproperty vector<Symbol> CppClass::members
 * Returns the list of members (both data and functions) of this class.
 */

bool operator==(const CppClass &left, const CppClass &right)
{
    return ((left.name == right.name) && (left.members == right.members));
}

} // namespace Core
