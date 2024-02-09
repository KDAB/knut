#include "DataExchange.h"

#include <QRegularExpressionValidator>
#include <QDoubleValidator>
#include <QIntValidator>

void TestClass::DoDataExchange()
{
    m_EchoText.setWidget(IDC_ECHO_AREA);
    m_MyControl.setWidget(IDC_MY_CONTROL);
    m_Check.setWidget(IDC_CHECK);
    m_timeValue.setWidget(IDC_CALENDAR);

    // Validators
    IDC_ECHO_AREA->setValidator(new QRegularExpressionValidator(QRegularExpression("^.{0,3}$"), IDC_ECHO_AREA));
    IDC_MY_CONTROL->setValidator(new QDoubleValidator(0.0, 100, IDC_MY_CONTROL));
    IDC_MY_CONTROL->setValidator(new QDoubleValidator(1.0, 100.1, IDC_MY_CONTROL));
    m_Unknown.widget()->setValidator(new QIntValidator(1, 11, m_Unknown.widget()));
    // KDAB_TODO: Set up Qt validator for "DDV_MinMaxLong(m_EchoText, 2, 22)"
    // KDAB_TODO: Set up Qt validator for "DDV_MinMaxLongLong(m_EchoText, 3, 33)"
    IDC_ECHO_AREA->setValidator(new QIntValidator(4, 44, IDC_ECHO_AREA));
    // KDAB_TODO: Set up Qt validator for "DDV_MinMaxUInt(m_EchoText, 4, 44)"
    // KDAB_TODO: Set up Qt validator for "DDV_MinMaxUnsigned(m_EchoText, 5, 55)"
    // KDAB_TODO: Set up Qt validator for "DDV_MinMaxULongLong(m_EchoText, 6, 66)"
    IDC_ECHO_AREA->setValidator(new QIntValidator(7, 77, IDC_ECHO_AREA));
    // KDAB_TODO: Set up Qt validator for "DDV_MinMaxDWord(m_EchoText, 8, 88)"
    // KDAB_TODO: Set up Qt validator for "DDV_MinMaxDateTime(m_timeValue, &minTime, &maxTime)"
    // KDAB_TODO: Set up Qt validator for "DDV_MinMaxMonth(m_timeValue, &minTime, &maxTime)"
    // KDAB_TODO: Set up Qt validator for "DDV_UNKNOWN(IDC_UNKNOWN, m_Unknown)"
}
