/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "searchabletableview.h"
#include "core/textdocument.h"
#include "highlightdelegate.h"

namespace Gui {

SearchableTableView::SearchableTableView(QWidget *parent)
    : QTableView(parent)
{
    setItemDelegate(new HighlightDelegate(this));
}

SearchableTableView::~SearchableTableView() = default;

void SearchableTableView::find(const QString &text, int options)
{
    if (text.isEmpty()) {
        // Clear the search results
        static_cast<HighlightDelegate *>(itemDelegate())->setHighlightedText({}, Core::TextDocument::NoFindFlags);
        m_searchResults.clear();
        m_currentResultIndex = 0;
        m_highlightedText.clear();
        m_options = 0;
        viewport()->update();
        return;
    }

    // Search only in case the search was not already processed
    if (text != m_highlightedText || options != m_options) {
        // Update delegate with the search text
        static_cast<HighlightDelegate *>(itemDelegate())->setHighlightedText(text, options);
        viewport()->update();

        m_searchResults = searchModel(text, options);
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
        selectionModel()->setCurrentIndex(m_searchResults.at(m_currentResultIndex), QItemSelectionModel::SelectCurrent);
    }
}

QModelIndexList SearchableTableView::searchModel(const QString &text, int options) const
{
    if (text.isEmpty())
        return {};

    QModelIndexList searchResults;
    for (int row = 0; row < model()->rowCount(); ++row) {
        for (int column = 0; column < model()->columnCount(); ++column) {
            const QModelIndex index = model()->index(row, column);
            const QString data = model()->data(index).toString();
            if (options & Core::TextDocument::FindRegexp) {
                QRegularExpression re(text);
                if (data.contains(re))
                    searchResults.append(index);
            } else {
                const bool caseSensitive = options & Core::TextDocument::FindCaseSensitively;
                if (data.contains(text, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive))
                    searchResults.append(index);
            }
        }
    }
    return searchResults;
}

} // namespace Gui
