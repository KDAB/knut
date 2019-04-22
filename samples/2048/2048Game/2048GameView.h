
// 2048GameView.h : interface of the CMy2048GameView class
//

#pragma once

#include <memory>

#include "StringContainer.h"
#include "2048GameDoc.h"
#include "Button.h"
#include "MainFrm.h"

using std::auto_ptr;

class CMy2048GameView : public CView
{
private:
	auto_ptr<StringContainer> m_pointsContainer;
	auto_ptr<LOGFONT> m_fontText;
	auto_ptr<Button> m_undoBtn;
	auto_ptr<Button> m_menuBtn;

protected:
	DECLARE_DYNCREATE(CMy2048GameView)

// Attributes
public:
	CMy2048GameView();

	CMy2048GameDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
protected:

// Implementation
public:
	virtual ~CMy2048GameView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void OnInitialUpdate();

	inline int GetColorByNumber(int number)
	{
		switch (number) {
		case 0:
			return RGB(214, 205, 196);
		case 2: 
			return RGB(238, 228, 218);
		case 4:
			return RGB(236, 224, 200);
		case 8:
			return RGB(242, 177, 121);
		case 16:
			return RGB(245, 149, 99);
		case 32:
			return RGB(243, 125, 95);
		case 64:
			return RGB(246, 93, 59);
		case 128:
		case 256:
		case 512:
		case 1024:
		case 2048:
			return RGB(237, 204, 99);
		default:
			return RGB(255, 30, 31);
		}
	};
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	void CreateUndoString(CString* input);
};

#ifndef _DEBUG  // debug version in 2048GameView.cpp
inline CMy2048GameDoc* CMy2048GameView::GetDocument() const
   { return reinterpret_cast<CMy2048GameDoc*>(m_pDocument); }
#endif

