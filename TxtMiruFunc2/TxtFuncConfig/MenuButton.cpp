#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "MenuButton.h"

CGrMenuButton::CGrMenuButton()
{
}

CGrMenuButton::~CGrMenuButton()
{
}

// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
LRESULT CGrMenuButton::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_DRAWITEM   , DrawItem     );
	case WM_MOUSEMOVE:
		if(!m_bHover){
			m_bHover = true;
			TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT)};
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hWnd;
			_TrackMouseEvent(&tme);
			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
		}
		break;
	case WM_MOUSELEAVE:
		m_bHover = false;
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;
	case WM_LBUTTONDOWN:
		m_bHover = false;
		break;
	case WM_LBUTTONUP:
		m_bHover = false;
		break;
	}
	return CGrComonCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrMenuButton::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	return (message == WM_DRAWITEM);
}

BOOL CGrMenuButton::Attach(HWND hWnd)
{
	auto bret = CGrComonCtrl::Attach(hWnd);
	ModifyStyle(0, BS_OWNERDRAW);
	return bret;
}

void CGrMenuButton::DrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItemStruct)
{
	auto hdc = lpDrawItemStruct->hDC;
	auto rc = lpDrawItemStruct->rcItem;
	TCHAR str[1024];
	GetWindowText(hwnd, str, _countof(str));
	int w = rc.bottom / 5;
	if(m_bChecked){
		auto curRect = rc;
		curRect.right = curRect.left + w;
		if(m_bHover){
			FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
			SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
			SetTextColor(hdc, GetSysColor(COLOR_HOTLIGHT));
		} else {
			FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
			SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
			SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
		}
		FillRect(hdc, &curRect, GetSysColorBrush(COLOR_HIGHLIGHT));
	} else if(m_bHover){
		FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
		SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
		SetTextColor(hdc, GetSysColor(COLOR_HOTLIGHT));
	} else {
		FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));
		SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
		SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
	}
	SetBkMode(hdc, TRANSPARENT);
	auto text_rc = rc;
	text_rc.left   += GetSystemMetrics(SM_CXEDGE) + w * 2;
	text_rc.top    += GetSystemMetrics(SM_CYEDGE);
	text_rc.right  -= GetSystemMetrics(SM_CXEDGE);
	text_rc.bottom -= GetSystemMetrics(SM_CYEDGE);

	DrawText(hdc, str, -1, &text_rc, DT_VCENTER | DT_SINGLELINE);
}

void CGrMenuButton::SetCheck(bool checked)
{
	if(m_bChecked != checked){
		m_bChecked = checked;
		InvalidateRect(m_hWnd, nullptr, FALSE);
		UpdateWindow(m_hWnd);
	}
}
