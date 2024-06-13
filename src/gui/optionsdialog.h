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
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    ~OptionsDialog() override;

    void addSettings(QWidget *widget);

private:
    void initializeSaveToLogFileSetting();
    void initializeScriptPathSettings();
    void initializeScriptBehaviorSettings();
    void initializeRcSettings();

    void openUserSettings();
    void openProjectSettings();
    void openLogFile();

    void addScriptPath();
    void removeScriptPath();
    void updateScriptPaths();

    void changeToggleSectionSetting();
    void changeAssetFlagsSetting();
    void changeAssetColorsSetting();
    void changeDialogFlagsSetting();
    void changeLanguageMap();
    void changeSaveLogsToFileSetting();

    void changePage();

private:
    std::unique_ptr<Ui::OptionsDialog> ui;
};

} // namespace Gui
