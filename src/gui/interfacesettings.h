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
    class InterfaceSettings;
}

class InterfaceSettings : public QWidget
{
    Q_OBJECT

public:
    explicit InterfaceSettings(QWidget *parent = nullptr);
    ~InterfaceSettings() override;

    void initialize();
    QString getHelpPath();

private:
    void browseHelpPath();
    std::unique_ptr<Ui::InterfaceSettings> ui;
};

} // namespace Gui
