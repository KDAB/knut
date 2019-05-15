#ifndef TOOLBARDIALOG_H
#define TOOLBARDIALOG_H

#include <QDialog>
#include <QVariantList>

namespace Ui {
class ToolbarDialog;
}

struct Data;
class OverviewFilterModel;

class ToolbarDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ToolbarDialog(Data *data, QWidget *parent = nullptr);
    ~ToolbarDialog();

private:
    void run();

private:
    Ui::ToolbarDialog *ui;
    Data *m_data = nullptr;
    OverviewFilterModel *m_filterModel = nullptr;
};

#endif // MENUDIALOG_H
