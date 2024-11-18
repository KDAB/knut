/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "findadapter.h"
#include "core/textdocument.h"
#include "highlightdelegate.h"

namespace Gui {

FindAdapter::FindAdapter(QAbstractItemView *view)
    : m_view(view)
{
    Q_ASSERT(view);
}

void FindAdapter::find(const QString &text, int options)
{
    if (text.isEmpty()) {
        // Clear the search results
        if (auto delegate = dynamic_cast<HighlightDelegate *>(m_view->itemDelegate())) {
            delegate->setHighlightedText({}, Core::TextDocument::NoFindFlags);
            m_view->viewport()->update();
        }

        m_searchResults.clear();
        m_currentResultIndex = 0;
        m_highlightedText.clear();
        m_options = 0;
        return;
    }

    // Search only in case the search was not already processed
    if (text != m_highlightedText || options != m_options) {
        // Update delegate with the search text
        if (auto delegate = dynamic_cast<HighlightDelegate *>(m_view->itemDelegate())) {
            delegate->setHighlightedText(text, options);
            m_view->viewport()->update();
        }

        m_searchResults = text.isEmpty() ? QModelIndexList {} : searchModel(text, options);
        m_currentResultIndex = 0;

        // Update the current search text
        m_highlightedText = text;
        m_options = options;
    } else {
        // Search was already processed. Handle options (Backward, Forward)
        if (!m_searchResults.isEmpty()) {
            if (options & Core::TextDocument::FindBackward) {
                m_currentResultIndex = (m_currentResultIndex - 1) % m_searchResults.count();
            } else {
                m_currentResultIndex = (m_currentResultIndex + 1) % m_searchResults.count();
            }
        }
    }

    if (!m_searchResults.isEmpty()) {
        m_view->selectionModel()->setCurrentIndex(m_searchResults.at(m_currentResultIndex),
                                                  QItemSelectionModel::SelectCurrent);
    }
}

void FindAdapter::updateItemDelegate()
{
    m_view->setItemDelegate(new HighlightDelegate(m_view));
}

QModelIndexList FindAdapter::searchModel(const QString &text, int options, const QModelIndex &parent) const
{
    QModelIndexList searchResults;
    auto model = m_view->model();
    for (int row = 0; row < model->rowCount(parent); ++row) {
        const QModelIndex index = model->index(row, 0, parent);
        for (int column = 0; column < model->columnCount(parent); ++column) {
            auto columnIndex = index.siblingAtColumn(column);
            const QString data = columnIndex.data().toString();
            if (options & Core::TextDocument::FindRegexp) {
                QRegularExpression re(text);
                if (data.contains(re))
                    searchResults.append(columnIndex);
            } else {
                const bool caseSensitive = options & Core::TextDocument::FindCaseSensitively;
                if (data.contains(text, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive))
                    searchResults.append(columnIndex);
            }
        }
        searchResults.append(searchModel(text, options, index));
    }
    return searchResults;
}

} // namespace Gui
