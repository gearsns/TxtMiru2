#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "TagDlg.h"
#include "TxtFunc.h"
#include "TxtMiruTheme.h"

CGrTagDlg::CGrTagDlg(TxtFuncBookmark::Tag &tag) : m_tag(tag)
{
}

CGrTagDlg::~CGrTagDlg()
{
}

int CGrTagDlg::DoModal(HWND hWnd)
{
	return DialogBoxParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_TAG), hWnd, (DLGPROC)CGrWinCtrl::WindowMapProc, (LPARAM)this);
}

LRESULT CGrTagDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG , OnInitDialog);
		HANDLE_MSG(hWnd, WM_COMMAND    , OnCommand   );
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrTagDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowPosCenter();

	m_EditTag.SetWnd(GetDlgItem(hwnd, IDC_EDIT_TAG));
	m_EditTag.SetText(m_tag.title.c_str());
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);

	return TRUE;
}

void CGrTagDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if(id == IDOK){
		m_EditTag.GetText(m_tag.title);
		::EndDialog(m_hWnd, IDOK);
	} else if(id == IDCANCEL){
		::EndDialog(m_hWnd, IDCANCEL);
	}
}
