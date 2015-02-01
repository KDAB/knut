#include "stdafx.h"
#include "TutorialApp.h"
#include "TutorialDlg.h"
#include "ui_TutorialDlg.h"

#include <QMouseEvent>

// DEBUG_NEW macro allows MFC applications to determine memory leak locations in debug builds
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CTutorialDlg::CTutorialDlg(QWidget *pParent)
    : QDialog(pParent)
    , m_EchoText("")
    , m_HSliderEcho("")
    , m_VSliderEcho("")
    , m_MouseEcho("")
    , m_TimerEcho("")
    , m_TimerCtrlSliders(true)
    , m_OkCount(0)
    , m_Seconds(0)
    , m_ui(new Ui::CTutorialDlg)
{
    m_ui->setupUi(this);

    m_hIcon = QIcon("res/MFC_UpdateGUI.ico");

    // the IDCANCEL is automatically connected to the reject
    connect(m_ui->PushButton_IDCANCEL, &QPushButton::clicked, this, &QDialog::reject);

    // Replace the MESSAGE_MAP
    connect(m_ui->Slider_IDC_H_SLIDER_BAR, &QSlider::valueChanged,
            this, &CTutorialDlg::OnHScroll);
    connect(m_ui->Slider_IDC_V_SLIDER_BAR, &QSlider::valueChanged,
            this, &CTutorialDlg::OnVScroll);
    connect(m_ui->PushButton_ID_BTN_ADD, &QPushButton::clicked,
            this, &CTutorialDlg::OnBnClickedBtnAdd);
    connect(m_ui->CheckBox_IDC_TIMER_CONTROL_SLIDERS, &QPushButton::clicked,
            this, &CTutorialDlg::OnBnClickedTimerControlSliders);

    // We need to track the mouse, due to the mouse move event
    setMouseTracking(true);
}

CTutorialDlg::~CTutorialDlg()
{
    delete m_ui;
}

void CTutorialDlg::UpdateData(bool fromUi)
{    
    m_VSliderBar = m_ui->Slider_IDC_V_SLIDER_BAR;
    m_HSliderBar = m_ui->Slider_IDC_H_SLIDER_BAR;

    if (fromUi) {
        m_EchoText = m_ui->Label_IDC_ECHO_AREA->text();
        m_HSliderEcho = m_ui->Label_IDC_H_SLIDER_ECHO->text();
        m_VSliderEcho = m_ui->Label_IDC_V_SLIDER_ECHO->text();
        m_MouseEcho = m_ui->Label_IDC_MOUSEECHO->text();
        m_TimerEcho = m_ui->Label_IDC_TIMERECHO->text();
        m_TimerCtrlSliders = m_ui->CheckBox_IDC_TIMER_CONTROL_SLIDERS->isChecked();
    } else {
        m_ui->Label_IDC_ECHO_AREA->setText(m_EchoText);
        m_ui->Label_IDC_H_SLIDER_ECHO->setText(m_HSliderEcho);
        m_ui->Label_IDC_V_SLIDER_ECHO->setText(m_VSliderEcho);
        m_ui->Label_IDC_MOUSEECHO->setText(m_MouseEcho);
        m_ui->Label_IDC_TIMERECHO->setText(m_TimerEcho);
        m_ui->CheckBox_IDC_TIMER_CONTROL_SLIDERS->setChecked(m_TimerCtrlSliders);
    }
}

// This is called when the dialog is first created and shown.
// It is a good spot to initialize member variables.
bool CTutorialDlg::OnInitDialog()
{
#ifdef KDAB_NOT_PORTED
    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);   // Set big icon
    SetIcon(m_hIcon, FALSE);  // Set small icon

    // Add extra initialization here.
    // We want to initialize the slider bars
    m_VSliderBar.SetRange(0, 100, true);
    m_VSliderBar.SetPos(50);
    m_VSliderEcho.Format("%d", 50);

    m_HSliderBar.SetRange(0, 10, true);
    m_HSliderBar.SetPos(5);
    m_HSliderEcho.Format("%d", 5);

    // Initialize the timer to go off every 1000 milliseconds (every second)
    // when timer "goes-off", our OnTimer() event handler function will be
    // called and it is upto us to decide what we want to do.
    SetTimer(0, 1000, NULL);

    UpdateData(false);

    return true;  // return true  unless you set the focus to a control
