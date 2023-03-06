#include "cppdocument_p.h"
#include "cppdocument.h"

#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QTextDocument>

#include <spdlog/spdlog.h>

#include <algorithm>

namespace Core {

bool IncludeHelper::Include::operator==(const Include &other) const
{
    return name == other.name && scope == other.scope;
}

IncludeHelper::IncludeHelper(CppDocument *document)
    : m_document(document)
{
    computeIncludes();
}

static QStringView getCommonPrefix(QString const &s1, QString const &s2)
{
    auto it1 = std::ranges::mismatch(s1, s2).in1;
    auto common = QStringView(s1.begin(), it1);
    return common;
}

std::optional<IncludeHelper::IncludePosition> IncludeHelper::includePositionForInsertion(const QString &text,
                                                                                         bool addNewGroup)
{
    // Not a well formed include => error
    auto include = includeForText(text);
    if (include.isNull())
        return {};

    // If there are no includes, return the first line
    computeIncludes();
    if (m_includes.empty())
        return findBestFirstIncludeLine();

    if (findInclude(include) != m_includes.end())
        return IncludePosition {};

    auto it = findBestIncludeGroup(include);
    if (addNewGroup || it == m_includeGroups.cend())
        return IncludePosition {m_includes.back().line + 1, true};
    return IncludePosition {it->lastLine + 1, false};
}

std::optional<int> IncludeHelper::includePositionForRemoval(const QString &text)
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

IncludeHelper::Include IncludeHelper::includeForText(const QString &text) const
{
    if ((!text.startsWith('<') || !text.endsWith('>')) && (!text.startsWith('"') || !text.endsWith('"')))
        return {};
    return {text.mid(1, text.length() - 2),
            text.startsWith('"') ? IncludeHelper::Include::Local : IncludeHelper::Include::Global, -1};
}

IncludeHelper::Includes::const_iterator IncludeHelper::findInclude(const Include &include) const
{
    return std::ranges::find(m_includes, include);
}

IncludeHelper::IncludeGroups::const_iterator IncludeHelper::findBestIncludeGroup(const Include &include) const
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

IncludeHelper::IncludePosition IncludeHelper::findBestFirstIncludeLine() const
{
    if (!m_document->isHeader())
        return IncludePosition {1, false};

    // Find `#pragma once`
    const auto pragmaQuery = QString(R"EOF(
        (translation_unit
            (preproc_call
                argument: (_) @value (#match? "once" @value)
            )
        )
    )EOF");

    auto result = m_document->query(pragmaQuery);
    if (result.isEmpty()) {
        // Find `#ifndef / #define`
        const auto guardsQuery = QString(R"EOF(
            (translation_unit
                (preproc_ifdef
                    "#ifndef"
                    name: (_) @name
                    (preproc_def
                        name: (_) @value (#eq? @name @value)
                    )
                )
            )
        )EOF");
        result = m_document->query(guardsQuery);
        if (result.isEmpty())
            return IncludePosition {1, false};
    }

    const auto &match = result.first();
    auto codeStart = match.get("value");

    int line;
    int col;
    m_document->convertPosition(codeStart.end(), &line, &col);

    return IncludePosition {line + 1, true};
}

void IncludeHelper::computeIncludes()
{
    const auto includeQuery = QString(R"EOF(
        (preproc_include
            path: (_) @path
        )
    )EOF");

    auto results = m_document->query(includeQuery);

    // Extract all includes
    int lastLine = -1;

    for (const auto &match : results) {
        auto includePath = match.get("path");
        int line; // 1-based
        int col;
        m_document->convertPosition(includePath.end(), &line, &col);

        auto include = includeForText(includePath.text());
        include.line = line;
        m_includes.push_back(include);
        if (line != lastLine + 1 || m_includeGroups.empty()) {
            IncludeGroup group;
            group.first = static_cast<int>(m_includes.size() - 1);
            m_includeGroups.push_back(group);
        }
        m_includeGroups.back().last = static_cast<int>(m_includes.size() - 1);
        m_includeGroups.back().lastLine = line;
        lastLine = line;
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

    return;
}
}
