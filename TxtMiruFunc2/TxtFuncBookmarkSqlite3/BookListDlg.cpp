#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <DelayImp.h>
#include <time.h>
#include "resource.h"
#include "BookListDlg.h"
#include "Text.h"
#include "TxtFunc.h"
#include "TxtFuncIParam.h"
#include "Image.h"
#include "PictRendererMgr.h"
#include "Shell.h"
#include "Font.h"
#include "csvtext.h"
#include "TxtFunc.h"
#include "TxtMiruDef.h"
#include "TxtMiruTheme.h"

CGrBookListDlg::CGrBookListDlg(HWND hWnd, LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark)
: m_filename(lpFileName), m_pBookmark(pBookmark), CGrModelessWnd(hWnd),
  m_tagWnd(*this), m_bookWnd(*this), m_subtitleWnd(*this)
{
}

CGrBookListDlg::~CGrBookListDlg()
{
}

void CGrBookListDlg::Show(HWND hWnd)
{
	if(hWnd){
		m_parentWnd = hWnd;
	}
	if(m_hWnd){
		PostRefresh();
	} else {
		auto hParent = m_parentWnd;
		auto &&param = CGrTxtFunc::Param();
		std::tstring ini_filename;
		CGrTxtFunc::GetBookmarkFolder(&param, ini_filename);
		ini_filename += _T("/bookmark.ini");
		TCHAR val[2048];
		::GetPrivateProfileString(_T("Bookmark"), _T("WindowParent"), _T(""), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
		if(_tcsicmp(val, _T("Top")) == 0){
			hParent = NULL;
		}
		::CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_BOOKLIST), hParent, reinterpret_cast<DLGPROC>(CGrModelessWnd::WindowMapProc), reinterpret_cast<LPARAM>(this));
	}
	if(m_hWnd){
		::ShowWindowAsync(m_hWnd, SW_SHOW);
		::BringWindowToTop(m_hWnd);
	}
}

