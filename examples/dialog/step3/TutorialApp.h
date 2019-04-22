// University of Washington Bothell Graphics Library
// Authors: Kelvin Sung, Steve Baer
// The accompanying library supports CSS Graphics courses taught at UW-Bothell
// See: http://courses.washington.edu/css450/
//      http://courses.washington.edu/css451/
///////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"  // main symbols

/// CTutorialApp:
/// All MFC Applications have a single instance of a subclass of CWinApp.
/// This object handles application start-up / shut-down and loading the GUI components
class CTutorialApp : public CWinApp
{
public:
    CTutorialApp();

    /// This could be considered the "entry point" for the application.
    /// The class is constructed on the stack and this function is called at the appropriate
    /// time to allow for command line parsing, setting up windows, ...
    virtual BOOL InitInstance();
    virtual BOOL Run();
    DECLARE_MESSAGE_MAP()
};

/// The one and only application object for this application. All objects
/// can reference this static object by including this header file.
extern CTutorialApp theApp;
