// University of Washington Bothell Graphics Library
// Authors: Kelvin Sung, Steve Baer
// The accompanying library supports CSS Graphics courses taught at UW-Bothell
// See: http://courses.washington.edu/css450/
//      http://courses.washington.edu/css451/
///////////////////////////////////////////////////////////////////////////////////
#pragma once

/// Specialized slider control that contains a text box.
///  - Echos slider value to a static text
///  - Support floating point slider range (with resolution of 100K)
/// Draw a slider on a dialog and create a member variable for the control
/// Replace the CSliderCtrl variable type with SliderCtrlWithEcho
/// \see MSDN documentation on the MFC CSliderCtrl class
class CSliderCtrlWithEcho : public CSliderCtrl
{
  /// macro for adding runtime type information to this class. The cpp
  /// file will also need an IMPLEMENT_DYNAMIC declaration
  DECLARE_DYNAMIC(CSliderCtrlWithEcho)

public:
  CSliderCtrlWithEcho();
  virtual ~CSliderCtrlWithEcho();

  /// Initialize the range and initial value of the control
  /// \param min The value that the slider represents when the tick is all the way to the left/bottom
  /// \param max The value that the slider represents when the tick is all the way to the right/top
  /// \param init initial value (position) that the tick should be located at
  void Initialize(float min, float max, float init);

  /// Set the controls tick pposition to represent a value between the min and max values of the slider
  /// See Initialize
  /// \param userValue The position to set the tick to that represents a value
  /// \return true on success. If userValue is outside the min/max range of the control, return false
  bool SetSliderValue(float userValue);

  ///
  /// Return the current value that the slider tick represents
  float GetSliderValue();

protected:
  ///
  /// Convert a floating point value to an integer tick position value
  int ConvertUserValueToSliderPosition(float userValue) const;

  ///
  /// Convert an integer tick position to a floating point value
  float ConvertSliderPositionToUserValue( int position ) const;

  /// Update the text in the static control that displays the current
  /// value the slider represents
  void UpdateSliderEcho();

  /// A message map is an MFC macro for mapping window's events (paint, size, mouse,...) that
  /// occur on a window to functions in this class.
  /// The functions in this class that begin with afx_msg are "handlers" for window's messages
  DECLARE_MESSAGE_MAP()

  /// Called when the parent dialog receives a WM_HSCROLL message and the dialog "reflects"
  /// the message back to this control
  /// See MSDN documentation for information on parameters
  afx_msg void HScroll(UINT nSBCode, UINT nPos);

  /// Called when the parent dialog receives a WM_VSCROLL message and the dialog "reflects"
  /// the message back to this control
  /// See MSDN documentation for information on parameters
  afx_msg void VScroll(UINT nSBCode, UINT nPos);

  /// Called when this control receives a WM_SHOWWINDOW message
  /// This control creates a static control to display the slider status. This message is
  /// caught to make sure the visibility state of the static text matches this control's state
  /// See MSDN documentation for information on parameters
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

protected:
  /// A slider control uses integer values for positions. The position value needs to be
  /// converted into a floating point value for the user. By using a high position resolution,
  /// we can get higher precision floating point values
  static const int SLIDER_RESOLUTION = 100000;

  /// floating point value representing the minimum value
  /// in the slider rance
  float m_minimum_value;

  /// Total range of values covered from minimum to maximum.
  /// The maximum value is m_minimum_value + m_value_range
  float m_value_range;

  /// A static "buddy" control for displaying the current slider
  /// value. The control is created and placed next to this control
  /// automatically
  CStatic m_MessageWnd;

  /// True if this control is oriented vertically
  /// False if this control is oriented horizontally
  bool IsOrientedVertical() const;

  // Override the OnScroll function if you want to perform an operation when this control
  // is scrolled
  virtual void OnScroll(float new_value);
};
