#pragma once

#include <memory>

#include "Container.h"
#include "FontFactory.h"

using std::auto_ptr;

class Button : public Container
{
	CString m_text;
	CBrush m_brush;
	CPen m_pen;
	COLORREF m_textColor;
	auto_ptr<LOGFONT> m_font;

public:
	Button(int textId, COLORREF backgrounColor, COLORREF textColor, CPoint position, CSize size);
	~Button();

	void OnDraw(CDC* pDC);

	void SetPenPropertiest(int penStyle, int width, COLORREF color);
	CString* GetTextPtr();
};

