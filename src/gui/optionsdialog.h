#pragma once

#include <QDialog>

#include <memory>

namespace Gui {

class GuiSettings;

namespace Ui {
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(GuiSettings *settings, QWidget *parent = nullptr);
    ~OptionsDialog();

    void initialize();

private:
    std::unique_ptr<Ui::OptionsDialog> ui;
    GuiSettings *m_settings = nullptr;
};

} // namespace Gui
