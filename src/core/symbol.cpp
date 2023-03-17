#include "symbol.h"

#include "classsymbol.h"
#include "cppfunctionsymbol.h"
#include "logger.h"

#include <core/lspdocument.h>
#include <core/project.h>

#include <kdalgorithms.h>
#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype Symbol
 * \brief Represent a symbol in the current file
 * \inqmlmodule Script
 * \ingroup LspDocument
 * \since 1.0
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
 * The range enclosing this symbol not including leading/trailing whitespace but everything else like comments. This
 * information is typically used to determine if the clients cursor is inside the symbol to reveal in the symbol in the
 * UI.
 */

/*!
 * \qmlproperty TextRange Symbol::selectionRange
 * The range that should be selected and revealed when this symbol is being picked, e.g. the name of a function. Must be
 * contained by the `range`.
 */

/*!
 * \qmlproperty array<TextLocation> Symbol::references
 * \since 1.1
 * List of all references of this symbol in the current project.
 */

Symbol::Symbol(QObject *parent, const QString &name, const QString &description, Kind kind, TextRange range,
               TextRange selectionRange)
    : QObject(parent)
    , m_name {name}
    , m_description {description}
    , m_kind {kind}
    , m_range {range}
    , m_selectionRange {selectionRange}
{
}

Symbol *Symbol::makeSymbol(QObject *parent, const QString &name, const QString &description, Kind kind, TextRange range,
                           TextRange selectionRange)
{
    if (kind == Method || kind == Function || kind == Constructor) {
        return new CppFunctionSymbol(parent, name, description, kind, range, selectionRange);
    }
    if (kind == Class || kind == Struct) {
        return new ClassSymbol(parent, name, description, kind, range, selectionRange);
    }
    return new Symbol(parent, name, description, kind, range, selectionRange);
}

Symbol *Symbol::makeSymbol(QObject *parent, const Lsp::DocumentSymbol &lspSymbol, TextRange range,
                           TextRange selectionRange, QString context /* = ""*/)
{
    auto description = QString::fromStdString(lspSymbol.detail.value_or(""));
    auto kind = static_cast<Symbol::Kind>(lspSymbol.kind);
    auto name = QString::fromStdString(lspSymbol.name);

    if (!context.isEmpty())
        name = context + "::" + name;

    return makeSymbol(parent, name, description, kind, range, selectionRange);
}

LspDocument *Symbol::document() const
{
    return qobject_cast<LspDocument *>(parent());
}

/*!
 * \qmlmethod bool Symbol::isClass()
 * \since 1.1
 *
 * Returns whether this Symbol refers to a class or struct.
 */
bool Symbol::isClass() const
{
    return qobject_cast<const ClassSymbol *>(this);
}

ClassSymbol *Symbol::toClass()
{
    LOG("Symbol::toClass");

    auto clazz = qobject_cast<ClassSymbol *>(this);

    if (!clazz)
        spdlog::warn("Symbol::toClass - {} should be a `Class`.", m_name.toStdString());

    return clazz;
}

/*!
 * \qmlmethod bool Symbol::isFunction()
 * \since 1.1
 *
 * Returns whether this Symbol refers to a function.
 * This includes constructors and methods
 */
bool Symbol::isFunction() const
{
    return qobject_cast<const CppFunctionSymbol *>(this);
}

CppFunctionSymbol *Symbol::toFunction()
{
    auto function = qobject_cast<CppFunctionSymbol *>(this);

    if (!function)
        spdlog::warn("Symbol::toFunction - {} should be either a method or a function.", m_name.toStdString());

    return function;
}

QString Symbol::name() const
{
    return m_name;
}

QString Symbol::description() const
{
    return m_description;
}

Symbol::Kind Symbol::kind() const
{
    return m_kind;
}

Core::TextRange Symbol::range() const
{
    return m_range;
}

Core::TextRange Symbol::selectionRange() const
{
    return m_selectionRange;
}

QVector<Core::TextLocation> Symbol::references() const
{
    LOG("Symbol::references");

    if (const auto lspdocument = document()) {
        auto references = lspdocument->references(selectionRange().start);
        kdalgorithms::erase_if(references, [this](const auto &reference) {
            return reference.range == this->selectionRange();
        });
        return references;
    }

    return {};
}

/*!
 * \qmlmethod bool Symbol::select()
 * \since 1.1
 *
 * Selects the current symbol.
 */
void Symbol::select()
{
    if (auto lspDocument = document()) {
        lspDocument->selectRange(selectionRange());
    }
}

bool Symbol::operator==(const Symbol &other) const
{
    return m_name == other.m_name && m_description == other.m_description && m_kind == other.m_kind
        && m_range == other.m_range && m_selectionRange == other.m_selectionRange;
}

} // namespace Core
