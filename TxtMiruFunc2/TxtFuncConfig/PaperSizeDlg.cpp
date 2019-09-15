#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PaperSizeDlg.h"
#include "TxtFunc.h"
#include "MessageBox.h"

CGrPaperSizeDlg::CGrPaperSizeDlg()
{
}

CGrPaperSizeDlg::~CGrPaperSizeDlg()
{
}

int CGrPaperSizeDlg::DoModal(HWND hWnd, const SIZE &paperSize)
{
	m_paperSize = paperSize;
	return DialogBoxParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_PAPER_SIZE), hWnd, reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
}

LRESULT CGrPaperSizeDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG    , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_COMMAND       , OnCommand      );
		break;
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrPaperSizeDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	if(m_paperSize.cx == 21000 && m_paperSize.cy == 14800){
		SendMessage(GetDlgItem(m_hWnd, IDC_RADIO_BUNKO), BM_SETCHECK, BST_CHECKED, 0);
	} else if(m_paperSize.cx == 20600 && m_paperSize.cy == 18200){
		SendMessage(GetDlgItem(m_hWnd, IDC_RADIO_SHINSHO), BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(GetDlgItem(m_hWnd, IDC_RADIO_ETC), BM_SETCHECK, BST_CHECKED, 0);
		SetDlgItemInt(hwnd, IDC_EDIT_PAPER_WIDTH , m_paperSize.cx/100, TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT_PAPER_HEIGHT, m_paperSize.cy/100, TRUE);
	}
	return TRUE;
}

extern void EnableDlgItemID(HWND hWnd, UINT id, BOOL flag);

void CGrPaperSizeDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDOK:
		{
			if(::IsDlgButtonChecked(m_hWnd, IDC_RADIO_BUNKO) == BST_CHECKED){
				m_paperSize.cx = 21000;
				m_paperSize.cy = 14800;
			} else if(::IsDlgButtonChecked(m_hWnd, IDC_RADIO_SHINSHO) == BST_CHECKED){
				m_paperSize.cx = 20600;
				m_paperSize.cy = 18200;
			} else {
				BOOL bTranslated;
				m_paperSize.cx = GetDlgItemInt(m_hWnd, IDC_EDIT_PAPER_WIDTH , &bTranslated, TRUE) * 100;
				m_paperSize.cy = GetDlgItemInt(m_hWnd, IDC_EDIT_PAPER_HEIGHT, &bTranslated, TRUE) * 100;
			}
			if(m_paperSize.cx < 10000 || m_paperSize.cy < 10000){
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_ERROR_LAY_INPUT);
				break;
			}
		}
		::EndDialog(m_hWnd, id);
		break;
	case IDCANCEL: ::EndDialog(m_hWnd, id); break;
	case IDC_RADIO_BUNKO:
	case IDC_RADIO_SHINSHO:
	case IDC_RADIO_ETC:
		{
			if(::IsDlgButtonChecked(m_hWnd, IDC_RADIO_ETC) == BST_CHECKED){
				Edit_SetReadOnly(GetDlgItem(m_hWnd, IDC_EDIT_PAPER_WIDTH ), FALSE);
				Edit_SetReadOnly(GetDlgItem(m_hWnd, IDC_EDIT_PAPER_HEIGHT), FALSE);
			} else {
				Edit_SetReadOnly(GetDlgItem(m_hWnd, IDC_EDIT_PAPER_WIDTH ), TRUE);
				Edit_SetReadOnly(GetDlgItem(m_hWnd, IDC_EDIT_PAPER_HEIGHT), TRUE);
			}
		}
		break;
	}
}

const SIZE CGrPaperSizeDlg::GetPaperSize() const
{
	return m_paperSize;
}
