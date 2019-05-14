#ifndef ACTIONDIALOG_H
#define ACTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ActionDialog;
}

struct Data;
class OverviewFilterModel;

class ActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ActionDialog(Data *data, QWidget *parent = nullptr);
    ~ActionDialog();

private:
    void run();

private:
    Ui::ActionDialog *ui;
    Data *m_data = nullptr;
    OverviewFilterModel *m_filterModel;
};

#endif // ACTIONDIALOG_H
