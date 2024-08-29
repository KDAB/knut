/*
  This file is part of Knut.

SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

SPDX-License-Identifier: GPL-3.0-only

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QSyntaxHighlighter>

class QTextDocument;
class QTextCharFormat;

namespace Core {

class LogHighlighter : public QSyntaxHighlighter
{
public:
    explicit LogHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QString m_keyword;
        QTextCharFormat m_format;
    };
    QList<HighlightingRule> m_rules;
};

} // namespace Core
