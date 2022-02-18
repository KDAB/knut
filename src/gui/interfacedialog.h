#pragma once

#include <QDialog>

#include <memory>

namespace Gui {

class InterfaceSettings;

namespace Ui {
    class InterfaceDialog;
}

class InterfaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InterfaceDialog(QWidget *parent = nullptr);
    ~InterfaceDialog();

    void initialize(InterfaceSettings *settings);

private:
    InterfaceSettings *m_settings = nullptr;
    std::unique_ptr<Ui::InterfaceDialog> ui;
};

} // namespace Gui
