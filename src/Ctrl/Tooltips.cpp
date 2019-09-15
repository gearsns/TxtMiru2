#define STRICT // 型を宣言および使用時に、より厳密な型チェックが行われます。

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "Tooltips.h"

CGrToolTips::CGrToolTips()
{
}

HWND CGrToolTips::Create(HWND hWnd)
{
	m_hWnd = ::CreateWindowEx(/*NULL*/WS_EX_TOPMOST, TOOLTIPS_CLASS,
							  NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX | TTS_NOPREFIX | TTS_NOANIMATE | TTS_NOFADE,
							  CW_USEDEFAULT, CW_USEDEFAULT,
							  CW_USEDEFAULT, CW_USEDEFAULT,
							  hWnd, NULL, GetWindowInstance(hWnd), NULL);
	Attach(m_hWnd);

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return m_hWnd;
}

// メインウィンドウ
LRESULT CGrToolTips::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

void CGrToolTips::SetRect(const RECT &rect)
{
	TOOLINFO ti = {sizeof (TOOLINFO)};
	ti.rect   = rect;
	::SendMessage(m_hWnd, TTM_NEWTOOLRECT, 0, LPARAM(&ti));
}

void CGrToolTips::AddToolTips(HWND targetWnd, int uId, LPCTSTR str, const RECT &rect)
{
	TOOLINFO ti = {sizeof (TOOLINFO)};

	ti.hwnd     = targetWnd;
	ti.hinst    = GetWindowInstance(m_hWnd);
	ti.uFlags   = TTF_SUBCLASS;
	ti.uId      = uId;
	ti.rect     = rect;
	ti.lpszText = const_cast<LPTSTR>(str);
	SendMessage(m_hWnd, TTM_ADDTOOL, 0, LPARAM(&ti));
}

int CGrToolTips::GetToolCount() const
{
	return ::SendMessage(m_hWnd,TTM_GETTOOLCOUNT,0,0);
}
void CGrToolTips::AllDellToolTips(HWND hWnd)
{
	for(int idx=GetToolCount();idx > 0; --idx){
		DellToolTips(hWnd, idx);
	}
}

void CGrToolTips::DellToolTips(HWND hWnd, int uId)
{
	TOOLINFO ti = {sizeof (TOOLINFO)};

	ti.hwnd     = hWnd;
	ti.uId      = uId;
	SendMessage(m_hWnd, TTM_DELTOOL, 0, LPARAM(&ti));
}

void CGrToolTips::SetTipsList(HWND hWnd, const TipsInfoList &tipsList)
{
	AllDellToolTips(hWnd);
	int len = tipsList.size();
	if(len > 0){
		const auto *it =&tipsList[0];
		for(int idx=1; len>0; --len, ++it, ++idx){
			const auto ti = (*it);
			AddToolTips(hWnd, idx, ti.str.c_str(), ti.rect);
		}
	}
	Pop();
}

void CGrToolTips::Pop()
{
	::SendMessage(m_hWnd, TTM_POP, 0, 0);
}
