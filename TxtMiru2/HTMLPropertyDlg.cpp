#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "HTMLPropertyDlg.h"
#include "TxtMiru.h"

CGrHTMLPropertyDlg::CGrHTMLPropertyDlg(const CGrTxtDocument *pDoc) : m_pDoc(pDoc)
{
}

CGrHTMLPropertyDlg::~CGrHTMLPropertyDlg()
{
}

int CGrHTMLPropertyDlg::DoModal(HWND hWnd)
{
	return DialogBoxParam(GetWindowInstance(hWnd), MAKEINTRESOURCE(IDD_HTMLPROPERTY), hWnd, reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
}

LRESULT CGrHTMLPropertyDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG , OnInitDialog);
		HANDLE_MSG(hWnd, WM_COMMAND    , OnCommand   );
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrHTMLPropertyDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowPosCenter();

	const auto &buffer = m_pDoc->GetTxtBuffer();
	std::tstring address;
	m_pDoc->GetFileName(address);
	SetDlgItemText(hwnd, IDC_TITLE     , m_pDoc->GetTitle().c_str()       );
	SetDlgItemText(hwnd, IDC_ADDRESS   , address.c_str()                  );
	SetDlgItemText(hwnd, IDC_UPDATEDATE, buffer.GetLastWriteTime().c_str());
	return TRUE;
}

void CGrHTMLPropertyDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDOK    : ::EndDialog(m_hWnd, id); break;
	case IDCANCEL: ::EndDialog(m_hWnd, id); break;
	}
}
