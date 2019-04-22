// University of Washington Bothell Graphics Library
// Authors: Kelvin Sung, Steve Baer
// The accompanying library supports CSS Graphics courses taught at UW-Bothell
// See: http://courses.washington.edu/css450/
//      http://courses.washington.edu/css451/
///////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <QDialog>
#include <QIcon>

namespace Ui {
class CTutorialDlg;
}

/// Main dialog for the sample application. This is created and displayed in
/// the CTutorialApp::InitInstance function.
class CTutorialDlg : public QDialog
{
    Q_OBJECT

public:
    CTutorialDlg(QWidget* pParent = 0);
    ~CTutorialDlg();

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

protected slots:
    void OnHScroll(int value);

    void OnVScroll(int value);

    void OnBnClickedBtnAdd();

private:
    int m_Seconds;
    int m_OkCount;

    QIcon m_hIcon;

    Ui::CTutorialDlg *m_ui;
};
