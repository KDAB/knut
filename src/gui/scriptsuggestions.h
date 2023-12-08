#pragma once

#include "KDSignalThrottler.h"
#include "core/querymatch.h"
#include "treesitter/query.h"

#include <QPointer>
#include <QSortFilterProxyModel>

namespace Core {
class Document;
class LspDocument;
}

namespace Gui {

class ScriptSuggestions : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ScriptSuggestions(QObject *parent = nullptr);
    ~ScriptSuggestions();

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

public slots:
    void run(const QModelIndex &index);

signals:
    void suggestionsUpdated(Core::Document *document);

private:
    void invalidate();
    void setCurrentDocument(Core::Document *document);

    std::optional<Core::QueryMatch> contextQuery(const treesitter::QueryList &queries) const;

    QPointer<Core::LspDocument> m_document;
    // Debounce editing changes, as they can be quite frequent (e.g. on every keystroke, or when running a script)
    // and it's not really important if updating them is a few milliseconds late.
    // Also, when typing, both the text and the position changed, which caused the filter to be invalidated and
    // recalculated twice.
    KDToolBox::KDSignalDebouncer *m_debouncer;
};

} // namespace Gui
