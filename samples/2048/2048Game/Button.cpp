#include "stdafx.h"
#include "Button.h"


Button::Button(int textId, COLORREF backgrounColor, COLORREF textColor, CPoint position, CSize size)
	: Container(position, size),
	m_textColor(textColor), 
	m_font(FontFactory::CreateFont(18))
{
	this->m_text.LoadStringW(textId);
	this->m_brush.CreateSolidBrush(backgrounColor);
	this->m_pen.CreatePen(PS_SOLID, 0, backgrounColor);
}

Button::~Button()
{
}

void Button::OnDraw(CDC* pDC)
{
	// set params
	int oldBkMode = pDC->SetBkMode(TRANSPARENT);
	int oldFrColor = pDC->SetTextColor(m_textColor);
	CBrush* oldBrush = pDC->SelectObject(&m_brush);
	CPen* oldPen = pDC->SelectObject(&m_pen);
	CRect rect(position.x, position.y, position.x + size.cx, position.y + size.cy);

	// draw
	CFont font;
	CFont* oldFont;
	
	font.CreateFontIndirect(m_font.get());

	oldFont = pDC->SelectObject(&font);

	pDC->RoundRect(rect, CPoint(5, 5));
	pDC->DrawTextW(m_text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// restore
	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldFrColor);
	pDC->SetBkMode(oldBkMode);
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
}

void Button::SetPenPropertiest(int penStyle, int width, COLORREF color)
{
	this->m_pen.Detach();

	this->m_pen.CreatePen(penStyle, width, color);
}

CString* Button::GetTextPtr()
{
	return &m_text;
}
