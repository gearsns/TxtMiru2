#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PropPageSize.h"

CGrPropPageSize::CGrPropPageSize(CGrTxtConfigDlg *pDlg) : m_pConfigDlg(pDlg)
{
}

LRESULT CGrPropPageSize::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_HSCROLL, OnHScroll);
		HANDLE_MSG(hWnd, WM_VSCROLL, OnVScroll);
		HANDLE_MSG(hWnd, WM_SIZE   , OnSize   );
	case WM_MOUSEWHEEL:
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wParam); // マルチモニタ環境では、負の値を持つことがあるので一旦 shortに変換する必要がある
			if((WHEEL_DELTA/6) <= delta){
				OnVScroll(hWnd, NULL, SB_LINEUP, 0);
			} else if(delta <= -(WHEEL_DELTA/6)){
				OnVScroll(hWnd, NULL, SB_LINEDOWN, 0);
			}
		}
		break;
	case WM_SETPAGEFOCUS:
		::SetFocus(reinterpret_cast<HWND>(wParam));
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrPropPageSize::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	m_point.y = 0;
	m_point.x = 0;
	RECT win_rect;
	::GetWindowRect(m_hWnd, &win_rect);
	m_minSize.cx = win_rect.right - win_rect.left;
	m_minSize.cy = win_rect.bottom - win_rect.top;
	GetClientRect(m_hWnd, &win_rect);
	m_minClientSize.cx = win_rect.right - win_rect.left;
	m_minClientSize.cy = win_rect.bottom - win_rect.top;
	setScrollRange(m_minSize.cx, m_minSize.cy);
	if(m_scroll_line < 0){
		RECT rect = {0,0,10,10};
		MapDialogRect(hwnd, &rect);
		m_scroll_line = rect.right;
	}

	return TRUE;
}

int CGrPropPageSize::getScrollPos(int fnBar)
{
	SCROLLINFO si = {sizeof(SCROLLINFO)};
	si.fMask = SIF_POS;
	::GetScrollInfo(m_hWnd, fnBar, &si);
	return si.nPos;
}

void CGrPropPageSize::setScrollPos(int fnBar, SCROLLINFO &si)
{
	::SetScrollInfo(m_hWnd, fnBar, &si, TRUE);
	if(fnBar == SB_HORZ){
		int nDelta = m_point.x - getScrollPos(fnBar);
		if(nDelta != 0){
			ScrollWindowEx(m_hWnd, nDelta, 0, nullptr, nullptr, NULL, nullptr, SW_INVALIDATE|SW_SCROLLCHILDREN|SW_ERASE);
			m_point.x -= nDelta;
		}
	} else {
		int nDelta = m_point.y - getScrollPos(fnBar);
		if(nDelta != 0){
			ScrollWindowEx(m_hWnd, 0, nDelta, nullptr, nullptr, NULL, nullptr, SW_INVALIDATE|SW_SCROLLCHILDREN|SW_ERASE);
			m_point.y -= nDelta;
		}
	}
}

