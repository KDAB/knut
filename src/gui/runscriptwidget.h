/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QDialog>
#include <memory>

namespace Gui {

namespace Ui {
    class RunScriptWidget;
}

class RunScriptWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RunScriptWidget(QWidget *parent = nullptr);
    ~RunScriptWidget() override;

    void open();

private:
    void chooseScript();
    void run();

private:
    std::unique_ptr<Ui::RunScriptWidget> ui;
};

} // namespace Gui
