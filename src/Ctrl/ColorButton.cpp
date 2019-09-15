// ColorButton.cpp
//
//
//

#include <windows.h>
#include <windowsx.h>
#include "ColorButton.h"

CGrColorButton::CGrColorButton()
{
	m_Color        = 0xFF000000L;
	m_DefaultColor = RGB(0x55, 0x77, 0xFF);
}

// リストボックスを作成します。
//   hWnd    [in]: 親ウインドウのハンドル
//   戻り値 [out]: 作成したステータスウインドウのハンドル
HWND CGrColorButton::Create(HINSTANCE hInst, HWND hWnd)
{
	m_hWnd = CreateWindow(_T("BUTTON"),
						  NULL,
						  WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER | WS_VSCROLL |
						  LBS_DISABLENOSCROLL | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT,
						  0, 0, 0, 0,
						  hWnd,
						  NULL,
						  hInst,
						  0);
	SetFocus(m_hWnd);

	return m_hWnd;
}

// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
LRESULT CGrColorButton::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_DRAWITEM   , DrawItem     );
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, OnLButtonDown);
	}
	return CGrComonCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrColorButton::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	return (message == WM_DRAWITEM);
}

BOOL CGrColorButton::Attach(HWND hWnd)
{
	auto bret = CGrComonCtrl::Attach(hWnd);
	ModifyStyle(0, BS_OWNERDRAW);
	return bret;
}

// 色設定
//   color [in]: COLORREF
void CGrColorButton::SetColor(COLORREF color)
{
	m_Color = color;
	InvalidateRect(m_hWnd, NULL, FALSE);
}

//***********************************************************************
//**                             Constants                             **
//***********************************************************************
static const int g_ciArrowSizeX = 4 ;
static const int g_ciArrowSizeY = 2 ;
void CGrColorButton::DrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItemStruct)
{
	auto hdc      = lpDrawItemStruct->hDC;
	auto state    = lpDrawItemStruct->itemState;
	auto rDraw    = lpDrawItemStruct->rcItem;
	RECT rArrow;

	//******************************************************
	//**                  Draw Outer Edge
	//******************************************************
	UINT uFrameState = DFCS_BUTTONPUSH|DFCS_ADJUSTRECT;

	if(state & ODS_SELECTED){
		uFrameState |= DFCS_PUSHED;
	}

	if(state & ODS_DISABLED){
		uFrameState |= DFCS_INACTIVE;
	}

	if(GetWindowLong(m_hWnd, GWL_STYLE) & BS_FLAT){
		uFrameState |= DFCS_FLAT | DFCS_MONO;
	}
	DrawFrameControl(hdc, &rDraw, DFC_BUTTON, uFrameState);

	if(state & ODS_SELECTED){
		OffsetRect(&rDraw, 1,1);
	}

	//******************************************************
	//**                     Draw Focus
	//******************************************************
	if (state & ODS_FOCUS)
	{
		RECT rFocus = {rDraw.left,
			rDraw.top,
			rDraw.right - 1,
			rDraw.bottom};
		DrawFocusRect(hdc, &rFocus);
	}

	InflateRect(&rDraw, -::GetSystemMetrics(SM_CXEDGE),
				-::GetSystemMetrics(SM_CYEDGE));

	//******************************************************
	//**                     Draw Arrow
	//******************************************************
	rArrow.left		= rDraw.right - g_ciArrowSizeX - ::GetSystemMetrics(SM_CXEDGE) /2;
	rArrow.right	= rArrow.left + g_ciArrowSizeX;
	rArrow.top		= (rDraw.bottom + rDraw.top)/2 - g_ciArrowSizeY / 2;
	rArrow.bottom	= (rDraw.bottom + rDraw.top)/2 + g_ciArrowSizeY / 2;

	DrawArrow(hdc,
			  &rArrow,
			  0,
			  (state & ODS_DISABLED)
			  ? ::GetSysColor(COLOR_GRAYTEXT)
			  : RGB(0,0,0));


	rDraw.right = rArrow.left - ::GetSystemMetrics(SM_CXEDGE)/2;

	//******************************************************
	//**                   Draw Separator
	//******************************************************
	DrawEdge(hdc, &rDraw,
			 EDGE_ETCHED,
			 BF_RIGHT);

	rDraw.right -= (::GetSystemMetrics(SM_CXEDGE) * 2) + 1 ;

	//******************************************************
	//**                     Draw Color
	//******************************************************
	if ((state & ODS_DISABLED) == 0){
		auto oldColor = SetBkColor(hdc, (m_Color == 0xFF000000L) ? m_DefaultColor : m_Color);
		ExtTextOut(hdc, rDraw.left, rDraw.top, ETO_OPAQUE|ETO_OPAQUE, &rDraw, NULL, 0, NULL);/* FillSolidRect */
		SetBkColor(hdc, oldColor);

		::FrameRect(hdc, &rDraw, static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)));
	}
}

