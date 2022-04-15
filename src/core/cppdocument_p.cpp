#include "cppdocument_p.h"
#include "cppdocument.h"

#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QTextDocument>

#include <spdlog/spdlog.h>

#include <algorithm>

namespace Core {

bool CppCache::Include::operator==(const Include &other) const
{
    return name == other.name && scope == other.scope;
}

CppCache::CppCache(CppDocument *document)
    : m_document(document)
{
}

void CppCache::clear()
{
    m_includeGroups.clear();
    m_includes.clear();
    m_flags = 0;
}

static QStringView getCommonPrefix(QString const &s1, QString const &s2)
{
    auto it1 = std::ranges::mismatch(s1, s2).in1;
    auto common = QStringView(s1.begin(), it1);
    return common;
}

std::optional<CppCache::IncludePosition> CppCache::includePositionForInsertion(const QString &text, bool addNewGroup)
{
    // Not a well formed include => error
    auto include = includeForText(text);
    if (include.isNull())
        return {};

    // If there are no includes, return the first line
    computeIncludes();
    if (m_includes.empty())
        return IncludePosition {0, false};

    if (findInclude(include) != m_includes.end())
        return IncludePosition {};

    auto it = findBestIncludeGroup(include);
    if (addNewGroup || it == m_includeGroups.cend())
        return IncludePosition {m_includes.back().line + 1, true};
    return IncludePosition {it->lastLine + 1, false};
}

std::optional<int> CppCache::includePositionForRemoval(const QString &text)
{
    // Not a well formed include => error
    auto include = includeForText(text);
    if (include.isNull())
        return {};

    computeIncludes();
    auto it = findInclude(include);
    if (it == m_includes.end())
        return -1;
    return it->line;
}

CppCache::Include CppCache::includeForText(const QString &text) const
{
    if ((!text.startsWith('<') || !text.endsWith('>')) && (!text.startsWith('"') || !text.endsWith('"')))
        return {};
    return {text.mid(1, text.length() - 2), text.startsWith('"') ? CppCache::Include::Local : CppCache::Include::Global,
            -1};
}

CppCache::Includes::const_iterator CppCache::findInclude(const Include &include) const
{
    return std::ranges::find(m_includes, include);
}

CppCache::IncludeGroups::const_iterator CppCache::findBestIncludeGroup(const Include &include)
{
    auto itEnd = m_includeGroups.cend();
    int commonLength = 0;
    auto bestIt = itEnd;
    for (auto it = m_includeGroups.cbegin(); it != itEnd; ++it) {
        if (!(it->scope & include.scope))
            continue;
        int prefixLength = getCommonPrefix(it->prefix, include.name).length();
        if (commonLength <= prefixLength) {
            bestIt = it;
            commonLength = prefixLength;
        }
    }
    return bestIt;
}

void CppCache::computeIncludes()
{
    if (m_flags & HasIncludes)
        return;

    static auto regexp = QRegularExpression(R"(^#include\s+(<\S+>|"\S+"))");
    auto document = m_document->textEdit()->document();
    QTextCursor cursor(document);

    // Extract all includes
    int line = 1;
    bool newGroup = true;

    while (!cursor.isNull()) {
        cursor.select(QTextCursor::SelectionType::LineUnderCursor);
        const QString &text = cursor.selectedText().simplified();
        if (text.startsWith("#include")) {
            auto match = regexp.match(text);
            if (match.hasMatch()) {
                auto include = includeForText(match.captured(1));
                include.line = line;
                m_includes.push_back(include);
                if (newGroup) {
                    IncludeGroup group;
                    group.first = static_cast<int>(m_includes.size() - 1);
                    m_includeGroups.push_back(group);
                    newGroup = false;
                }
                m_includeGroups.back().last = static_cast<int>(m_includes.size() - 1);
                m_includeGroups.back().lastLine = line;
            }
        } else {
            newGroup = true;
        }

        if (!cursor.movePosition(QTextCursor::Down))
            break;
        ++line;
    }

    // Find common denominator in groups
    auto processGroup = [this](IncludeGroup &group) {
        group.prefix = m_includes.at(group.first).name;
        for (int i = group.first; i <= group.last; ++i) {
            const auto &include = m_includes.at(i);
            group.scope |= include.scope;
            group.prefix = getCommonPrefix(group.prefix, include.name).toString();
        }
    };
    for (auto &group : m_includeGroups)
        processGroup(group);

    m_flags |= HasIncludes;
    return;
}

}
