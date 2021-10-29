#pragma once

#include <QDialog>

namespace Gui {

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private:
    void openUserSettings();
    void openProjectSettings();

    void addScriptPath();
    void removeScriptPath();
    void updateScriptPaths();

private:
    Ui::SettingsDialog *const ui;
};

} // namespace Gui
