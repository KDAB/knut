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
    class RunScriptDialog;
}

class RunScriptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunScriptDialog(QWidget *parent = nullptr);
    ~RunScriptDialog() override;

public slots:
    void accept() override;

private:
    void chooseScript();

private:
    std::unique_ptr<Ui::RunScriptDialog> ui;
};

} // namespace Gui