#endif
    return true;
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.
void CTutorialDlg::paintEvent(QPaintEvent *event)
{
#ifdef KDAB_NOT_PORTED
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
#endif
    QDialog::paintEvent(event);
}

void CTutorialDlg::OnBnClickedBtnAdd()
{
#ifdef KDAB_NOT_PORTED
    m_OkCount++;
    m_EchoText.Format("%d", m_OkCount);

    // Notice, without UpdateData() status area will _NOT_ be updated.
    UpdateData(false);
#endif
}


void CTutorialDlg::OnHScroll()
{
#ifdef KDAB_NOT_PORTED
    // We should check to make sure we know which slider bar is generating the events
    if (pScrollBar == (CScrollBar *) &m_HSliderBar)
    {
        int value = m_HSliderBar.GetPos();
        m_HSliderEcho.Format("%d", value);
        UpdateData(false);
    }
    else
        CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
#endif
}

void CTutorialDlg::OnVScroll()
{
#ifdef KDAB_NOT_PORTED
    // We should check to make sure we know which slider bar is generating the events
    if (pScrollBar == (CScrollBar *) &m_VSliderBar)
    {
        int value = m_VSliderBar.GetPos();
        m_VSliderEcho.Format("%d", value);
        UpdateData(false);
    }
    else
        CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
#endif
}

void CTutorialDlg::timerEvent(QTimerEvent *event)
{
#ifdef KDAB_NOT_PORTED
    m_Seconds++;

    if (m_TimerCtrlSliders)
    {
        // Get ready to decrease the sliders ...
        int hvalue = m_HSliderBar.GetPos();
        if (hvalue > 0)
        {
            m_HSliderBar.SetPos(hvalue-1);
            m_HSliderEcho.Format("%d", hvalue-1);
        }

        int vvalue = m_VSliderBar.GetPos();
        if (vvalue > 0)
        {
            m_VSliderBar.SetPos(vvalue-1);
            m_VSliderEcho.Format("%d", vvalue-1);
        }

        if ( (hvalue==0) && (vvalue==0) )
            m_TimerCtrlSliders = false;
    }

    m_TimerEcho.Format("%d: Seconds have passed", m_Seconds);
    UpdateData(false);
#endif
    QDialog::timerEvent(event);
}

void CTutorialDlg::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
    {
#ifdef KDAB_NOT_PORTED
        QString prefix;
        if(nFlags & MK_CONTROL)
            prefix = "[CTRL]";
        if(nFlags & MK_SHIFT)
            prefix+= "[SHIFT]";
        m_MouseEcho.Format("%sLeft mouse down at %d,%d", prefix, point.x, point.y);
        UpdateData(false);
#endif
    }
        break;
    case Qt::RightButton:
    {
#ifdef KDAB_NOT_PORTED
        QString prefix;
        if(nFlags & MK_CONTROL)
            prefix = "[CTRL]";
        if(nFlags & MK_SHIFT)
            prefix+= "[SHIFT]";
        m_MouseEcho.Format("%sRight mouse down at %d,%d", prefix, point.x, point.y);
        UpdateData(false);
#endif
    }
        break;
    }
    QDialog::mousePressEvent(event);
}


void CTutorialDlg::showEvent(QShowEvent *event)
{
    static bool initialized = false;
    if (!initialized) {
        OnInitDialog();
        initialized = true;
    }
    QDialog::showEvent(event);
}

void CTutorialDlg::mouseMoveEvent(QMouseEvent *event)
{
#ifdef KDAB_NOT_PORTED
    QString prefix;
    if(nFlags & MK_CONTROL)
        prefix = "[CTRL]";
    if(nFlags & MK_SHIFT)
        prefix+= "[SHIFT]";
    m_MouseEcho.Format("%sMouse move at %d,%d", prefix, point.x, point.y);
    UpdateData(false);
#endif
    QDialog::mouseMoveEvent(event);
}

void CTutorialDlg::OnBnClickedTimerControlSliders()
{
#ifdef KDAB_NOT_PORTED
    UpdateData(true);
    // This will fill all UI-connected variables with whatever
    // value that is showing on the UI control objects.
    //
    // In this case, we care most about the value for m_TimerCtrlSliders
#endif
}
