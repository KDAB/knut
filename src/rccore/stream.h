/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QChar>
#include <QString>

class QIODevice;

namespace RcCore {

class Stream
{
public:
    explicit Stream(QIODevice *device);
    Stream(const QString &text);

    bool atEnd() const;
    int line() const;

    QChar next();
    QChar peek() const;

    QString content() const;

private:
    QString m_content;
    int m_pos = 0;
    int m_line = 1;
};

} // namespace RcCore
