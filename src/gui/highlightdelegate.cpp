/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "highlightdelegate.h"
#include "core/textdocument.h"

#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QTextDocument>

namespace Gui {

HighlightDelegate::HighlightDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void HighlightDelegate::setHighlightedText(const QString &searchText, int options)
{
    m_highlightedText = searchText;
    m_options = options;
}

QString HighlightDelegate::transform(QString text, const QString &textColor, const QString &backgroundColor) const
{
    if (m_highlightedText.isEmpty())
        return text;

    if (m_options & Core::TextDocument::FindRegexp) {
        const auto re = QRegularExpression {m_highlightedText};
        QRegularExpressionMatch match;
        int index = text.indexOf(re, 0, &match);
        while (index != -1) {
            const auto oldText = match.captured(0);
            const auto newText = QString("<span style='color: %1; background-color: %2;'>%3</span>")
                                     .arg(textColor, backgroundColor, oldText);
            text.replace(index, oldText.size(), newText);
            index = text.indexOf(re, index + newText.size(), &match);
        }
    } else {
        const bool caseSensitive = m_options & Core::TextDocument::FindCaseSensitively;
        int index = text.indexOf(m_highlightedText, 0, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
        while (index != -1) {
            const auto oldText = text.mid(index, m_highlightedText.size());
            const auto newText = QString("<span style='color: %1; background-color: %2;'>%3</span>")
                                     .arg(textColor, backgroundColor, oldText);
            text.replace(index, oldText.size(), newText);
            index = text.indexOf(m_highlightedText, index + newText.size(),
                                 caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
        }
    }
    return text;
}

void HighlightDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const auto textColor = option.palette.color(QPalette::HighlightedText).name();
    const auto backgroundColor = option.palette.color(QPalette::Highlight).name();

    // Display the text with the search string highlighted.
    QString text = transform(index.data(Qt::DisplayRole).toString(), textColor, backgroundColor);
    QTextDocument doc;
    doc.setHtml(text);

    painter->save();

    // Adjust the painter's transformation to fit the text within the given rectangle
    painter->translate(option.rect.topLeft());
    QRect clip(0, 0, option.rect.width(), option.rect.height());
    doc.setTextWidth(option.rect.width());

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.clip = clip;
    doc.documentLayout()->draw(painter, ctx);

    painter->restore();
}

} // namespace Gui
