#include "rangemark.h"

#include "mark.h"
#include "textdocument.h"

#include "spdlog/spdlog.h"

#include <QPlainTextEdit>

namespace Core {

RangeMark::RangeMark(TextDocument *editor, int start, int end)
    : m_editor(editor)
    , m_start(start)
    , m_end(end)
{
    ensureInvariant();

    Q_ASSERT(editor);
    Q_ASSERT(isValid());

    auto document = editor->textEdit()->document();
    connect(document, &QTextDocument::contentsChange, this, &RangeMark::update);
}

bool RangeMark::isValid() const
{
    return checkEditor() && m_start >= 0 && m_end >= 0;
}

int RangeMark::start() const
{
    return m_start;
}

int RangeMark::end() const
{
    return m_end;
}

int RangeMark::length() const
{
    return m_end - m_start;
}

QString RangeMark::text() const
{
    // <= here instead of < because m_end is exclusive
    if (isValid() && m_end <= m_editor->text().size())
        return m_editor->text().sliced(m_start, m_end - m_start);
    return {};
}

QString RangeMark::toString() const
{
    return QString("[%1, %2)").arg(m_start).arg(m_end);
}

void RangeMark::select()
{
    if (isValid())
        m_editor->selectRegion(m_start, m_end);
}

void RangeMark::ensureInvariant()
{
    if (m_start > m_end) {
        spdlog::warn("RangeMark::ensureInvariant: invariant violated: m_start > m_end ({} > {})", m_start, m_end);
        std::swap(m_start, m_end);
    }
}

bool RangeMark::checkEditor() const
{
    if (!m_editor) {
        spdlog::error("RangeMark::checkEditor - document does not exist anymore");
        return false;
    }
    return true;
}

void RangeMark::update(int from, int charsRemoved, int charsAdded)
{
    Mark::updateMark(m_start, from, charsRemoved, charsAdded);
    Mark::updateMark(m_end, from, charsRemoved, charsAdded);
    ensureInvariant();
}

}
