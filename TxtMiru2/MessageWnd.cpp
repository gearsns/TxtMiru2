#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "MessageWnd.h"
#include "TxtMiruTheme.h"

CGrMessageWnd::CGrMessageWnd() : m_bHide(true)
{
}

CGrMessageWnd::~CGrMessageWnd()
{
}

// ウィンドウ・クラスの登録
BOOL CGrMessageWnd::InitApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm)
{
	WNDCLASSEX wc = {sizeof(WNDCLASSEX)};

	wc.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
	wc.lpszClassName = szClassNm;
	wc.hInstance     = hInst;
	wc.lpfnWndProc   = fnWndProc;

	return RegisterClassEx(&wc);
}

#define CGRTYPEID _T("CGrMessageWnd")
void CGrMessageWnd::Show(HWND hWnd, LPCTSTR lpStr)
{
	if(!m_bHide || !lpStr || *lpStr == '\0'){
		Hide();
		return;
	}
	if(!m_hWnd){
		auto hInst = GetWindowInstance(hWnd);
		InitApp(hInst, CGrWinCtrl::WindowMapProc, CGRTYPEID);
		m_hWnd = CreateWindowEx(
			0,
			CGRTYPEID,
			NULL,
			WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0, 0, 100, 100,
			hWnd,
			NULL,
			hInst,
			this);
		auto hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
		if(hFont){
			LOGFONT lf = {};
			if(GetObject(hFont, sizeof(lf), &lf)){
				m_font.CreateFontIndirect(lf);
			}
		}
		SetWindowFont(m_hWnd, m_font, TRUE);
	}
	m_bHide = false;
	//
	SIZE size = {};
	auto hdc = CreateCompatibleDC(NULL);
	auto hOldFont = SelectFont(hdc, m_font);
	{
		RECT rect = {};
		::DrawText(hdc, lpStr, -1, &rect, DT_CALCRECT);
		size.cx = rect.right - rect.left;
		size.cy = rect.bottom - rect.top;
	}
	SelectFont(hdc, hOldFont);
	DeleteDC(hdc);
	//
	auto padding = size.cy / 2;
	RECT rect = {};
	GetClientRect(hWnd, &rect);
	rect.top = rect.bottom - size.cy - padding;
	rect.right = rect.left + size.cx + padding;
	MoveWindow(m_hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE);
	ShowWindow(m_hWnd, SW_SHOW);
	m_message = lpStr;
}

void CGrMessageWnd::Hide()
{
	if(m_bHide){
		return;
	}
	m_bHide = true;
	if(m_hWnd){
		ShowWindow(m_hWnd, SW_HIDE);
	}
}

LRESULT CGrMessageWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG   , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_SIZE         , OnSize         );
		HANDLE_MSG(hWnd, WM_PAINT        , OnPaint        );
	case WM_ERASEBKGND: return TRUE;
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrMessageWnd::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	return TRUE;
}

void CGrMessageWnd::OnPaint(HWND hwnd)
{
	PAINTSTRUCT lpps;
	auto hdc = ::BeginPaint(hwnd, &lpps);
	auto oldBkColor = SetBkColor(hdc, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
	auto oldTextColor = SetTextColor(hdc, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
	auto hOldFont = SelectFont(hdc, m_font);
	//
	RECT rect = {};
	GetClientRect(hwnd, &rect);
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
	DrawText(hdc, m_message.c_str(), -1, &rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	//
	SetBkColor(hdc, oldBkColor);
	SetTextColor(hdc, oldTextColor);
	SelectFont(hdc, hOldFont);

	::EndPaint(hwnd, &lpps);
}

void CGrMessageWnd::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
		return;
	}
	setWindowSize(cx, cy);
}

void CGrMessageWnd::setWindowSize(int cx, int cy)
{
}
