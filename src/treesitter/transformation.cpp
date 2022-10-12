#include "transformation.h"

#include "tree.h"

#include <QObject>

namespace treesitter {

Transformation::Transformation(const QString &source, Parser &&parser, Query &&query, QString transformationTarget)
    : m_source(source)
    , m_parser(std::move(parser))
    , m_query(std::move(query))
    , m_to(transformationTarget)
{
}

QString Transformation::run()
{
    auto resultText = m_source;

    m_replacements = 0;

    QueryCursor cursor;
    std::optional<QueryMatch> match;

    do {
        const auto tree = m_parser.parseString(resultText);
        if (!tree.has_value()) {
            throw Error {.description = "Unknown parser error!"};
        }
        cursor.execute(m_query, tree->rootNode());

        match = cursor.nextMatch();
        if (match) {
            std::unordered_map<QString, QString> context;

            auto captures = match->captures();
            for (const auto &capture : captures) {
                auto captureName = m_query.captureAt(capture.id).name;
                context[captureName] = capture.node.textIn(resultText);
            }

            const auto fromIt = std::find_if(captures.cbegin(), captures.cend(), [this](const auto &capture) {
                return m_query.captureAt(capture.id).name == "from";
            });

            if (fromIt == captures.cend()) {
                throw Error {.description = QObject::tr("'@from' capture not found!")};
            }

            const auto fromStart = fromIt->node.startPosition();
            const auto fromEnd = fromIt->node.endPosition();

            QString after = m_to;
            for (const auto &[name, value] : context) {
                after.replace("@" + name, value);
            }

            resultText.replace(fromStart, fromEnd - fromStart, after);
            if (++m_replacements >= m_max_replacements) {
                throw Error {.description = QObject::tr("Maximum number of allowed transformations reached.\nPossibly "
                                                        "your transformation is recursive?")};
            }
        }
    } while (match.has_value());

    return resultText;
}

} // namespace treesitter
