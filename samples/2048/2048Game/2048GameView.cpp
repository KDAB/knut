
// 2048GameView.cpp : implementation of the CMy2048GameView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "2048Game.h"
#endif

#include "2048GameDoc.h"
#include "2048GameView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMy2048GameView

IMPLEMENT_DYNCREATE(CMy2048GameView, CView)

BEGIN_MESSAGE_MAP(CMy2048GameView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMy2048GameView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// CMy2048GameView construction/destruction

CMy2048GameView::CMy2048GameView() :
	m_pointsContainer(0),
	m_fontText(FontFactory::CreateFont(40)),
	m_menuBtn(new Button(IDS_MENU, RGB(255, 255, 255), (COLORREF) 0, CPoint(), CSize())),
	m_undoBtn(new Button(IDS_UNDO, RGB(255, 255, 255), (COLORREF) 0, CPoint(), CSize()))
{
	m_menuBtn->SetPenPropertiest(PS_SOLID, 1, (COLORREF)0);
	m_undoBtn->SetPenPropertiest(PS_SOLID, 1, (COLORREF) 0);
}

CMy2048GameView::~CMy2048GameView()
{
}


void CMy2048GameView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	m_pointsContainer = auto_ptr<StringContainer>(new StringContainer(
		IDS_POINTS, 
		&GetDocument()->points, 
		CPoint(), 
		CSize()));

	SetFocus();
}


// CMy2048GameView drawing
void CMy2048GameView::OnDraw(CDC* pDC)
{
	CMy2048GameDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc || !pDoc->cells.size() || !m_pointsContainer.get())
		return;

	CRect rect;
	GetClientRect(&rect);

	float size = (min(rect.Width(), rect.Height() - m_pointsContainer->GetHeight() - 10)) / pDoc->size;
	float x = 0, y = 0, offset = .05f * size;

	m_menuBtn->size.SetSize(rect.Width() * .3, rect.Height() * .06);
	CreateUndoString(m_undoBtn->GetTextPtr());
	m_undoBtn->size.SetSize(rect.Width() * .3, rect.Height() * .06);

	m_pointsContainer->position.SetPoint(offset, offset);

	CFont font;
	CFont* font_st;

	CRect valRect;
	CBrush brush;
	brush.CreateSolidBrush(pDoc->bkColor);
	CString val;

	if (pDoc->start && pDoc->cells.size()) {
		if (!pDoc->GetFreeCellsCount()) {
			pDoc->start = FALSE;
			AfxMessageBox(IDS_LOSE);
			return;
		}

		font_st = pDC->SelectObject(&font);

		font.CreateFontIndirect(m_fontText.get());
		pDC->SetBkColor(pDoc->bkColor);
		pDC->SetTextColor(RGB(255, 255, 255));

		pDC->SelectObject(&font);

		float startY = m_pointsContainer->GetHeight() + 2 * offset;

		// draw background rect		
		CBrush b(RGB(188, 176, 162));
		CPen p;
		p.CreatePen(PS_SOLID, 0, RGB(188, 176, 162));
		CBrush* oldB = pDC->SelectObject(&b);
		CPen* oldP = pDC->SelectObject(&p);

		CRect bkRect(0, startY, size * pDoc->size, startY + size * pDoc->size);
		pDC->RoundRect(bkRect, CPoint(10, 10));

		pDC->SelectObject(oldB);
		pDC->SelectObject(oldP);
		//

		for (int i = 0; i < pDoc->size; i++) {
			y = startY;
			for (int j = 0; j < pDoc->size; j++) {
				valRect.left = x + offset;
				valRect.top = y + offset;
				valRect.right = x + size - offset;
				valRect.bottom = y + size - offset;

				int color = GetColorByNumber(pDoc->cells.top()[i][j]);

				CPen pen;
				pen.CreatePen(PS_SOLID, 0, color);
				CBrush brush(color);
				CBrush* oldBrush = pDC->SelectObject(&brush);
				CPen* oldPen = pDC->SelectObject(&pen);
				int oldBkMode = pDC->SetBkMode(TRANSPARENT);
				
				pDC->RoundRect(valRect, CPoint(10, 10));

				if (pDoc->cells.top()[i][j]) {
					int oldTextColor = 0;
					if (pDoc->cells.top()[i][j] < 8)
						oldTextColor = pDC->SetTextColor(RGB(126, 117, 100));

					val.Format(L"%d", pDoc->cells.top()[i][j]);
					pDC->DrawTextW(val, valRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

					if (oldTextColor)
						pDC->SetTextColor(oldTextColor);
				}

				y += size;

				pDC->SetBkMode(oldBkMode);
				pDC->SelectObject(oldPen);
				pDC->SelectObject(oldBrush);
			}
			x += size;
		}
		
		m_pointsContainer->OnDraw(pDC);

		pDC->SelectObject(font_st);

		m_menuBtn->position.SetPoint(size * pDoc->size - m_menuBtn->size.cx, offset);
		m_undoBtn->position.SetPoint(size * pDoc->size - m_undoBtn->size.cx, offset + rect.Height() * .08);

		m_menuBtn->OnDraw(pDC);
		m_undoBtn->OnDraw(pDC);
	}
}


void CMy2048GameView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMy2048GameView diagnostics

