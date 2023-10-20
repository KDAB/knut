#include "stdafx.h"
#include "TutorialApp.h"
#include "TutorialDlg.h"
#include "ui_TutorialDlg.h"

#include <QMouseEvent>

// DEBUG_NEW macro allows MFC applications to determine memory leak locations in debug builds
#ifdef _DEBUG
  #define new DEBUG_NEW
#endif


CTutorialDlg::CTutorialDlg(CWnd* pParent)
: CDialog(CTutorialDlg::IDD, pParent)
, m_EchoText(L"")
, m_HSliderEcho(L"")
, m_VSliderEcho(L"")
, m_MouseEcho(L"")
, m_TimerEcho(L"")
, m_TimerCtrlSliders(true)
, m_OkCount(0)
, m_Seconds(0)
{
    m_ui = std::make_unique<Ui::CTutorialDlg>();
    m_ui->setupUi(this);

  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    
    connect(m_ui->cancel, &QAbstractButton::clicked, this, &CTutorialDlg::reject);
    connect(m_ui->v_slider_bar, &QSlider::valueChanged, this, &CTutorialDlg::OnHScroll);
    connect(m_ui->h_slider_bar, &QSlider::valueChanged, this, &CTutorialDlg::OnHScroll);
    connect(m_ui->v_slider_bar, &QSlider::valueChanged, this, &CTutorialDlg::OnVScroll);
    connect(m_ui->h_slider_bar, &QSlider::valueChanged, this, &CTutorialDlg::OnVScroll);
    connect(m_ui->btn_add, &QAbstractButton::clicked, this, &CTutorialDlg::OnBnClickedBtnAdd);
    connect(m_ui->timer_control_sliders, &QAbstractButton::clicked, this, &CTutorialDlg::OnBnClickedTimerControlSliders);
    
    // Create the mapping between the MFC proxy classes and the Qt widget
    DoDataExchange();
    
    // KDAB_TODO Mfc delay the initialisation to the first time the dialog is shown
    // This is usually not needed, if it is you need to call OnInitDialog from showEvent once
    OnInitDialog();
}

void CTutorialDlg::DoDataExchange()
{
    m_EchoText.setWidget(m_ui->echo_area);
    m_HSliderEcho.setWidget(m_ui->h_slider_echo);
    m_VSliderEcho.setWidget(m_ui->v_slider_echo);
    m_VSliderBar.setWidget(m_ui->v_slider_bar);
    m_HSliderBar.setWidget(m_ui->h_slider_bar);
    m_MouseEcho.setWidget(m_ui->mouseecho);
    m_TimerEcho.setWidget(m_ui->timerecho);
    m_TimerCtrlSliders.setWidget(m_ui->timer_control_sliders);
}

BEGIN_MESSAGE_MAP(CTutorialDlg, CDialog)
  
  
  
  
  
  
  
  
  
END_MESSAGE_MAP()

// This is called when the dialog is first created and shown.
// It is a good spot to initialize member variables.
bool CTutorialDlg::OnInitDialog()
{

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, true);   // Set big icon
  SetIcon(m_hIcon, false);  // Set small icon

  // Add extra initialization here.
  // We want to initialize the slider bars
  m_VSliderBar.SetRange(0, 100, true);
  m_VSliderBar.SetPos(50);
  m_VSliderEcho.Format(L"%d", 50);

  m_HSliderBar.SetRange(0, 10, true);
  m_HSliderBar.SetPos(5);
  m_HSliderEcho.Format(L"%d", 5);

  // Initialize the timer to go off every 1000 milliseconds (every second)
  // when timer "goes-off", our OnTimer() event handler function will be
  // called and it is upto us to decide what we want to do.
  startTimer(1000);

	
  return true;  // return true  unless you set the focus to a control
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
  m_EchoText.Format(L"%d", m_OkCount);

  // Notice, without UpdateData() status area will _NOT_ be updated.
}


void CTutorialDlg::OnHScroll(int nPos)
{
  // We should check to make sure we know which slider bar is generating the events
  if (sender() == m_HSliderBar.widget())
  {
    int value = m_HSliderBar.GetPos();
    m_HSliderEcho.Format(L"%d", value);
  }
  else
    {/*KDAB_TODO(remove): CDialog::OnHScroll(nSBCode, nPos, sender())*/};
}

void CTutorialDlg::OnVScroll(int nPos)
{
  // We should check to make sure we know which slider bar is generating the events
  if (sender() == m_VSliderBar.widget())
  {
    int value = m_VSliderBar.GetPos();
    m_VSliderEcho.Format(L"%d", value);
  }
  else
    {/*KDAB_TODO(remove): CDialog::OnVScroll(nSBCode, nPos, sender())*/};
}

void CTutorialDlg::timerEvent(QTimerEvent *event)
{
  m_Seconds++;

  if (m_TimerCtrlSliders) 
  {
    // Get ready to decrease the sliders ...
    int hvalue = m_HSliderBar.GetPos();
    if (hvalue > 0) 
    {
      m_HSliderBar.SetPos(hvalue-1);
      m_HSliderEcho.Format(L"%d", hvalue-1);
    }

    int vvalue = m_VSliderBar.GetPos();
    if (vvalue > 0) 
    {
      m_VSliderBar.SetPos(vvalue-1);
      m_VSliderEcho.Format(L"%d", vvalue-1);
    }
    
    if ( (hvalue==0) && (vvalue==0) )
      m_TimerCtrlSliders = false;
  }

  m_TimerEcho.Format(L"%d: Seconds have passed", m_Seconds);
}

void CTutorialDlg::OnLButtonDown(QMouseEvent *event)
{
  CString prefix;
  if(event->modifiers() & Qt::ControlModifier)
    prefix = L"[CTRL]";
  if(event->modifiers() & Qt::ShiftModifier)
    prefix+= L"[SHIFT]";
  m_MouseEcho.Format(L"%sLeft mouse down at %d,%d", prefix, event->pos().x(), event->pos().y());
}

void CTutorialDlg::mouseMoveEvent(QMouseEvent *event)
{
  CString prefix;
  if(event->modifiers() & Qt::ControlModifier)
    prefix = L"[CTRL]";
  if(event->modifiers() & Qt::ShiftModifier)
    prefix+= L"[SHIFT]";
  m_MouseEcho.Format(L"%sMouse move at %d,%d", prefix, event->pos().x(), event->pos().y());
}

void CTutorialDlg::OnRButtonDown(QMouseEvent *event)
{
  CString prefix;
  if(event->modifiers() & Qt::ControlModifier)
    prefix = L"[CTRL]";
  if(event->modifiers() & Qt::ShiftModifier)
    prefix+= L"[SHIFT]";
  m_MouseEcho.Format(L"%sRight mouse down at %d,%d", prefix, event->pos().x(), event->pos().y());
}

void CTutorialDlg::OnBnClickedTimerControlSliders()
{
  // This will fill all UI-connected variables with whatever 
  // value that is showing on the UI control objects.
  // 
  // In this case, we care most about the value for m_TimerCtrlSliders
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