LRESULT CGrBookListDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG   , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_TIMER        , OnTimer        );
		HANDLE_MSG(hWnd, WM_COMMAND      , OnCommand      );
		HANDLE_MSG(hWnd, WM_NOTIFY       , OnNotify       );
		HANDLE_MSG(hWnd, WM_SIZE         , OnSize         );
		HANDLE_MSG(hWnd, WM_GETMINMAXINFO, OnGetMinMaxInfo);
	case WM_DESTROY:
		{
			ClearUpdateList();
			auto &&param = CGrTxtFunc::Param();
			std::tstring ini_filename;
			CGrTxtFunc::GetBookmarkFolder(&param, ini_filename);
			ini_filename += _T("/bookmark.ini");
			TCHAR val[2048];
			WINDOWPLACEMENT wndp = {sizeof(WINDOWPLACEMENT)};
			GetWindowPlacement(m_hWnd, &wndp);
			_stprintf_s(val, _T("%d,%d,%d,%d,%d,%d,%d"),
						wndp.rcNormalPosition.left, wndp.rcNormalPosition.top, wndp.rcNormalPosition.right, wndp.rcNormalPosition.bottom,
						m_bBookmark ? 1 : 0,
						m_bSubtitle ? 1 : 0,
						m_bStayOn   ? 1 : 0
						);
			::WritePrivateProfileString(_T("Bookmark"), _T("WindowPos"), val, ini_filename.c_str());
			_stprintf_s(val, _T("%d"), m_splitRWnd.GetPosition());
			::WritePrivateProfileString(_T("Bookmark"), _T("TagBookSplitPos"), val, ini_filename.c_str());
			_stprintf_s(val, _T("%d"), m_splitLWnd.GetPosition());
			::WritePrivateProfileString(_T("Bookmark"), _T("BookSubtitleSplitPos"), val, ini_filename.c_str());
			_stprintf_s(val, _T("%d"), m_subtitleWnd.GetSplitPos());
			::WritePrivateProfileString(_T("Bookmark"), _T("ImageSplitPos"), val, ini_filename.c_str());
			{
				//m_tagWnd
				std::tstring str;
				int count = m_tagWnd.GetColumnCount();
				for(int i=0; i<count; ++i){
					_stprintf_s(val, _T("%d,"), m_tagWnd.GetColumnWidth(i));
					str += val;
				}
				::WritePrivateProfileString(_T("Bookmark"), _T("TagColumnWidth"), str.c_str(), ini_filename.c_str());
			}
			{
				//m_bookWnd
				std::tstring str;
				int count = m_bookWnd.GetColumnCount();
				for(int i=0; i<count; ++i){
					_stprintf_s(val, _T("%d,"), m_bookWnd.GetColumnWidth(i));
					str += val;
				}
				::WritePrivateProfileString(_T("Bookmark"), _T("BookColumnWidth"), str.c_str(), ini_filename.c_str());
			}
			{
				//m_subtitleWnd
				std::tstring str;
				int count = m_subtitleWnd.GetColumnCount();
				for(int i=0; i<count; ++i){
					_stprintf_s(val, _T("%d,"), m_subtitleWnd.GetColumnWidth(i));
					str += val;
				}
				::WritePrivateProfileString(_T("Bookmark"), _T("SubtitleColumnWidth"), str.c_str(), ini_filename.c_str());
			}
			{

			}
			_stprintf_s(val, _T("%d"), m_bookWnd.GetSelectID());
			::WritePrivateProfileString(_T("Bookmark"), _T("LatestTagID"), val, ini_filename.c_str());
			_stprintf_s(val, _T("%d"), m_subtitleWnd.GetSelectID());
			::WritePrivateProfileString(_T("Bookmark"), _T("LatestBookID"), val, ini_filename.c_str());
			PostQuitMessage(0);
		}
		break;
	case WM_SYSCOMMAND:
		if(wParam == SC_CLOSE){
			::ShowWindow(m_hWnd, SW_HIDE);
			::SendMessage(m_parentWnd, WM_COMMAND, TxtDocMessage::UPDATE_CMD_UI, 0);
			return true;
		}
		break;
	case WM_SETTINGCHANGE:
		if (TxtMiruTheme_IsImmersiveColorSet(lParam)) {
			if (m_tagWnd) {
				::SendMessage(m_tagWnd, uMsg, wParam, lParam);
			}
			if (m_bookWnd) {
				::SendMessage(m_bookWnd, uMsg, wParam, lParam);
			}
			if (m_subtitleWnd) {
				::SendMessage(m_subtitleWnd, uMsg, wParam, lParam);
			}
			TxtMiruTheme_UpdateDarkMode(m_hWnd);
			InvalidateRect(m_hWnd, nullptr, TRUE);
		}
		break;
	default:
		return CGrModelessWnd::WndProc(hWnd, uMsg, wParam, lParam);
	}
	return CGrModelessWnd::WndProc(hWnd, uMsg, wParam, lParam);
}

static LPCTSTR l_treeicon_name_list[static_cast<int>(TxtFuncBookmark::ImageTreeIcon::iti_max)] = {
	_T("stat_tree_collapsed"         ),
	_T("stat_tree_expanded"          ),
	_T("stat_tree_none"              ),
};
static LPCTSTR l_icon_name_list[static_cast<int>(TxtFuncBookmark::ImageIcon::ii_max)] = {
	_T("stat_tag_favorite"           ),
	_T("stat_book_normal"            ),
	_T("stat_book_normal_update"     ),
	_T("stat_book_current"           ),
	_T("stat_book_current_update"    ),
	_T("stat_book_check"             ),
	_T("stat_bookmark_normal"        ),
	_T("stat_bookmark_normal_update" ),
	_T("stat_bookmark_current"       ),
	_T("stat_bookmark_current_update"),
	_T("stat_link_normal"            ),
	_T("stat_link_normal_update"     ),
	_T("stat_link_current"           ),
	_T("stat_link_current_update"    ),
	_T("tag_add"                     ),
	_T("tag_modify"                  ),
	_T("tag_delete"                  ),
	_T("tag_up"                      ),
	_T("tag_down"                    ),
	_T("book_add"                    ),
	_T("book_modify"                 ),
	_T("book_delete"                 ),
	_T("book_update"                 ),
	_T("book_update_all"             ),
	_T("book_up"                     ),
	_T("book_down"                   ),
	_T("book_open"                   ),
	_T("book_abort"                  ),
	_T("subtitle_open"               ),
	_T("stay_on_top"                 ),
	_T("bookmark"                    ),
};

