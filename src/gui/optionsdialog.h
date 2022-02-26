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
    ~OptionsDialog();

    void initialize();

private:
    std::unique_ptr<Ui::OptionsDialog> ui;
};

} // namespace Gui
