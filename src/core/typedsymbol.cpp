/*
 This file is part of Knut.

 SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

 SPDX-License-Identifier: GPL-3.0-only

 Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "typedsymbol.h"

#include <kdalgorithms.h>

namespace Core {

/*!
 * \qmltype TypedSymbol
 * \brief Represents a symbol with a type
 * \ingroup CodeDocument
 *
 * This symbol has a type associated with it, like a variable or a member of a class.
 */

/*!
 * \qmlproperty string TypedSymbol::type
 * The type of this symbol.
 *
 * The type is the part of the symbol that describes what kind of value it holds. For example, the type of a variable.
 * This symbol will extract the type as-written in the original source, but with whitespace
 * [simplified](https://doc.qt.io/qt-6/qstring.html#simplified). So if e.g. the source code is `const string  &`, it
 * will be extracted as `const string &`. The type will **not** be resolved to a fully qualified path (like
 * `std::string` for the previous example).
 *
 * This property is read-only.
 */

TypedSymbol::TypedSymbol(QObject *parent, const QueryMatch &match, Kind kind)
    : Symbol(parent, match, kind)
{
    // C++ has a strange way of parsing, where pointer and reference specifiers are parsed as a parent
    // of the identifier, and not part of the type. The easiest way to remedy this is to get the whole
    // range spanning the type and name and then erase the name.
    auto typeAndName = match.getAllJoined("typeAndName");
    if (typeAndName.isValid()) {
        m_type = typeAndName.textExcept(match.get("name")).simplified();
    } else {
        m_type = kdalgorithms::transformed(match.getAll("type"), &RangeMark::text).join(" ").simplified();
    }
}

QString TypedSymbol::type() const
{
    return m_type;
}

}
