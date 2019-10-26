#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PropPageBookmark.h"
#include "TxtConfigDlg.h"
#include "Text.h"
#include "Shell.h"
#include "CurrentDirectory.h"
#include "TxtFuncConfig.h"
#include "TxtFunc.h"
#include "TxtConfigFunc.h"
#include "MessageBox.h"

CGrPropPage* PASCAL CGrPropPageBookmark::CreateProp(CGrTxtConfigDlg *pDlg){
	auto *pPage = new CGrPropPageBookmark(pDlg);
	pPage->m_hWnd = CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_PROPPAGE_BOOKMARK), pDlg->GetWnd(), reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(pPage));
	pPage->Attach(pPage->m_hWnd);
	return pPage;
}

CGrPropPageBookmark::CGrPropPageBookmark(CGrTxtConfigDlg *pDlg) : CGrPropPageSize(pDlg)
{
}

CGrPropPageBookmark::~CGrPropPageBookmark()
{
}

// メインウィンドウ
LRESULT CGrPropPageBookmark::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

BOOL CGrPropPageBookmark::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	CGrPropPageSize::OnInitDialog(hwnd, hwndFocus, lParam);
	auto &&param = CGrTxtFunc::Param();
	// 栞の自動保存
	int auto_save_mode[2] = {};
	param.GetPoints(CGrTxtFuncIParam::PointsType::BookMarkAutoSave, auto_save_mode, sizeof(auto_save_mode)/sizeof(int));
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_BOOKMARKAUTOSAVE, (auto_save_mode[0] == 1));
	SetDlgItemInt(hwnd, IDC_EDIT_BOOKMARKAUTOSAVE_INTERVAL, auto_save_mode[1], true);
	// 栞ファイルは栞フォルダに保存
	bool bBookMarkToFolder = param.GetBoolean(CGrTxtFuncIParam::PointsType::BookMarkToFolder);
	::SetCheckDlgItemID(hwnd, IDC_RADIO_BOOKMARKSAVE_FOLDER, bBookMarkToFolder ); // 栞ファイルは栞フォルダに保存
	::SetCheckDlgItemID(hwnd, IDC_RADIO_BOOKMARKSAVE_SAME  , !bBookMarkToFolder); // テキストファイルと同じフォルダに作成
	// 最大 iBookMarkNum 個まで保存
	int iBookMarkNum = 0;
	param.GetPoints(CGrTxtFuncIParam::PointsType::BookMarkNum, &iBookMarkNum, 1);
	SetDlgItemInt(hwnd, IDC_EDIT_MAXBOOKMARK, iBookMarkNum, true);
	::SendMessage(GetDlgItem(hwnd, IDC_SPIN_MAXBOOMARK), UDM_SETRANGE32, (WPARAM)10, (LPARAM)1);
	// 栞フォルダ
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_BOOKMARKFOLDER , CGrTxtFuncIParam::TextType::BookMarkFolder );
	// お気に入り画面 背面に移動を許可（※再起動）
	std::tstring ini_filename;
	CGrTxtFunc::GetBookmarkFolder(&param, ini_filename);
	ini_filename += _T("/bookmark.ini");
	TCHAR val[2048];
	::GetPrivateProfileString(_T("Bookmark"), _T("WindowParent"), _T(""), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_BMWND_BK, lstrcmpi(val, _T("Top")) == 0);

	return TRUE;
}

