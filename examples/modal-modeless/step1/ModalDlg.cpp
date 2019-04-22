// ModalDlg.cpp : implementation file
//

#include "ModalDlg.h"
#include "ui_ModalDlg.h"

// CModalDlg dialog

CModalDlg::CModalDlg(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui::CModalDlg)
{
    m_ui->setupUi(this);
    connect(m_ui->PushButton_IDOK2, &QPushButton::clicked,
            this, &QDialog::accept);
    connect(m_ui->PushButton_IDCANCEL1, &QPushButton::clicked,
            this, &QDialog::reject);
}

CModalDlg::~CModalDlg()
{
    delete m_ui;
}
