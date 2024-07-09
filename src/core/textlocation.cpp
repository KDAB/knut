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
 * \inqmlmodule Knut
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

} // namespace Core
