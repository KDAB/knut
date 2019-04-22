// ModelessDlg.cpp : implementation file
//

#include "ModelessDlg.h"
#include "ui_ModelessDlg.h"

// CModelessDlg dialog

CModelessDlg::CModelessDlg(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui::CModelessDlg)
{
    m_ui->setupUi(this);
    connect(m_ui->PushButton_IDOK3, &QPushButton::clicked,
            this, &QDialog::accept);
    connect(m_ui->PushButton_IDCANCEL2, &QPushButton::clicked,
            this, &QDialog::reject);
}

CModelessDlg::~CModelessDlg()
{
    delete m_ui;
}
