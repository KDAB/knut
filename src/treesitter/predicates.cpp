#include "predicates.h"

#include "languages.h"

#include "kdalgorithms.h"
#include "spdlog/spdlog.h"

#include <ranges>
#include <set>

#include <QRegularExpression>

namespace treesitter {

QString QString_identity(const QString &string)
{
    return string;
}

QString QString_no_whitespace(const QString &string)
{
    auto simplified = string.simplified();
    simplified.replace(" ", "");
    return simplified;
}

Predicates::Filters Predicates::filters()
{
    Predicates::Filters filters;
#define REGISTER_FILTER(NAME)                                                                                          \
    filters.filterFunctions[#NAME "?"] = &Predicates::filter_##NAME;                                                   \
    filters.checkFunctions[#NAME "?"] = &Predicates::checkFilter_##NAME

    REGISTER_FILTER(eq);
    REGISTER_FILTER(eq_except);
    REGISTER_FILTER(like);
    REGISTER_FILTER(like_except);
    REGISTER_FILTER(match);
    REGISTER_FILTER(in_message_map);
    REGISTER_FILTER(not_is);
#undef REGISTER_FILTER

    return filters;
}

Predicates::Commands Predicates::commands()
{
    Predicates::Commands commands;

#define REGISTER_COMMAND(NAME)                                                                                         \
    commands.commandFunctions[#NAME "!"] = &Predicates::command_##NAME;                                                \
    commands.checkFunctions[#NAME "!"] = &Predicates::checkCommand_##NAME;

    REGISTER_COMMAND(exclude);
#undef REGISTER_COMMAND
    return commands;
}

std::optional<QString> Predicates::checkPredicate(const Query::Predicate &predicate)
{
    const auto filters = Predicates::filters();
    auto it = filters.checkFunctions.find(predicate.name);
    if (it != filters.checkFunctions.cend()) {
        return it->second(predicate.arguments);
    }

    const auto commands = Predicates::commands();
    it = commands.checkFunctions.find(predicate.name);
    if (it != commands.checkFunctions.cend()) {
        return it->second(predicate.arguments);
    }

    return "Unknown predicate";
}

Predicates::Predicates(QString source)
    : m_source(std::move(source))
{
}

void Predicates::executeCommands(QueryMatch &match) const
{
    const auto patterns = match.query()->patterns();
    const auto pattern = patterns.at(match.patternIndex());

    for (const auto &predicate : pattern.predicates) {
        const auto commands = Predicates::commands();
        const auto it = commands.commandFunctions.find(predicate.name);
        if (it != commands.commandFunctions.cend()) {
            const auto commandPredicate = it->second;
            (this->*(commandPredicate))(match, predicate.arguments);
        }
    }
}

bool Predicates::filterMatch(const QueryMatch &match) const
{
    const auto patterns = match.query()->patterns();
    const auto pattern = patterns.at(match.patternIndex());

    for (const auto &predicate : pattern.predicates) {
        const auto filters = Predicates::filters();
        const auto it = filters.filterFunctions.find(predicate.name);
        if (it != filters.filterFunctions.cend()) {
            const auto filterPredicate = it->second;
            if (!(this->*(filterPredicate))(match, predicate.arguments)) {
                return false;
            }
        }
    }

    return true;
}

std::optional<QString> Predicates::checkCommand_exclude(const Predicates::PredicateArguments &arguments)
{
    if (arguments.size() < 2) {
        return "Too few arguments";
    }

    bool has_capture = kdalgorithms::any_of(arguments, [](const auto &arg) {
        return std::holds_alternative<Query::Capture>(arg);
    });
    if (!has_capture) {
        return "No capture in arguments";
    }

    bool has_string = kdalgorithms::any_of(arguments, [](const auto &arg) {
        return std::holds_alternative<QString>(arg);
    });
    if (!has_string) {
        return "No string in arguments";
    }

    return {};
}

void Predicates::command_exclude(QueryMatch &match, const Predicates::PredicateArguments &arguments) const
{
    auto to_string = [](const auto &variant) {
        return std::get<QString>(variant);
    };
    auto is_string = [](const auto &variant) {
        return std::holds_alternative<QString>(variant);
    };
    auto exclusions = kdalgorithms::filtered_transformed<std::set>(arguments, to_string, is_string);

    auto to_capture_id = [](const auto &variant) {
        return std::get<treesitter::Query::Capture>(variant).id;
    };
    auto is_capture = [](const auto &variant) {
        return std::holds_alternative<treesitter::Query::Capture>(variant);
    };
    auto capture_ids = kdalgorithms::filtered_transformed<std::set>(arguments, to_capture_id, is_capture);

    auto new_captures = kdalgorithms::filtered(match.captures(), [&exclusions, &capture_ids](const auto &capture) {
        if (!capture_ids.contains(capture.id)) {
            return true;
        }

        return !exclusions.contains(capture.node.type());
    });

    match.setCaptures(std::move(new_captures));
}

std::optional<QString> Predicates::checkFilter_eq(const Predicates::PredicateArguments &arguments)
{
    if (arguments.size() < 2) {
        return "Too few arguments";
    }
    return {};
}
bool Predicates::filter_eq_with(const QueryMatch &match,
                                const QVector<std::variant<Query::Capture, QString>> &arguments,
                                const std::function<QString(const QString &)> &textTransform) const
{
    std::set<QString> texts;

    const auto matched = matchArguments(match, arguments);
    for (const auto &arg : matched) {
        if (const auto *capture = std::get_if<QueryMatch::Capture>(&arg)) {
            texts.emplace(textTransform(capture->node.textIn(m_source)));
        } else if (const auto *string = std::get_if<QString>(&arg)) {
            texts.emplace(textTransform(*string));
        } else if (std::holds_alternative<MissingCapture>(arg)) {
            spdlog::warn("Predicates: #eq? - Unmatched capture!");
            // Insert an empty string into the set if we find an unmatched capture.
            // This likely means we have encountered a quantified capture that matched 0 times.
            // By inserting an empty string, we can check that all other things are also "empty".
            texts.emplace("");
        } else {
            spdlog::warn("Predicates: #eq? - Impossible argument type!");
            return false;
        }
    }
    return texts.size() == 1;
}

bool Predicates::filter_eq(const QueryMatch &match,
                           const QVector<std::variant<Query::Capture, QString>> &arguments) const
{
    return filter_eq_with(match, arguments, QString_identity);
}

std::optional<QString> Predicates::checkFilter_eq_except(const Predicates::PredicateArguments &arguments)
{
    if (arguments.size() < 3) {
        return "Too few arguments";
    }
    auto args = arguments;

    if (!std::holds_alternative<QString>(args.front())) {
        return "First argument must be a string";
    }
    args.pop_front();

    if (!std::holds_alternative<Query::Capture>(args.front())) {
        return "Second argument must be a capture";
    }
    args.pop_front();

    for (const auto &arg : std::as_const(args)) {
        if (!std::holds_alternative<QString>(arg)) {
            return "Non-QString type Argument";
        }
    }

    return {};
}

bool Predicates::filter_like(const QueryMatch &match,
                             const QVector<std::variant<Query::Capture, QString>> &arguments) const
{
    return filter_eq_with(match, arguments, QString_no_whitespace);
}
bool Predicates::filter_eq_except_with(const QueryMatch &match,
                                       const QVector<std::variant<Query::Capture, QString>> &arguments,
                                       const std::function<QString(const QString &)> &textTransform) const
{
    auto args = arguments;
    if (const auto *rawExpected = std::get_if<QString>(&args.front())) {
        auto expected = textTransform(*rawExpected);
        args.pop_front();
        if (const auto *rawCapture = std::get_if<Query::Capture>(&args.front())) {
            // we need to copy the capture here, as otherwise it might get dropped
            // when we pop_front() on args
            auto capture = *rawCapture;
            args.pop_front();

            auto types = QVector<QString>();
            for (const auto &arg : std::as_const(args)) {
                if (const auto *type = std::get_if<QString>(&arg)) {
                    types.push_back(*type);
                }
            }

            const auto idCaptures = match.capturesWithId(capture.id);
            if (idCaptures.isEmpty()) {
                spdlog::warn("Predicates: #eq_except? - No captures");
                // Insert an empty string into the set if we find an unmatched capture.
                // This likely means we have encountered a quantified capture that matched 0 times.
                // So check whether the expected string is also empty
                return expected.isEmpty();
            }

            for (const auto &idCapture : idCaptures) {
                if (expected != textTransform(idCapture.node.textExcept(m_source, types))) {
                    return false;
                }
            }

            return true;
        } else {
            spdlog::warn("Predicates: #eq_except? - Expected Capture argument");
            return false;
        }
    } else {
        spdlog::warn("Predicates: #eq_except? - Non-string expected argument");
        return false;
    }
}

bool Predicates::filter_eq_except(const QueryMatch &match, const PredicateArguments &arguments) const
{
    return filter_eq_except_with(match, arguments, QString_identity);
}

bool Predicates::filter_like_except(const QueryMatch &match, const PredicateArguments &arguments) const
{
    return filter_eq_except_with(match, arguments, QString_no_whitespace);
}

bool Predicates::filter_not_is(const QueryMatch &match, const PredicateArguments &arguments) const
{
    const auto matched = matchArguments(match, arguments);

    auto captures = QVector<QueryMatch::Capture>();
    auto types = QVector<QString>();

    for (const auto &arg : matched) {
        if (auto capture = std::get_if<QueryMatch::Capture>(&arg)) {
            captures.push_back(*capture);
        }
        if (auto type_name = std::get_if<QString>(&arg)) {
            types.push_back(*type_name);
        }
        // Unmatched captures are ignored, they are definitely not of the forbidden type.
    }

    for (const auto &capture : std::as_const(captures)) {
        if (types.contains(capture.node.type())) {
            return false;
        }
    }
    return true;
}

std::optional<QString> Predicates::checkFilter_like(const Predicates::PredicateArguments &arguments)
{
    return Predicates::checkFilter_eq(arguments);
}

std::optional<QString> Predicates::checkFilter_like_except(const Predicates::PredicateArguments &arguments)
{
    return Predicates::checkFilter_eq_except(arguments);
}

std::optional<QString> Predicates::checkFilter_match(const Predicates::PredicateArguments &arguments)
{
    if (arguments.size() < 2) {
        return QString("Too few arguments");
    }

    if (const auto regexString = std::get_if<QString>(&arguments.first())) {
        const QRegularExpression regex(*regexString);
        if (!regex.isValid()) {
            return "Invalid Regex";
        }
    } else {
        return "Missing regex";
    }

    for (const auto &arg : arguments | std::views::drop(1)) {
        if (!std::holds_alternative<Query::Capture>(arg)) {
            return "Argument is not a capture";
        }
    }

    return {};
}

std::optional<QString> Predicates::checkFilter_not_is(const Predicates::PredicateArguments &arguments)
{
    if (arguments.size() < 2) {
        return "Too few arguments";
    }

    auto is_capture = [](const auto &arg) {
        return std::holds_alternative<Query::Capture>(arg);
    };

    if (!kdalgorithms::any_of(arguments, is_capture)) {
        return "You need to provide at least one capture";
    }

    auto is_string = [](const auto &arg) {
        return std::holds_alternative<QString>(arg);
    };
    if (!kdalgorithms::any_of(arguments, is_string)) {
        return "You need to provide at least one node type to match against";
    }

    return std::nullopt;
}

bool Predicates::filter_match(const QueryMatch &match,
                              const QVector<std::variant<Query::Capture, QString>> &arguments) const
{
    const auto matched = matchArguments(match, arguments);

    if (arguments.size() < 2) {
        return false;
    }

    if (const auto regexString = std::get_if<QString>(&matched.first())) {
        const QRegularExpression regex(*regexString);
        if (!regex.isValid()) {
            spdlog::warn("Predicates: #match? - Invalid regex");
            return false;
        }

        for (const auto &argument : matched | std::views::drop(1)) {
            if (const auto *capture = std::get_if<QueryMatch::Capture>(&argument)) {
                auto source = capture->node.textIn(m_source);
                if (!regex.match(source).hasMatch()) {
                    return false;
                }
            } else if (std::holds_alternative<MissingCapture>(argument)) {
                spdlog::warn("Predicates: #match? - Unmatched capture argument");
                return false;
            } else {
                spdlog::warn("Predicates: #match? - Argument is not a capture");
                return false;
            }
        }
    } else {

        spdlog::warn("Predicates: #match? - First argument is not a string");
        return false;
    }

    return true;
}

void Predicates::insertCache(std::unique_ptr<PredicateCache> cache) const
{
    m_caches.emplace_back(std::move(cache));
}

class MessageMapCache : public PredicateCache
{
public:
    MessageMapCache(const Node &begin, const Node &end)
        : m_begin(begin)
        , m_end(end)
    {
    }

    ~MessageMapCache() override = default;

    Node m_begin;
    Node m_end;
};

void Predicates::findMessageMap() const
{
    if (findCache<MessageMapCache>()) {
        // Already found it!
        return;
    }

    if (!m_rootNode.has_value()) {
        spdlog::warn("Predicates::findMessageMap: No rootNode!");
        return;
    }

    // This variable is "static" because Query construction is actually a non-trivial task.
    // We don't want to construct the query every time we call this function, as it's always the same query anyway.
    //
    // This has caused performance problems in the past, when combined with the ScriptSuggestions model, which queries
    // after every keystroke.
    static auto query = std::make_shared<Query>(tree_sitter_cpp() /*in_message_map only makes sense in C++*/, R"EOF(
(
(expression_statement
    (call_expression
        function: (identifier) @begin (#eq? @begin "BEGIN_MESSAGE_MAP")
        arguments: (argument_list . (_) @class)))
.
(expression_statement)*
.
(expression_statement (call_expression
    function: (identifier) @end (#eq? @end "END_MESSAGE_MAP")))
)
    )EOF");

    QueryCursor cursor;
    cursor.execute(query, *m_rootNode, std::make_unique<Predicates>(m_source));

    auto match = cursor.nextMatch();
    if (match.has_value()) {
        auto begin = match->capturesNamed("begin");
        auto end = match->capturesNamed("end");

        if (!begin.isEmpty() && !end.isEmpty()) {
            insertCache(std::make_unique<MessageMapCache>(begin.first().node, end.first().node));
        }
    }
}

std::optional<QString> Predicates::checkFilter_in_message_map(const Predicates::PredicateArguments &arguments)
{
    if (arguments.isEmpty()) {
        return "Too few arguments";
    }

    for (const auto &arg : arguments) {
        if (!std::holds_alternative<Query::Capture>(arg)) {
            return "Non-Capture Argument";
        }
    }

    return {};
}

bool Predicates::filter_in_message_map(const QueryMatch &match, const PredicateArguments &arguments) const
{
    findMessageMap();

    if (const auto *message_map = findCache<MessageMapCache>()) {
        const auto matched = matchArguments(match, arguments);

        for (const auto &argument : matched) {
            if (const auto capture = std::get_if<QueryMatch::Capture>(&argument)) {
                if (!(message_map->m_begin.endPosition() <= capture->node.startPosition()
                      && capture->node.endPosition() <= message_map->m_end.startPosition())) {
                    // We're outside of the message map
                    return false;
                }
            } else {
                spdlog::warn("Predicate: #in_message_map? - Non-Capture Argument!");
                return false;
            }
        }

        return true;
    } else {
        spdlog::warn("Predicate: #in_message_map? - No MESSAGE_MAP found!");
        return false;
    }
}

QVector<std::variant<QString, QueryMatch::Capture, Predicates::MissingCapture>>
Predicates::matchArguments(const QueryMatch &match, const Predicates::PredicateArguments &arguments) const
{
    QVector<std::variant<QString, QueryMatch::Capture, Predicates::MissingCapture>> result;

    for (const auto &argument : arguments) {
        if (const auto string = std::get_if<QString>(&argument)) {
            result.emplace_back(*string);
        } else if (const auto captureArgument = std::get_if<Query::Capture>(&argument)) {
            const auto captures = match.capturesWithId(captureArgument->id);

            // Multiple captures for the same ID may exist, if quantifiers are used.
            // Add all of them.
            for (const auto &capture : captures) {
                result.emplace_back(capture);
            }
            if (captures.isEmpty())
                result.emplace_back(MissingCapture {.capture = *captureArgument});
        }
    }
    return result;
}

void Predicates::setRootNode(const Node &node)
{
    m_rootNode = node;
}
}
