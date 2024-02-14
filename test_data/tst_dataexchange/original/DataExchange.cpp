#include "DataExchange.h"

void TestClass::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_Text(pDX, IDC_ECHO_AREA, /*some comment*/ m_EchoText);
  DDV_MaxChars(pDX, m_EchoText, 3);
  DDX_Control(pDX, /* more comments*/ IDC_MY_CONTROL, m_MyControl);
  // Added a comment here for testing
  DDX_Check(pDX, IDC_CHECK, m_Check);
  DDV_MinMaxDouble(pDX, m_MyControl, 0.0, 100);
  DDV_MinMaxFloat(pDX, m_MyControl, 1.0, 100.1);

  DDV_MinMaxInt(pDX, m_Unknown, 1 /*comment here, because why not?*/, 11);
  DDV_MinMaxLong(pDX, m_EchoText, 2, 22);
  DDV_MinMaxLongLong(pDX, m_EchoText, 3, 33);
  DDV_MinMaxShort(pDX, /*another comment*/ m_EchoText, 4, 44);
  DDV_MinMaxUInt(pDX, m_EchoText, 4, 44);
  DDV_MinMaxUnsigned(pDX, m_EchoText, 5, 55);
  DDV_MinMaxULongLong(pDX, m_EchoText, 6, 66);
  DDV_MinMaxByte(pDX, m_EchoText, 7, 77);
  DDV_MinMaxDWord(pDX, m_EchoText, 8, 88);

  DDX_DateTimeCtrl(pDX, IDC_CALENDAR, m_timeValue);
  CTime minTime(2019, 12, 1);
  CTime maxTime(2024, 2, 9);
  DDV_MinMaxDateTime(pDX, m_timeValue, &minTime, &maxTime);
  DDV_MinMaxMonth(pDX, m_timeValue, &minTime, &maxTime);



  DDV_UNKNOWN(pDX, IDC_UNKNOWN, m_Unknown);
}
