#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "AddURLDlg.h"
#include "TxtMiruTheme.h"


CGrAddURLDlg::CGrAddURLDlg()
{
}


CGrAddURLDlg::~CGrAddURLDlg()
{
}


int CGrAddURLDlg::DoModal(HINSTANCE hinstance, HWND hWnd)
{
	return DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_ADDCACHE), hWnd, (DLGPROC)CGrWinCtrl::WindowMapProc, (LPARAM)this);
}

LRESULT CGrAddURLDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		HANDLE_MSG(hWnd, WM_INITDIALOG, OnInitDialog);
		HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrAddURLDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowPosCenter();

	m_EditURL.SetWnd(GetDlgItem(hwnd, IDC_EDIT_URL));
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);

	return TRUE;
}

void CGrAddURLDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDOK) {
		m_EditURL.GetText(m_url);
		::EndDialog(m_hWnd, IDOK);
	}
	else if (id == IDCANCEL) {
		::EndDialog(m_hWnd, IDCANCEL);
	}
}

void CGrAddURLDlg::Get(std::tstring &outurl)
{
	outurl = m_url;
}