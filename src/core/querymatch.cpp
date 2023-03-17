#include "querymatch.h"

#include "lspdocument.h"
#include "rangemark.h"
#include "textdocument.h"

#include <kdalgorithms.h>
#include <spdlog/spdlog.h>
#include <treesitter/query.h>

#include <QJSEngine>

namespace Core {

/*!
 * \qmltype QueryCapture
 * \brief Defines a capture made by a query.
 * \inqmlmodule Script
 * \since 1.1
 * \sa QueryMatch
 */

/*!
 * \qmlproperty string QueryCapture::name
 * Name of the capture inside the query.
 */
/*!
 * \qmlproperty RangeMark QueryCapture::range
 * This read-only property contains the range of the capture in the document.
 */

QString QueryCapture::toString() const
{
    return QString("QueryCapture{'%1', %2}").arg(name, range.toString());
}

/*!
 * \qmltype QueryMatch
 * \brief Contains all matches for a query.
 * \inqmlmodule Script
 * \since 1.1
 * \sa LspDocument::query
 *
 * The QueryMatch object allows you to get access to all the captures made by the query.
 * The query is using the [TreeSitter](https://tree-sitter.github.io/tree-sitter/) query, you can find more information
 * on this page: [Pattern Matching with
 * Queries](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries).
 */

/*!
 * \qmlproperty array<QueryCapture> QueryMatch::captures
 * List of all the captures in the current document.
 */

QueryMatch::QueryMatch(TextDocument &document, const treesitter::QueryMatch &match)
{
    const auto captures = match.captures();
    for (const auto &capture : captures) {
        auto name = match.query()->captureAt(capture.id).name;

        const auto &node = capture.node;
        auto range = document.createRangeMark(node.startPosition(), node.endPosition());

        m_captures.emplace_back(QueryCapture {.name = name, .range = range});
    }
}

const QVector<QueryCapture> &QueryMatch::captures() const
{
    return m_captures;
}

/*!
 * \qmlmethod RangeMark QueryMatch::getAll(string name)
 * Returns all matches for the query with the given `name`
 */
QVector<RangeMark> QueryMatch::getAll(const QString &name) const
{
    QVector<RangeMark> result;

    for (const auto &capture : m_captures) {
        if (capture.name == name)
            result.emplace_back(capture.range);
    }

    return result;
}

/*!
 * \qmlmethod RangeMark QueryMatch::getAllInRange(string name, RangeMark range)
 * Returns all matches for the query with the given `name` in the given `range`.
 */
Q_INVOKABLE QVector<Core::RangeMark> QueryMatch::getAllInRange(const QString &name, const Core::RangeMark &range) const
{
    auto captureMatch = [&name, &range](const QueryCapture &capture) {
        return capture.name == name && range.contains(capture.range);
    };
    auto toRange = [](const QueryCapture &capture) {
        return capture.range;
    };
    return kdalgorithms::filtered_transformed(m_captures, toRange, captureMatch);
}

/*!
 * \qmlmethod RangeMark QueryMatch::get(string name)
 * Returns the first match for the query with the given `name`.
 */
RangeMark QueryMatch::get(const QString &name) const
{
    for (const auto &capture : m_captures) {
        if (capture.name == name)
            return capture.range;
    }

    return RangeMark();
}

/*!
 * \qmlmethod RangeMark QueryMatch::getInRange(string name, RangeMark range)
 * Returns the first match for the query with the given `name` in the given `range`
 */
Q_INVOKABLE Core::RangeMark QueryMatch::getInRange(const QString &name, const Core::RangeMark &range) const
{
    auto captureMatch = [&name, &range](const QueryCapture &capture) {
        return capture.name == name && range.contains(capture.range);
    };
    auto result = kdalgorithms::find_if(m_captures, captureMatch);
    if (result)
        return result->range;
    return {};
}

/*!
 * \qmlmethod RangeMark QueryMatch::getAllJoined(string name)
 * Returns a smallest range that contains all captures for the given `name`.
 */
RangeMark QueryMatch::getAllJoined(const QString &name) const
{
    auto ranges = getAll(name);

    if (ranges.isEmpty())
        return RangeMark();

    return kdalgorithms::accumulate(ranges, &RangeMark::join, ranges.at(0));
}

/**
 * \qmlmethod array<QueryMatch> QueryMatch::queryIn(capture, query)
 * \param capture The name of the capture to query in
 * \param query The treesitter query to run
 *
 * Executes the treesitter `query` on all nodes that were captured under the `capture` name.
 *
 * This is useful if you want to query for nodes that might be nested arbitrarily deeply within a larger construct.
 * E.g. searching for all "return" statements within a function, no matter how deep they are nested.
 * \sa LspDocument::query
 */
QVector<QueryMatch> QueryMatch::queryIn(const QString &capture, const QString &query) const
{
    QVector<QueryMatch> result;

    auto ranges = getAll(capture);
    for (const auto &range : ranges) {
        auto document = qobject_cast<LspDocument *>(range.document());
        if (document) {
            result.append(document->queryInRange(range, query));
        } else {
            spdlog::warn("QueryMatch::queryIn: RangeMark is not backed by LspDocument!");
        }
    }

    return result;
}

QString QueryMatch::toString() const
{
    return QString("QueryMatch{%1}").arg(m_captures.size());
}

} // namespace Core
