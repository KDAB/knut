#include "mark.h"

#include "logger.h"
#include "textdocument.h"

#include <QPlainTextEdit>
#include <QTextDocument>

#include <spdlog/spdlog.h>

namespace Core {

/*!
 * \qmltype Mark
 * \brief Keeps track of a position in a text document.
 * \instantiates Core::Mark
 * \inqmlmodule Script
 * \since 4.0
 * \sa TextDocument
 *
 * The Mark object helps you track a logical possition in a file.
 * Its `line` and `column` properties will change as text are deleted or inserted before the mark.
 *
 * A mark is always created by a [TextDocument](textdocument.md).
 */

/*!
 * \qmlproperty bool Mark::line
 * \qmlproperty bool Mark::column
 * \qmlproperty bool Mark::position
 *
 * Those read-only properties holds the position of the mark. They will be updated if text is
 * deleted or inserted.
 */
/*!
 * \qmlproperty bool Mark::isValid
 * This read-only property indicates if the mark is valid.
 */

Mark::Mark(TextDocument *editor, int pos)
    : QObject(editor)
    , m_editor(editor)
    , m_pos(pos)
{
    Q_ASSERT(editor);
    auto document = editor->textEdit()->document();
    connect(document, &QTextDocument::contentsChange, this, &Mark::update);
}

bool Mark::isValid() const
{
    return m_editor;
}

int Mark::position() const
{
    return m_pos;
}

int Mark::line() const
{
    if (!checkEditor())
        return 0;

    int line, column;
    m_editor->convertPosition(m_pos, &line, &column);
    return line;
}

int Mark::column() const
{
    if (!checkEditor())
        return 0;

    int line, column;
    m_editor->convertPosition(m_pos, &line, &column);
    return column;
}

QString Mark::toString() const
{
    return QStringLiteral("{%1}").arg(position());
}

/*!
 * \qmlmethod Mark::restore()
 * Returns the cursor position in the editor to the position saved by this object.
 */
void Mark::restore()
{
    LOG("Mark::restore");
    if (checkEditor())
        m_editor->gotoMark(this);
}

bool Mark::checkEditor() const
{
    if (!m_editor) {
        spdlog::error("Mark::checkEditor - document does not exist anymore");
        return false;
    }
    return true;
}

void Mark::update(int from, int charsRemoved, int charsAdded)
{
    if (m_pos <= from)
        return;

    // Overlap the position
    if (m_pos <= from + charsRemoved)
        m_pos = from;
    else
        m_pos += charsAdded - charsRemoved;
}

} // namespace Core
