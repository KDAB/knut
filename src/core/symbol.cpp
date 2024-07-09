/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "symbol.h"
#include "classsymbol.h"
#include "codedocument.h"
#include "functionsymbol.h"
#include "logger.h"
#include "project.h"
#include "utils/log.h"

#include <kdalgorithms.h>

namespace Core {

/*!
 * \qmltype Symbol
 * \brief Represent a symbol in the current file
 * \inqmlmodule Knut
 * \ingroup CodeDocument
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

Symbol::Symbol(QObject *parent, const QueryMatch &match, Kind kind)
    : QObject(parent)
    , m_name {match.get("name").text()}
    , m_kind {kind}
    , m_range {match.get("range").toTextRange()}
    , m_selectionRange {match.get("selectionRange").toTextRange()}
    , m_queryMatch {match}
{
}

Symbol *Symbol::makeSymbol(QObject *parent, const QueryMatch &match, Kind kind)
{

    if (kind == Method || kind == Function || kind == Constructor) {
        return new FunctionSymbol(parent, match, kind);
    }
    if (kind == Class || kind == Struct) {
        return new ClassSymbol(parent, match, kind);
    }
    return new Symbol(parent, match, kind);
}

void Symbol::assignContext(const QList<Symbol *> &contexts)
{
    auto names = kdalgorithms::transformed<QStringList>(contexts, &Symbol::name);
    if (!names.isEmpty()) {
        m_name = names.join("::") + "::" + m_name;
    }
    auto is_class = [](const auto &symbol) {
        return symbol->kind() == Kind::Class;
    };
    if (m_kind == Kind::Function && kdalgorithms::any_of(contexts, is_class)) {
        m_kind = Kind::Method;
    }
}

CodeDocument *Symbol::document() const
{
    return qobject_cast<CodeDocument *>(parent());
}

/*!
 * \qmlmethod bool Symbol::isClass()
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
        spdlog::warn("Symbol::toClass - {} should be a `Class`.", m_name);

    return clazz;
}

/*!
 * \qmlmethod bool Symbol::isFunction()
 *
 * Returns whether this Symbol refers to a function.
 * This includes constructors and methods
 */
bool Symbol::isFunction() const
{
    return qobject_cast<const FunctionSymbol *>(this);
}

FunctionSymbol *Symbol::toFunction()
{
    auto function = qobject_cast<FunctionSymbol *>(this);

    if (!function)
        spdlog::warn("Symbol::toFunction - {} should be either a method or a function.", m_name);

    return function;
}

QString Symbol::name() const
{
    return m_name;
}

QString Symbol::description() const
{
    return "";
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

QList<Core::TextLocation> Symbol::references() const
{
    LOG("Symbol::references");

    if (const auto codedocument = document()) {
        auto references = codedocument->references(selectionRange().start);
        kdalgorithms::erase_if(references, [this](const auto &reference) {
            return reference.range == this->selectionRange();
        });
        return references;
    }

    return {};
}

/*!
 * \qmlmethod bool Symbol::select()
 *
 * Selects the current symbol.
 */
void Symbol::select()
{
    if (auto codeDocument = document()) {
        codeDocument->selectRange(selectionRange());
    }
}

bool Symbol::operator==(const Symbol &other) const
{
    return m_name == other.m_name && m_kind == other.m_kind && m_range == other.m_range
        && m_selectionRange == other.m_selectionRange;
}

} // namespace Core
