#pragma once

#include <memory>

#include "Button.h"
#include "FontFactory.h"

// PreviewView view

using std::auto_ptr;

class PreviewView : public CView
{
	DECLARE_DYNCREATE(PreviewView)

	auto_ptr<Button> m_startButton;
	auto_ptr<Button> m_aboutButton;
	auto_ptr<LOGFONT> m_titleFont;

	PreviewView();

protected:	
	virtual ~PreviewView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


