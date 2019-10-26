#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PropPageRubyList.h"
#include "TxtConfigDlg.h"
#include "Text.h"
#include "Shell.h"
#include "CurrentDirectory.h"
#include "TxtFuncConfig.h"
#include "TxtFunc.h"
#include "TxtConfigFunc.h"
#include "MessageBox.h"

CGrPropPage* PASCAL CGrPropPageRubyList::CreateProp(CGrTxtConfigDlg *pDlg)
{
	auto *pPage = new CGrPropPageRubyList(pDlg);
	pPage->m_hWnd = CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_PROPPAGE_RUBYLIST), pDlg->GetWnd(), reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(pPage));
	pPage->Attach(pPage->m_hWnd);
	return pPage;
}

CGrPropPageRubyList::CGrPropPageRubyList(CGrTxtConfigDlg *pDlg) : CGrPropPageSize(pDlg)
{
}

CGrPropPageRubyList::~CGrPropPageRubyList()
{
}

// メインウィンドウ
LRESULT CGrPropPageRubyList::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG, OnInitDialog );
		HANDLE_MSG(hWnd, WM_NOTIFY    , OnNotify     );
		HANDLE_MSG(hWnd, WM_COMMAND   , OnCommand    );
	case WM_SETPAGEFOCUS:
		::SetFocus(reinterpret_cast<HWND>(wParam));
		break;
	}
	return CGrPropPageSize::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrPropPageRubyList::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	CGrPropPageSize::OnInitDialog(hwnd, hwndFocus, lParam);
	auto &&param = CGrTxtFunc::Param();
	//
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_RUBYIGNORE, CGrTxtFuncIParam::TextType::RubyListIgnore);
	//
	return TRUE;
}

void CGrPropPageRubyList::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDC_EDIT_RUBYIGNORE:
		if(codeNotify == EN_CHANGE){
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	}
}

bool CGrPropPageRubyList::Apply()
{
	auto &&param = CGrTxtFunc::Param();
	//
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_RUBYIGNORE, CGrTxtFuncIParam::TextType::RubyListIgnore);
	//
	param.UpdateConfig(GetParent(GetParent(m_hWnd)));
	//
	return true;
}
