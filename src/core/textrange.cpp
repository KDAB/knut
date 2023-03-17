#include "textrange.h"

namespace Core {

/*!
 * \qmltype TextRange
 * \brief Defines a range of text in a text document
 * \inqmlmodule Script
 * \ingroup TextDocument
 * \since 1.0
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

bool TextRange::contains(int pos) const
{
    return (pos >= start && pos <= end);
}

bool TextRange::contains(const TextRange &range) const
{
    return contains(range.start) && contains(range.end);
}

} // Core
