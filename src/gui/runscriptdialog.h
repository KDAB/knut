#pragma once

#include <QDialog>

#include <memory>

namespace Gui {

namespace Ui {
    class RunScriptDialog;
}

class RunScriptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunScriptDialog(QWidget *parent = nullptr);
    ~RunScriptDialog();

public slots:
    void accept() override;

private:
    void chooseScript();

private:
    std::unique_ptr<Ui::RunScriptDialog> ui;
};

} // namespace Gui
