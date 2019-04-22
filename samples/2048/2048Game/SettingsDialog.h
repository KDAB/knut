#pragma once
#include "afxwin.h"


// SettingsDialog dialog

class SettingsDialog : public CDialogEx
{
	DECLARE_DYNAMIC(SettingsDialog)

public:
	SettingsDialog(int* size, CString* name, CWnd* pParent = NULL);   // standard constructor
	virtual ~SettingsDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int* size;
	CString* name;
	CComboBox squareCountVar;
	virtual BOOL OnInitDialog();
//	int squareCountInt;
	afx_msg void OnBnClickedOk();
	CString nameVar;
};
