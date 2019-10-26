#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "TxtSearchDlg.h"
#include "MessageBox.h"
#include "TxtFuncIParam.h"
#include "TxtFunc.h"
#include "TxtFuncBookmarkDef.h"
#include "BookmarkDB.h"
#include "TxtMiruTheme.h"

CGrTxtSearchDlg::CGrTxtSearchDlg(void *pDoc, const TxtMiru::TextPoint &point) : m_pDoc(pDoc), m_curPoint(point)
{
}

CGrTxtSearchDlg::~CGrTxtSearchDlg()
{
}

int CGrTxtSearchDlg::DoModal(HWND hWnd)
{
	return DialogBoxParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_SEARCH), hWnd, (DLGPROC)CGrWinCtrl::WindowMapProc, (LPARAM)this);
}

LRESULT CGrTxtSearchDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG , OnInitDialog);
		HANDLE_MSG(hWnd, WM_COMMAND    , OnCommand   );
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrTxtSearchDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowPosCenter();
	refreshList();

	const auto &param = CGrTxtFunc::Param();
	bool bLoop = param.GetBoolean(CGrTxtFuncIParam::PointsType::SearchLoop);
	Button_SetCheck(GetDlgItem(m_hWnd, IDC_CHECK_LOOP), bLoop);
	bool bRegExp = param.GetBoolean(CGrTxtFuncIParam::PointsType::UseRegExp);
	Button_SetCheck(GetDlgItem(m_hWnd, IDC_USE_REGEXP), bRegExp);
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);

	return TRUE;
}

void CGrTxtSearchDlg::refreshList()
{
	const auto &param = CGrTxtFunc::Param();
	auto chWnd = GetDlgItem(m_hWnd, IDC_COMBO_SEARCH);
	ComboBox_ResetContent(chWnd);

	std::vector<SearchHistory> history_list;
	CGrDBSearchHistory db;
	bool bret = db.Open();
	if(bret){
		bret = db.Get(history_list, _T("cur"), 30);
	}
	db.Close();
	if(bret){
		for(const auto &item : history_list){
			ComboBox_AddString(chWnd, item.value.c_str());
		}
	} else {
		for(int idx=0; idx<9; ++idx){
			TCHAR buf[2048] = {};
			param.GetText((CGrTxtFuncIParam::TextType)(static_cast<int>(CGrTxtFuncIParam::TextType::HistSearchWord9)-idx), buf, _countof(buf));
			if(buf[0]){
				ComboBox_AddString(chWnd, buf);
			}
		}
	}
	ComboBox_SetCurSel(chWnd, 0);
}

using TxtMiruFunc_SearchCurrent = bool (cdecl *)(void *pDoc, LPCTSTR lpSrc, TxtMiru::TextPoint &pos, bool bLoop, bool bUseRegExp, bool bDown);

void CGrTxtSearchDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if(id == IDOK || id == IDPREVSEARCH){
		int num = GetWindowTextLength(GetDlgItem(m_hWnd, IDC_COMBO_SEARCH));
		std::vector<TCHAR> buf;
		buf.resize(max(num, 2048)+1);
		GetDlgItemText(m_hWnd, IDC_COMBO_SEARCH, &buf[0], buf.size()-1);
		auto &&param = CGrTxtFunc::Param();
		CGrDBSearchHistory db;
		bool bret = db.Open();
		if(bret){
			bret = db.Put(_T("cur"), &buf[0]);
		}
		db.Close();
		bool bLoop = Button_GetCheck(GetDlgItem(m_hWnd, IDC_CHECK_LOOP)) == TRUE;
		param.SetBoolean(CGrTxtFuncIParam::PointsType::SearchLoop, bLoop);
		bool bRegExp = Button_GetCheck(GetDlgItem(m_hWnd, IDC_USE_REGEXP)) == TRUE;
		param.SetBoolean(CGrTxtFuncIParam::PointsType::UseRegExp, bRegExp);
		refreshList();
		auto func = reinterpret_cast<TxtMiruFunc_SearchCurrent>(GetProcAddress(GetModuleHandle(NULL), "TxtMiruFunc_SearchCurrent"));
		if(!func){
			return;
		}
		if(func(m_pDoc, &buf[0], m_curPoint, bLoop, bRegExp, id==IDOK)){
			::EndDialog(m_hWnd, IDOK);
		} else {
			CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_SEARCH_NOT_FOUND, CGrTxtFunc::AppName());
		}
	} else if(id == IDCANCEL){
		::EndDialog(m_hWnd, id);
	}
}
