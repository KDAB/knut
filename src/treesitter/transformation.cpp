#include "transformation.h"

#include "predicates.h"
#include "tree.h"

#include <QObject>

namespace treesitter {

Transformation::Transformation(const QString &source, Parser &&parser, const std::shared_ptr<Query> &query,
                               QString transformationTarget)
    : m_source(source)
    , m_parser(std::move(parser))
    , m_query(query)
    , m_to(transformationTarget)
{
}

QString Transformation::run()
{
    auto resultText = m_source;

    m_replacements = 0;

    QueryCursor cursor;
    bool shouldContinue;
    do {
        const auto tree = m_parser.parseString(resultText);
        if (!tree.has_value()) {
            throw Error {.description = "Unknown parser error!"};
        }
        cursor.execute(m_query, tree->rootNode(), std::make_unique<Predicates>(resultText));

        shouldContinue = runOneTransformation(cursor, resultText);
    } while (shouldContinue);

    return resultText;
}

bool Transformation::runOneTransformation(QueryCursor &cursor, QString &resultText)
{
    std::unordered_map<QString, QString> context;

    bool hasMatch = false;
    // We want to allow multiple patterns, where not every pattern
    // has a @from capture, but can provide additional context.
    // Loop over them until we find a @from capture.
    while (auto match = cursor.nextMatch()) {
        hasMatch = true;
        auto captures = match->captures();
        for (const auto &capture : captures) {
            auto captureName = m_query->captureAt(capture.id).name;
            context[captureName] = capture.node.textIn(resultText);
        }

        const auto from = match->capturesNamed("from");
        if (!from.isEmpty()) {
            const auto fromStart = from.first().node.startPosition();
            const auto fromEnd = from.first().node.endPosition();

            QString after = m_to;
            for (const auto &[name, value] : context) {
                after.replace("@" + name, value);
            }

            resultText.replace(fromStart, fromEnd - fromStart, after);
            if (++m_replacements >= m_max_replacements) {
                throw Error {.description = QObject::tr("Maximum number of allowed transformations reached.\nPossibly "
                                                        "your transformation is recursive?")};
            }
            context = std::unordered_map<QString, QString>();
            return true;
        }
    }

    if (hasMatch && m_replacements == 0) {
        // We found at least one match, but no @from capture and didn't make any replacements before.
        throw Error {.description = QObject::tr("'@from' capture not found!")};
    }

    return false;
}

} // namespace treesitter
