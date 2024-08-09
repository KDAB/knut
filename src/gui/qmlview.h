/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "textview.h"

namespace Gui {

class QmlView : public TextView
{
    Q_OBJECT
public:
    explicit QmlView(QWidget *parent = nullptr);
    ~QmlView() override;

private:
    void runQml();
};

} // namespace Gui
