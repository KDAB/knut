/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QPlainTextEdit>

namespace Gui {

class LogPanel : public QPlainTextEdit
{
public:
    explicit LogPanel(QWidget *parent = nullptr);
    ~LogPanel() override;

    QWidget *toolBar() const;

private:
    QWidget *const m_toolBar = nullptr;
};

} // namespace Gui
