#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "TxtSearchFiles.h"
#include "TxtFunc.h"
#include "Text.h"
#include "CSVText.h"
#include "Image.h"
#include "PictRendererMgr.h"
#include "TxtFuncBookmarkDef.h"
#include "MessageBox.h"
#include "TxtMiruTheme.h"

LRESULT CGrComboBoxFile::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_DROPFILES:
		{
			TCHAR fileName[256];
			auto hDrop = (HDROP)wParam;
			::DragQueryFile(hDrop, 0, fileName, sizeof(fileName)/sizeof(TCHAR));
			::DragFinish(hDrop);
			SetWindowText(hWnd, fileName);
		}
		break;
	}
	return CGrComonCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

CGrTxtSearchFiles::CGrTxtSearchFiles(HWND hWnd) : CGrModelessWnd(hWnd)
{
}

CGrTxtSearchFiles::~CGrTxtSearchFiles()
{
}

void CGrTxtSearchFiles::Show(HWND hWnd)
{
	if(hWnd){
		m_parentWnd = hWnd;
	}
	if(!m_hWnd){
		::CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_FILESEARCH), m_parentWnd, (DLGPROC)CGrWinCtrl::WindowMapProc, (LPARAM)this);
	}
	if(m_hWnd){
		::ShowWindowAsync(m_hWnd, SW_SHOW);
		::BringWindowToTop(m_hWnd);
	}
}

