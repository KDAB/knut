#include "query.h"

#include "node.h"

#include <tree_sitter/api.h>

#include <QStringList>

namespace treesitter {

Query::Query(TSLanguage *language, const QString &query)
    : m_query(nullptr)
{
    m_utf8_text = query.toUtf8();
    uint32_t error_offset;
    TSQueryError error_type;
    m_query = ts_query_new(language, m_utf8_text.data(), m_utf8_text.size(), &error_offset, &error_type);
    if (!m_query) {
        static QStringList errorNames {"No Error",      "Syntax Error",  "Invalid node type",
                                       "Invalid field", "Capture Error", "Structure Error"};

        auto description = error_type < errorNames.size() ? errorNames.at(error_type) : "Unknown Error";
        throw Error {
            .utf8_offset = error_offset,
            .description = description,
        };
    }
}

Query::Query(Query &&other) noexcept
    : m_query(other.m_query)
{
    other.m_query = nullptr;
}

Query::~Query()
{
    if (m_query) {
        ts_query_delete(m_query);
    }
}

Query &Query::operator=(Query &&other) noexcept
{
    Query(std::move(other)).swap(*this);
    return *this;
}

void Query::swap(Query &other) noexcept
{
    std::swap(m_query, other.m_query);
}

QVector<Query::Predicate> Query::predicatesForPattern(uint32_t index) const
{
    uint32_t predicatesLength;
    const auto predicateSteps = ts_query_predicates_for_pattern(m_query, index, &predicatesLength);

    QVector<Query::Predicate> predicates;
    Predicate predicate {.name = QString(), .arguments = QVector<std::variant<Capture, QString>> {}};
    for (uint32_t predicateIndex = 0; predicateIndex < predicatesLength; ++predicateIndex) {
        const auto &step = predicateSteps[predicateIndex];
        // We don't really need the length here, but TreeSitter crashes if we give it a nullptr.
        uint32_t length;

        switch (step.type) {
        case TSQueryPredicateStepTypeString:
            // The name of the predicate is always the first PredicateStepTypeString.
            if (predicate.name.isEmpty()) {
                predicate.name = ts_query_string_value_for_id(m_query, step.value_id, &length);
            } else {
                predicate.arguments.emplace_back(std::in_place_type<QString>,
                                                 ts_query_string_value_for_id(m_query, step.value_id, &length));
            }
            break;
        case TSQueryPredicateStepTypeCapture:
            predicate.arguments.emplace_back(std::in_place_type<Capture>, captureAt(step.value_id));
            break;
        case TSQueryPredicateStepTypeDone:
            predicates.emplace_back(std::move(predicate));
            predicate = Predicate {.name = QString(), .arguments = QVector<std::variant<Capture, QString>>()};
            break;
        }
    }

    return predicates;
}

QVector<Query::Pattern> Query::patterns() const
{
    QVector<Query::Pattern> result;

    auto count = ts_query_pattern_count(m_query);
    result.reserve(count);
    for (uint32_t patternIndex = 0; patternIndex < count; ++patternIndex) {
        auto start_byte = ts_query_start_byte_for_pattern(m_query, patternIndex);
        auto predicates = predicatesForPattern(patternIndex);

        result.emplace_back(Pattern {.predicates = std::move(predicates), .utf8_start_byte = start_byte});
    }
    return result;
}

QVector<Query::Capture> Query::captures() const
{
    uint32_t length = ts_query_capture_count(m_query);

    QVector<Query::Capture> results;
    for (uint32_t i = 0; i < length; i++) {
        results.emplace_back(captureAt(i));
    }
    return results;
}

Query::Capture Query::captureAt(uint32_t index) const
{
    uint32_t length;
    auto name = ts_query_capture_name_for_id(m_query, index, &length);

    return Capture {.name = QString(name), .id = index};
}

// ------------------------ QueryMatch --------------------
QueryMatch::QueryMatch(const TSQueryMatch &match)
    : m_match(match)
{
}

uint32_t QueryMatch::id() const
{
    return m_match.id;
}

uint32_t QueryMatch::patternIndex() const
{
    return m_match.pattern_index;
}

QVector<QueryMatch::Capture> QueryMatch::captures() const
{
    QVector<Capture> result;
    result.reserve(m_match.capture_count);

    for (uint16_t i = 0; i < m_match.capture_count; ++i) {
        auto &ts_capture = m_match.captures[i];
        Capture capture {
            .id = ts_capture.index,
            .node = Node(ts_capture.node),
        };
        result.emplace_back(std::move(capture));
    }
    return result;
}

// ----------------------- QueryCursor --------------------
QueryCursor::QueryCursor()
    : m_cursor(ts_query_cursor_new())
{
}

QueryCursor::~QueryCursor()
{
    if (m_cursor) {
        ts_query_cursor_delete(m_cursor);
    }
}

QueryCursor::QueryCursor(QueryCursor &&other) noexcept
    : m_cursor(std::move(other.m_cursor))
{
    other.m_cursor = nullptr;
}

QueryCursor &QueryCursor::operator=(QueryCursor &&other) noexcept
{
    QueryCursor(std::move(other)).swap(*this);
    return *this;
}

void QueryCursor::swap(QueryCursor &other) noexcept
{
    std::swap(m_cursor, other.m_cursor);
}

void QueryCursor::execute(const Query &query, const Node &node)
{
    ts_query_cursor_exec(m_cursor, query.m_query, node.m_node);
}

std::optional<QueryMatch> QueryCursor::nextMatch()
{
    TSQueryMatch match;
    if (ts_query_cursor_next_match(m_cursor, &match)) {
        return QueryMatch(match);
    }
    return {};
}

}
