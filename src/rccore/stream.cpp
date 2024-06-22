/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

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

QChar Stream::peek() const
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
