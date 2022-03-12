#include "symbol.h"

#include "logger.h"

#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype Symbol
 * \brief Represent a symbol in the current file
 * \instantiates Core::Symbol
 * \inqmlmodule Script
 * \since 4.0
 */

/*!
 * \qmlproperty string Symbol::name
 * Return the name of this symbol.
 */

/*!
 * \qmlproperty string Symbol::description
 * Return more detail for this symbol, e.g the signature of a function.
 */

/*!
 * \qmlproperty Kind Symbol::kind
 * Return the kind of this symbol. Available symbol kinds are:
 *
 * - `Symbol.File`
 * - `Symbol.Module`
 * - `Symbol.Namespace`
 * - `Symbol.Package`
 * - `Symbol.Class`
 * - `Symbol.Method`
 * - `Symbol.Property`
 * - `Symbol.Field`
 * - `Symbol.Constructor`
 * - `Symbol.Enum`
 * - `Symbol.Interface`
 * - `Symbol.Function`
 * - `Symbol.Variable`
 * - `Symbol.Constant`
 * - `Symbol.String`
 * - `Symbol.Number`
 * - `Symbol.Boolean`
 * - `Symbol.Array`
 * - `Symbol.Object`
 * - `Symbol.Key`
 * - `Symbol.Null`
 * - `Symbol.EnumMember`
 * - `Symbol.Struct`
 * - `Symbol.Event`
 * - `Symbol.Operator`
 * - `Symbol.TypeParameter`
 */

/*!
 * \qmlproperty TextRange Symbol::range
 * The range enclosing this symbol not including leading/trailing whitespace
 * but everything else like comments. This information is typically used to
 * determine if the clients cursor is inside the symbol to reveal in the
 * symbol in the UI.
 */

/*!
 * \qmlproperty TextRange Symbol::selectionRange
 * The range that should be selected and revealed when this symbol is being
 * picked, e.g. the name of a function. Must be contained by the `range`.
 */

/*!
 * \qmlmethod Core::CppFunction Core::Symbol::toFunction()
 * Returns a `Core::CppFunction` structure for current `Core::Symbol`.
 * The method checks if the `Kind` of `Core::Symbol` is either `Kind::Method` or `Kind::Function`.
 * If so then it extracts information from `Symbol::description`, fills it in `CppFunction` structure, and returns it.
 * If not, then it returns an empty Core::CppFunction structure.
 */

CppFunction Symbol::toFunction()
{
    LOG("Symbol::toFunction");

    if ((kind == Method) || (kind == Function)) {
        QString desc = this->description;
        // TODO: Add logic to handle type-qualifiers.
        // For now, discard type-qualifier, if found any.
        if (desc.startsWith("static "))
            desc.remove(0, 7);
        desc.chop((desc.length() - desc.lastIndexOf(')') - 1));

        QString returnType = desc.left(desc.indexOf('(')).trimmed();
        int argStart = desc.indexOf('(') + 1;
        QString args = desc.mid(argStart, desc.length() - argStart - 1);

        QStringList argsList = args.split(',', Qt::SkipEmptyParts);

        QVector<Argument> arguments;
        for (const auto &arg : qAsConst(argsList)) {
            arguments.push_back(Argument {.type = arg.trimmed(), .name = ""});
        }

        return CppFunction {name, returnType, arguments, range};
    } else {
        spdlog::warn("Symbol::toFunction {} - should be either a `Method` or a `Function`.", name.toStdString());

        return CppFunction();
    }
}

} // namespace Core
