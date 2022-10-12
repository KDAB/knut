#pragma once

#include <tree_sitter/api.h>

#include "node.h"

#include <QByteArray>
#include <QString>
#include <QVector>

struct TSLanguage;
struct TSQuery;
struct TSQueryCursor;

namespace treesitter {

class Node;

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
    Query(TSLanguage *language, const QString &query);

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

    Query(TSQuery *query);

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

    QVector<Capture> captures() const;

private:
    QueryMatch(const TSQueryMatch &match);

    TSQueryMatch m_match;

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

    void execute(const Query &query, const Node &node);

    std::optional<QueryMatch> nextMatch();

private:
    TSQueryCursor *m_cursor;
};

}
