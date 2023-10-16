#include "scriptsuggestions.h"

#include "scriptsinpath.h"

#include "core/logger.h"
#include "core/lspdocument.h"
#include "core/project.h"
#include "core/scriptmanager.h"
#include "core/textdocument.h"

using ScriptManager = Core::ScriptManager;

namespace Gui {

ScriptSuggestions::ScriptSuggestions(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    auto allScripts = new ScriptsInPath(this);
    setSourceModel(allScripts);

    connect(ScriptManager::instance(), &ScriptManager::scriptAdded, this, &ScriptSuggestions::invalidate);
    connect(ScriptManager::instance(), &ScriptManager::scriptRemoved, this, &ScriptSuggestions::invalidate);

    auto *project = Core::Project::instance();
    connect(project, &Core::Project::currentDocumentChanged, this, &ScriptSuggestions::setCurrentDocument);
}

ScriptSuggestions::~ScriptSuggestions() = default;

bool ScriptSuggestions::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!m_document)
        return false;

    if (sourceRow < 0 || sourceRow >= sourceModel()->rowCount(sourceParent))
        return false;

    const auto index = sourceModel()->index(sourceRow, ScriptsInPath::Column::NameColumn, sourceParent);
    const auto queries = sourceModel()->data(index, ScriptsInPath::Role::ContextQueriesRole).toStringList();

    Core::LoggerDisabler loggerDisabler(true);
    return contextQuery(queries).has_value();
}

void ScriptSuggestions::setCurrentDocument(Core::Document *document)
{
    if (m_document == document)
        return;

    if (m_document) {
        m_document->disconnect(this);
    }

    m_document = qobject_cast<Core::LspDocument *>(document);

    if (m_document) {
        connect(m_document, &Core::TextDocument::textChanged, this, &ScriptSuggestions::invalidate);
        connect(m_document, &Core::TextDocument::positionChanged, this, &ScriptSuggestions::invalidate);
    }
    invalidate();
}

void ScriptSuggestions::run(const QModelIndex &index)
{
    const auto queries = data(index, ScriptsInPath::Role::ContextQueriesRole).toStringList();
    const auto fileName = data(index, ScriptsInPath::Role::PathRole).toString();

    if (fileName.isEmpty()) {
        spdlog::warn("ScriptSuggestions::run - fileName is empty");
        return;
    }

    const auto match = contextQuery(queries);
    if (match) {
        ScriptManager::instance()->runScriptInContext(fileName, *match);
    }
}

void ScriptSuggestions::invalidate()
{
    invalidateRowsFilter();
    emit suggestionsUpdated(m_document);
}

std::optional<Core::QueryMatch> ScriptSuggestions::contextQuery(const QStringList &queries) const
{
    if (!m_document)
        return {};

    for (const auto &query : queries) {
        for (const auto &match : m_document->query(query)) {
            const auto hereRanges = match.getAll("here");

            // If there is no @here capture in the match, it is assumed that the
            // existence of the match is enough context to suggest the script.
            //
            // e.g. if we find the message map of a CDialog we can suggest the
            // mfc-convert-dialog script, no matter where the cursor is.
            if (hereRanges.empty()) {
                return match;
            }

            for (const auto &range : hereRanges) {
                if (range.contains(m_document->position())) {
                    return match;
                }
            }
        }
    }
    return {};
}

} // namespace Gui