LRESULT CGrTxtSearchFiles::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG   , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_COMMAND      , OnCommand      );
		HANDLE_MSG(hWnd, WM_NOTIFY       , OnNotify       );
		HANDLE_MSG(hWnd, WM_SIZE         , OnSize         );
		HANDLE_MSG(hWnd, WM_GETMINMAXINFO, OnGetMinMaxInfo);
		HANDLE_MSG(hWnd, WM_TIMER        , OnTimer        );
	case WM_DESTROY:
		{
			saveSetteing();
			PostQuitMessage(0);
		}
		break;
	case WM_SETTINGCHANGE:
		if (TxtMiruTheme_IsImmersiveColorSet(lParam)) {
			if (m_listview) {
				ListView_SetBkColor(m_listview, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
				ListView_SetTextColor(m_listview, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
				ListView_SetTextBkColor(m_listview, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
			}
			TxtMiruTheme_UpdateDarkMode(m_hWnd);
			InvalidateRect(m_hWnd, nullptr, TRUE);
		}
		break;
	case WM_SYSCOMMAND:
		if(wParam == SC_CLOSE){
			::ShowWindow(m_hWnd, SW_HIDE);
			::SendMessage(m_parentWnd, WM_COMMAND, TxtDocMessage::UPDATE_CMD_UI, 0);
			return true;
		}
		break;
	default:
		return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

static int CALLBACK listViewCompare(LPARAM lparam1, LPARAM lparam2, LPARAM lParamSort)
{
	return 0;
}

#include "Shell.h"

bool CGrTxtSearchFiles::loadSetteing()
{
	std::vector<SearchHistory> text_list;
	std::vector<SearchHistory> path_list;
	std::vector<SearchHistory> type_list;
	CGrDBSearchHistory db;
	bool bret = db.Open();
	if(bret){
		bret = db.Get(text_list, _T("sf_text"), 30);
		bret = db.Get(path_list, _T("sf_path"), 30);
		bret = db.Get(type_list, _T("sf_type"), 30);
	}
	db.Close();
	struct {
		std::vector<SearchHistory> &history_list;
		UINT id;
	} comblist[] = {
		{text_list, IDC_SEARCH_TEXT},
		{path_list, IDC_SEARCH_PATH},
		{type_list, IDC_SEARCH_FILE},
	};
	for(const auto &comb : comblist){
		auto hWnd = GetDlgItem(m_hWnd, comb.id);
		ComboBox_ResetContent(hWnd);
		for(const auto &item : comb.history_list){
			ComboBox_AddString(hWnd, item.value.c_str());
		}
		ComboBox_SetCurSel(hWnd, 0);
	}
	return true;
}

bool CGrTxtSearchFiles::saveSetteing()
{
	return true;
}

BOOL CGrTxtSearchFiles::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	auto hInst = CGrTxtFunc::GetDllModuleHandle();
	SetWindowPosCenter();
	SendMessage(hwnd, WM_SETICON, ICON_BIG  , reinterpret_cast<LPARAM>(LoadIcon (CGrTxtFunc::GetDllModuleHandle(),MAKEINTRESOURCE(IDI_APP))));
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(LoadImage(CGrTxtFunc::GetDllModuleHandle(),MAKEINTRESOURCE(IDI_APP),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR)));
	RECT rc_dialog = {};
	::GetWindowRect(hwnd, &rc_dialog);
	m_minWindowSize.x = rc_dialog.right - rc_dialog.left;
	m_minWindowSize.y = rc_dialog.bottom - rc_dialog.top;
	{
		/* ツールバーボタンにビットマップを貼り付け */
		// イメージリストの作成
		LOGFONT logfont = {0};
		GetObject(GetWindowFont(m_hWnd), sizeof(logfont), &logfont);
		{
			m_titleFont.CreateFontIndirect(logfont);
			m_pageFont.CreateFontIndirect(logfont);
		}
		logfont.lfHeight *= 2;
		CGrFont font;
		font.CreateFontIndirect(logfont);
		int height = font.Height();
		/*HIMAGELIST*/m_hImg = ImageList_Create(height, height, ILC_COLOR32, 1, 1);
		//
		const int icon_num = 7;
		// 0:IDADDBOOKMARK
		// 1:IDDELBOOKMARK
		// 2:IDGOTOPAGE
		// 3:IDLINKSTAY
		// 4:IDBOOKMARK
		// 5:IDSUBTITLE
		// 6:IDSUBTITLE Link
		CGrPictRendererMgr pictMgr;
		pictMgr.Initialize();
		auto lpDir = CGrTxtFunc::GetDataPath();
		TCHAR curPath[MAX_PATH];
		CGrShell::GetExePath(curPath);
		RECT rc = {0,0,height*icon_num,height};
		CGrBitmap bmp;
		bmp.Create(height, height);
		for(int i=0; i<icon_num; ++i){
			TCHAR path[_MAX_PATH] = {};
			_stprintf_s(path, _T("%s\\toolbar_icon\\toolbar_btn%d"), lpDir, i);
			std::tstring image_filename;
			if(pictMgr.GetSupportedFile(image_filename, path)){
				pictMgr.Draw(bmp, i, 0, height, height, image_filename.c_str());
			} else {
				_stprintf_s(path, _T("%s\\toolbar_icon\\toolbar_btn%d"), curPath, i);
				if(pictMgr.GetSupportedFile(image_filename, path)){
					pictMgr.Draw(bmp, i, 0, height, height, image_filename.c_str());
				}
			}
			ImageList_Add(m_hImg, bmp, NULL);
		}
	}
	//
	setState(false);
	const auto &param = CGrTxtFunc::Param();
	//
	RECT client_rect;
	::GetClientRect(hwnd, &client_rect);

	std::tstring text;
	auto hListWnd = GetDlgItem(m_hWnd, IDC_LINK_LIST);
	m_listview.Attach(hListWnd);
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;

	CGrText::LoadString(IDS_BMH_URL, text);
	lvc.pszText = const_cast<LPTSTR>(text.c_str());
	lvc.iSubItem = 0;
	lvc.cx = 100;
	m_listview.InsertColumn(0, &lvc);

	CGrText::LoadString(IDS_BMH_TITLE, text);
	lvc.pszText = const_cast<LPTSTR>(text.c_str());
	lvc.iSubItem = 1;
	lvc.cx = 100;
	m_listview.InsertColumn(1, &lvc);

	CGrText::LoadString(IDS_BMH_AUTHOR, text);
	lvc.pszText = const_cast<LPTSTR>(text.c_str());
	lvc.iSubItem = 2;
	lvc.cx = 60;
	m_listview.InsertColumn(2, &lvc);

	CGrText::LoadString(IDS_BMH_TOTALPAGE, text);
	lvc.pszText = const_cast<LPTSTR>(text.c_str());
	lvc.iSubItem = 3;
	lvc.cx = 40;
	m_listview.InsertColumn(3, &lvc);

	CGrText::LoadString(IDS_BMH_SEARCH_RESULT, text);
	lvc.pszText = const_cast<LPTSTR>(text.c_str());
	lvc.iSubItem = 4;
	lvc.cx = 300;
	m_listview.InsertColumn(4, &lvc);

	UINT styleex = ListView_GetExtendedListViewStyle(hListWnd);
	styleex |= LVS_EX_FLATSB | LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hListWnd, styleex);
	TxtMiruTheme_SetWindowTheme(m_listview, L"Explorer", NULL);
	ListView_SetBkColor(m_listview, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
	ListView_SetTextColor(m_listview, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
	ListView_SetTextBkColor(m_listview, TxtMiruTheme_GetSysColor(COLOR_WINDOW));

	TCHAR buf[2048];
	if(!loadSetteing()){
		std::tstring type = _T("TXT");
		param.GetText(CGrTxtFuncIParam::FileTypeHtml  , buf, sizeof(buf)/sizeof(TCHAR));
		if(lstrlen(buf) > 0){
			type += _T(",");
			type += buf;
		}
		param.GetText(CGrTxtFuncIParam::FileTypeArc7z , buf, sizeof(buf)/sizeof(TCHAR));
		if(lstrlen(buf) > 0){
			type += _T(",");
			type += buf;
		}
		param.GetText(CGrTxtFuncIParam::FileTypeArcCab, buf, sizeof(buf)/sizeof(TCHAR));
		if(lstrlen(buf) > 0){
			type += _T(",");
			type += buf;
		}
		param.GetText(CGrTxtFuncIParam::FileTypeArcLzh, buf, sizeof(buf)/sizeof(TCHAR));
		if(lstrlen(buf) > 0){
			type += _T(",");
			type += buf;
		}
		param.GetText(CGrTxtFuncIParam::FileTypeArcRar, buf, sizeof(buf)/sizeof(TCHAR));
		if(lstrlen(buf) > 0){
			type += _T(",");
			type += buf;
		}
		param.GetText(CGrTxtFuncIParam::FileTypeArcZip, buf, sizeof(buf)/sizeof(TCHAR));
		if(lstrlen(buf) > 0){
			type += _T(",");
			type += buf;
		}
		SetDlgItemText(m_hWnd, IDC_SEARCH_FILE, type.c_str());
	}

	GetCurrentDirectory(sizeof(buf)/sizeof(TCHAR), buf);
	SetDlgItemText(m_hWnd, IDC_SEARCH_PATH, buf);

	m_path.Attach(GetDlgItem(m_hWnd, IDC_SEARCH_PATH));
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);
	TxtMiruTheme_UpdateDarkMode(m_hWnd);

	setWindowSize(client_rect.right, client_rect.bottom);

	return TRUE;
}

bool CGrTxtSearchFiles::addSearchFile(LPCTSTR lpFilePath)
{
	WIN32_FIND_DATA wfd;
	if(lpFilePath){
		m_searchPath = lpFilePath;
		std::tstring find_path;
		CGrText::FormatMessage(find_path, _T("%1!s!*.*"), m_searchPath.c_str());
		m_hFindFile = ::FindFirstFile(find_path.c_str(), &wfd);
	}
	if(m_hFindFile == INVALID_HANDLE_VALUE){
		return false;
	}
	while(lpFilePath || ::FindNextFile(m_hFindFile, &wfd)){
		if(m_curSearchItem < 0){
			break;
		}
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			;
		} else if(std::regex_search(wfd.cFileName, m_regexFileCond)){
			std::tstring next_path;
			CGrText::FormatMessage(next_path, _T("%1!s!%2!s!"), m_searchPath.c_str(), wfd.cFileName);
			m_filname_list.push_back(next_path);
			return true;
		}
		lpFilePath = nullptr;
	}
	CloseHandle(m_hFindFile);
	m_hFindFile = INVALID_HANDLE_VALUE;
	return false;
}

