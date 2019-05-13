#ifndef MENUDIALOG_H
#define MENUDIALOG_H

#include <QDialog>
#include <QVariantList>

namespace Ui {
class MenuDialog;
}

struct Data;
class OverviewFilterModel;

struct Menu
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(bool isSeparator MEMBER isSeparator)
    Q_PROPERTY(bool isAction MEMBER isAction)
    Q_PROPERTY(QVariantList children MEMBER children)

public:
    QString id;
    QString title;
    bool isSeparator = false;
    bool isAction = false;
    QVariantList children;
};
Q_DECLARE_METATYPE(Menu);

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
