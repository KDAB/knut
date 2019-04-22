
// 2048GameDoc.cpp : implementation of the CMy2048GameDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "2048Game.h"
#endif

#include "2048GameDoc.h"
#include "SettingsDialog.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMy2048GameDoc

IMPLEMENT_DYNCREATE(CMy2048GameDoc, CDocument)

BEGIN_MESSAGE_MAP(CMy2048GameDoc, CDocument)
	ON_COMMAND(ID_SETTINGS_FIELDSETTINGS, &CMy2048GameDoc::OnSettingsFieldsettings)
	ON_COMMAND(ID_SETTINGS_BACKGROUNDCOLOR, &CMy2048GameDoc::OnSettingsBackgroundcolor)
END_MESSAGE_MAP()


// CMy2048GameDoc construction/destruction

CMy2048GameDoc::CMy2048GameDoc()
{
	points = 0;
	size = 4;
	bkColor = RGB(214, 205, 196);
}

CMy2048GameDoc::~CMy2048GameDoc()
{
	if (cells.size()) {
		while (cells.size()) {
			int** cell = cells.top();
			cells.pop();

			for (int i = 0; i < size; i++)
				delete cell[i];

			delete[] cell;
		}
	}
}

BOOL CMy2048GameDoc::OnNewDocument()
{
	srand(time(NULL));

	if (!CDocument::OnNewDocument())
		return FALSE;

	int** cell = new int*[size];
	for (int i = 0; i < size; i++) {
		cell[i] = new int[size];
		for (int j = 0; j < size; j++)
			cell[i][j] = 0;
	}

	int filledCount = rand() % (size * 2 - 1) + 2;
	std::set<std::pair<int, int>> uniqElem;
	std::set<std::pair<int, int>>::iterator it;

	while (filledCount >= 0) {
		int value = (rand() % 2 + 1) * 2;
		std::pair<int, int> coord;

		do {
			coord.first = rand() % size;
			coord.second = rand() % size;

			it = uniqElem.find(coord);
		} while (it != uniqElem.end());

		cell[coord.first][coord.second] = value;

		uniqElem.insert(coord);

		filledCount--;
	}

	start = true;
	cells.push(cell);

	return TRUE;
}


#ifdef SHARED_HANDLERS

// Support for thumbnails
void CMy2048GameDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CMy2048GameDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CMy2048GameDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CMy2048GameDoc diagnostics

#ifdef _DEBUG
void CMy2048GameDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMy2048GameDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMy2048GameDoc commands


void CMy2048GameDoc::OnSettingsFieldsettings()
{
	SettingsDialog settings(&size, &name);
	if (settings.DoModal() != IDOK) return;

	SetModifiedFlag();
	OnNewDocument();
	UpdateAllViews(0);
}


void CMy2048GameDoc::OnSettingsBackgroundcolor()
{
	CColorDialog color(bkColor);
	if (color.DoModal() == IDOK)
	{
		bkColor = color.GetColor();
		//SetModifiedFlag();
		UpdateAllViews(0);
	}
}


BOOL CMy2048GameDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	CStdioFile writer;

	if (writer.Open(lpszPathName, CFile::modeCreate | CFile::modeWrite)) {
		CString str = L"";
		str.Format(L"%s;%d\n", name, points);
		writer.WriteString(str);
	}
	else
	{
		CString strErrorMsg;
		strErrorMsg.Format(L"Ошибка записи в файл %s", lpszPathName);
		AfxMessageBox(strErrorMsg);
		return FALSE;
	}

	writer.Close();

	return CDocument::OnSaveDocument(lpszPathName);
}


void CMy2048GameDoc::GenerateNewRandomCell()
{
	if (cells.top() == nullptr) return;

	std::vector<std::pair<int, int>> freeCells;

	// get free cells
	GetFreeCells(freeCells);
	
	// fill random cell
	pair<int, int> randCoord = freeCells[rand() % freeCells.size()];
	cells.top()[randCoord.first][randCoord.second] = (rand() % 2 + 1) * 2;
}


// Function that fill std::vector elements with std::pair that contain coord of free cells
void CMy2048GameDoc::GetFreeCells(vector<pair<int, int>>& in)
{
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			if (!cells.top()[i][j]) in.push_back(pair<int, int>(i, j));
}


int CMy2048GameDoc::GetFreeCellsCount()
{
	vector<pair<int, int>> freeCells;
	GetFreeCells(freeCells);

	return freeCells.size();
}
