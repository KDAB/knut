#include "stdafx.h"
#include "TutorialApp.h"
#include "TutorialDlg.h"

#include <qmfcapp.h>
#include <qwinwidget.h>
#
// DEBUG_NEW macro allows MFC applications to determine memory leak locations in debug builds
#ifdef _DEBUG
  #define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CTutorialApp, CWinApp)
END_MESSAGE_MAP()

CTutorialApp::CTutorialApp()
{
}

// The one and only CTutorialApp object
CTutorialApp theApp;

BOOL CTutorialApp::InitInstance()
{
  // InitCommonControls() is required on Windows XP if an application
  // manifest specifies use of ComCtl32.dll version 6 or later to enable
  // visual styles.  Otherwise, any window creation will fail.
  InitCommonControls();

  // Qt initialization
  QMfcApp::instance(this);

  // Create and show our dialog
  CTutorialDlg dlg;
  dlg.exec();

  // Since the dialog has been closed, return FALSE so that we exit the
  //  application, rather than start the application's message pump.
  return false;
}

BOOL CTutorialApp::Run()
{
    int result = QMfcApp::run(this);
    delete qApp;
    return result;
}