void CGrTxtSearchFiles::setState(bool bStart)
{
	if(bStart){
		ShowWindow(GetDlgItem(m_hWnd, IDC_ABORT), SW_SHOW);
		ShowWindow(GetDlgItem(m_hWnd, IDOK), SW_HIDE);
	} else {
		ShowWindow(GetDlgItem(m_hWnd, IDC_ABORT), SW_HIDE);
		ShowWindow(GetDlgItem(m_hWnd, IDOK), SW_SHOW);
		std::tstring text;
		CGrText::LoadString(IDS_FILESEARCH_TITLE, text);
		SetWindowText(m_hWnd, text.c_str());
	}
}

void CGrTxtSearchFiles::endDialog(UINT id)
{
	switch(id){
	case IDCANCEL:
		{
			DestroyWindow(m_hWnd);
		}
		break;
	case IDOK:
		{
			setState();
			m_filname_list.clear();
			m_curSearchItem = 0;
			m_search_list.clear();
			m_listview.DeleteAllItems();
			//
			TCHAR path[2048];
			GetDlgItemText(m_hWnd, IDC_SEARCH_PATH, path, sizeof(path)/sizeof(TCHAR));
			TCHAR type[2048];
			GetDlgItemText(m_hWnd, IDC_SEARCH_FILE, type, sizeof(type)/sizeof(TCHAR));
			TCHAR text[2048];
			GetDlgItemText(m_hWnd, IDC_SEARCH_TEXT, text, sizeof(text)/sizeof(TCHAR));
			CGrDBSearchHistory db;
			bool bret = db.Open();
			if(bret){
				bret = db.Put(_T("sf_text"), text);
				bret = db.Put(_T("sf_path"), path);
				bret = db.Put(_T("sf_type"), type);
			}
			db.Close();
			//
			auto add = [&](UINT id, LPCTSTR data)
			{
				auto hWnd = GetDlgItem(m_hWnd, id);
				int idx = ComboBox_FindStringExact(hWnd, 0, data);
				if(idx != CB_ERR){
					ComboBox_DeleteString(hWnd, idx);
				}
				ComboBox_InsertString(hWnd, 0, data);
				ComboBox_SetText(hWnd, data);
			};
			add(IDC_SEARCH_PATH, path);
			add(IDC_SEARCH_FILE, type);
			add(IDC_SEARCH_TEXT, text);
			//
			bool bUseRegExp = Button_GetCheck(GetDlgItem(m_hWnd, IDC_USE_REGEXP)) == TRUE;
			std::tstring filename;
			CGrShell::ToPrettyFileName(path, filename);
			if(CGrShell::IsURI(filename.c_str())){
				m_filname_list.push_back(filename);
			} else {
				std::map<std::tstring,int> ext_map;
				auto &&param = CGrTxtFunc::Param();
				CGrCSVText text;
				text.AddTail(type);
				for(const auto &row : text.GetCSVROW()){
					for(const auto &item : row){
						std::tstring str(item);
						std::replace(str, L".", L"\\.");
						std::replace(str, L"*", L".*");
						std::replace(str, L"?", L".");
						ext_map[str] = 1;
					}
				}
				std::tstring str_filecond;
				for(const auto &item : ext_map){
					if(str_filecond.empty()){
						str_filecond = _T("\\.(");
					} else {
						str_filecond += _T("|");
					}
					str_filecond += item.first;
				}
				str_filecond += _T(")$");
				try {
					m_regexFileCond.assign(str_filecond.c_str(), std::regex_constants::icase);
				}
				catch (...) {

				}
				WIN32_FIND_DATA wfd;
				CGrShell::getFileInfo(filename.c_str(), wfd);
				if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
					CGrShell::AddBackSlash(path);
					if(m_hFindFile != INVALID_HANDLE_VALUE){
						::FindClose(m_hFindFile);
					}
					if(!addSearchFile(path)){
						break;
					}
				} else {
					m_filname_list.push_back(filename);
				}
			}
			m_cond = text;
			m_bUseRegExp = bUseRegExp;
			m_curSearchresult.url = _T("");
			m_curSearchresult.title = _T("");
			m_curSearchresult.author = _T("");
			if(Search()){
				;
			}
		}
		break;
	}
}

