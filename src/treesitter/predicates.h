/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "node.h"
#include "query.h"

#include <QString>

namespace treesitter {

// a common superclass for all caches to make sure the
// destructors are correctly dispatched with virtual destructor calls.
class PredicateCache
{
public:
    PredicateCache() = default;
    virtual ~PredicateCache() = default;
};

// At the moment, predicates are just member functions of the Predicates class.
// However, in the future we may want to separate the Predicates class into two:
// 1. A PredicateList class, containing a list of predicates, but no context for the predicates to execute
//      This could be passed to a Query during its construction to check for the supported predicates
//      Should also be stored by the Query for later access.
// 2. A PredicateContext class, containing the source string, node and any other per-cursor data (like the caches)
//      This would replace the existing Predicates class to support execution of the predicates.
class Predicates
{
    using PredicateArguments = QVector<std::variant<Query::Capture, QString>>;
    struct Filters
    {
        std::unordered_map<QString, bool (Predicates::*)(const QueryMatch &, const PredicateArguments &) const>
            filterFunctions;

        std::unordered_map<QString, std::optional<QString> (*)(const PredicateArguments &)> checkFunctions;
    };

    struct Commands
    {
        std::unordered_map<QString, std::optional<QString> (*)(const PredicateArguments &)> checkFunctions;
        std::unordered_map<QString, void (Predicates::*)(QueryMatch &, const PredicateArguments &) const>
            commandFunctions;
    };

    static Filters filters();
    static Commands commands();

public:
    explicit Predicates(QString source);

    // Returns an error message if the predicate is not supported
    static std::optional<QString> checkPredicate(const Query::Predicate &predicate);

    // Executes all command-predicates (e.g. exclude!) on the match.
    void executeCommands(QueryMatch &match) const;

    // Returns true if the match fulfills all query predicates.
    bool filterMatch(const QueryMatch &match) const;

private:
    // ################# Commands #########################
#define PREDICATE_COMMAND(NAME)                                                                                        \
    void command_##NAME(QueryMatch &match, const PredicateArguments &arguments) const;                                 \
    static std::optional<QString> checkCommand_##NAME(const PredicateArguments &arguments);

    PREDICATE_COMMAND(exclude)
#undef PREDCIATE_COMMAND

    // ################## Filters #########################
#define PREDICATE_FILTER(NAME)                                                                                         \
    bool filter_##NAME(const QueryMatch &match, const PredicateArguments &arguments) const;                            \
    static std::optional<QString> checkFilter_##NAME(const PredicateArguments &arguments)

    PREDICATE_FILTER(eq);
    PREDICATE_FILTER(eq_except);
    PREDICATE_FILTER(like);
    PREDICATE_FILTER(like_except);
    PREDICATE_FILTER(match);
    PREDICATE_FILTER(in_message_map);
    PREDICATE_FILTER(not_is);
#undef PREDICATE_FILTER

    bool filter_eq_with(const QueryMatch &match, const QVector<std::variant<Query::Capture, QString>> &arguments,
                        const std::function<QString(const QString &)> &textTransform) const;
    bool filter_eq_except_with(const QueryMatch &match, const QVector<std::variant<Query::Capture, QString>> &arguments,
                               const std::function<QString(const QString &)> &textTransform) const;

    // ################## Argument matching #########################
    // Marker type indicating a capture is missing
    struct MissingCapture
    {
        Query::Capture capture;
    };

    QVector<std::variant<QString, QueryMatch::Capture, MissingCapture>>
    matchArguments(const QueryMatch &match, const PredicateArguments &arguments) const;

    // ################## Caches #########################
    mutable std::vector<std::unique_ptr<PredicateCache>> m_caches;

    template <class T>
    T *findCache() const
    {
        for (auto &cache : m_caches) {
            if (auto *result = dynamic_cast<T *>(cache.get())) {
                return result;
            }
        }
        return nullptr;
    }

    void insertCache(std::unique_ptr<PredicateCache>) const;

    void findMessageMap() const;

    // ################## Context data #########################
    friend class QueryCursor;
    void setRootNode(const Node &node);

    const QString m_source;
    std::optional<Node> m_rootNode;
};

}
