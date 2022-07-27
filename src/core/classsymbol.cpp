#include "classsymbol.h"

#include "lspdocument.h"
#include <algorithm>
#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype CppClass
 * \brief Represents a class in the current file
 * \instantiates Core::CppClass
 * \inqmlmodule Script
 * \since 4.0
 * \todo
 */

ClassSymbol::ClassSymbol(QObject *parent, const QString &name, const QString &description, Kind kind, TextRange range,
                         TextRange selectionRange)
    : Symbol(parent, name, description, kind, range, selectionRange)
    , m_members()
{
}

QVector<Symbol *> ClassSymbol::findMembers() const
{
    if (auto lspDocument = qobject_cast<Core::LspDocument *>(parent())) {
        QVector<Symbol *> members;
        for (auto &symbol : lspDocument->symbols()) {
            if (m_range.contains(symbol->range()) && m_name != symbol->name())
                members.append(symbol);
        }
        return members;
    }
    spdlog::warn("Parent of CppClass {} is not an LspDocument!", m_name.toStdString());
    return {};
}

const QVector<Symbol *> &ClassSymbol::members() const
{
    if (!m_members.has_value()) {
        m_members = {findMembers()};
    }
    return *m_members;
}

/*!
 * \qmlproperty vector<Symbol> CppClass::members
 * Returns the list of members (both data and functions) of this class.
 */

bool operator==(const ClassSymbol &left, const ClassSymbol &right)
{
    return left.Symbol::operator==(right) && left.members() == right.members();
}
} // namespace Core
