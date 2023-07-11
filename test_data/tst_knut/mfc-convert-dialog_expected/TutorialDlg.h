// University of Washington Bothell Graphics Library
// Authors: Kelvin Sung, Steve Baer
// The accompanying library supports CSS Graphics courses taught at UW-Bothell
// See: http://courses.washington.edu/css450/
//      http://courses.washington.edu/css451/
///////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <memory>
#include <QDialog>

namespace Ui {
class CTutorialDlg;
}

/// Main dialog for the sample application. This is created and displayed in
/// the CTutorialApp::InitInstance function.
class CTutorialDlg : public CDialog
{
public:
  CTutorialDlg(CWnd* pParent = NULL);

  /// The IDD enum is a common technique in MFC to associate a dialog with
  /// a resource that you edit in the dialog editor
  enum { IDD = IDD_UPDATEGUI_DIALOG };

protected:
  /// The virtual DoDataExchange is an MFC method for synchronizing values in this
  /// class with their corresponding controls on the dialog.
  /// See MSDN documentation for more information
  virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

  /// Called immediately after a dialog is created, but before it is displayed.
  /// This is where all of the control initialization usually occurs. This is
  /// also where the graphics window is set up
  virtual BOOL OnInitDialog();

  /// A message map is an MFC macro for mapping window's events (paint, size, mouse,...) that
  /// occur on a window to functions in this class.
  /// The functions in this class that begin with afx_msg are "handlers" for window's messages
  DECLARE_MESSAGE_MAP()

  /// Called when this dialog receives a WM_PAINT message (event)
  /// The MFC wizard adds code to paint properly if this dialog is minimized to the taskbar
  void paintEvent(QPaintEvent *event) override;

  /// Called when this dialog receives slider scroll messages from the slider controls
  /// Horizontal Scroll
  void OnHScroll(int nPos);

  /// Called when this dialog receives slider scroll messages from the slider controls
  /// Vertical Scroll
  void OnVScroll(int nPos);
  void timerEvent(QTimerEvent *event) override;
  void OnLButtonDown(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event) override;
  void OnRButtonDown(QMouseEvent *event);
  ///
  /// Called when this dialog receives button click message from the add button
   void OnBnClickedBtnAdd();
   void OnBnClickedTimerControlSliders();
  void mousePressEvent(QMouseEvent * event) override;

private:
  int m_Seconds;
  int m_OkCount;
  CString m_EchoText;

  CString m_HSliderEcho;
  CString m_VSliderEcho;
  CSliderCtrl m_VSliderBar;
  CSliderCtrl m_HSliderBar;

  CString m_MouseEcho;
  CString m_TimerEcho;

  BOOL m_TimerCtrlSliders;
  HICON m_hIcon;
private:
	std::unique_ptr<Ui::CTutorialDlg> m_ui;
};
