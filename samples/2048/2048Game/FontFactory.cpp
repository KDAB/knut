#include "stdafx.h"
#include "FontFactory.h"

LOGFONT* FontFactory::CreateFont(int size, CString fontName)
{
	LOGFONT* font = new LOGFONT;
	font->lfHeight = -size;
	font->lfWidth = 0;
	font->lfEscapement = 0;
	font->lfOrientation = 0;
	font->lfWeight = FW_NORMAL;
	font->lfItalic = 0;
	font->lfUnderline = 0;
	font->lfStrikeOut = 0;
	font->lfCharSet = ANSI_CHARSET;
	font->lfOutPrecision = OUT_DEFAULT_PRECIS;
	font->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	font->lfQuality = PROOF_QUALITY;
	font->lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
	wcscpy_s(font->lfFaceName, fontName);

	return font;
}
