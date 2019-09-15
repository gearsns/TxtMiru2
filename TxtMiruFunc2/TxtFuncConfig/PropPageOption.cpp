#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PropPageOption.h"
#include "TxtConfigDlg.h"
#include "Text.h"
#include "Shell.h"
#include "CurrentDirectory.h"
#include "TxtFuncConfig.h"
#include "TxtFunc.h"
#include "TxtConfigFunc.h"
#include "MessageBox.h"
#include "CSVText.h"

static struct PointSet l_txt_point_set[] = {
	{IDC_EDIT_MAXARCFILESIZE      , CGrTxtFuncIParam::ArcMaxFileSize  },
};
static struct PointRange l_txt_point_range[] = {
	{IDC_SPIN_MAXARCFILESIZE      , 0, 1000, 0                         },
	{IDC_SPIN_USEPREPARSERSIZE    , 0, 1000, 0                         },
};

void loadMenuName(std::map<std::tstring, std::tstring> &menu_list)
{
	std::tstring filename = _T("menu_name.lis");
	auto lpDataPath = CGrTxtFunc::GetDataPath();
	if (lpDataPath) {
		CGrShell::ToPrettyFileName(lpDataPath, filename);
		filename += _T("/menu_name.lis");
	}
	CGrCSVText csv;
	if (csv.Open(filename.c_str())) {
		for (const auto &item : csv.GetCSVROW()) {
			if (item.size() < 2) {
				continue;
			}
			if (item[1].size() > 0) {
				menu_list[item[0]] = item[1];
			}
		}
	}
}

void saveMenuName(std::map<std::tstring, std::tstring> &menu_list)
{
	std::tstring filename = _T("menu_name.lis");
	auto lpDataPath = CGrTxtFunc::GetDataPath();
	if (lpDataPath) {
		CGrShell::ToPrettyFileName(lpDataPath, filename);
		filename += _T("/menu_name.lis");
	}
	CGrCSVText csv;
	for (const auto &item : menu_list) {
		if (item.second.size() > 0) {
			csv.AddFormatTail(L"ss", item.first.c_str(), item.second.c_str());
		}
	}
	csv.Save(filename.c_str());
}

CGrPropPage* PASCAL CGrPropPageOption::CreateProp(CGrTxtConfigDlg *pDlg)
{
	auto *pPage = new CGrPropPageOption(pDlg);
	pPage->m_hWnd = CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_PROPPAGE_OPTION), pDlg->GetWnd(), reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(pPage));
	pPage->Attach(pPage->m_hWnd);
	return pPage;
}

CGrPropPageOption::CGrPropPageOption(CGrTxtConfigDlg *pDlg) : CGrPropPageSize(pDlg)
{
}

CGrPropPageOption::~CGrPropPageOption()
{
}

// メインウィンドウ
LRESULT CGrPropPageOption::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

