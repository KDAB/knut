#pragma once

#include "Container.h"

class StringContainer : public Container
{
public:
	CString* descriptionText;
	int* value;
	CFont descriptionTextFont;
	CFont valueTextFont;

	StringContainer(int msgId, int* value, CPoint position, CSize size);
	~StringContainer();

	int GetWidth();
	int GetHeight();

	void OnDraw(CDC* pDC);
};

