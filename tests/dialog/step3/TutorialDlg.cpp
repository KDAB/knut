#include "TutorialDlg.h"
#include "ui_TutorialDlg.h"

#include <QMouseEvent>


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
    connect(m_ui->cancel, &QPushButton::clicked, this, &QDialog::reject);

    // Replace the MESSAGE_MAP
    connect(m_ui->hSliderBar, &QSlider::valueChanged,
            this, &CTutorialDlg::OnHScroll);
    connect(m_ui->vSliderBar, &QSlider::valueChanged,
            this, &CTutorialDlg::OnVScroll);
    connect(m_ui->btnAdd, &QPushButton::clicked,
            this, &CTutorialDlg::OnBnClickedBtnAdd);
    connect(m_ui->timerControlSliders, &QPushButton::clicked,
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
    if (fromUi) {
        m_EchoText = m_ui->echoArea->text();
        m_HSliderEcho = m_ui->hSliderEcho->text();
        m_VSliderEcho = m_ui->vSliderEcho->text();
        m_MouseEcho = m_ui->mouseecho->text();
        m_TimerEcho = m_ui->timerecho->text();
        m_TimerCtrlSliders = m_ui->timerControlSliders->isChecked();
    } else {
        m_ui->echoArea->setText(m_EchoText);
        m_ui->hSliderEcho->setText(m_HSliderEcho);
        m_ui->vSliderEcho->setText(m_VSliderEcho);
        m_ui->mouseecho->setText(m_MouseEcho);
        m_ui->timerecho->setText(m_TimerEcho);
        m_ui->timerControlSliders->setChecked(m_TimerCtrlSliders);
    }
}

// This is called when the dialog is first created and shown.
// It is a good spot to initialize member variables.
bool CTutorialDlg::OnInitDialog()
{
    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    setWindowIcon(m_hIcon);   // Set big icon

    // Add extra initialization here.
    // We want to initialize the slider bars
    m_ui->vSliderBar->setRange(0, 100);
    m_ui->vSliderBar->setValue(50);
    m_VSliderEcho = QString::number(50);

    m_ui->hSliderBar->setRange(0, 10);
    m_ui->hSliderBar->setValue(5);
    m_HSliderEcho = QString::number(5);

    // Initialize the timer to go off every 1000 milliseconds (every second)
    // when timer "goes-off", our OnTimer() event handler function will be
    // called and it is upto us to decide what we want to do.
    startTimer(1000);

    UpdateData(false);

    return true;  // return true  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.
void CTutorialDlg::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);
}

void CTutorialDlg::OnBnClickedBtnAdd()
{
    m_OkCount++;
    m_EchoText = QString::number(m_OkCount);

    // Notice, without UpdateData() status area will _NOT_ be updated.
    UpdateData(false);
}


void CTutorialDlg::OnHScroll()
{
    // We should check to make sure we know which slider bar is generating the events
    if (sender() == m_ui->hSliderBar)
    {
        int value = m_ui->hSliderBar->value();
        m_HSliderEcho = QString::number(value);
        UpdateData(false);
    }
}

void CTutorialDlg::OnVScroll()
{
    // We should check to make sure we know which slider bar is generating the events
    if (sender() == m_ui->vSliderBar)
    {
        int value = m_ui->vSliderBar->value();
        m_VSliderEcho = QString::number(value);
        UpdateData(false);
    }
}

void CTutorialDlg::timerEvent(QTimerEvent *event)
{
    m_Seconds++;

    if (m_TimerCtrlSliders)
    {
        // Get ready to decrease the sliders ...
        int hvalue = m_ui->hSliderBar->value();
        if (hvalue > 0)
        {
            m_ui->hSliderBar->setValue(hvalue-1);
            m_HSliderEcho = QString::number(hvalue-1);
        }

        int vvalue = m_ui->vSliderBar->value();
        if (vvalue > 0)
        {
            m_ui->vSliderBar->setValue(vvalue-1);
            m_VSliderEcho = QString::number(vvalue-1);
        }

        if ( (hvalue==0) && (vvalue==0) )
            m_TimerCtrlSliders = false;
    }

    m_TimerEcho = tr("%1: Seconds have passed").arg(m_Seconds);
    UpdateData(false);
    QDialog::timerEvent(event);
}

void CTutorialDlg::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
    {
        QString prefix;
        if(event->modifiers() & Qt::ControlModifier)
            prefix = "[CTRL]";
        if(event->modifiers() & Qt::ShiftModifier)
            prefix+= "[SHIFT]";
        m_MouseEcho = tr("%1Left mouse down at %2,%3")
                .arg(prefix).arg(event->pos().x()).arg(event->pos().y());
        UpdateData(false);
    }
        break;
    case Qt::RightButton:
    {
        QString prefix;
        if(event->modifiers() & Qt::ControlModifier)
            prefix = "[CTRL]";
        if(event->modifiers() & Qt::ShiftModifier)
            prefix+= "[SHIFT]";
        m_MouseEcho = tr("%1Right mouse down at %2,%3")
                .arg(prefix).arg(event->pos().x()).arg(event->pos().y());
        UpdateData(false);
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
    QString prefix;
    if(event->modifiers() & Qt::ControlModifier)
        prefix = "[CTRL]";
    if(event->modifiers() & Qt::ShiftModifier)
        prefix+= "[SHIFT]";
    m_MouseEcho = tr("%1Mouse move at %2,%3").arg(prefix)
            .arg(event->pos().x()).arg(event->pos().y());
    UpdateData(false);
    QDialog::mouseMoveEvent(event);
}

void CTutorialDlg::OnBnClickedTimerControlSliders()
{
    UpdateData(true);
    // This will fill all UI-connected variables with whatever
    // value that is showing on the UI control objects.
    //
    // In this case, we care most about the value for m_TimerCtrlSliders
}
