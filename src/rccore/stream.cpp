#include "stream.h"

#include <QTextStream>

namespace RcCore {

Stream::Stream(QIODevice *device)
{
    QTextStream stream(device);
    m_content = stream.readAll();
}

Stream::Stream(const QString &text)
{
    m_content = text;
}

bool Stream::atEnd() const
{
    return m_pos == m_content.size();
}

int Stream::line() const
{
    return m_line;
}

QChar Stream::next()
{
    if (atEnd())
        return {};
    const QChar &ch = m_content.at(m_pos++);
    if (ch == '\n')
        ++m_line;
    return ch;
}

QChar Stream::peek()
{
    if (atEnd())
        return {};
    return m_content.at(m_pos);
}

QString Stream::content() const
{
    return m_content;
}

} // namespace RcCore
