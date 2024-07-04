/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "textlocation.h"
#include "codedocument.h"
#include "project.h"

namespace Core {

/*!
 * \qmltype TextLocation
 * \brief Defines a range of text in a file.
 * \inqmlmodule Script
 * \ingroup TextDocument
 * \sa CodeDocument
 *
 * A mark is always created by a [CodeDocument](codedocument.md).
 */

/*!
 * \qmlproperty CodeDocument TextLocation::document
 * This read-only property contains the source document for this text location.
 */
/*!
 * \qmlproperty TextRange TextLocation::range
 * This read-only property contains the range of text in the document.
 */

QString TextLocation::toString() const
{
    return QString("{'%1', %2}").arg(document->fileName(), range.toString());
}

QList<TextLocation> TextLocation::fromLsp(const std::vector<Lsp::Location> &locations)
{
    QList<Core::TextLocation> textLocations;

    for (const auto &location : locations) {
        const auto url = QUrl::fromEncoded(QByteArray::fromStdString(location.uri));
        if (!url.isLocalFile()) {
            continue;
        }
        const auto filepath = url.toLocalFile();

        if (auto *document = qobject_cast<CodeDocument *>(Project::instance()->get(filepath))) {
            const auto range = document->toRange(location.range);

            textLocations.emplace_back(TextLocation {.document = document, .range = range});
        }
    }

    return textLocations;
}

} // namespace Core