#ifdef _DEBUG
void CMy2048GameView::AssertValid() const
{
	CView::AssertValid();
}

void CMy2048GameView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMy2048GameDoc* CMy2048GameView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMy2048GameDoc)));
	return (CMy2048GameDoc*)m_pDocument;
}
#endif //_DEBUG

void CMy2048GameView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CMy2048GameDoc* pDoc = GetDocument();
	
	if (!pDoc->start) return;

	int** cell = new int*[pDoc->size];
	int** oldCell = pDoc->cells.top();

	for (int i = 0; i < pDoc->size; i++) {
		cell[i] = new int[pDoc->size];
		for (int j = 0; j < pDoc->size; j++) {
			cell[i][j] = oldCell[i][j];
		}
	}
	
	pDoc->cells.push(cell);

	int numMoves = 0;
	if (nChar == 37) {
		for (int j = 0; j < pDoc->size; j++) {
			for (int i = 0; i < pDoc->size; i++) {
				for (int k = i + 1; k < pDoc->size; k++) {
					if (pDoc->cells.top()[k][j] != 0) {						
						if (pDoc->cells.top()[i][j] == 0) {
							numMoves++;

							pDoc->cells.top()[i][j] = pDoc->cells.top()[k][j];
							pDoc->cells.top()[k][j] = 0;
						} else {
							if (pDoc->cells.top()[i][j] == pDoc->cells.top()[k][j]) {
								numMoves++;

								pDoc->cells.top()[i][j] += pDoc->cells.top()[k][j];
								pDoc->points += pDoc->cells.top()[i][j];
								pDoc->cells.top()[k][j] = 0;
							}
							break;
						}
					}
				}
			}
		}
	}
	else if (nChar == 38) {
		for (int j = 0; j < pDoc->size; j++) {
			for (int i = 0; i < pDoc->size; i++) {
				for (int k = i + 1; k < pDoc->size; k++) {
					if (pDoc->cells.top()[j][k] != 0) {
						if (pDoc->cells.top()[j][i] == 0) {
							numMoves++;

							pDoc->cells.top()[j][i] = pDoc->cells.top()[j][k];
							pDoc->cells.top()[j][k] = 0;
						} else { 
							if (pDoc->cells.top()[j][i] == pDoc->cells.top()[j][k]) {
								numMoves++;

								pDoc->cells.top()[j][i] += pDoc->cells.top()[j][k];
								pDoc->points += pDoc->cells.top()[j][i];
								pDoc->cells.top()[j][k] = 0;
							}
							break;
						}
					}
				}
			}
		}
	} else if (nChar == 39) {
		for (int j = pDoc->size - 1; j >= 0; j--) {
			for (int i = pDoc->size - 1; i >= 0; i--) {
				for (int k = i - 1; k >= 0; k--) {
					if (pDoc->cells.top()[k][j] != 0) {
						if (pDoc->cells.top()[i][j] == 0) {
							numMoves++;

							pDoc->cells.top()[i][j] = pDoc->cells.top()[k][j];
							pDoc->cells.top()[k][j] = 0;
						} else {
							if (pDoc->cells.top()[i][j] == pDoc->cells.top()[k][j]) {
								numMoves++;

								pDoc->cells.top()[i][j] += pDoc->cells.top()[k][j];
								pDoc->points += pDoc->cells.top()[i][j];
								pDoc->cells.top()[k][j] = 0;
							}
							break;
						}
					}
				}
			}
		}
	}
	else if (nChar == 40) {
		for (int j = pDoc->size - 1; j >= 0; j--) {
			for (int i = pDoc->size - 1; i >= 0; i--) {
				for (int k = i - 1; k >= 0; k--) {
					if (pDoc->cells.top()[j][k] != 0) {
						if (pDoc->cells.top()[j][i] == 0) {
							numMoves++;

							pDoc->cells.top()[j][i] = pDoc->cells.top()[j][k];
							pDoc->cells.top()[j][k] = 0;
						} else {
							if (pDoc->cells.top()[j][i] == pDoc->cells.top()[j][k]) {
								numMoves++;

								pDoc->cells.top()[j][i] += pDoc->cells.top()[j][k];
								pDoc->points += pDoc->cells.top()[j][i];
								pDoc->cells.top()[j][k] = 0;
							}
							break;
						}
					}
				}
			}
		}
	}

	if (numMoves) pDoc->GenerateNewRandomCell();
	else pDoc->cells.pop();

	Invalidate();

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CMy2048GameView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_menuBtn.get() || !m_undoBtn.get()) return;

	if (m_menuBtn->PtInRect(point))
		((CMainFrame*)AfxGetMainWnd())->ShowPreview();
	else if (m_undoBtn->PtInRect(point)) {
		CMy2048GameDoc* pDoc = GetDocument();

		pDoc->cells.pop();
		Invalidate();
	}
}


void CMy2048GameView::CreateUndoString(CString* input)
{
	CMy2048GameDoc* pDoc = GetDocument();

	CString undoBtnText, 
			undoCount;

	undoBtnText.LoadStringW(IDS_UNDO);
	undoCount.Format(L"(%d)", pDoc->cells.size() - 1);
	undoBtnText += undoCount;

	input->SetString(undoBtnText);
}
