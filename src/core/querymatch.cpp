#include "querymatch.h"

#include "rangemark.h"
#include "textdocument.h"

#include <treesitter/query.h>

#include <QJSEngine>

namespace Core {

QString QueryCapture::toString() const
{
    return QString("QueryCapture{'%1', %2}").arg(name).arg(range.toString());
}

QueryMatch::QueryMatch(TextDocument &document, const treesitter::QueryMatch &match)
{
    Q_UNUSED(document);

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

QVector<RangeMark> QueryMatch::getAll(const QString &name) const
{
    QVector<RangeMark> result;

    for (const auto &capture : m_captures) {
        if (capture.name == name)
            result.emplace_back(capture.range);
    }

    return result;
}

RangeMark QueryMatch::get(const QString &name) const
{
    for (const auto &capture : m_captures) {
        if (capture.name == name)
            return capture.range;
    }

    return RangeMark();
}

QString QueryMatch::toString() const
{
    return QString("QueryMatch{%1}").arg(m_captures.size());
}

} // namespace Core