#define ID_CHK_NEXT (10)
void CGrTxtSearchFiles::OnTimer(HWND hwnd, UINT id)
{
	switch(id){
	case ID_CHK_NEXT:
		{
			KillTimer(m_hWnd, ID_CHK_NEXT);
			if(m_curSearchItem < 0){
				break;
			}
			if(!Search()){
				;
			}
		}
		break;
	}
}


void CGrTxtSearchFiles::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDC_ABORT:
		Abort();
		setState(false);
		break;
	case IDOPEN:
		{
			int iItem = m_listview.GetNextItem(-1, LVNI_SELECTED);
			int index = m_listview.GetParam(iItem);
			CGrTxtFunc::OpenFile(m_parentWnd, m_search_list[index].url.c_str(), m_search_list[index].page);
		}
		break;
	case ID_CHK_NEXT:
		{
			KillTimer(m_hWnd, ID_CHK_NEXT);
			if(m_curSearchItem < 0){
				setState(false);
				break;
			}
			if(m_hFindFile == INVALID_HANDLE_VALUE && m_curSearchItem == 0){
				{
					TCHAR buf[2048];
					std::map<std::tstring,int> ext_map;
					auto &&param = CGrTxtFunc::Param();
					CGrCSVText text;
					param.GetText(CGrTxtFuncIParam::FileTypeHtml, buf, sizeof(buf)/sizeof(TCHAR)); text.AddTail(buf);
					for(const auto &row : text.GetCSVROW()){
						for(const auto &item : row){
							ext_map[item] = 1;
						}
					}
					std::tstring str_filecond;
					for(const auto &item : ext_map){
						if(str_filecond.empty()){
							str_filecond = _T("\\.(");
						} else {
							str_filecond += _T("|");
						}
						str_filecond += item.first;
					}
					str_filecond += _T(")$");
					m_regexFileCond.assign(str_filecond.c_str(), std::regex_constants::icase);
				}
				if(!m_filname_list.empty()){
					auto filename = m_filname_list[0].c_str();
					if(CGrShell::IsURI(filename) || std::regex_search(filename, m_regexFileCond)){
						bool bRet = true;
						sqlite3_stmt *pstmt_txtmiru_st_link = nullptr;
						CGrBookmarkDB db;
						do {
							if(!db.Open()){
								bRet = false;
								break;
							}
							auto *pSqlite3 = db.GetSqlite3();
							if(!pSqlite3){
								bRet = false;
								break;
							}
							int ret = sqlite3_prepare16(pSqlite3, _T("SELECT S.URL")
														_T(" FROM TXTMIRU_PLACES P INNER JOIN TXTMIRU_SUBTITLES S ON S.PLACE_ID=P.ID ")
														_T(" WHERE P.URL=@URL AND S.URL!='' ORDER BY S.B_ORDER"), -1, &pstmt_txtmiru_st_link, nullptr);
							if(ret != SQLITE_OK || !pstmt_txtmiru_st_link) {
								// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
								bRet = false;
								break;
							}
							sqlite3_clear_bindings(pstmt_txtmiru_st_link);
							CGrDBFunc::PutValue(pstmt_txtmiru_st_link, "@URL", filename);
							// 実行
							bRet = false;
							while(true){
								ret = sqlite3_step(pstmt_txtmiru_st_link);
								if(ret != SQLITE_ROW){
									break;
								}
								std::tstring url;
								CGrDBFunc::GetValue(pstmt_txtmiru_st_link, 0, url);
								m_filname_list.push_back(url);
							}
						} while(0);
					}
				} /*if(!m_filname_list.empty()*/
			}
			++m_curSearchItem;
			if(m_curSearchItem >= static_cast<signed int>(m_filname_list.size())){
				if(!addSearchFile()){
					setState(false);
					break;
				}
			}
			if(static_cast<signed int>(m_filname_list.size()) > m_curSearchItem){
				std::wregex re(LR"(^(:?http|ftp)s*:\/)", std::regex_constants::icase);
				if(std::regex_search(m_filname_list[m_curSearchItem].c_str(), re)){
					SetTimer(m_hWnd, ID_CHK_NEXT, 5000, nullptr);
				} else {
					SetTimer(m_hWnd, ID_CHK_NEXT, 1, nullptr);
				}
			} else {
				SetTimer(m_hWnd, ID_CHK_NEXT, 1, nullptr);
			}
		}
		break;
	case IDOK:
	case IDCANCEL:
		endDialog(id);
		break;
	}
}

