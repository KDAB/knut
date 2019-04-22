#pragma once
class FontFactory
{
public:
	
	static LOGFONT* CreateFont(int size, CString fontName = L"Times New Roman");
	
};

