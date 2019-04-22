#include "stdafx.h"
#include "Container.h"


Container::Container(CPoint position, CSize size)
{
	this->position = position;
	this->size = size;
}

Container::Container(Container& container) 
{
	this->position = container.position;
	this->size = container.size;
}

Container::~Container()
{
}

bool Container::PtInRect(CPoint point)
{
	CRect rect(position.x, position.y, position.x + size.cx, position.y + size.cy);
	return rect.PtInRect(point);
}
