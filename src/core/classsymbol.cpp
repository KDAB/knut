#include "classsymbol.h"

#include "lspdocument.h"
#include <algorithm>
#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype ClassSymbol
 * \brief Represents a class in the current file
 * \inqmlmodule Script
 * \ingroup LspDocument
 * \since 1.0
 * \todo
 */
/*!
 * \qmlproperty vector<Symbol> ClassSymbol::members
 * Returns the list of members (both data and functions) of this class.
 */

ClassSymbol::ClassSymbol(QObject *parent, const QString &name, const QString &description,
                         const QString &importLocation, Kind kind, TextRange range, TextRange selectionRange)
    : Symbol(parent, name, description, importLocation, kind, range, selectionRange)
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

bool operator==(const ClassSymbol &left, const ClassSymbol &right)
{
    return left.Symbol::operator==(right) && left.members() == right.members();
}
} // namespace Core
