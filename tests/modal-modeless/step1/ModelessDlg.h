#include <QDialog>


namespace Ui {
class CModelessDlg;
}
// CModelessDlg dialog

class CModelessDlg : public QDialog
{
    Q_OBJECT

public:
    CModelessDlg(QWidget *parent = 0);   // standard constructor
    virtual ~CModelessDlg();

private:
    Ui::CModelessDlg *m_ui;
};
