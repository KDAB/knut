#include "TutorialDlg.h"
#include "ui_TutorialDlg.h"

#include <QMouseEvent>


CTutorialDlg::CTutorialDlg(QWidget *pParent)
    : QDialog(pParent)
    , m_OkCount(0)
    , m_Seconds(0)
    , m_ui(new Ui::CTutorialDlg)
{
    m_ui->setupUi(this);

    m_hIcon = QIcon("res/MFC_UpdateGUI.ico");
    setWindowIcon(m_hIcon);

    // the IDCANCEL is automatically connected to the reject
    connect(m_ui->cancel, &QPushButton::clicked, this, &QDialog::reject);

    // Replace the MESSAGE_MAP
    connect(m_ui->hSliderBar, &QSlider::valueChanged,
            this, &CTutorialDlg::OnHScroll);
    connect(m_ui->vSliderBar, &QSlider::valueChanged,
            this, &CTutorialDlg::OnVScroll);
    connect(m_ui->btnAdd, &QPushButton::clicked,
            this, &CTutorialDlg::OnBnClickedBtnAdd);

    // We need to track the mouse, due to the mouse move event
    setMouseTracking(true);

    // We want to initialize the slider bars
    m_ui->vSliderBar->setRange(0, 100);
    m_ui->vSliderBar->setValue(50);
    m_ui->vSliderEcho->setText(QString::number(50));

    m_ui->hSliderBar->setRange(0, 10);
    m_ui->hSliderBar->setValue(5);
    m_ui->hSliderEcho->setText(QString::number(5));

    m_ui->echoArea->setText("");
    m_ui->mouseecho->setText("");
    m_ui->timerecho->setText("");
    m_ui->timerControlSliders->setChecked(true);
}

CTutorialDlg::~CTutorialDlg()
{
    delete m_ui;
}

void CTutorialDlg::OnBnClickedBtnAdd()
{
    m_OkCount++;
    m_ui->echoArea->setText(QString::number(m_OkCount));
}


void CTutorialDlg::OnHScroll(int value)
{
    m_ui->hSliderEcho->setText(QString::number(value));
}

void CTutorialDlg::OnVScroll(int value)
{
    m_ui->vSliderEcho->setText(QString::number(value));
}

void CTutorialDlg::timerEvent(QTimerEvent *event)
{
    m_Seconds++;

    if (m_ui->timerControlSliders->isChecked())
    {
        // Get ready to decrease the sliders ...
        int hvalue = m_ui->hSliderBar->value();
        if (hvalue > 0)
        {
            m_ui->hSliderBar->setValue(hvalue-1);
            m_ui->hSliderEcho->setText(QString::number(hvalue-1));
        }

        int vvalue = m_ui->vSliderBar->value();
        if (vvalue > 0)
        {
            m_ui->vSliderBar->setValue(vvalue-1);
            m_ui->vSliderEcho->setText(QString::number(vvalue-1));
        }

        if ( (hvalue==0) && (vvalue==0) )
            m_ui->timerControlSliders->setChecked(false);
    }

    m_ui->timerecho->setText(tr("%1: Seconds have passed").arg(m_Seconds));

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
        m_ui->mouseecho->setText(tr("%1Left mouse down at %2,%3")
                .arg(prefix).arg(event->pos().x()).arg(event->pos().y()));
    }
        break;
    case Qt::RightButton:
    {
        QString prefix;
        if(event->modifiers() & Qt::ControlModifier)
            prefix = "[CTRL]";
        if(event->modifiers() & Qt::ShiftModifier)
            prefix+= "[SHIFT]";
        m_ui->mouseecho->setText(tr("%1Right mouse down at %2,%3")
                .arg(prefix).arg(event->pos().x()).arg(event->pos().y()));
    }
        break;
    }

    QDialog::mousePressEvent(event);
}


void CTutorialDlg::showEvent(QShowEvent *event)
{
    static bool initialized = false;
    if (!initialized) {
        startTimer(1000);
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
    m_ui->mouseecho->setText(tr("%1Mouse move at %2,%3").arg(prefix)
            .arg(event->pos().x()).arg(event->pos().y()));

    QDialog::mouseMoveEvent(event);
}
