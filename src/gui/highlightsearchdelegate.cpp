/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "highlightsearchdelegate.h"
#include <QPainter>

namespace Gui {

HighlightSearchDelegate::HighlightSearchDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void HighlightSearchDelegate::setSearchText(const QString &searchText, int offset)
{
    m_searchText = searchText;
    m_offset = offset;
}

void HighlightSearchDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QString text = index.data().toString();
    if (m_searchText.isEmpty() || !text.contains(m_searchText, Qt::CaseInsensitive)) {
        QItemDelegate::paint(painter, option, index);
        return;
    }

    // Check for multiline strings
    QStringList multilineStrings = text.split("\n", Qt::SkipEmptyParts);
    bool isMultiline = multilineStrings.count() > 1;
    if (isMultiline) {
        // Make it a one line text for the search highlight,
        // otherwise the painting is error prone (needs too much manipulation).
        // That does not impact the model itself.
        text = text.replace("\n", " - ");
    }
    // Find the substring to highlight
    const QRegularExpression regex(m_searchText, QRegularExpression::CaseInsensitiveOption);
    int start = regex.match(text).capturedStart();
    int end = start + regex.match(text).capturedLength();

    // Calculate the drawing positions.
    // The TreeViews or TableView display its Items excentred in relation to the left side of the cell.
    // Correct the x position accordingly using the caller offset value.
    int x = option.rect.left() + m_offset;
    int y = option.rect.top();
    int height = option.rect.height();
    int width = painter->fontMetrics().horizontalAdvance(text.left(start));

    // Paint the seach result string 'bold'.
    QFont highlightFont = option.font;
    highlightFont.setBold(true);

    painter->save();
    // Make sure we don't paint over the selected item highlighted background.
    if (option.state & QStyle::State_Selected) {
        // Set the background color to the selection color
        painter->fillRect(option.rect, option.palette.brush(QPalette::Active, QPalette::Highlight));
    }

    painter->drawText(x, y, width, height, option.displayAlignment, text.left(start));
    // Paint the searched string with bold font.
    painter->setFont(highlightFont);
    x += width;
    width = painter->fontMetrics().horizontalAdvance(text.mid(start, end - start));
    painter->drawText(x, y, width, height, option.displayAlignment, text.mid(start, end - start));
    // Reset the painter font to original font.
    painter->setFont(option.font);
    x += width;
    width = painter->fontMetrics().horizontalAdvance(text.right(text.length() - end));
    painter->drawText(x, y, width, height, option.displayAlignment, text.right(text.length() - end));
    painter->restore();
}

} // namespace Gui
