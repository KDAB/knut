// SettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "2048Game.h"
#include "SettingsDialog.h"
#include "afxdialogex.h"


// SettingsDialog dialog

IMPLEMENT_DYNAMIC(SettingsDialog, CDialogEx)

SettingsDialog::SettingsDialog(int* size, CString* name, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, nameVar(_T(""))
{
	this->size = size;
	this->name = name;
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, squareCountVar);
	//  DDX_CBIndex(pDX, IDC_COMBO1, squareCountInt);
	DDX_CBString(pDX, IDC_COMBO2, nameVar);
}


BEGIN_MESSAGE_MAP(SettingsDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &SettingsDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// SettingsDialog message handlers


BOOL SettingsDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (size) {
		CString str;
		str.Format(L"%d", *size);
		//AfxMessageBox(str);
		int ind = squareCountVar.FindString(0, str);
		if (ind == -1)
			ind = 0;
		
		squareCountVar.SetCurSel(ind);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void SettingsDialog::OnBnClickedOk()
{
	UpdateData();

	CString str;
	squareCountVar.GetLBText(squareCountVar.GetCurSel(), str);
	*size = _wtoi(str);
	*name = nameVar;

	CDialogEx::OnOK();
}
