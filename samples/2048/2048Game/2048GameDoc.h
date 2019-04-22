
// 2048GameDoc.h : interface of the CMy2048GameDoc class
//

#include <stdlib.h>
#include <time.h>
#include <propkey.h>
#include <iostream>
#include <set>
#include <stack>
#include <vector>

#include "FontFactory.h"

#pragma once

using std::stack;
using std::vector;
using std::pair;

class CMy2048GameDoc : public CDocument
{
protected: // create from serialization only
	CMy2048GameDoc();
	DECLARE_DYNCREATE(CMy2048GameDoc)

// Attributes
public:
	CString msg;
	CString name;
	int points;
	int size;
	stack<int**> cells;

	COLORREF bkColor;
	bool start;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CMy2048GameDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	afx_msg void OnSettingsFieldsettings();
	afx_msg void OnSettingsBackgroundcolor();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	void GenerateNewRandomCell();
	void GetFreeCells(std::vector<std::pair<int, int>>& in);
	int GetFreeCellsCount();
};
