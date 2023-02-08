#include "cppfunctionsymbol.h"

#include "cpplanguagedata.h"
#include "lsp/lsp_utils.h"
#include "lspdocument.h"

#include <QRegularExpression>

#include <kdalgorithms.h>
#include <spdlog/spdlog.h>

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

Argument Argument::fromHover(const QString &parameter)
{
    static auto identifierRegexp = QRegularExpression(R"([a-zA-Z_][a-zA-Z0-9_]*)");

    auto words = Lsp::Utils::removeTypeAliasInformation(parameter).split(" ");

    // In C++, parameter names are optional, therefore we need to figure out if the parameter even has a name.
    auto withoutKeywords = words;
    withoutKeywords.removeIf([](const auto &word) {
        return Cpp::keywords.contains(word);
    });
    const auto hasName = withoutKeywords.size() >= 2 && identifierRegexp.match(words.last()).hasMatch()
        && !Cpp::primitiveTypes.contains(words.last());

    Argument argument;
    if (hasName) {
        argument.name = words.last();

        words.pop_back();
    }

    argument.type = words.join(" ");

    return argument;
}

CppFunctionSymbol::CppFunctionSymbol(QObject *parent, const QString &name, const QString &description, Kind kind,
                                     TextRange range, TextRange selectionRange)
    : Symbol(parent, name, description, kind, range, selectionRange)
    , m_returnType {}
    , m_arguments {}
{
    if (m_description.isEmpty()) {
        const auto args = arguments();
        auto toType = [](const Argument &arg) {
            return arg.type;
        };
        QStringList argumentTypes = kdalgorithms::transformed<QStringList>(args, toType);

        m_description = QString("%1 (%2)").arg(returnType()).arg(argumentTypes.join(", "));
    }
}

QString CppFunctionSymbol::returnTypeFromDescription() const
{
    auto desc = m_description;
    // TODO: Add logic to handle type-qualifiers.
    // For now, discard type-qualifier, if found any.
    if (desc.startsWith("static "))
        desc.remove(0, 7);
    desc.chop((desc.length() - desc.lastIndexOf(')') - 1));

    return desc.left(desc.indexOf('(')).trimmed();
}

std::optional<QString> CppFunctionSymbol::returnTypeFromLSP() const
{
    if (auto lspdocument = document()) {
        auto hover = lspdocument->hover(selectionRange().start);

        auto lines = hover.split("\n");
        while (!lines.isEmpty()) {
            auto line = lines.front();
            lines.pop_front();

            if (line.startsWith("→ ")) {
                line.remove(0, 2);

                return Lsp::Utils::removeTypeAliasInformation(line);
            }
        }

        return "";
    }

    return {};
}

QVector<Argument> CppFunctionSymbol::argumentsFromDescription() const
{
    int argStart = m_description.indexOf('(') + 1;
    QString args = m_description.mid(argStart, m_description.lastIndexOf(')') - argStart);

    const QStringList argsList = args.split(',', Qt::SkipEmptyParts);

    QVector<Argument> arguments;
    arguments.reserve(argsList.size());
    for (const auto &arg : qAsConst(argsList)) {
        arguments.push_back(Argument {.type = arg.trimmed(), .name = ""});
    }

    return arguments;
}

std::optional<QVector<Argument>> CppFunctionSymbol::argumentsFromLSP() const
{
    if (auto lspdocument = document()) {
        auto hover = lspdocument->hover(selectionRange().start);

        spdlog::debug("CppFunctionSymbol::argumentsFromLSP: Hover string:\n{}", hover.toStdString());

        auto lines = hover.split("\n");
        while (!lines.isEmpty() && !lines.first().contains("Parameters:")) {
            lines.pop_front();
        }
        if (!lines.isEmpty()) {
            // We found a Parameters listing
            lines.pop_front(); // Remove the 'Parameters:' text

            QVector<Core::Argument> arguments;
            while (!lines.isEmpty() && lines.first().startsWith("- ")) {
                auto parameter = lines.first();
                lines.pop_front();

                // remove the "- "
                parameter.remove(0, 2);

                arguments.emplaceBack(Argument::fromHover(parameter));
            }

            return arguments;
        }

        // No arguments found
        return QVector<Core::Argument>();
    } else {
        spdlog::warn("Symbol '{}' doesn't have an associated document!", name().toStdString());
    }

    return {};
}

QString CppFunctionSymbol::returnType() const
{
    if (!m_returnType.has_value()) {
        m_returnType = returnTypeFromLSP();
    }

    if (!m_returnType.has_value()) {
        m_returnType = std::make_optional(returnTypeFromDescription());
    }

    return m_returnType.value();
}
const QVector<Argument> &CppFunctionSymbol::arguments() const
{
    if (!m_arguments.has_value()) {
        m_arguments = argumentsFromLSP();
    }

    if (!m_arguments.has_value()) {
        m_arguments = std::make_optional(argumentsFromDescription());
    }

    return m_arguments.value();
}

bool operator==(const CppFunctionSymbol &left, const CppFunctionSymbol &right)
{
    return left.Symbol::operator==(right) && left.returnType() == right.returnType()
        && left.arguments() == right.arguments();
}

bool operator==(const Argument &left, const Argument &right)
{
    return ((left.type == right.type) && (left.name == right.name));
}

} // namespace Core
