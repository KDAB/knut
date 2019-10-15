#ifndef UIDIALOG_H
#define UIDIALOG_H

#include <QDialog>

namespace Ui {
class UiDialog;
}

struct Data;
class OverviewFilterModel;

class UiDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UiDialog(Data *data, QWidget *parent = nullptr);
    ~UiDialog();

    void accept() override;

private:
    Ui::UiDialog *ui;
    Data *m_data = nullptr;
    OverviewFilterModel *m_filterModel = nullptr;
};

#endif // UIDIALOG_H