#include "TxtMiruFunc.h"

BOOL CGrBookListDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowPosCenter();
	auto &&param = CGrTxtFunc::Param();
	//
	SendMessage(hwnd, WM_SETICON, ICON_BIG  , reinterpret_cast<LPARAM>(LoadIcon (CGrTxtFunc::GetDllModuleHandle(),MAKEINTRESOURCE(IDI_APP))));
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(LoadImage(CGrTxtFunc::GetDllModuleHandle(),MAKEINTRESOURCE(IDI_APP),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR)));
	{
		LOGFONT logfont = {};
		GetObject(GetWindowFont(m_hWnd), sizeof(logfont), &logfont);
		logfont.lfHeight *= 2;
		CGrFont font;
		font.CreateFontIndirect(logfont);
		int height = font.Height();
		/*HIMAGELIST*/m_hImg     = ImageList_Create(height  , height, ILC_COLOR32, 1, 1);
		/*HIMAGELIST*/m_hTreeImg = ImageList_Create(height/2, height, ILC_COLOR32, 1, 1);
		//
		CGrBitmap bmp_list;
		CGrBitmap bmp_tree;
		bmp_list.Create(height*static_cast<int>(TxtFuncBookmark::ImageIcon::ii_max), height);
		bmp_tree.Create(height*static_cast<int>(TxtFuncBookmark::ImageTreeIcon::iti_max) / 2, height);
		{
			{
				auto color = GetSysColor(COLOR_WINDOW);
				int r = GetRValue(color);
				int g = GetGValue(color);
				int b = GetBValue(color);
				{
					int l = height*static_cast<int>(TxtFuncBookmark::ImageIcon::ii_max) * height;
					auto lpRGB = bmp_list.GetBits();
					for(; l>0; --l, ++lpRGB){
						lpRGB->rgbRed = r;
						lpRGB->rgbGreen = g;
						lpRGB->rgbBlue = b;
						lpRGB->rgbReserved = 0;
					}
				}
				{
					int l = height*static_cast<int>(TxtFuncBookmark::ImageTreeIcon::iti_max) / 2 * height;
					auto lpRGB = bmp_tree.GetBits();
					for(; l>0; --l, ++lpRGB){
						lpRGB->rgbRed = r;
						lpRGB->rgbGreen = g;
						lpRGB->rgbBlue = b;
						lpRGB->rgbReserved = 0;
					}
				}
			}
			CGrPictRendererMgr pictMgr;
			pictMgr.Initialize();
			auto lpDir = CGrTxtFunc::GetDataPath();
			TCHAR curPath[MAX_PATH];
			CGrShell::GetExePath(curPath);
			for(int i=0; i< static_cast<int>(TxtFuncBookmark::ImageIcon::ii_max); ++i){
				TCHAR path[_MAX_PATH] = {};
				_stprintf_s(path, _T("%s\\toolbar_icon\\list_icon_%s"), lpDir, l_icon_name_list[i]);
				std::tstring image_filename;
				if(pictMgr.GetSupportedFile(image_filename, path)){
					pictMgr.Draw(bmp_list, i*height, 0, height, height, image_filename.c_str());
				} else {
					_stprintf_s(path, _T("%s\\toolbar_icon\\list_icon_%s"), curPath, l_icon_name_list[i]);
					if(pictMgr.GetSupportedFile(image_filename, path)){
						pictMgr.Draw(bmp_list, i*height, 0, height, height, image_filename.c_str());
					}
				}
			}
			int width = height / 2;
			for(int i=0; i< static_cast<int>(TxtFuncBookmark::ImageTreeIcon::iti_max); ++i){
				TCHAR path[_MAX_PATH] = {};
				_stprintf_s(path, _T("%s\\toolbar_icon\\list_icon_%s"), lpDir, l_treeicon_name_list[i]);
				std::tstring image_filename;
				if(pictMgr.GetSupportedFile(image_filename, path)){
					pictMgr.Draw(bmp_tree, i*width, 0, width, height, image_filename.c_str());
				} else {
					_stprintf_s(path, _T("%s\\toolbar_icon\\list_icon_%s"), curPath, l_treeicon_name_list[i]);
					if(pictMgr.GetSupportedFile(image_filename, path)){
						pictMgr.Draw(bmp_tree, i*width, 0, width, height, image_filename.c_str());
					}
				}
			}
		}
		ImageList_Add(m_hImg, bmp_list, NULL);
		ImageList_Add(m_hTreeImg, bmp_tree, NULL);
	}
	//
	int tag_book_split_pos = 700;
	int book_subtitle_split_pos = 100;
	std::vector<int> tag_column_width_list;
	std::vector<int> book_column_width_list;
	std::vector<int> subtitle_column_width_list;
	int tag_id = -1;
	int book_id = -1;
	{
		std::tstring ini_filename;
		CGrTxtFunc::GetBookmarkFolder(&param, ini_filename);
		ini_filename += _T("/bookmark.ini");
		TCHAR val[2048];
		::GetPrivateProfileString(_T("Bookmark"), _T("WindowPos"), _T(""), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
		WINDOWPLACEMENT wndp = {sizeof(WINDOWPLACEMENT)};
		GetWindowPlacement(m_hWnd, &wndp);
		CGrCSVText csv;
		CSV_COLMN *pcsv_colmn;
		csv.AddTail(val);
		int row = csv.GetRowSize() - 1;
		if(row >= 0 && csv.GetColmnSize(row) >= 4){
			RECT rect;
			rect.left   = csv.GetInteger(row, 0, 0);
			rect.top    = csv.GetInteger(row, 1, 0);
			rect.right  = csv.GetInteger(row, 2, 0);
			rect.bottom = csv.GetInteger(row, 3, 0);
			m_bBookmark = csv.GetInteger(row, 4, 1) == 1;
			m_bSubtitle = csv.GetInteger(row, 5, 1) == 1;
			m_bStayOn   = csv.GetInteger(row, 6, 0) == 1;
			wndp.rcNormalPosition = rect;
			wndp.showCmd = SW_HIDE;
			POINT point = {wndp.rcNormalPosition.left, wndp.rcNormalPosition.top};
			auto hMonitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST); // 指定した点に最も近い位置にあるディスプレイモニタのハンドルが返ります。
			MONITORINFO mi = {sizeof(MONITORINFO)};
			RECT desktop_rc = {};
			if(hMonitor && ::GetMonitorInfo(hMonitor, &mi)){
				if(rect.right - rect.left <= 0 || rect.bottom - rect.top <= 0){
					desktop_rc = mi.rcMonitor;
				}
			} else {
				::SystemParametersInfo(SPI_GETWORKAREA, 0, &desktop_rc, 0);
			}
			// center;
			if(desktop_rc.right - desktop_rc.left > 0 && desktop_rc.bottom - desktop_rc.top > 0){
				int w0 = desktop_rc.right - desktop_rc.left;
				int h0 = desktop_rc.bottom - desktop_rc.top;
				int w = w0 * 80 / 100;
				int h = h0 * 80 / 100;
				int x = w0 / 2 - w / 2;// + desktop_rc.left;
				int y = h0 / 2 - h / 2;// + desktop_rc.top ;
				wndp.rcNormalPosition.left   = x    ;
				wndp.rcNormalPosition.right  = x + w;
				wndp.rcNormalPosition.top    = y    ;
				wndp.rcNormalPosition.bottom = y + h;
			}
			SetWindowPlacement(m_hWnd, &wndp);
		}
		int ival;
		::GetPrivateProfileString(_T("Bookmark"), _T("TagBookSplitPos"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
		ival = _ttol(val);
		if(ival >= 0){
			tag_book_split_pos = ival;
		}
		::GetPrivateProfileString(_T("Bookmark"), _T("BookSubtitleSplitPos"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
		ival = _ttol(val);
		if(ival >= 0){
			book_subtitle_split_pos = ival;
		}
		::GetPrivateProfileString(_T("Bookmark"), _T("ImageSplitPos"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
		ival = _ttol(val);
		if(ival >= 0){
			m_subtitleWnd.SetSplitPos(ival);
		}
		::GetPrivateProfileString(_T("Bookmark"), _T("TagColumnWidth"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
		csv.Clear();
		csv.AddTail(val);
		pcsv_colmn = csv.GetRow(0);
		if(pcsv_colmn){
			for(const auto &item : *pcsv_colmn){
				tag_column_width_list.push_back(_ttol(item.c_str()));
			}
		}
		::GetPrivateProfileString(_T("Bookmark"), _T("BookColumnWidth"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
		csv.Clear();
		csv.AddTail(val);
		pcsv_colmn = csv.GetRow(0);
		if(pcsv_colmn){
			for(const auto &item : *pcsv_colmn){
				book_column_width_list.push_back(_ttol(item.c_str()));
			}
		}
		::GetPrivateProfileString(_T("Bookmark"), _T("SubtitleColumnWidth"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
		csv.Clear();
		csv.AddTail(val);
		pcsv_colmn = csv.GetRow(0);
		if(pcsv_colmn){
			for(const auto &item : *pcsv_colmn){
				subtitle_column_width_list.push_back(_ttol(item.c_str()));
			}
		}
		::GetPrivateProfileString(_T("Bookmark"), _T("LatestTagID"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
		tag_id = _ttol(val);
		::GetPrivateProfileString(_T("Bookmark"), _T("LatestBookID"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
		book_id = _ttol(val);
	}
	//
	//
	RECT client_rect;
	::GetClientRect(hwnd, &client_rect);

	m_tagWnd     .Create(hwnd, m_hImg, m_hTreeImg);
	m_bookWnd    .Create(hwnd, m_hImg);
	m_subtitleWnd.Create(hwnd, m_hImg);

	m_splitRWnd.Create(hwnd, m_bookWnd, m_subtitleWnd);
	m_splitRWnd.SetPosition(tag_book_split_pos);
	m_splitLWnd.Create(hwnd, m_tagWnd, NULL);
	m_splitLWnd.SetSplitRWnd(&m_splitRWnd);
	m_splitLWnd.SetPosition(book_subtitle_split_pos);

	setWindowSize(client_rect.right, client_rect.bottom);
	{
		{
			//m_tagWnd
			std::tstring str;
			unsigned int count = m_tagWnd.GetColumnCount();
			if(tag_column_width_list.size() >= count){
				for(unsigned int i=0; i<count; ++i){
					if(tag_column_width_list[i] > 0){
						m_tagWnd.SetColumnWidth(i, tag_column_width_list[i]);
					}
				}
			}
		}
		{
			//m_bookWnd
			std::tstring str;
			unsigned int count = m_bookWnd.GetColumnCount();
			if(book_column_width_list.size() >= count){
				for(unsigned int i=0; i<count; ++i){
					if(book_column_width_list[i] > 0){
						m_bookWnd.SetColumnWidth(i, book_column_width_list[i]);
					}
				}
			}
		}
		{
			//m_subtitleWnd
			std::tstring str;
			unsigned int count = m_subtitleWnd.GetColumnCount();
			if(subtitle_column_width_list.size() >= count){
				for(unsigned int i=0; i<count; ++i){
					if(subtitle_column_width_list[i] > 0){
						m_subtitleWnd.SetColumnWidth(i, subtitle_column_width_list[i]);
					}
				}
			}
		}
	}
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);
	TxtMiruTheme_UpdateDarkMode(m_hWnd);

	ShowWindow(m_tagWnd     , SW_SHOW);
	ShowWindow(m_bookWnd    , SW_SHOW);
	ShowWindow(m_subtitleWnd, SW_SHOW);

	PostSelect(tag_id, book_id);

	return TRUE;
}

void CGrBookListDlg::ShowBookList(const TxtFuncBookmark::Tag &tag)
{
	m_bookWnd.Set(tag);
}

void CGrBookListDlg::ShowSubtitleList(const TxtFuncBookmark::Book &book)
{
	m_subtitleWnd.Set(book);
}

#define TIMERID_REFRESH         1
#define TIMERID_SBUTITLEREFRESH 2
#define TIMERID_INITSELECT      3

void CGrBookListDlg::PostRefresh()
{
	KillTimer(m_hWnd, TIMERID_SBUTITLEREFRESH);
	SetTimer(m_hWnd, TIMERID_REFRESH, 100, nullptr);
}

void CGrBookListDlg::PostRefreshSubtitleList()
{
	SetTimer(m_hWnd, TIMERID_SBUTITLEREFRESH, 100, nullptr);
}

void CGrBookListDlg::PostSelect(int tag_id, int book_id)
{
	m_iCurrentTag = tag_id;
	m_iCurrentBook = book_id;
	KillTimer(m_hWnd, TIMERID_REFRESH);
	KillTimer(m_hWnd, TIMERID_SBUTITLEREFRESH);
	SetTimer(m_hWnd, TIMERID_INITSELECT, 100, nullptr);
}

void CGrBookListDlg::OnTimer(HWND hwnd, UINT id)
{
	switch(id){
	case TIMERID_REFRESH:
		{
			KillTimer(m_hWnd, TIMERID_REFRESH);
			KillTimer(m_hWnd, TIMERID_SBUTITLEREFRESH);
			SetFilename(m_pBookmark->GetParamString(_T("FileName")));
			RefreshBookList();
			RefreshSubtitleList();
		}
		break;
	case TIMERID_SBUTITLEREFRESH:
		{
			KillTimer(m_hWnd, TIMERID_SBUTITLEREFRESH);
			SetFilename(m_pBookmark->GetParamString(_T("FileName")));
			RefreshSubtitleList();
		}
		break;
	case TIMERID_INITSELECT:
		{
			KillTimer(m_hWnd, TIMERID_INITSELECT);
			m_tagWnd.Select(m_iCurrentTag);
			m_bookWnd.Select(m_iCurrentBook);
		}
		break;
	}
}

#define ID_CHK_NEXT (10)
void CGrBookListDlg::ClearUpdateList()
{
	if(IsUpdating()){
		AbortUpdate();
	} else {
		m_curUpdateItem = -1;
		m_updateCheckList.clear();
	}
}

bool CGrBookListDlg::AddUpdateList(int iItem)
{
	m_updateCheckList.push_back(iItem);
	return true;
}

bool CGrBookListDlg::AbortUpdate()
{
	int cnt = static_cast<signed int>(m_updateCheckList.size());
	if(m_curUpdateItem >= 0 && m_curUpdateItem < cnt){
		m_bookWnd.RefreshItem(m_updateCheckList[m_curUpdateItem]);
	}
	m_curUpdateItem = -1;
	m_updateCheckList.clear();
	SetWorking(false);
	if(m_hBackgroundPrc){
		PostThreadMessage(m_BackgroundThreadID, WM_DESTROY, 0, 0);
	}
	return true;
}

bool CGrBookListDlg::IsUpdating()
{
	return m_curUpdateItem >= 0 || m_hBackgroundPrc != NULL;
}

void CGrBookListDlg::SetWorking(bool bWorking)
{
	m_bookWnd.SetWorking(bWorking);
	m_tagWnd.SetWorking(bWorking);
}

bool CGrBookListDlg::UpdateCheck()
{
	if(m_curUpdateItem < 0){
		m_curUpdateItem = 0;
	} else {
		++m_curUpdateItem;
	}
	int cnt = static_cast<signed int>(m_updateCheckList.size());
	if(cnt <= 0 || m_curUpdateItem >= cnt){
		m_curUpdateItem = -1;
		m_updateCheckList.clear();
		SetWorking(false);
		return false;
	} else {
		TxtFuncBookmark::Book book;
		int iItem = m_updateCheckList[m_curUpdateItem];
		if(m_bookWnd.GetBook(iItem, book)){
			m_bookWnd.SetUpdating(iItem);
			UpdateCheck(book.url.c_str());
		} else {
			UpdateCheck();
		}
	}
	return true;
}

void CGrBookListDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case ID_CHK_NEXT:
		{
			int cnt = static_cast<signed int>(m_updateCheckList.size());
			if(m_curUpdateItem >= 0 && m_curUpdateItem < cnt){
				m_bookWnd.RefreshItem(m_updateCheckList[m_curUpdateItem]);
			}
			UpdateCheck();
		}
		break;
	}
}

LRESULT CGrBookListDlg::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	return TRUE;
}

#define PADDING 10
void CGrBookListDlg::setWindowSize(int cx, int cy)
{
	if(cx <= 0 || cy <= 0){
		return;
	}
	RECT client_rect = {0,0,cx,cy};
	if(m_splitLWnd){
		m_splitLWnd.SetWindowSize(&client_rect);
	}
}

void CGrBookListDlg::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
		return;
	}
	setWindowSize(cx, cy);
}

void CGrBookListDlg::RefreshBookList(int parent)
{
	m_bookWnd.DispList(true, parent);
}

void CGrBookListDlg::RefreshSubtitleList(int book_id)
{
	m_subtitleWnd.Refresh(book_id);
}

void CGrBookListDlg::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
}

int CGrBookListDlg::GetCurBookIDSubtitleList()
{
	return m_subtitleWnd.GetSelectID();
}

int CGrBookListDlg::GetCurrentPage()
{
	if(m_pBookmark){
		return CGrTxtFunc::GetCurrentPage(m_parentWnd);
	}
	return -1;
}

bool CGrBookListDlg::OpenFile(LPCTSTR lpUrl, int page/*=-1*/)
{
	CGrTxtFunc::OpenFile(m_parentWnd, lpUrl, page);
	if(!m_bStayOn){
		if(IsUpdating()){
			AbortUpdate();
		}
		ShowWindow(m_hWnd, SW_HIDE);
		::SendMessage(m_parentWnd, WM_COMMAND, TxtDocMessage::UPDATE_CMD_UI, 0);
	}
	return true;
}

using TxtMiruFunc_CGrDocOpenProcCallbak = void (__stdcall *)(int ret, LPARAM lParam);
using TxtMiruFunc_UpdateCheck =  HANDLE (cdecl *)(HWND hWnd, LPCTSTR lpFilename, TxtMiruFunc_CGrDocOpenProcCallbak callback, LPARAM lParam, unsigned int *threadid);
void __stdcall CGrBookListDlg::CGrDocOpenProcCallbak(int ret, LPARAM lParam)
{
	auto *pDlg = reinterpret_cast<CGrBookListDlg*>(lParam);
	if(pDlg){
		if(pDlg->m_hBackgroundPrc){
			CloseHandle(pDlg->m_hBackgroundPrc);
			pDlg->m_hBackgroundPrc = NULL;
			pDlg->m_BackgroundThreadID = 0;
		}
		FORWARD_WM_COMMAND(pDlg->GetWnd(), ID_CHK_NEXT, 0, 0, PostMessage);
	}
}

bool CGrBookListDlg::UpdateCheck(LPCTSTR lpUrl)
{
	if(m_hBackgroundPrc){
		return false;
	}
	auto func = reinterpret_cast<TxtMiruFunc_UpdateCheck>(GetProcAddress(GetModuleHandle(NULL), "TxtMiruFunc_UpdateCheck"));
	if(!func){
		return false;
	}
	auto h = func(m_hWnd, lpUrl, CGrDocOpenProcCallbak, reinterpret_cast<LPARAM>(this), &m_BackgroundThreadID);
	if(!m_hBackgroundPrc){
		return false;
	}
	m_hBackgroundPrc = h;
	return true;
}

bool CGrBookListDlg::SetFilename(LPCTSTR lpFilename)
{
	if(m_filename != lpFilename){
		m_filename = lpFilename;
		return true;
	}
	return false;
}
