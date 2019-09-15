#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "TxtDocInfoDlg.h"
#include "Text.h"
#include "TxtFunc.h"
#include "TxtMiruFunc.h"
#include "TxtMiruTheme.h"

CGrTxtDocInfoDlg::CGrTxtDocInfoDlg(LPCTSTR title, LPCTSTR author, LPCTSTR filename, LPCTSTR text, LPCTSTR writetime, int page, int maxpage)
: m_title(title), m_author(author), m_filename(filename), m_text(text), m_writetime(writetime), m_page(page), m_maxpage(maxpage)
{
}

CGrTxtDocInfoDlg::~CGrTxtDocInfoDlg()
{
}

int CGrTxtDocInfoDlg::DoModal(HWND hWnd)
{
	return DialogBoxParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_DOCINFO), hWnd, reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
}

LRESULT CGrTxtDocInfoDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG   , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_COMMAND      , OnCommand      );
		HANDLE_MSG(hWnd, WM_SIZE         , OnSize         );
		HANDLE_MSG(hWnd, WM_GETMINMAXINFO, OnGetMinMaxInfo);
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrTxtDocInfoDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowPosCenter();

	HICON hIcon = static_cast<HICON>(LoadImage(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 32, 32, LR_SHARED));
	SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
	HICON hIconSm = reinterpret_cast<HICON>(LoadImage(CGrTxtFunc::GetDllModuleHandle(),MAKEINTRESOURCE(IDI_APP),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR));
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSm));

	std::tstring page_str;
	CGrText::FormatMessage(CGrTxtFunc::GetDllModuleHandle(), page_str, IDS_MAXPAGE, m_maxpage);
	Edit_SetText(GetDlgItem(hwnd, IDC_EDIT_TITLE    ), m_title    .c_str());
	Edit_SetText(GetDlgItem(hwnd, IDC_EDIT_AUTHOR   ), m_author   .c_str());
	Edit_SetText(GetDlgItem(hwnd, IDC_EDIT_FILENAME ), m_filename .c_str());
	Edit_SetText(GetDlgItem(hwnd, IDC_EDIT_WRITETIME), m_writetime.c_str());
	Edit_SetText(GetDlgItem(hwnd, IDC_EDIT_DOC_INFO ), m_text     .c_str());
	Edit_SetText(GetDlgItem(hwnd, IDC_EDIT_PAGE     ), page_str   .c_str());
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);

	RECT win_rect;
	::GetWindowRect(m_hWnd, &win_rect);
	m_minSize.cx = win_rect.right - win_rect.left;
	m_minSize.cy = win_rect.bottom - win_rect.top;

	setWindowSize(-1, -1);
	return TRUE;
}

void CGrTxtDocInfoDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if(id == IDOK || id == IDCANCEL){
		::EndDialog(m_hWnd, IDOK);
	}
}

void CGrTxtDocInfoDlg::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	lpMinMaxInfo->ptMinTrackSize.x = m_minSize.cx;
	lpMinMaxInfo->ptMinTrackSize.y = m_minSize.cy;
}

void CGrTxtDocInfoDlg::OnSize(HWND hwnd, UINT nType, int cx, int cy)
{
	setWindowSize(cx, cy);
}

void CGrTxtDocInfoDlg::setWindowSize(int cx, int cy)
{
	if(cx <= 0 || cy <= 0){
		RECT win_rect;
		::GetClientRect(m_hWnd, &win_rect);
		cx = win_rect.right - win_rect.left;
		cy = win_rect.bottom - win_rect.top;
	}
	auto hWnd = GetDlgItem(m_hWnd, IDC_EDIT_DOC_INFO);
	if(hWnd){
		RECT rect = {0,0,cx,cy};
		auto hdwp = BeginDeferWindowPos(6);

		int bottom = cy;
		{
			auto hCWnd = GetDlgItem(m_hWnd, IDOK);
			GetWindowRect(hCWnd, &rect);
			ScreenToClientRect(&rect);
			int height = rect.bottom - rect.top;
			int width = rect.right - rect.left;
			bottom -= height;
			rect.left = cx /2 - width / 2;
			hdwp = DeferWindowPos(hdwp, hCWnd, NULL, rect.left, bottom, -1, -1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
		}
		for(auto id : {IDC_EDIT_TITLE, IDC_EDIT_AUTHOR, IDC_EDIT_FILENAME, IDC_EDIT_WRITETIME, IDC_EDIT_PAGE})
		{
			auto hCWnd = GetDlgItem(m_hWnd, id);
			GetWindowRect(hCWnd, &rect);
			ScreenToClientRect(&rect);
			int height = rect.bottom - rect.top;
			int width = cx - rect.left;
			hdwp = DeferWindowPos(hdwp, hCWnd, NULL, -1, -1, width, height, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
		}
		{
			GetWindowRect(hWnd, &rect);
			ScreenToClientRect(&rect);
			int height = bottom - rect.top;
			int width = cx - rect.left * 2;
			hdwp = DeferWindowPos(hdwp, hWnd, NULL, -1, -1, width, height, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
		}
		EndDeferWindowPos(hdwp);
	}
}

