/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QString>

namespace Gui {

/**
 * @brief Find interface to handle find and replace in views.
 */
class FindInterface
{
public:
    enum FindFlag {
        NoFind = 0x0,
        CanSearch = 0x1,
        CanReplace = 0x2,
    };
    FindInterface(int flags)
        : m_findFlags(flags) {};

    int findFlags() const { return m_findFlags; }

    virtual void find(const QString &text, int options)
    {
        Q_UNUSED(text);
        Q_UNUSED(options);
    };
    virtual void cancelFind() {};
    virtual void replace(const QString &before, const QString &after, int options, bool replaceAll)
    {
        Q_UNUSED(before);
        Q_UNUSED(after);
        Q_UNUSED(options);
        Q_UNUSED(replaceAll);
    };

private:
    int m_findFlags = NoFind;
};

} // namespace Gui
