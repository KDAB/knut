/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QObject>

namespace Core {

struct TextRange
{
    Q_GADGET
    Q_PROPERTY(int start MEMBER start CONSTANT)
    Q_PROPERTY(int end MEMBER end CONSTANT)
    Q_PROPERTY(int length READ length CONSTANT)

public:
    int start;
    int end;

    Q_INVOKABLE QString toString() const { return QString("{%1, %2}").arg(start).arg(end); }

    bool contains(int pos) const;
    bool contains(const TextRange &range) const;
    int length() const;

    auto operator<=>(const TextRange &) const = default;
};

} // namespace Core

Q_DECLARE_METATYPE(Core::TextRange)
