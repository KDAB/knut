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