LRESULT CGrTxtSearchFiles::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	if(lpnmhdr->hwndFrom == m_listview){
		switch(lpnmhdr->code){
		case NM_DBLCLK:
			FORWARD_WM_COMMAND(m_hWnd, IDOPEN, 0, 0, PostMessage);
			break;
		}
	}
	return TRUE;
}

#define PADDING 10
void CGrTxtSearchFiles::setWindowSize(int cx, int cy)
{
	if(cx <= 0 || cy <= 0){
		return;
	}
	RECT client_rect = {0,0,cx,cy};
	auto hListWnd = GetDlgItem(m_hWnd, IDC_LINK_LIST);
	auto hBtnWnd = GetDlgItem(m_hWnd, IDOPEN);
	RECT rect;
	GetWindowRect(hListWnd, &rect);
	::ScreenToClient(m_hWnd, reinterpret_cast<LPPOINT>(&rect));
	client_rect.top = rect.top;
	client_rect.left += PADDING;
	client_rect.right -= PADDING;
	client_rect.bottom -= PADDING;
	//
	GetClientRect(hBtnWnd, &rect);
	SetWindowPos(hBtnWnd, NULL, client_rect.left, client_rect.bottom - rect.bottom, 0, 0, SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING|SWP_NOSIZE|SWP_NOZORDER);
	hBtnWnd = GetDlgItem(m_hWnd, IDCANCEL);
	GetClientRect(hBtnWnd, &rect);
	SetWindowPos(hBtnWnd, NULL, client_rect.right - rect.right, client_rect.bottom - rect.bottom, 0, 0, SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING|SWP_NOSIZE|SWP_NOZORDER);
	client_rect.bottom -= rect.bottom + PADDING;
	//
	SetWindowPosAllClient(hListWnd, &client_rect);
}

