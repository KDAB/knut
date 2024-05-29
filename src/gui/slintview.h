/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "textview.h"

namespace Core {
class SlintDocument;
}

class QProcess;

namespace Gui {

class SlintView : public TextView
{
    Q_OBJECT

public:
    explicit SlintView(QWidget *parent = nullptr);
    ~SlintView() override;

private:
    void runSlint();

private:
    QProcess *m_process = nullptr;
};

}
