#include <QDialog>


namespace Ui {
class CModalDlg;
}
// CModalDlg dialog

class CModalDlg : public QDialog
{
    Q_OBJECT

public:
    CModalDlg(QWidget *parent = 0);   // standard constructor
	virtual ~CModalDlg();

private:
    Ui::CModalDlg *m_ui;
};
