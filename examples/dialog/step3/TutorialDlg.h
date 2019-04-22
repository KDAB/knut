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
    //virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    virtual void UpdateData(bool fromUi) ;

    //virtual BOOL OnInitDialog();
    virtual bool OnInitDialog();

    //afx_msg void OnPaint();
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    //afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    //afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    //afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

    //afx_msg void OnTimer(UINT nIDEvent);
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

protected slots:
    void OnHScroll();

    void OnVScroll();

    void OnBnClickedBtnAdd();

    void OnBnClickedTimerControlSliders();

private:
    int m_Seconds;
    int m_OkCount;
    QString m_EchoText;

    QString m_HSliderEcho;
    QString m_VSliderEcho;

    QString m_MouseEcho;
    QString m_TimerEcho;

    bool m_TimerCtrlSliders;
    QIcon m_hIcon;

    Ui::CTutorialDlg *m_ui;
};
