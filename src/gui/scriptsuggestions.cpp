#include "scriptsuggestions.h"

#include "core/logger.h"
#include "core/lspdocument.h"
#include "core/project.h"
#include "core/scriptmanager.h"
#include "core/scriptmodel.h"
#include "core/textdocument.h"

using ScriptManager = Core::ScriptManager;

namespace Gui {

ScriptSuggestions::ScriptSuggestions(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_debouncer(new KDToolBox::KDSignalDebouncer(this))
{
    setSourceModel(Core::ScriptManager::model());

    connect(ScriptManager::instance(), &ScriptManager::scriptAdded, this, &ScriptSuggestions::invalidate);
    connect(ScriptManager::instance(), &ScriptManager::scriptRemoved, this, &ScriptSuggestions::invalidate);

    auto *project = Core::Project::instance();
    connect(project, &Core::Project::currentDocumentChanged, this, &ScriptSuggestions::setCurrentDocument);

    m_debouncer->setTimeout(200 /*ms*/);
    connect(m_debouncer, &KDToolBox::KDSignalDebouncer::triggered, this, &ScriptSuggestions::invalidate);
}

ScriptSuggestions::~ScriptSuggestions() = default;

bool ScriptSuggestions::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!m_document)
        return false;

    if (sourceRow < 0 || sourceRow >= sourceModel()->rowCount(sourceParent))
        return false;

    const auto index = sourceModel()->index(sourceRow, Core::ScriptModel::NameColumn, sourceParent);
    const auto queriesVariant = sourceModel()->data(index, Core::ScriptModel::ContextQueriesRole);
    const auto queries = queriesVariant.value<treesitter::QueryList>();

    Core::LoggerDisabler loggerDisabler(true);
    return contextQuery(queries).has_value();
}

void ScriptSuggestions::setCurrentDocument(Core::Document *document)
{
    if (m_document == document)
        return;

    if (m_document) {
        m_document->disconnect(m_debouncer);
    }

    m_document = qobject_cast<Core::LspDocument *>(document);

    if (m_document) {
        connect(m_document, &Core::TextDocument::textChanged, m_debouncer, &KDToolBox::KDSignalDebouncer::throttle);
        connect(m_document, &Core::TextDocument::positionChanged, m_debouncer, &KDToolBox::KDSignalDebouncer::throttle);
    }
    invalidate();
}

void ScriptSuggestions::run(const QModelIndex &index)
{
    const auto queriesVariant = data(index, Core::ScriptModel::ContextQueriesRole);
    const auto queries = queriesVariant.value<treesitter::QueryList>();
    const auto fileName = data(index, Core::ScriptModel::PathRole).toString();

    if (fileName.isEmpty()) {
        spdlog::warn("ScriptSuggestions::run - fileName is empty");
        return;
    }

    const auto match = contextQuery(queries);
    if (match) {
        ScriptManager::instance()->runScriptInContext(fileName, *match);
    } else {
        spdlog::warn("ScriptSuggestions::run - couldn't find the context for this script!");
    }
}

void ScriptSuggestions::invalidate()
{
    invalidateRowsFilter();
    emit suggestionsUpdated(m_document);
}

std::optional<Core::QueryMatch> ScriptSuggestions::contextQuery(const treesitter::QueryList &queries) const
{
    if (!m_document)
        return {};

    for (const auto &query : queries) {
        const auto matches = m_document->query(query);
        for (const auto &match : matches) {
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
