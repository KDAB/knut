#include "stdafx.h"
#include "StringContainer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

StringContainer::StringContainer(int msgId, int* value, CPoint position, CSize size) :
	Container(position, size)
{
	this->descriptionText = new CString;
	this->descriptionText->LoadString(msgId);
	this->value = value;

	descriptionTextFont.CreatePointFont(200, L"Times New Roman");
	valueTextFont.CreatePointFont(300, L"Times New Roman");

	margin = 10;
}


StringContainer::~StringContainer()
{
	delete descriptionText;
	// delete position;
}

int StringContainer::GetWidth()
{
	return 0;
}

int StringContainer::GetHeight()
{
	LOGFONT lf1, lf2;
	valueTextFont.GetLogFont(&lf1);
	descriptionTextFont.GetLogFont(&lf2);

	return margin * 2 + abs(lf1.lfHeight) + abs(lf2.lfHeight);
}

void StringContainer::OnDraw(CDC* pDC)
{
	CBrush brushBlue(RGB(188, 172, 157));
	LOGFONT descriptionFont;
	descriptionTextFont.GetLogFont(&descriptionFont);
	CPen grayPen(PS_SOLID, 0, RGB(188, 172, 157));

	int oldBkMode = pDC->SetBkMode(TRANSPARENT);
	CFont* oldFont = pDC->SelectObject(&descriptionTextFont);
	CBrush* oldBrush = pDC->SelectObject(&brushBlue);
	CPen* oldPen = pDC->SelectObject(&grayPen);

	//value
	CString valueText;
	valueText.Format(L"%d", *value);
	pDC->SelectObject(&valueTextFont);

	CSize valueSize = pDC->GetTextExtent((valueText.GetLength() < 3) ? CString(L"123") : valueText);

	// rect
	CRect rect(
		position.x, position.y, 
		position.x + valueSize.cx + 2 * margin, GetHeight());
	pDC->RoundRect(rect, CPoint(17, 17));

	pDC->DrawTextW(valueText, rect, DT_CENTER | DT_BOTTOM | DT_SINGLELINE);

	//description
	pDC->SelectObject(&descriptionTextFont);
	pDC->DrawTextW(*descriptionText, rect, DT_CENTER | DT_TOP | DT_SINGLELINE);

	pDC->SetBkMode(oldBkMode);
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
	pDC->SelectObject(oldFont);
}
