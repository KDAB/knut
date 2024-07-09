/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "textrange.h"

namespace Core {

/*!
 * \qmltype TextRange
 * \brief Defines a range of text in a text document
 * \inqmlmodule Knut
 * \ingroup TextDocument
 * \sa TextDocument
 */
/*!
 * \qmlproperty int TextRange::start
 * This read-only property defines the start position of the range.
 */
/*!
 * \qmlproperty int TextRange::end
 * This read-only property defines the end position of the range.
 */
/*!
 * \qmlproperty int TextRange::length
 * This read-only property returns the length of the range (end - start)
 */

bool TextRange::contains(int pos) const
{
    return (pos >= start && pos <= end);
}

bool TextRange::contains(const TextRange &range) const
{
    return contains(range.start) && contains(range.end);
}

int TextRange::length() const
{
    return end - start;
}

} // Core