void CGrPropPageSize::setScrollRange(int cx, int cy)
{
	if(m_bCalc){
		return;
	}
	m_bCalc = true;
	LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
	{
		BOOL bShowHorz = FALSE;
		BOOL bShowVert = FALSE;
		bool bReCalc = false;
		if(style & WS_HSCROLL){
			bShowHorz = TRUE;
		}
		if(style & WS_VSCROLL){
			bShowVert = TRUE;
		}
		int cnt = 0;
		do {
			++cnt;
			bReCalc = false;
			if(/**/bShowHorz && bShowVert
			   /**/&& cx + GetSystemMetrics(SM_CYVSCROLL) >= m_minClientSize.cx
			   /**/&& cy + GetSystemMetrics(SM_CXHSCROLL) >= m_minClientSize.cy){
				cy += GetSystemMetrics(SM_CXHSCROLL);
				cx += GetSystemMetrics(SM_CYVSCROLL);
				bShowHorz = FALSE;
				bShowVert = FALSE;
				break;
			}
			if(cx >= m_minClientSize.cx){
				if(bShowHorz){
					cy += GetSystemMetrics(SM_CXHSCROLL);
					bReCalc = true;
				}
				bShowHorz = FALSE;
			} else {
				if(!bShowHorz){
					cy -= GetSystemMetrics(SM_CXHSCROLL);
					bReCalc = true;
				}
				bShowHorz = TRUE;
			}
			if(cy >= m_minClientSize.cy){
				if(bShowVert){
					cx += GetSystemMetrics(SM_CYVSCROLL);
					bReCalc = true;
				}
				bShowVert = FALSE;
			} else {
				if(!bShowVert){
					cx -= GetSystemMetrics(SM_CYVSCROLL);
					bReCalc = true;
				}
				bShowVert = TRUE;
			}
			if(cnt > 10){
				bReCalc = false;
			}
		} while(bReCalc);
		ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
		ShowScrollBar(m_hWnd, SB_VERT, FALSE);
		SCROLLINFO si = {sizeof(SCROLLINFO)};
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nPage = cx;
		si.nMax  = m_minSize.cx;
		if(!bShowHorz){
			si.nPage = si.nMax;
		}
		SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);
		si.nPage = cy;
		si.nMax  = m_minSize.cy;
		if(!bShowVert){
			si.nPage = si.nMax;
		}
		SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
		ShowScrollBar(m_hWnd, SB_HORZ, bShowHorz);
		ShowScrollBar(m_hWnd, SB_VERT, bShowVert);
	}
	m_bCalc = false;
}

void CGrPropPageSize::OnSize(HWND hwnd, UINT nType, int cx, int cy)
{
	setScrollRange(cx, cy);
	int to_hpos = getScrollPos(SB_HORZ);
	int to_vpos = getScrollPos(SB_VERT);
	int nHDelta = m_point.x - to_hpos;
	int nVDelta = m_point.y - to_vpos;
	if(nHDelta != 0 || nVDelta != 0){
		ScrollWindowEx(m_hWnd, nHDelta, nVDelta, nullptr, nullptr, NULL, nullptr, SW_INVALIDATE|SW_SCROLLCHILDREN|SW_ERASE);
		m_point.x -= nHDelta;
		m_point.y -= nVDelta;
	}
}

void CGrPropPageSize::onxScroll(int fnBar, UINT code, int pos)
{
	SCROLLINFO si = {sizeof(SCROLLINFO)};
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
	::GetScrollInfo(m_hWnd, fnBar, &si);

	switch(code){
	case SB_LINEUP:
		si.nPos -= m_scroll_line;
		if(si.nPos < 0){
			si.nPos = 0;
		}
		break;
	case SB_LINEDOWN:
		si.nPos += m_scroll_line;
		if(si.nPos >= static_cast<signed int>(si.nMax - si.nPage)){
			si.nPos = static_cast<signed int>(si.nMax - si.nPage);
		}
		break;
	case SB_THUMBPOSITION:
		si.nPos = pos;
		break;
	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		break;
	case SB_PAGEUP:
		si.nPos -= si.nPage;
		break;
	case SB_PAGEDOWN:
		si.nPos += si.nPage;
		break;
	case SB_TOP:
		si.nPos = 0;
		break;
	case SB_BOTTOM:
		si.nPos = si.nMax - si.nPage;
		break;
	}
	setScrollPos(fnBar, si);
}

void CGrPropPageSize::OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
	onxScroll(SB_HORZ, code, pos);
}

void CGrPropPageSize::OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
	onxScroll(SB_VERT, code, pos);
}

void CGrPropPageSize::setFocus(UINT id  )
{
	::PostMessage(m_hWnd, WM_SETPAGEFOCUS, (WPARAM)GetDlgItem(m_hWnd, id), 0L);
}

void CGrPropPageSize::setFocus(HWND hwnd)
{
	::PostMessage(m_hWnd, WM_SETPAGEFOCUS, (WPARAM)hwnd, 0L);
}
