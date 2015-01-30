#include "stdafx.h"
#include "SliderCtrlWithEcho.h"

// DEBUG_NEW macro allows MFC applications to determine memory leak locations in debug builds
#ifdef _DEBUG
  #define new DEBUG_NEW
#endif

// This macro adds IsKindOf function to this class to provide for
// runtime type information
IMPLEMENT_DYNAMIC(CSliderCtrlWithEcho, CSliderCtrl)


CSliderCtrlWithEcho::CSliderCtrlWithEcho()
: m_minimum_value(0.0f)
, m_value_range(1.0f)
{
}

CSliderCtrlWithEcho::~CSliderCtrlWithEcho(){}

void CSliderCtrlWithEcho::Initialize(float min, float max, float init)
{
  // check for valid input and that this control exists as a window
  if( !IsWindow(m_hWnd) || max <= min || init<min || init>max)
    return;

  // Create the buddy text control if it has not already been created
  if( !IsWindow(m_MessageWnd.m_hWnd) )
  {
    // Get the parent window for this slider control
    CWnd* pParent = GetParent();
    if( pParent )
    {
      CRect controlRect;
      GetWindowRect(&controlRect);
      CRect msgRect(controlRect);

      if( !IsOrientedVertical() )
      {
        msgRect.left = msgRect.right-50;
        controlRect.right = controlRect.right-50;
      }
      else
      {
        msgRect.left = controlRect.left+controlRect.Width()/2-50/2;
        msgRect.right = msgRect.left+50;
        msgRect.top = controlRect.top;
        msgRect.bottom = msgRect.top+20;
        controlRect.top = controlRect.top+20;
      }

      pParent->ScreenToClient(&msgRect);
      pParent->ScreenToClient(&controlRect);

      m_MessageWnd.Create( L"XXXX",SS_CENTERIMAGE|WS_CHILD|WS_VISIBLE|SS_SUNKEN,msgRect,pParent);
      MoveWindow(&controlRect);
    
      // Make the control's font match the font used on the parent window
      m_MessageWnd.SetFont( pParent->GetFont() );
    }
  }

  m_minimum_value = min;
  m_value_range = max - min;
  SetRange(0, SLIDER_RESOLUTION, TRUE);
  SetSliderValue(init);
  UpdateSliderEcho();
}


BEGIN_MESSAGE_MAP(CSliderCtrlWithEcho, CSliderCtrl)
  ON_WM_HSCROLL_REFLECT()
  ON_WM_VSCROLL_REFLECT()
  ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

void CSliderCtrlWithEcho::HScroll(UINT nSBCode, UINT nPos)
{
  int slider_position = GetPos();
  float userValue = ConvertSliderPositionToUserValue( slider_position );
  // update the text in our static control
  UpdateSliderEcho();
  // allow subclasses to respond to this event
  OnScroll(userValue);
}
void CSliderCtrlWithEcho::VScroll(UINT nSBCode, UINT nPos)
{
  int slider_position = GetPos();
  float userValue = ConvertSliderPositionToUserValue( slider_position );
  // update the text in our static control
  UpdateSliderEcho();
  // allow subclasses to respond to this event
  OnScroll(userValue);
}

void CSliderCtrlWithEcho::OnShowWindow(BOOL bShow, UINT nStatus)
{
  CSliderCtrl::OnShowWindow(bShow, nStatus);
  // update the static buddy text control visibility state to
  // match this slider control
  if( IsWindow(m_MessageWnd.m_hWnd) )
  {
    if(bShow)
      m_MessageWnd.ShowWindow(SW_SHOW);
    else
      m_MessageWnd.ShowWindow(SW_HIDE);
  }
}


bool CSliderCtrlWithEcho::SetSliderValue(float userValue)
{
  if(userValue < m_minimum_value || userValue > (m_minimum_value+m_value_range) )
    return false;
  int controlPosition = ConvertUserValueToSliderPosition(userValue);
  SetPos(controlPosition);
  UpdateSliderEcho();
  return true;
}

float CSliderCtrlWithEcho::GetSliderValue()
{
  int slider_position = GetPos();
  if( IsOrientedVertical())
    slider_position = SLIDER_RESOLUTION - slider_position;

  return ConvertSliderPositionToUserValue( slider_position );
}

int CSliderCtrlWithEcho::ConvertUserValueToSliderPosition(float userValue) const
{
  int slider_position = int(((userValue - m_minimum_value) * SLIDER_RESOLUTION / m_value_range)+0.5f);
  if(IsOrientedVertical())
    slider_position = SLIDER_RESOLUTION - slider_position;
  return slider_position;
}

float CSliderCtrlWithEcho::ConvertSliderPositionToUserValue( int position ) const
{
  return m_minimum_value + (float(position) / float(SLIDER_RESOLUTION) * m_value_range );
}

void CSliderCtrlWithEcho::UpdateSliderEcho()
{
  if(IsWindow(m_MessageWnd.m_hWnd))
  {
    CString msg;
    msg.Format(L" %.2f",GetSliderValue());
    m_MessageWnd.SetWindowText(msg);
  }
}

void CSliderCtrlWithEcho::OnScroll(float new_value)
{
}

bool CSliderCtrlWithEcho::IsOrientedVertical() const
{
  bool bVert = false;
  if( IsWindow(m_hWnd ) )
  {
    DWORD style = GetStyle();
    bVert = ( TBS_VERT == (style & TBS_VERT) );
  }
  return bVert;
}