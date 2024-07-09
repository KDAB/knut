/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "mark.h"
#include "logger.h"
#include "mark_p.h"
#include "textdocument.h"
#include "utils/log.h"

#include <QPlainTextEdit>
#include <QTextDocument>

namespace Core {

/*!
 * \qmltype Mark
 * \brief Keeps track of a position in a text document.
 * \inqmlmodule Knut
 * \ingroup TextDocument
 * \sa TextDocument
 *
 * The Mark object helps you track a logical position in a file.
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
/*!
 * \qmlproperty TextDocument Mark::document
 * This read-only property returns the document the mark is coming from.
 */

bool MarkPrivate::checkEditor() const
{
    if (!m_editor) {
        spdlog::error("Mark::checkEditor - document does not exist anymore");
        return false;
    }
    return true;
}

bool MarkPrivate::isValid() const
{
    return m_editor && m_pos >= 0;
}

int MarkPrivate::line() const
{
    if (!checkEditor())
        return -1;

    int line, column;
    m_editor->convertPosition(m_pos, &line, &column);
    return line;
}

int MarkPrivate::column() const
{
    if (!checkEditor())
        return -1;

    int line, column;
    m_editor->convertPosition(m_pos, &line, &column);
    return column;
}

void MarkPrivate::update(int from, int charsRemoved, int charsAdded)
{
    Mark::updateMark(m_pos, from, charsRemoved, charsAdded);
}

MarkPrivate::MarkPrivate(TextDocument *editor, int pos)
    : QObject(nullptr) // Important to have nullptr here, MarkPrivate is managed by shared_ptrs in Mark, MarkPrivate can
                       // deal with the editor being deleted.
    , m_editor(editor)
    , m_pos(pos)
{
    Q_ASSERT(editor);
    auto document = editor->textEdit()->document();
    connect(document, &QTextDocument::contentsChange, this, &MarkPrivate::update);
}

Mark::Mark(TextDocument *editor, int pos)
    : d(std::make_shared<MarkPrivate>(editor, pos))
{
}

bool Mark::isValid() const
{
    return d && d->isValid();
}

int Mark::position() const
{
    return d ? d->m_pos : -1;
}

int Mark::line() const
{
    return d ? d->line() : -1;
}

int Mark::column() const
{
    return d ? d->column() : -1;
}

QString Mark::toString() const
{
    return QStringLiteral("{%1}").arg(position());
}

TextDocument *Mark::document() const
{
    return d ? d->m_editor : nullptr;
}

/*!
 * \qmlmethod Mark::restore()
 * Returns the cursor position in the editor to the position saved by this object.
 */
void Mark::restore() const
{
    LOG("Mark::restore");
    if (isValid()) {
        document()->gotoMark(*this);
    }
}

void Mark::updateMark(int &mark, int from, int charsRemoved, int charsAdded)
{
    if (mark < from)
        return;

    // Overlap the position
    if (mark < from + charsRemoved)
        mark = from;
    else
        mark += charsAdded - charsRemoved;
}

bool operator==(const Mark &left, const Mark &right)
{
    return left.document() == right.document() && left.column() == right.column() && left.line() == right.line();
}

} // namespace Core
