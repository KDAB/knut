/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/knutcore.h"

namespace Gui {

class KnutMain : public Core::KnutCore
{
    Q_OBJECT
public:
    explicit KnutMain(QObject *parent = nullptr);

protected:
    void initParser(QCommandLineParser &parser) const override;
    void doParse(const QCommandLineParser &parser) const override;
};

} // namespace Gui
