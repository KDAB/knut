/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "rangemark.h"
#include "mark.h"
#include "rangemark_p.h"
#include "textdocument.h"
#include "utils/log.h"

#include <QPlainTextEdit>

namespace Core {

/*!
 * \qmltype RangeMark
 * \brief Keeps track of a range within a text document.
 * \ingroup TextDocument
 * \sa TextDocument
 *
 * The RangeMark object helps you track a logical range within a file.
 * It's `start` and `end` properties will change as text is deleted or inserted before or even within the range.
 *
 * A RangeMark is always created by a [TextDocument](textdocument.md).
 */

/*!
 * \qmlproperty bool RangeMark::start
 * \qmlproperty bool RangeMark::end
 * \qmlproperty bool RangeMark::length
 *
 * These read-only properties holds the start, end and length of the range. They will be updated
 * as the text of the TextDocument changes.
 */

/*!
 * \qmlproperty bool RangeMark::isValid
 * This read-only property indicates if the RangeMark is valid.
 */

/*!
 * \qmlproperty bool RangeMark::text
 * This read-only property returns the text covered by the range.
 */

RangeMarkPrivate::RangeMarkPrivate(TextDocument *editor, int start, int end)
    : m_editor(editor)
    , m_start(start)
    , m_end(end)
{
    ensureInvariant();

    Q_ASSERT(editor);
    Q_ASSERT(isValid());

    auto document = editor->textEdit()->document();
    connect(document, &QTextDocument::contentsChange, this, &RangeMarkPrivate::update);
}

bool RangeMarkPrivate::checkEditor() const
{
    if (!m_editor) {
        spdlog::error("{}: document does not exist anymore", FUNCTION_NAME);
        return false;
    }
    return true;
}

void RangeMarkPrivate::ensureInvariant()
{
    if (m_start > m_end) {
        spdlog::warn("{}: invariant violated: m_start > m_end ({} > {})", FUNCTION_NAME, m_start, m_end);
        std::swap(m_start, m_end);
    }
}

void RangeMarkPrivate::update(int from, int charsRemoved, int charsAdded)
{
    Mark::updateMark(m_start, from, charsRemoved, charsAdded);
    Mark::updateMark(m_end, from, charsRemoved, charsAdded);
    ensureInvariant();
}

bool RangeMarkPrivate::isValid() const
{
    return checkEditor() && m_start >= 0 && m_end >= 0;
}

RangeMark::RangeMark(TextDocument *editor, int start, int end)
    : d(new RangeMarkPrivate(editor, start, end))
{
}

bool RangeMark::isValid() const
{
    return d && d->isValid();
}

int RangeMark::start() const
{
    return d ? d->m_start : -1;
}

int RangeMark::end() const
{
    return d ? d->m_end : -1;
}

int RangeMark::length() const
{
    return end() - start();
}

bool RangeMark::contains(int position) const
{
    return isValid() && position >= start() && position < end();
}

bool RangeMark::contains(const RangeMark &other) const
{
    return other.isValid() && other.document() == document() && start() <= other.start() && other.end() <= end();
}

TextDocument *RangeMark::document() const
{
    return d ? d->m_editor : nullptr;
}

QString RangeMark::text() const
{
    if (!isValid())
        return {};

    auto text = temporaryDocumentText.isEmpty() ? document()->text() : temporaryDocumentText;
    // <= here instead of < because m_end is exclusive
    if (isValid() && end() <= text.size()) {
        text = text.sliced(start(), end() - start());
        text.squeeze();
        return text;
    }
    return {};
}

QString RangeMark::toString() const
{
    return QString("[%1, %2]").arg(start()).arg(end());
}

/*!
 * \qmlmethod RangeMark::select()
 * Selects the text defined by this range in the source document.
 */
void RangeMark::select() const
{
    if (isValid())
        document()->selectRegion(start(), end());
}

/*!
 * \qmlmethod RangeMark::replace(string text)
 * Replaces the text defined by this range with the `text` string in the source document.
 */
void RangeMark::replace(const QString &text) const
{
    if (isValid())
        document()->replace(start(), end(), text);
}

/*!
 * \qmlmethod RangeMark::remove()
 * Deletes the text defined by this range in the source document.
 */
void RangeMark::remove() const
{
    if (isValid())
        document()->deleteRegion(start(), end());
}

/*!
 * \qmlmethod RangeMark RangeMark::join(RangeMark other)
 * Joins the two `RangeMark` and creates a new one.
 *
 * The new `RangeMark` is spanning from the minimum of the start to the maximum of the end.
 */
RangeMark RangeMark::join(const RangeMark &other) const
{
    if (!isValid() || !other.isValid() || document() != other.document())
        return {};
    return RangeMark(document(), std::min(start(), other.start()), std::max(end(), other.end()));
}

/*!
 * \qmlmethod string RangeMark::textExcept(RangeMark other)
 *
 * Returns the text of this range without the text of the other range.
 * This assumes that both ranges overlap.
 *
 * Otherwise, the entire text is returned.
 */
QString RangeMark::textExcept(const RangeMark &other) const
{
    if (!isValid()) {
        spdlog::error("{}: invalid range", FUNCTION_NAME);
        return "";
    }
    if (!other.isValid()) {
        spdlog::debug("{}: invalid other range", FUNCTION_NAME);
        return text();
    }
    if (document() != other.document()) {
        spdlog::error("{}: different documents", FUNCTION_NAME);
        return text();
    }

    const QString text = this->text();
    QString result;
    if (start() < other.start())
        result += text.first(std::min(other.start() - start(), static_cast<int>(text.size())));
    if (end() > other.end())
        result += text.last(std::min(end() - other.end(), static_cast<int>(text.size())));
    return result;
}

bool RangeMark::operator==(const RangeMark &other) const
{
    return d == other.d || (document() == other.document() && start() == other.start() && end() == other.end());
}

}
