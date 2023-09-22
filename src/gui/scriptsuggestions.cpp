#include "scriptsuggestions.h"

#include "core/logger.h"
#include "core/lspdocument.h"
#include "core/scriptmanager.h"
#include "gui/scriptsinpath.h"

using ScriptManager = Core::ScriptManager;

namespace Gui {

ScriptSuggestions::ScriptSuggestions(Core::LspDocument &document, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_document(document)
{
    auto invalidate = [this]() {
        this->invalidateRowsFilter();
    };
    connect(ScriptManager::instance(), &ScriptManager::scriptAdded, this, invalidate);
    connect(ScriptManager::instance(), &ScriptManager::scriptRemoved, this, invalidate);
    connect(&m_document, &Core::TextDocument::textChanged, this, invalidate);
    connect(&m_document, &Core::TextDocument::positionChanged, this, invalidate);
}

bool ScriptSuggestions::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (sourceRow < 0 || sourceRow >= sourceModel()->rowCount(sourceParent))
        return false;

    const auto index = sourceModel()->index(sourceRow, ScriptsInPath::Column::Name, sourceParent);
    const auto queries = sourceModel()->data(index, ScriptsInPath::Role::ContextQueries).toStringList();

    Core::LoggerDisabler loggerDisabler(true);
    return contextQuery(queries).has_value();
}

void ScriptSuggestions::run(const QModelIndex &index)
{
    auto fileNameIndex = this->index(index.row(), ScriptsInPath::Column::Path, index.parent());

    auto queries = data(fileNameIndex, ScriptsInPath::Role::ContextQueries).toStringList();
    auto fileName = data(fileNameIndex).toString();

    if (fileName.isEmpty()) {
        spdlog::warn("ScriptSuggestions::run - fileName is empty");
        return;
    }

    auto match = contextQuery(queries);
    if (match) {
        ScriptManager::instance()->runScriptInContext(fileName, *match);
    }
}

std::optional<Core::QueryMatch> ScriptSuggestions::contextQuery(const QStringList &queries) const
{
    for (const auto &query : queries) {
        for (const auto &match : m_document.query(query)) {
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
                if (range.contains(m_document.position())) {
                    return match;
                }
            }
        }
    }
    return {};
}

} // namespace Gui
