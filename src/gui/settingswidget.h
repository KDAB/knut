#pragma once

#include <QWidget>

#include <memory>

namespace Gui {

namespace Ui {
    class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

private:
    void openUserSettings();
    void openProjectSettings();

    void addScriptPath();
    void removeScriptPath();
    void updateScriptPaths();

private:
    std::unique_ptr<Ui::SettingsWidget> ui;
};

} // namespace Gui