void CGrPropPageBookmark::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDC_RADIO_BOOKMARKSAVE_FOLDER:
		::SetCheckDlgItemID(hwnd, IDC_RADIO_BOOKMARKSAVE_FOLDER, TRUE ); // 栞ファイルは栞フォルダに保存
		::SetCheckDlgItemID(hwnd, IDC_RADIO_BOOKMARKSAVE_SAME  , FALSE); // テキストファイルと同じフォルダに作成
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_RADIO_BOOKMARKSAVE_SAME:
		::SetCheckDlgItemID(hwnd, IDC_RADIO_BOOKMARKSAVE_FOLDER, FALSE); // 栞ファイルは栞フォルダに保存
		::SetCheckDlgItemID(hwnd, IDC_RADIO_BOOKMARKSAVE_SAME  , TRUE ); // テキストファイルと同じフォルダに作成
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_CHECKBOX_USEPREPARSER:
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_BUTTON_BOOKMARKFOLDER : BrowseForFolder(hwnd, IDC_EDIT_BOOKMARKFOLDER , IDS_BOOKMARK_FOLDER ); break;
	case IDC_BUTTON_ASSOC: // 栞ファイルに関連付け
		{
			CGrCurrentDirectory curDir;
			TCHAR curPath[MAX_PATH];
			CGrShell::GetExePath(curPath);
			TCHAR exeFileName[MAX_PATH];
			_stprintf_s(exeFileName, _T("%s\\TxtMiruAssoc"), curPath);
			TCHAR parameters[1024];
			_stprintf_s(parameters, _T("\"-A=%s\\TxtMiruCli.exe\" \"-D=%s\""), curPath, CGrTxtFunc::GetDataPath());

			ShellExecute(hwnd, NULL, exeFileName, parameters, curPath, SW_SHOWDEFAULT);
		}
		break;
	case IDC_BUTTON_REMOVEASSOC: // 関連付け解除
		{
			CGrCurrentDirectory curDir;
			TCHAR curPath[MAX_PATH];
			CGrShell::GetExePath(curPath);
			TCHAR exeFileName[MAX_PATH];
			_stprintf_s(exeFileName, _T("%s\\TxtMiruAssoc"), curPath);

			ShellExecute(hwnd, NULL, exeFileName, _T("-DEL"), curPath, SW_SHOWDEFAULT);
		}
		break;
	case IDC_CHECKBOX_BOOKMARKAUTOSAVE: // 栞の自動保存
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_EDIT_MAXBOOKMARK              : // 最大 n 個まで保存
	case IDC_EDIT_BOOKMARKFOLDER           : // 栞フォルダ
	case IDC_EDIT_BOOKMARKAUTOSAVE_INTERVAL: // 自動保存の間隔
		if(codeNotify == EN_CHANGE){
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	}
}

bool CGrPropPageBookmark::Apply()
{
	auto &&param = CGrTxtFunc::Param();
	// 栞の自動保存
	int auto_save_mode[2] = {};
	int iAutoSaveFlag;
	auto_save_mode[0] = (::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_BOOKMARKAUTOSAVE) == BST_CHECKED) ? 1 : 0;
	auto_save_mode[1] = GetDlgItemInt(m_hWnd, IDC_EDIT_BOOKMARKAUTOSAVE_INTERVAL, &iAutoSaveFlag, true);
	param.SetPoints(CGrTxtFuncIParam::PointsType::BookMarkAutoSave, auto_save_mode, sizeof(auto_save_mode)/sizeof(int));
	// 栞ファイルは栞フォルダに保存
	param.SetBoolean(CGrTxtFuncIParam::PointsType::BookMarkToFolder, ::GetCheckDlgItemID(m_hWnd, IDC_RADIO_BOOKMARKSAVE_FOLDER) == BST_CHECKED);
	// 最大 iBookMarkNum 個まで保存
	int iBookMarkNumFlag;
	int iBookMarkNum = GetDlgItemInt(m_hWnd, IDC_EDIT_MAXBOOKMARK, &iBookMarkNumFlag, true);
	param.SetPoints(CGrTxtFuncIParam::PointsType::BookMarkNum, &iBookMarkNum, 1);
	// 栞フォルダ
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_BOOKMARKFOLDER, CGrTxtFuncIParam::TextType::BookMarkFolder);

	// お気に入り画面 背面に移動を許可（※再起動）
	const TCHAR *pVal = nullptr;
	if(::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_BMWND_BK) == BST_CHECKED){
		pVal = _T("Top");
	} else {
		pVal = _T("TxtMiru");
	}
	std::tstring ini_filename;
	CGrTxtFunc::GetBookmarkFolder(&param, ini_filename);
	if(!CGrShell::GetSearchDir(ini_filename.c_str())){
		if(!CGrShell::CreateFolder(ini_filename.c_str())){
			return false;
		}
	}
	ini_filename += _T("/bookmark.ini");
	::WritePrivateProfileString(_T("Bookmark"), _T("WindowParent"), pVal, ini_filename.c_str());
	//
	param.UpdateConfig(GetParent(GetParent(m_hWnd)));
	//
	return true;
}