BOOL CGrPropPageOption::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	CGrPropPageSize::OnInitDialog(hwnd, hwndFocus, lParam);
	auto &&param = CGrTxtFunc::Param();
	// ウインドウサイズの保存
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_SAVESIZE, param.GetBoolean(CGrTxtFuncIParam::SaveWindowSize));
	// タイトルバーの書式
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_TITLEFORMAT, CGrTxtFuncIParam::TitleFormat);
	// susieフォルダ
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_SPIPLUGINFOLDER, CGrTxtFuncIParam::SpiPluginFolder);
	//
	int iUsePreParser[3] = {};
	param.GetPoints(CGrTxtFuncIParam::UsePreParser, iUsePreParser, sizeof(iUsePreParser)/sizeof(int));
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_USEPREPARSER      , iUsePreParser[0] == 1);
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_SHOWPREPARSERERROR, iUsePreParser[1] == 1);
	::SetDlgItemInt(m_hWnd, IDC_EDIT_USEPREPARSERSIZE, iUsePreParser[2], true);
	//
	::SetDlgItemPointSet(hwnd, l_txt_point_set, sizeof(l_txt_point_set)/sizeof(PointSet));
	::SetDlgItemPointRange(hwnd, l_txt_point_range, sizeof(l_txt_point_range)/sizeof(PointRange));
	int iFileAutoReload[3] = {};
	param.GetPoints(CGrTxtFuncIParam::FileAutoReload, iFileAutoReload, sizeof(iFileAutoReload)/sizeof(int));
	// ファイルの更新時、自動的に再読み込みを行う
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTORELOAD     , iFileAutoReload[0] == 1);
	// 再読み込時、排他モードで開く
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTORELOADLOCK , iFileAutoReload[1] == 1);
	// 監視間隔
	if(iFileAutoReload[2] <= 0){
		iFileAutoReload[2] = 1000;
	}
	::SetDlgItemInt(m_hWnd, IDC_EDIT_AUTORELOAD_TIME, iFileAutoReload[2], true);
	// 長押間隔
	int iKeyInterval[1] = {};
	//int iKeyIntervalFlag;
	param.GetPoints(CGrTxtFuncIParam::KeyInterval, iKeyInterval, sizeof(iKeyInterval)/sizeof(int));
	if(iKeyInterval[0] <= 0){
		iKeyInterval[0] = 1000;
	}
	::SetDlgItemInt(m_hWnd, IDC_EDIT_KEYINTERVAL, iKeyInterval[0], true);
	int iSpSelectionMode[2] = {};
	param.GetPoints(CGrTxtFuncIParam::SpSelectionMode, iSpSelectionMode, sizeof(iSpSelectionMode)/sizeof(int));
	// 非選択モードの時、シフトキー押下で一時的に選択モードに切り替える
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_SPSELECTIONMODE, iSpSelectionMode[0] == 1);
	// 非選択モードの時、シフトキー押下で一時的に選択モードに切り替える
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_CLICK_SPSELECTIONMODE, iSpSelectionMode[1] == 1);
	// 文字選択終了後、自動でクリップボードにコピーする
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTOCOPYMODE   , param.GetBoolean(CGrTxtFuncIParam::AutoCopyMode   ));
	// 自動でクリップボードにコピーするかわりに、以下のプログラム実行する
	::SetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_RUNEXECCOPYTEXT, param.GetBoolean(CGrTxtFuncIParam::RunExecCopyText));
	// プログラム名、引数
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_COPYTEXTEXE   , CGrTxtFuncIParam::CopyTextExe     );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_COPYTEXTPRM   , CGrTxtFuncIParam::CopyTextPrm     );
	// ファイルタイプ
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEHTML  , CGrTxtFuncIParam::FileTypeHtml    );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEARC7Z , CGrTxtFuncIParam::FileTypeArc7z   );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEARCCAB, CGrTxtFuncIParam::FileTypeArcCab  );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEARCLZH, CGrTxtFuncIParam::FileTypeArcLzh  );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEARCRAR, CGrTxtFuncIParam::FileTypeArcRar  );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_FILETYPEARCZIP, CGrTxtFuncIParam::FileTypeArcZip  );
	// 外部プログラム
	// プログラム名、引数
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEEXE   , CGrTxtFuncIParam::OpenFileExe     );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEPRM   , CGrTxtFuncIParam::OpenFilePrm     );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEEXE1  , CGrTxtFuncIParam::OpenFileExe1    );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEPRM1  , CGrTxtFuncIParam::OpenFilePrm1    );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEEXE2  , CGrTxtFuncIParam::OpenFileExe2    );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_OPENFILEPRM2  , CGrTxtFuncIParam::OpenFilePrm2    );
	std::map<std::tstring, std::tstring> menu_list;
	loadMenuName(menu_list);
	SetDlgItemText(hwnd, IDC_EDIT_OPENFILE_MENUNAME1, menu_list[L"ExecOpenFiile"].c_str());
	SetDlgItemText(hwnd, IDC_EDIT_OPENFILE_MENUNAME2, menu_list[L"ExecOpenFiile1"].c_str());
	SetDlgItemText(hwnd, IDC_EDIT_OPENFILE_MENUNAME3, menu_list[L"ExecOpenFiile2"].c_str());
	// リンク先を外部プログラム
	// プログラム名、引数
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_LINKEXE       , CGrTxtFuncIParam::OpenLinkExe     );
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_LINKPRM       , CGrTxtFuncIParam::OpenLinkPrm     );
	// 文字コード判定DLL
	::SetDlgItemTextType(hwnd, param, IDC_EDIT_DETECTENC     , CGrTxtFuncIParam::GuessEncodingDLL);
	// タッチ操作時、メニューの文字間隔を広くする
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_TOUCHMENU, param.GetBoolean(CGrTxtFuncIParam::TouchMenu));
	// キーリピート
	::SetCheckDlgItemID(hwnd, IDC_CHECKBOX_KEYREPAT , param.GetBoolean(CGrTxtFuncIParam::KeyRepeat));
	return TRUE;
}

