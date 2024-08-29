/*
  This file is part of Knut.

SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

SPDX-License-Identifier: GPL-3.0-only

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "loghighlighter.h"

#include <QTextCharFormat>
#include <QTextDocument>

namespace Core {

LogHighlighter::LogHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    auto getFormat = [](const QColor &color, const QColor &background = {}) {
        QTextCharFormat format;
        format.setForeground(color);
        if (background.isValid())
            format.setBackground(background);
        return format;
    };
    m_rules = {
        {"[trace]", getFormat(QColor(128, 128, 128))},
        {"[debug]", getFormat(Qt::cyan)},
        {"[info]", getFormat(Qt::green)},
        {"[warning]", getFormat(QColor(255, 220, 0))},
        {"[error]", getFormat(Qt::red)},
        {"[critical]", getFormat(Qt::white, Qt::red)},
    };
}

void LogHighlighter::highlightBlock(const QString &text)
{
    for (const auto &rule : std::as_const(m_rules)) {
        if (int start = text.indexOf(rule.m_keyword); start != -1)
            setFormat(start + 1, rule.m_keyword.length() - 2, rule.m_format);
    }
}

} // namespace Core
