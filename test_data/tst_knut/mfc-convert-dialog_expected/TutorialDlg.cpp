#include "stdafx.h"
#include "TutorialApp.h"
#include "TutorialDlg.h"
#include "ui_TutorialDlg.cpp"

// DEBUG_NEW macro allows MFC applications to determine memory leak locations in debug builds
#ifdef _DEBUG
  #define new DEBUG_NEW
#endif


CTutorialDlg::CTutorialDlg(CWnd* pParent)
: CDialog(CTutorialDlg::IDD, pParent)
, m_ui->EchoText(L"")
, m_ui->HSliderEcho(L"")
, m_ui->VSliderEcho(L"")
, m_ui->MouseEcho(L"")
, m_ui->TimerEcho(L"")
, m_ui->TimerCtrlSliders(TRUE)
, m_OkCount(0)
, m_Seconds(0)
{
    m_ui = std::make_unique<Ui::CTutorialDlg>();
    m_ui->setupUi(this);

  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    connect(m_ui->VSliderBar, &QSlider::valueChanged, this, &CTutorialDlg::OnHScroll);
    connect(m_ui->HSliderBar, &QSlider::valueChanged, this, &CTutorialDlg::OnHScroll);
    connect(m_ui->VSliderBar, &QSlider::valueChanged, this, &CTutorialDlg::OnVScroll);
    connect(m_ui->HSliderBar, &QSlider::valueChanged, this, &CTutorialDlg::OnVScroll);
    connect(m_ui->BTN_ADD, &QAbstractButton::clicked, this, &CTutorialDlg::OnBnClickedBtnAdd);
    connect(IDC_TIMER_CONTROL_SLIDERS, &QAbstractButton::clicked, this, &CTutorialDlg::OnBnClickedTimerControlSliders);
}

void CTutorialDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_ECHO_AREA, m_ui->EchoText);
  DDX_Text(pDX, IDC_H_SLIDER_ECHO, m_ui->HSliderEcho);
  DDX_Text(pDX, IDC_V_SLIDER_ECHO, m_ui->VSliderEcho);
  DDX_Control(pDX, IDC_V_SLIDER_BAR, m_ui->VSliderBar);
  DDX_Control(pDX, IDC_H_SLIDER_BAR, m_ui->HSliderBar);
  DDX_Text(pDX, IDC_MOUSEECHO, m_ui->MouseEcho);
  DDX_Text(pDX, IDC_TIMERECHO, m_ui->TimerEcho);
  DDX_Check(pDX, IDC_TIMER_CONTROL_SLIDERS, m_ui->TimerCtrlSliders);
}

BEGIN_MESSAGE_MAP(CTutorialDlg, CDialog)
  
  
  
  
  
  
  
  
  
END_MESSAGE_MAP()

// This is called when the dialog is first created and shown.
// It is a good spot to initialize member variables.
BOOL CTutorialDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);   // Set big icon
  SetIcon(m_hIcon, FALSE);  // Set small icon

  // Add extra initialization here.
  // We want to initialize the slider bars
  m_ui->VSliderBar.SetRange(0, 100, TRUE);
  m_ui->VSliderBar.SetPos(50);
  m_ui->VSliderEcho.Format(L"%d", 50);

  m_ui->HSliderBar.SetRange(0, 10, TRUE);
  m_ui->HSliderBar.SetPos(5);
  m_ui->HSliderEcho.Format(L"%d", 5);

  // Initialize the timer to go off every 1000 milliseconds (every second)
  // when timer "goes-off", our OnTimer() event handler function will be
  // called and it is upto us to decide what we want to do.
  startTimer(1000);

  UpdateData(false);
	
  return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.
void CTutorialDlg::paintEvent(QPaintEvent *event) 
{
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
}

void CTutorialDlg::OnBnClickedBtnAdd()
{
  m_OkCount++;
  m_ui->EchoText.Format(L"%d", m_OkCount);

  // Notice, without UpdateData() status area will _NOT_ be updated.
  UpdateData(FALSE);
}