void CGrPropPageOption::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDC_CHECKBOX_USEPREPARSER:
	case IDC_CHECKBOX_SHOWPREPARSERERROR:
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_BUTTON_SPIPLUGINFOLDER: BrowseForFolder(hwnd, IDC_EDIT_SPIPLUGINFOLDER, IDS_SPI_FOLDER      ); break;
	case IDC_CHECKBOX_SAVESIZE        : // ウインドウサイズの保存
	case IDC_CHECKBOX_AUTORELOAD      : // ファイルの更新時、自動的に再読み込み
	case IDC_CHECKBOX_AUTORELOADLOCK  : // ファイルの更新時、自動的に再読み込みを行う
	case IDC_CHECKBOX_SPSELECTIONMODE : // 非選択モードの時、シフトキー押下で一時的に選択モードに切り替える
	case IDC_CHECKBOX_CLICK_SPSELECTIONMODE: // 非選択モードの時、シフトキー押下で一時的に選択モードに切り替える
	case IDC_CHECKBOX_AUTOCOPYMODE    : // 文字選択終了後、自動でクリップボードにコピーする
	case IDC_CHECKBOX_RUNEXECCOPYTEXT : // 自動でクリップボードにコピーするかわりに、以下のプログラム実行する
	case IDC_CHECKBOX_TOUCHMENU       : // タッチ操作時、メニューの文字間隔を広くする
	case IDC_CHECKBOX_KEYREPAT        : // キーリピート
		PropSheet_Changed(::GetParent(hwnd), hwnd);
		break;
	case IDC_EDIT_TITLEFORMAT     :
	case IDC_EDIT_SPIPLUGINFOLDER : // SUSIEプラグインフォルダ
	case IDC_EDIT_COPYTEXTEXE     : //
	case IDC_EDIT_COPYTEXTPRM     : //
	case IDC_EDIT_OPENFILEEXE     : //
	case IDC_EDIT_OPENFILEPRM     : //
	case IDC_EDIT_OPENFILEEXE1    : //
	case IDC_EDIT_OPENFILEPRM1    : //
	case IDC_EDIT_OPENFILEEXE2    : //
	case IDC_EDIT_OPENFILEPRM2    : //
	case IDC_EDIT_LINKEXE         : //
	case IDC_EDIT_LINKPRM         : //
	case IDC_EDIT_FILETYPEHTML    : //
	case IDC_EDIT_FILETYPEARC7Z   : //
	case IDC_EDIT_FILETYPEARCCAB  : //
	case IDC_EDIT_FILETYPEARCLZH  : //
	case IDC_EDIT_FILETYPEARCRAR  : //
	case IDC_EDIT_FILETYPEARCZIP  : //
	case IDC_EDIT_DETECTENC       : // 文字コード判定DLL
	case IDC_EDIT_AUTORELOAD_TIME : // 監視間隔
	case IDC_EDIT_KEYINTERVAL     : // 長押間隔
	case IDC_EDIT_USEPREPARSERSIZE: // (n)MB以上のファイルはPre-Parserを使用しない
		if(codeNotify == EN_CHANGE){
			PropSheet_Changed(::GetParent(hwnd), hwnd);
		}
		break;
	case IDC_BUTTON_DETECTENC: // 文字コード判定DLL
		{
			CGrCurrentDirectory curdir;
			std::tstring str;
			TCHAR filter[2048] = {};
			CGrText::LoadString(IDS_DETECTENCFILE, filter, sizeof(filter)/sizeof(TCHAR));
			CGrText::LoadString(IDS_OPENFILE, str);

			OPENFILENAME of = {sizeof(OPENFILENAME)};
			TCHAR dlg_fileName[MAX_PATH] = {};
			TCHAR fileName[MAX_PATH] = {};
			GetDlgItemText(m_hWnd, IDC_EDIT_DETECTENC, dlg_fileName, sizeof(dlg_fileName)/sizeof(TCHAR));

			LPTSTR lpFilePart;
			::GetFullPathName(dlg_fileName, sizeof(fileName)/sizeof(TCHAR), fileName, &lpFilePart);

			of.hwndOwner       = m_hWnd;
			of.lpstrFilter     = filter;
			of.lpstrTitle      = str.c_str();
			of.nMaxCustFilter  = 40;
			of.lpstrFile       = fileName;
			of.nMaxFile        = MAX_PATH - 1;
			of.lpstrInitialDir = _T(".\\");
			of.Flags           = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
			if(GetOpenFileName(&of)){
				SetDlgItemText(m_hWnd, IDC_EDIT_DETECTENC, fileName);
			}
		}
		break;
	}
}