void CGrTxtSearchFiles::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
		return;
	}
	if(!GetDlgItem(m_hWnd, IDC_LINK_LIST)){
		return;
	}
	setWindowSize(cx, cy);
}

void CGrTxtSearchFiles::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	lpMinMaxInfo->ptMinTrackSize = m_minWindowSize;
}

void CGrTxtSearchFiles::AddResult(int page, LPCTSTR lpSrc)
{
	do {
		if(m_curSearchItem < 0 && m_curSearchItem >= (signed int)m_filname_list.size()){
			return;
		}
		const auto &filename = m_filname_list[m_curSearchItem];
		std::tstring url;
		CGrShell::ToPrettyFileName(filename.c_str(), url);
		if(m_curSearchresult.url == url){
			break;
		}
		m_curSearchresult.url = filename;
		m_curSearchresult.title = _T("");
		m_curSearchresult.author = _T("");
		CGrBookmarkDB db;
		if(!db.Open()){
			break;
		}
		Place place;
		if(!db.GetPlace(place, url.c_str())){
			break;
		}
		m_curSearchresult.title = place.title;
		m_curSearchresult.author = place.author;
	} while(0);
	//
	m_curSearchresult.page = page;
	m_curSearchresult.text = lpSrc;
	m_search_list.push_back(m_curSearchresult);

	LVITEM item = {0};
	item.mask     = LVIF_PARAM;
	item.iItem    = m_listview.GetItemCount();
	item.lParam   = m_search_list.size()-1;
	m_listview.InsertItem(&item);

	item.mask     = LVIF_TEXT;
	item.pszText  = const_cast<LPTSTR>(m_curSearchresult.url.c_str());
	item.iSubItem = 0;
	m_listview.SetItem(&item);

	item.pszText  = const_cast<LPTSTR>(m_curSearchresult.title.c_str());
	item.iSubItem = 1;
	m_listview.SetItem(&item);

	item.pszText  = const_cast<LPTSTR>(m_curSearchresult.author.c_str());
	item.iSubItem = 2;
	m_listview.SetItem(&item);

	std::tstring text;
	TCHAR strPage[512];
	if(page == 0){
		CGrText::LoadString(IDS_TOPPAGE, text);
		item.pszText  = const_cast<LPTSTR>(text.c_str());
	} else {
		_stprintf_s(strPage, _T("%d"), page);
		item.pszText  = strPage;
	}
	item.iSubItem = 3;
	m_listview.SetItem(&item);

	item.pszText  = const_cast<LPTSTR>(lpSrc);
	item.iSubItem = 4;
	m_listview.SetItem(&item);
}

