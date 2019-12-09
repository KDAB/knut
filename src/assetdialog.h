#ifndef ASSETDIALOG_H
#define ASSETDIALOG_H

#include <QDialog>

namespace Ui {
class AssetDialog;
}

struct Data;

class AssetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AssetDialog(Data *data, QWidget *parent = nullptr);
    ~AssetDialog();

    void accept() override;

private:
    Ui::AssetDialog *ui;
    Data *m_data = nullptr;
};

#endif // ASSETDIALOG_H