bool CGrPropPageOption::Apply()
{
	auto &&param = CGrTxtFunc::Param();
	::GetDlgItemPointSet(m_hWnd, l_txt_point_set, sizeof(l_txt_point_set)/sizeof(PointSet));
	// ウインドウサイズの保存
	param.SetBoolean(CGrTxtFuncIParam::SaveWindowSize, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_SAVESIZE) == BST_CHECKED);
	// タイトルバーの書式
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_TITLEFORMAT, CGrTxtFuncIParam::TitleFormat);
	// susieフォルダ
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_SPIPLUGINFOLDER, CGrTxtFuncIParam::SpiPluginFolder);
	//
	int iUsePreParserFlag;
	int iUsePreParser[3] = {};
	iUsePreParser[0] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_USEPREPARSER      ) == BST_CHECKED ? 1 : 0;
	iUsePreParser[1] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_SHOWPREPARSERERROR) == BST_CHECKED ? 1 : 0;
	iUsePreParser[2] = ::GetDlgItemInt(m_hWnd, IDC_EDIT_USEPREPARSERSIZE, &iUsePreParserFlag, true);
	param.SetPoints(CGrTxtFuncIParam::UsePreParser, iUsePreParser, sizeof(iUsePreParser)/sizeof(int));
	int iFileAutoReload[3] = {};
	// ファイルの更新時、自動的に再読み込みを行う
	iFileAutoReload[0] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTORELOAD    ) == BST_CHECKED ? 1 : 0;
	// 再読み込時、排他モードで開く
	iFileAutoReload[1] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTORELOADLOCK) == BST_CHECKED ? 1 : 0;
	// 監視間隔
	int iFileAutoReloadFlag;
	iFileAutoReload[2] = ::GetDlgItemInt(m_hWnd, IDC_EDIT_AUTORELOAD_TIME, &iFileAutoReloadFlag, true);
	if(iFileAutoReload[2] <= 0){
		iFileAutoReload[2] = 1000;
	}
	param.SetPoints(CGrTxtFuncIParam::FileAutoReload, iFileAutoReload, sizeof(iFileAutoReload)/sizeof(int));
	// 長押間隔
	int iKeyInterval[1];
	int iKeyIntervalFlag;
	iKeyInterval[0] = ::GetDlgItemInt(m_hWnd, IDC_EDIT_KEYINTERVAL, &iKeyIntervalFlag, true);
	param.SetPoints(CGrTxtFuncIParam::KeyInterval, iKeyInterval, sizeof(iKeyInterval)/sizeof(int));
	int iSpSelectionMode[2] = {};
	// 非選択モードの時、シフトキー押下で一時的に選択モードに切り替える
	iSpSelectionMode[0] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_SPSELECTIONMODE      ) == BST_CHECKED ? 1 : 0;
	// 非選択モードの時、シフトキー押下で一時的に選択モードに切り替える
	iSpSelectionMode[1] = ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_CLICK_SPSELECTIONMODE) == BST_CHECKED ? 1 : 0;
	param.SetPoints(CGrTxtFuncIParam::SpSelectionMode, iSpSelectionMode, sizeof(iSpSelectionMode)/sizeof(int));
	// 文字選択終了後、自動でクリップボードにコピーする
	param.SetBoolean(CGrTxtFuncIParam::AutoCopyMode, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_AUTOCOPYMODE) == BST_CHECKED);
	// 自動でクリップボードにコピーするかわりに、以下のプログラム実行する
	param.SetBoolean(CGrTxtFuncIParam::RunExecCopyText, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_RUNEXECCOPYTEXT) == BST_CHECKED);
	// プログラム名、引数
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_COPYTEXTEXE   , CGrTxtFuncIParam::CopyTextExe     );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_COPYTEXTPRM   , CGrTxtFuncIParam::CopyTextPrm     );
	// ファイルタイプ
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEHTML  , CGrTxtFuncIParam::FileTypeHtml    );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEARC7Z , CGrTxtFuncIParam::FileTypeArc7z   );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEARCCAB, CGrTxtFuncIParam::FileTypeArcCab  );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEARCLZH, CGrTxtFuncIParam::FileTypeArcLzh  );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEARCRAR, CGrTxtFuncIParam::FileTypeArcRar  );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_FILETYPEARCZIP, CGrTxtFuncIParam::FileTypeArcZip  );
	// 外部プログラム
	// プログラム名、引数
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEEXE   , CGrTxtFuncIParam::OpenFileExe     );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEPRM   , CGrTxtFuncIParam::OpenFilePrm     );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEEXE1  , CGrTxtFuncIParam::OpenFileExe1    );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEPRM1  , CGrTxtFuncIParam::OpenFilePrm1    );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEEXE2  , CGrTxtFuncIParam::OpenFileExe2    );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_OPENFILEPRM2  , CGrTxtFuncIParam::OpenFilePrm2    );
	std::map<std::tstring, std::tstring> menu_list;
	loadMenuName(menu_list);
	TCHAR buf[2048];
	GetDlgItemText(m_hWnd, IDC_EDIT_OPENFILE_MENUNAME1, buf, _countof(buf)); menu_list[L"ExecOpenFiile"] = buf;
	GetDlgItemText(m_hWnd, IDC_EDIT_OPENFILE_MENUNAME2, buf, _countof(buf)); menu_list[L"ExecOpenFiile1"] = buf;
	GetDlgItemText(m_hWnd, IDC_EDIT_OPENFILE_MENUNAME3, buf, _countof(buf)); menu_list[L"ExecOpenFiile2"] = buf;
	saveMenuName(menu_list);
	// リンク先を外部プログラム
	// プログラム名、引数
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_LINKEXE       , CGrTxtFuncIParam::OpenLinkExe     );
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_LINKPRM       , CGrTxtFuncIParam::OpenLinkPrm     );
	// 文字コード判定DLL
	::GetDlgItemTextType(m_hWnd, param, IDC_EDIT_DETECTENC     , CGrTxtFuncIParam::GuessEncodingDLL);
	// タッチ操作時、メニューの文字間隔を広くする
	param.SetBoolean(CGrTxtFuncIParam::TouchMenu, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_TOUCHMENU) == BST_CHECKED);
	// キーリピート
	param.SetBoolean(CGrTxtFuncIParam::KeyRepeat, ::GetCheckDlgItemID(m_hWnd, IDC_CHECKBOX_KEYREPAT ) == BST_CHECKED);
	//
	param.UpdateConfig(GetParent(GetParent(m_hWnd)));
	//
	return true;
}