//***********************************************************************
// Method:	CColorButton::DrawArrow()
// Notes:	None.
//***********************************************************************
void CGrColorButton::DrawArrow(HDC hdc,
							   RECT* pRect,
							   int iDirection,
							   COLORREF clrArrow /*= RGB(0,0,0)*/)
{
	POINT ptsArrow[3] = {};

	switch (iDirection)
	{
	case 0 : // Down
		{
			ptsArrow[0].x = pRect->left;
			ptsArrow[0].y = pRect->top;
			ptsArrow[1].x = pRect->right;
			ptsArrow[1].y = pRect->top;
			ptsArrow[2].x = (pRect->left + pRect->right)/2;
			ptsArrow[2].y = pRect->bottom;
			break;
		}

	case 1 : // Up
		{
			ptsArrow[0].x = pRect->left;
			ptsArrow[0].y = pRect->bottom;
			ptsArrow[1].x = pRect->right;
			ptsArrow[1].y = pRect->bottom;
			ptsArrow[2].x = (pRect->left + pRect->right)/2;
			ptsArrow[2].y = pRect->top;
			break;
		}

	case 2 : // Left
		{
			ptsArrow[0].x = pRect->right;
			ptsArrow[0].y = pRect->top;
			ptsArrow[1].x = pRect->right;
			ptsArrow[1].y = pRect->bottom;
			ptsArrow[2].x = pRect->left;
			ptsArrow[2].y = (pRect->top + pRect->bottom)/2;
			break;
		}

	case 3 : // Right
		{
			ptsArrow[0].x = pRect->left;
			ptsArrow[0].y = pRect->top;
			ptsArrow[1].x = pRect->left;
			ptsArrow[1].y = pRect->bottom;
			ptsArrow[2].x = pRect->right;
			ptsArrow[2].y = (pRect->top + pRect->bottom)/2;
			break;
		}
	}

	HBRUSH brsArrow;
	HPEN   penArrow;

	brsArrow = CreateSolidBrush(clrArrow);
	penArrow = CreatePen(PS_SOLID, 1, clrArrow);

	auto pOldBrush = static_cast<HBRUSH>(SelectObject(hdc, brsArrow));
	auto pOldPen   = static_cast<HPEN>(SelectObject(hdc, penArrow));

	SetPolyFillMode(hdc, WINDING);
	Polygon(hdc, ptsArrow, 3);

	SelectObject(hdc, pOldBrush);
	SelectObject(hdc, pOldPen);
	DeleteObject(brsArrow);
	DeleteObject(penArrow);
}

void CGrColorButton::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	if(!fDoubleClick){
		if(chooseColor()){
			auto hpwnd = GetParent(hwnd);
			SendMessage(hpwnd, WM_COMMAND, GET_WM_COMMAND_MPS(GetDlgCtrlID(hwnd), hwnd, NULL));
		}
	}
}

// 色を選択します。
//   戻り値 : 選択した場合は TRUE
BOOL CGrColorButton::chooseColor()
{
	CHOOSECOLOR cc = {sizeof(CHOOSECOLOR)};
	static COLORREF acrCustClr[16] = {
		RGB(255, 255, 255), RGB(239, 239, 239),
		RGB(223, 223, 223), RGB(207, 207, 207),
		RGB(191, 191, 191), RGB(175, 175, 175),
		RGB(159, 159, 159), RGB(143, 143, 143),
		RGB(127, 127, 127), RGB(111, 111, 111),
		RGB(95, 95, 95),    RGB(79, 79, 79),
		RGB(63, 63, 63),    RGB(47, 47, 47),
		RGB(31, 31, 31),    RGB(15, 15, 15)
		};

	cc.rgbResult    = m_Color;
	cc.hwndOwner    = m_hWnd;
	cc.lpCustColors = acrCustClr;
	cc.Flags        = CC_FULLOPEN | CC_RGBINIT;
	// ダイアログの表示
	if(ChooseColor(&cc)){
		SetColor(cc.rgbResult);
		return TRUE;
	}
	return FALSE;
}
