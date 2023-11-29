#pragma once

#include <QDialog>

#include <memory>

namespace Gui {

namespace Ui {
    class RunScriptWidget;
}

class RunScriptWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RunScriptWidget(QWidget *parent = nullptr);
    ~RunScriptWidget() override;

    void open();

private:
    void chooseScript();
    void run();

private:
    std::unique_ptr<Ui::RunScriptWidget> ui;
};

} // namespace Gui
