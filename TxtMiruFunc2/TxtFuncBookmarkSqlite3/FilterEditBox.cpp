#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "FilterEditBox.h"
#include <memory>

/* void Cls_OnImeChar(HWND hwnd, TCHAR ch, int cRepeat) */
#define HANDLE_WM_IME_CHAR(hwnd, wParam, lParam, fn) \
	((fn)((hwnd), (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)
#define FORWARD_WM_IME_CHAR(hwnd, ch, cRepeat, fn) \
	(void)(fn)((hwnd), WM_IME_CHAR, (WPARAM)(TCHAR)(ch), MAKELPARAM((cRepeat),0))


CGrFilterEditBox::CGrFilterEditBox()
{
}

CGrFilterEditBox::~CGrFilterEditBox()
{
}

void CGrFilterEditBox::OnChar(HWND hWnd, TCHAR ch, int cRepeat)
{
	CGrWinCtrl::WndProc(hWnd, WM_CHAR, (WPARAM)(TCHAR)(ch), MAKELPARAM((cRepeat),0));
	UpdateRect();
}

void CGrFilterEditBox::OnImeChar(HWND hWnd, TCHAR ch, int cRepeat)
{
	CGrWinCtrl::WndProc(hWnd, WM_IME_CHAR, (WPARAM)(TCHAR)(ch), MAKELPARAM((cRepeat),0));
	UpdateRect();
}

BOOL CGrFilterEditBox::OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	if (codeHitTest == HTCLIENT && m_barrow) {
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return TRUE;
	}
	return FALSE;
}

void CGrFilterEditBox::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	FORWARD_WM_LBUTTONDOWN(hwnd, fDoubleClick, x, y, keyFlags, CGrWinCtrl::WndProc);
	if (!m_empty) {
		RECT rect;
		GetClientRect(hwnd, &rect);
		if(x > rect.right - (rect.bottom - rect.top)){
			SetWindowText(hwnd, L"");
			SetCursor(LoadCursor(NULL, IDC_IBEAM));
			m_barrow = false;
			m_empty = true;
			InvalidateRect(hwnd, nullptr, TRUE);
			FORWARD_WM_COMMAND(GetParent(hwnd), 0, m_hWnd, EN_CHANGE, PostMessage);
		}
	}
}

void CGrFilterEditBox::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	if (!m_empty) {
		RECT rect;
		GetClientRect(hwnd, &rect);
		m_barrow = (x > rect.right - (rect.bottom - rect.top));
		if (m_barrow) {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		} else {
			SetCursor(LoadCursor(NULL, IDC_IBEAM));
		}
	}
}

void CGrFilterEditBox::OnPaint(HWND hwnd)
{
	FORWARD_WM_PAINT(hwnd, CGrWinCtrl::WndProc);
	if(m_bg_text.size() > 0 && m_empty){
		const auto hdc = GetDC(hwnd);
		RECT rect;
		GetClientRect(hwnd, &rect);
		SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
		SetBkMode(hdc, TRANSPARENT);
		auto hfont = SelectObject(hdc, GetWindowFont(hwnd));
		auto text = m_bg_text.c_str();
		auto size = m_bg_text.size();
		ExtTextOut(hdc, 1, 1, ETO_CLIPPED, &rect, text, size, nullptr);
		SelectObject(hdc, hfont);
		ReleaseDC(hwnd, hdc);
	}
}

BOOL CGrFilterEditBox::OnEraseBkgnd(HWND hwnd, HDC hdc)
{
	FORWARD_WM_ERASEBKGND(hwnd, hdc, CGrWinCtrl::WndProc);
	RECT rect = {};
	GetClientRect(hwnd, &rect);
	if (!m_empty) {
		rect.left = rect.right - (rect.bottom - rect.top);
		DrawFrameControl(hdc, &rect, DFC_CAPTION, DFCS_CAPTIONCLOSE | DFCS_FLAT | DFCS_TRANSPARENT | DFCS_ADJUSTRECT);
	}
	return FALSE;
}

void CGrFilterEditBox::SetBackgroundText(LPCTSTR lpstr)
{
	if (lpstr) {
		m_bg_text = lpstr;
	}
	else {
		m_bg_text.clear();
	}
	InvalidateRect(m_hWnd, nullptr, TRUE);
}

int CGrFilterEditBox::GetText(std::tstring &outstr)
{
	auto len = Edit_GetTextLength(m_hWnd);
	std::unique_ptr<TCHAR> ptr(new TCHAR[len+2]);
	auto ret = Edit_GetText(m_hWnd, ptr.get(), len+1);
	if (ret > 0) {
		outstr = ptr.get();
	}
	else {
		outstr.clear();
	}
	return ret;
}

void CGrFilterEditBox::UpdateRect()
{
	RECT rect = {};
	GetClientRect(m_hWnd, &rect);
	if (Edit_GetTextLength(m_hWnd) == 0) {
		m_empty = true;
	} else {
		m_empty = false;
		rect.right = rect.right - (rect.bottom - rect.top);
	}
	SendMessage(m_hWnd, EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rect));
}

// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
LRESULT CGrFilterEditBox::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_SETCURSOR    , OnSetCursor    );
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN  , OnLButtonDown  );
		HANDLE_MSG(hWnd, WM_MOUSEMOVE    , OnMouseMove    );
		HANDLE_MSG(hWnd, WM_PAINT        , OnPaint        );
		HANDLE_MSG(hWnd, WM_ERASEBKGND   , OnEraseBkgnd   );
		HANDLE_MSG(hWnd, WM_CHAR         , OnChar         );
		HANDLE_MSG(hWnd, WM_IME_CHAR     , OnImeChar      );
	case WM_SIZE:
		{
			auto ret = CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
			UpdateRect();
			return ret;
		}
	default:
		return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
