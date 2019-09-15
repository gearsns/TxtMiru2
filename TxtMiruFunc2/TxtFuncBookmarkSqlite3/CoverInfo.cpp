#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "CoverInfo.h"
#include "TxtMiruTheme.h"

CGrCoverInfo::CGrCoverInfo()
{
}

CGrCoverInfo::~CGrCoverInfo()
{
}

HWND CGrCoverInfo::Create(HWND hWnd)
{
	auto hInst = CGrTxtFunc::GetDllModuleHandle();
	m_hWnd = ::CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_COVERINFO), hWnd, reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
	::ShowWindow(m_hWnd, SW_SHOW);
	return m_hWnd;
}

void CGrCoverInfo::Attach(HWND hWnd)
{
	CGrWinCtrl::Attach(hWnd);
}

void CGrCoverInfo::setWindowSize(int cx, int cy)
{
	RECT rect = {0,0,cx,cy};
	RECT rc;
	m_EditTitle.GetRect(&rc);
	int height = rc.bottom - rc.top;
	if (m_EditTitle) { SetWindowPos(m_EditTitle, NULL, 0, 0, cx, height, SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER); }
	if (m_EditURL) { SetWindowPos(m_EditURL, NULL, 0, 0, cx, height, SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER); }
	if (m_EditAuthor) { SetWindowPos(m_EditAuthor, NULL, 0, 0, cx, height, SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER); }
	InvalidateRect(m_hWnd, nullptr, FALSE);
	UpdateWindow(m_hWnd);
}

LRESULT CGrCoverInfo::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_INITDIALOG:
		{
			m_EditTitle.SetWnd(GetDlgItem(m_hWnd, IDC_EDIT_TITLE));
			m_EditURL.SetWnd(GetDlgItem(m_hWnd, IDC_EDIT_URL));
			m_EditAuthor.SetWnd(GetDlgItem(m_hWnd, IDC_EDIT_AUTHOR));
			TxtMiruTheme_SetWindowSubclass(m_hWnd);
			RECT rc;
			GetClientRect(m_hWnd, &rc);
			setWindowSize(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;
	case WM_SIZE:
		{
			auto state = static_cast<UINT>(wParam);
			auto cx = static_cast<int>(static_cast<short>(LOWORD(lParam)));
			auto cy = static_cast<int>(static_cast<short>(HIWORD(lParam)));
			setWindowSize(cx, cy);
		}
		break;
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}


void CGrCoverInfo::Set(const TxtFuncBookmark::Book &book)
{
	m_book = book;
	m_EditTitle .SetText(m_book.title .c_str());
	m_EditURL   .SetText(m_book.url   .c_str());
	m_EditAuthor.SetText(m_book.author.c_str());
}
