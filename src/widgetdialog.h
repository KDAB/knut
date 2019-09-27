#ifndef WIDGETDIALOG_H
#define WIDGETDIALOG_H

#include <QDialog>
#include <QVariantList>

namespace Ui {
class WidgetDialog;
}

struct Data;
class OverviewFilterModel;

class WidgetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WidgetDialog(Data *data, QWidget *parent = nullptr);
    ~WidgetDialog();

private:
    void run();
    void preview();

private:
    Ui::WidgetDialog *ui;
    Data *m_data = nullptr;
    OverviewFilterModel *m_filterModel = nullptr;
};

#endif // WIDGETDIALOG_H
