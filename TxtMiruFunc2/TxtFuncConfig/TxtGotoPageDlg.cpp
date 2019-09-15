#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "TxtGotoPageDlg.h"
#include "TxtFunc.h"
#include "TxtMiruFunc.h"
#include "TxtMiruTheme.h"

CGrTxtGotoPageDlg::CGrTxtGotoPageDlg(int maxnum, int cur_page) : m_nMaxPage(maxnum), m_nCurPage(cur_page)
{
}

CGrTxtGotoPageDlg::~CGrTxtGotoPageDlg()
{
}

int CGrTxtGotoPageDlg::DoModal(HWND hWnd)
{
	return DialogBoxParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_GOTOPAGE), hWnd, reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
}

LRESULT CGrTxtGotoPageDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG , OnInitDialog);
		HANDLE_MSG(hWnd, WM_COMMAND    , OnCommand   );
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrTxtGotoPageDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	::SetDlgItemInt(hwnd, IDC_EDIT_PAGE, m_nCurPage, true);
	::SendMessage(GetDlgItem(hwnd, IDC_SPIN_PAGE), UDM_SETRANGE32, m_nMaxPage, 0);

	TxtMiruTheme_SetWindowSubclass(m_hWnd);
	SetWindowPosCenter();
	return TRUE;
}

void CGrTxtGotoPageDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if(id == IDOK || id == IDCANCEL){
		BOOL translated;
		m_nCurPage = GetDlgItemInt(hwnd, IDC_EDIT_PAGE, &translated, true);
		EndDialog(hwnd, id);
	}
}
