#ifndef ACTIONDIALOG_H
#define ACTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ActionDialog;
}

struct Data;
class OverviewFilterModel;

struct Action
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(QStringList shortcuts MEMBER shortcuts)
    Q_PROPERTY(QString toolTip MEMBER toolTip)
    Q_PROPERTY(QString statusTip MEMBER statusTip)
    Q_PROPERTY(bool checkable MEMBER checkable)

public:
    QString id;
    QString title;
    QStringList shortcuts;
    QString toolTip;
    QString statusTip;
    bool checkable;
};
Q_DECLARE_METATYPE(Action);

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
