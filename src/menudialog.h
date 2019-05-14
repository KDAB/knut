#ifndef MENUDIALOG_H
#define MENUDIALOG_H

#include <QDialog>
#include <QVariantList>

namespace Ui {
class MenuDialog;
}

struct Data;
class OverviewFilterModel;

class MenuDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MenuDialog(Data *data, QWidget *parent = nullptr);
    ~MenuDialog();

private:
    void run();

private:
    Ui::MenuDialog *ui;
    Data *m_data = nullptr;
    OverviewFilterModel *m_filterModel;
};

#endif // MENUDIALOG_H
