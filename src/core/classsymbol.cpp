/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "classsymbol.h"
#include "codedocument.h"
#include "utils/log.h"

namespace Core {

/*!
 * \qmltype ClassSymbol
 * \brief Represents a class in the current file
 * \inqmlmodule Knut
 * \ingroup CodeDocument
 * \todo
 */
/*!
 * \qmlproperty vector<Symbol> ClassSymbol::members
 * Returns the list of members (both data and functions) of this class.
 */

ClassSymbol::ClassSymbol(QObject *parent, const QueryMatch &match, Kind kind)
    : Symbol(parent, match, kind)
{
}

QList<Symbol *> ClassSymbol::findMembers() const
{
    if (auto codeDocument = qobject_cast<Core::CodeDocument *>(parent())) {
        QList<Symbol *> members;
        for (auto &symbol : codeDocument->symbols()) {
            if (m_range.contains(symbol->range()) && m_name != symbol->name())
                members.append(symbol);
        }
        return members;
    }
    spdlog::warn("Parent of CppClass {} is not an CodeDocument!", m_name);
    return {};
}

const QList<Symbol *> &ClassSymbol::members() const
{
    if (!m_members.has_value()) {
        m_members = {findMembers()};
    }
    return *m_members;
}

QString ClassSymbol::description() const
{
    return "Class with " + QString::number(members().size()) + " members";
}

bool operator==(const ClassSymbol &left, const ClassSymbol &right)
{
    return left.Symbol::operator==(right) && left.members() == right.members();
}
} // namespace Core
