#include "functionsymbol.h"

#include "codedocument.h"
#include "cpplanguagedata.h"
#include "lsp/lsp_utils.h"

#include <QRegularExpression>

#include <kdalgorithms.h>
#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype FunctionArgument
 * \brief Represents an argument to be passed to the function
 * \inqmlmodule Script
 * \ingroup CodeDocument
 * \since 1.0
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

FunctionArgument FunctionArgument::fromHover(const QString &parameter, Document::Type type)
{
    static auto identifierRegexp = QRegularExpression(R"([a-zA-Z_][a-zA-Z0-9_]*)");

    auto words = Lsp::Utils::removeTypeAliasInformation(parameter).split(" ");

    bool hasName = true;

    if (type == Document::Type::Cpp) {
        // In C++, parameter names are optional, therefore we need to figure out if the parameter even has a name.
        auto withoutKeywords = words;
        withoutKeywords.removeIf([](const auto &word) {
            return Cpp::keywords.contains(word);
        });
        hasName = withoutKeywords.size() >= 2 && identifierRegexp.match(words.last()).hasMatch()
            && !Cpp::primitiveTypes.contains(words.last());
    }

    FunctionArgument argument;
    if (hasName) {
        argument.name = words.last();
        words.pop_back();
    }

    argument.type = words.join(" ");

    return argument;
}

/*!
 * \qmltype FunctionSymbol
 * \brief Represents a function or a method in the current file
 * \inqmlmodule Script
 * \ingroup CodeDocument
 * \since 1.0
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
const QVector<FunctionArgument> &FunctionSymbol::arguments() const
{
    if (!m_arguments.has_value()) {
        m_arguments = std::make_optional(argumentsFromQueryMatch());
    }

    return m_arguments.value();
}

QVector<FunctionArgument> FunctionSymbol::argumentsFromQueryMatch() const
{
    auto arguments = m_queryMatch.getAll("parameter");
    auto to_function_arg = [this](const RangeMark &argument) {
        auto result = document()->queryInRange(argument, "(identifier) @name");
        auto nameRange = result.isEmpty() ? RangeMark() : result.first().get("name");
        auto name = nameRange.text().simplified();
        auto type = argument.textExcept(nameRange).simplified();

        return FunctionArgument {.type = type, .name = name};
    };

    return kdalgorithms::transformed<QVector<FunctionArgument>>(arguments, to_function_arg);
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
