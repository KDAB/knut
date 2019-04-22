#pragma once
class Container
{
public:
	CPoint position;
	CSize size;
	int margin;

public:
	explicit Container(CPoint position, CSize size);
	Container(Container& container);
	~Container();

	virtual void OnDraw(CDC* pDC) = 0;
	bool PtInRect(CPoint point);
};