void CTutorialDlg::OnHScroll(int nPos)
{
  // We should check to make sure we know which slider bar is generating the events
  if (sender() == (CScrollBar *) &m_ui->HSliderBar)
  {
    int value = m_ui->HSliderBar.GetPos();
    m_ui->HSliderEcho.Format(L"%d", value);
    UpdateData(false);
  }
  else
    {/*DEPRECATED: CDialog::OnHScroll(nSBCode, nPos, sender())*/};
}

void CTutorialDlg::OnVScroll(int nPos)
{
  // We should check to make sure we know which slider bar is generating the events
  if (sender() == (CScrollBar *) &m_ui->VSliderBar)
  {
    int value = m_ui->VSliderBar.GetPos();
    m_ui->VSliderEcho.Format(L"%d", value);
    UpdateData(false);
  }
  else
    {/*DEPRECATED: CDialog::OnVScroll(nSBCode, nPos, sender())*/};
}

void CTutorialDlg::timerEvent(QTimerEvent *event)
{
  m_Seconds++;

  if (m_ui->TimerCtrlSliders) 
  {
    // Get ready to decrease the sliders ...
    int hvalue = m_ui->HSliderBar.GetPos();
    if (hvalue > 0) 
    {
      m_ui->HSliderBar.SetPos(hvalue-1);
      m_ui->HSliderEcho.Format(L"%d", hvalue-1);
    }

    int vvalue = m_ui->VSliderBar.GetPos();
    if (vvalue > 0) 
    {
      m_ui->VSliderBar.SetPos(vvalue-1);
      m_ui->VSliderEcho.Format(L"%d", vvalue-1);
    }
    
    if ( (hvalue==0) && (vvalue==0) )
      m_ui->TimerCtrlSliders = false;
  }

  m_ui->TimerEcho.Format(L"%d: Seconds have passed", m_Seconds);
  UpdateData(false);
}

void CTutorialDlg::OnLButtonDown(QMouseEvent *event)
{
  CString prefix;
  if(event->modifiers() & MK_CONTROL)
    prefix = L"[CTRL]";
  if(event->modifiers() & MK_SHIFT)
    prefix+= L"[SHIFT]";
  m_ui->MouseEcho.Format(L"%sLeft mouse down at %d,%d", prefix, event->pos().x, event->pos().y);
  UpdateData(false);
}

void CTutorialDlg::mouseMoveEvent(QMouseEvent *event)
{
  CString prefix;
  if(event->modifiers() & MK_CONTROL)
    prefix = L"[CTRL]";
  if(event->modifiers() & MK_SHIFT)
    prefix+= L"[SHIFT]";
  m_ui->MouseEcho.Format(L"%sMouse move at %d,%d", prefix, event->pos().x, event->pos().y);
  UpdateData(false);
}

void CTutorialDlg::OnRButtonDown(QMouseEvent *event)
{
  CString prefix;
  if(event->modifiers() & MK_CONTROL)
    prefix = L"[CTRL]";
  if(event->modifiers() & MK_SHIFT)
    prefix+= L"[SHIFT]";
  m_ui->MouseEcho.Format(L"%sRight mouse down at %d,%d", prefix, event->pos().x, event->pos().y);
  UpdateData(false);
}

void CTutorialDlg::OnBnClickedTimerControlSliders()
{
  UpdateData(true);
  // This will fill all UI-connected variables with whatever 
  // value that is showing on the UI control objects.
  // 
  // In this case, we care most about the value for m_ui->TimerCtrlSliders
}

void CTutorialDlg::mousePressEvent(QMouseEvent * event) {

    switch (event->button()) {
    
    case Qt::RightButton:
    {
        OnRButtonDown(event);
        break;
    }
case Qt::LeftButton:
    {
        OnLButtonDown(event);
        break;
    }
    default:
        break;
    }

    QDialog::mousePressEvent(event);
            
}
