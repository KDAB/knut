#pragma once

#include "node.h"

#include <QByteArray>
#include <QString>
#include <QVector>
#include <functional>
#include <tree_sitter/api.h>

struct TSLanguage;
struct TSQuery;
struct TSQueryCursor;

namespace treesitter {

class Node;
class Predicates;

class Query
{
public:
    struct Capture
    {
        QString name;
        uint32_t id;
    };

    struct Predicate
    {
        QString name;
        QVector<std::variant<Capture, QString>> arguments;
    };

    struct Pattern
    {
        QVector<Predicate> predicates;
        uint32_t utf8_start_byte;
    };

    struct Error
    {
        uint32_t utf8_offset;
        QString description;
    };

    // throws a Query::Error if the query is ill-formed.
    Query(const TSLanguage *language, const QString &query);

    Query(const Query &) = delete;
    Query(Query &&) noexcept;

    ~Query();

    Query &operator=(const Query &) = delete;
    Query &operator=(Query &&) noexcept;

    void swap(Query &other) noexcept;

    QVector<Pattern> patterns() const;

    QVector<Capture> captures() const;
    Capture captureAt(uint32_t index) const;

private:
    QVector<Predicate> predicatesForPattern(uint32_t index) const;

    QByteArray m_utf8_text;
    TSQuery *m_query;

    friend class QueryCursor;
};

// TODO: Should this also be a member-class of Query?
class QueryMatch
{
public:
    struct Capture
    {
        uint32_t id;
        Node node;
    };

    QueryMatch(const QueryMatch &) = default;
    QueryMatch(QueryMatch &&) = default;

    QueryMatch &operator=(const QueryMatch &) = default;
    QueryMatch &operator=(QueryMatch &&) = default;

    uint32_t id() const;
    uint32_t patternIndex() const;

    void setCaptures(QVector<Capture> &&captures);
    QVector<Capture> captures() const;
    QVector<Capture> capturesWithId(uint32_t id) const;

    // Captures with quantifiers may return multiple values for the same capture.
    QVector<Capture> capturesNamed(const QString &) const;

    std::shared_ptr<Query> query() const;

private:
    QueryMatch(const TSQueryMatch &match, std::shared_ptr<Query> query);

    uint32_t m_id;
    uint16_t m_pattern_index;
    QVector<Capture> m_captures;

    std::shared_ptr<Query> m_query;

    friend class QueryCursor;
};

// TODO: Should this also be a member-class of Query?
class QueryCursor
{
public:
    QueryCursor();

    QueryCursor(const QueryCursor &) = delete;
    QueryCursor(QueryCursor &&) noexcept;

    QueryCursor &operator=(const QueryCursor &) = delete;
    QueryCursor &operator=(QueryCursor &&) noexcept;

    ~QueryCursor();

    void swap(QueryCursor &other) noexcept;

    void execute(const std::shared_ptr<Query> &query, const Node &node, std::unique_ptr<Predicates> &&predicates);

    std::optional<QueryMatch> nextMatch();

    // Get all remaining matches.
    // This will consume the cursor and nextMatch()
    // will no longer return new matches.
    QVector<QueryMatch> allRemainingMatches();

    // The progress callback is called after each match is found, even if it is discarded later by the predicate engine.
    // It allows the UI to update and remain responsive while the query is running.
    void setProgressCallback(std::function<void()> callback);

private:
    // The query must be kept alive for as long as the cursor is alive.
    // Otherwise, no new matches can be returned and the Predicates can't be executed.
    std::shared_ptr<Query> m_query;
    std::function<void()> m_progressCallback;

    std::unique_ptr<Predicates> m_predicates;
    TSQueryCursor *m_cursor;
};

using QueryList = QVector<std::shared_ptr<Query>>;

}
