/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "querymatch.h"
#include "codedocument.h"
#include "rangemark.h"
#include "textdocument.h"
#include "utils/log.h"

#include <QJSEngine>
#include <kdalgorithms.h>
#include <treesitter/query.h>

namespace Core {

/*!
 * \qmltype QueryCapture
 * \brief Defines a capture made by a query.
 * \inqmlmodule Knut
 * \ingroup CodeDocument
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
 * \brief Contains all captures for a query match.
 * \inqmlmodule Knut
 * \ingroup CodeDocument
 * \sa CodeDocument::query
 *
 * The QueryMatch object allows you to get access to all the captures made by a [Tree-sitter
 * query](https://tree-sitter.github.io/tree-sitter/using-parsers#pattern-matching-with-queries).
 *
 * Some high-level functions on CodeDocument and its subclasses also return QueryMatch instances.
 * Usually these functions list which captures their matches will include.
 *
 * !!! note
 *     If you expect a query will only return a single QueryMatch, you can uses Javascripts
 *     [destructuring assignment][destructuring] to easily get the right match:
 *     ``` javascript
 *     // Note the [] surrounding `match`
 *     let [match] = document.query("...");
 *     if (match) { // In case the query fails, match will be undefined.
 *         // ...
 *     }
 *     ```
 *  [destructuring]:
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Destructuring_assignment?retiredLocale=de
 */

/*!
 * \qmlproperty array<QueryCapture> QueryMatch::captures
 * List of all the captures in the current document.
 *
 * This allows you to get access to both the [range](./rangemark.md) and the name of the capture.
 *
 * !!! note
 *      Usually you won't need to access the captures directly.
 *      Instead prefer to use the getter functions.
 */
/*!
 * \qmlproperty bool QueryMatch::isEmpty
 * Return true if the `QueryMatch` is empty.
 */

QueryMatch::QueryMatch(TextDocument &document, const treesitter::QueryMatch &match)
{
    const auto captures = match.captures();
    for (const auto &capture : captures) {
        const auto name = match.query()->captureAt(capture.id).name;

        const auto &node = capture.node;
        const auto range = document.createRangeMark(node.startPosition(), node.endPosition());

        m_captures.emplace_back(QueryCapture {.name = name, .range = range});
    }
}

const QList<QueryCapture> &QueryMatch::captures() const
{
    return m_captures;
}

bool QueryMatch::isEmpty() const
{
    return m_captures.isEmpty();
}

/*!
 * \qmlmethod vector<RangeMark> QueryMatch::getAll(string name)
 * Returns all ranges that are covered by the captures of the given `name`
 */
Core::RangeMarkList QueryMatch::getAll(const QString &name) const
{
    Core::RangeMarkList result;

    for (const auto &capture : m_captures) {
        if (capture.name == name)
            result.emplace_back(capture.range);
    }

    return result;
}

/*!
 * \qmlmethod vector<RangeMark> QueryMatch::getAllInRange(string name, RangeMark range)
 * Returns all ranges that are covered by the captures of the given `name` in the given `range`.
 */
Core::RangeMarkList QueryMatch::getAllInRange(const QString &name, const Core::RangeMark &range) const
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
 * Returns the range covered by the first capture with the given `name`.
 *
 * This allows you to easily interact with a capture, if you know it will only cover a single node.
 * ``` javascript
 * let [function] = document.query("...");
 *
 * // Print the captured text
 * Message.log(match.get("parameter-list").text);
 * // Replace the captured text with something else
 * match.get("parameter-list").replace("(int myParameter)");
 * ```
 *
 * See the [RangeMark](rangemark.md) documentation for more information.
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
 * Returns the range covered by the first capture with the given `name` in the given `range`.
 */
Core::RangeMark QueryMatch::getInRange(const QString &name, const Core::RangeMark &range) const
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
 *
 * E.g. To search for all "return" statements within a function, no matter how deep they are nested:
 * ``` javascript
 * let [function] = document.query(`
 *      (function_definition
 *          declarator: (
 *              ; Some query to find a specific function
 *          )
 *          body: (compound_statement) @body)
 * `);
 * let return_statements = function.queryIn("body", "(return_statement) @return");
 * ```
 * \sa CodeDocument::query
 */
Core::QueryMatchList QueryMatch::queryIn(const QString &capture, const QString &query) const
{
    Core::QueryMatchList result;

    const auto ranges = getAll(capture);
    for (const auto &range : ranges) {
        auto document = qobject_cast<CodeDocument *>(range.document());
        if (document) {
            result.append(document->queryInRange(range, query));
        } else {
            spdlog::warn("QueryMatch::queryIn: RangeMark is not backed by CodeDocument!");
        }
    }

    return result;
}

QString QueryMatch::toString() const
{
    return QString("QueryMatch{%1}").arg(m_captures.size());
}

} // namespace Core