BOOL CGrTxtSearchFiles::IsDialogMessage(HWND hwnd, MSG &msg)
{
	if(::IsDialogMessage(m_hWnd, &msg)){
		return TRUE;
	}
	return FALSE;
}

using CGrDocSearchProcCallbak = void (__stdcall *)(int page, LPCTSTR lpSrc, LPARAM lParam);
using TxtMiruFunc_Search = HANDLE (cdecl *)(HWND hWnd, LPCTSTR lpFilename, LPCTSTR lpSrc, bool bUseRegExp, CGrDocSearchProcCallbak func, LPARAM lParam, unsigned int *threadid);
void __stdcall CGrTxtSearchFiles::CGrDocOpenProcCallbak(int page, LPCTSTR lpSrc, LPARAM lParam)
{
	auto *pDlg = reinterpret_cast<CGrTxtSearchFiles*>(lParam);
	if(pDlg){
		if(lpSrc){
			pDlg->AddResult(page, lpSrc);
		} else {
			if(pDlg->m_hBackgroundPrc){
				CloseHandle(pDlg->m_hBackgroundPrc);
				pDlg->m_hBackgroundPrc = NULL;
				pDlg->m_BackgroundThreadID = 0;
			}
			FORWARD_WM_COMMAND(pDlg->GetWnd(), ID_CHK_NEXT, 0, 0, PostMessage);
		}
	}
}

bool CGrTxtSearchFiles::Search()
{
	if(m_hBackgroundPrc){
		return false;
	}
	if(m_curSearchItem >= static_cast<signed int>(m_filname_list.size())){
		return false;
	}
	const auto &filename = m_filname_list[m_curSearchItem];

	std::tstring text;
	CGrText::FormatMessage(CGrTxtFunc::GetDllModuleHandle(), text, IDS_FILESEARCH_RUNNING, filename.c_str());
	SetWindowText(m_hWnd, text.c_str());

	auto func = reinterpret_cast<TxtMiruFunc_Search>(GetProcAddress(GetModuleHandle(NULL), "TxtMiruFunc_Search"));
	if(!func){
		return false;
	}
	auto h = func(m_hWnd, filename.c_str(), m_cond.c_str(), m_bUseRegExp, CGrDocOpenProcCallbak, reinterpret_cast<LPARAM>(this), &m_BackgroundThreadID);
	if(!m_hBackgroundPrc){
		return false;
	}
	m_hBackgroundPrc = h;
	return true;
}

bool CGrTxtSearchFiles::Abort()
{
	m_curSearchItem = -1;
	m_filname_list.clear();
	if(m_hBackgroundPrc){
		PostThreadMessage(m_BackgroundThreadID, WM_DESTROY, 0, 0);
	}
	return true;
}

