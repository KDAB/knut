#include "textrange.h"

namespace Core {

/*!
 * \qmltype TextRange
 * \brief Define a range of text in a text document
 * \inqmlmodule Script
 * \since 4.0
 * \sa TextDocument
 */
/*!
 * \qmlproperty int TextRange::start
 * Start position of the range.
 */
/*!
 * \qmlproperty int TextRange::end
 * End position of the range.
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
