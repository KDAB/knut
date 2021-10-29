#ifndef GUI_RUNSCRIPTDIALOG_H
#define GUI_RUNSCRIPTDIALOG_H

#include <QDialog>

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
    Ui::RunScriptDialog *const ui;
};

} // namespace Gui
#endif // GUI_RUNSCRIPTDIALOG_H
