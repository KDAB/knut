#pragma once

#include "core/querymatch.h"

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

    std::optional<Core::QueryMatch> contextQuery(const QStringList &queries) const;

    QPointer<Core::LspDocument> m_document;
};

} // namespace Gui
