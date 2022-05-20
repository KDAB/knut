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
    ~InterfaceSettings();

    void initialize();

private:
    std::unique_ptr<Ui::InterfaceSettings> ui;
};

} // namespace Gui
