#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PanelSubtitle.h"
#include "BookListDlg.h"
#include "BookmarkDB.h"
#include <regex>
#include "CharRenderer.h"
#include "PictRenderer.h"
#include "MessageBox.h"
#include "TxtMiruTheme.h"
#include "Text.h"
//#define __DBG__
#include "Debug.h"

CGrPanelSubtitleWnd::CGrPanelSubtitleWnd(CGrBookListDlg &dlg) : CGrPanelWnd(dlg)
{
	m_book.id = -1;
}

CGrPanelSubtitleWnd::~CGrPanelSubtitleWnd()
{
}

bool CGrPanelSubtitleWnd::Create(HWND hParent, HIMAGELIST hImg)
{
	if(!CGrPanelWnd::Create(hParent, hImg)){
		return false;
	}
	SetWindowFont(m_listView, GetWindowFont(hParent), TRUE);
	InsertColumn(IDS_BMH_TITLE, 0, 200, LVCFMT_LEFT);
	InsertColumn(IDS_BMH_READPAGE, 1, 50, LVCFMT_RIGHT);
	InsertColumn(IDS_BMH_TOTALPAGE, 2, 50, LVCFMT_RIGHT);
	InsertColumn(IDS_BMH_VISIT_DATE, 3, 150, LVCFMT_LEFT);
	InsertColumn(IDS_BMH_URL, 4, 200, LVCFMT_LEFT);
	auto styleex = m_listView.GetExtendedListViewStyle();
	styleex |= LVS_EX_FLATSB | LVS_EX_FULLROWSELECT;
	m_listView.SetExtendedListViewStyle(styleex);
	//
	auto &&toolBar = m_coolBar.GetToolBar();
	TBBUTTON tbb[] = {
		//iBitmap                                                        idCommand          fsState          fsStyle                         bReserved[2]  dwData iString
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_subtitle_open), IDM_SUBTITLE_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_SUBTITLE_OPEN},
		{I_IMAGENONE                                                   , IDS_BMH_TITLE    , TBSTATE_ENABLED, BTNS_AUTOSIZE  | BTNS_SHOWTEXT, 0, 0        , 0    , IDS_ERROR_BOOK        },
	};
	for(auto &&item : tbb){
		if(item.iString != 0){
			std::tstring tips;
			CGrText::LoadString(item.iString, tips);
			item.iString = SendMessage(toolBar, TB_ADDSTRING, 0, reinterpret_cast<LPARAM>(tips.c_str()));
		} else {
			item.iString = sizeof(tbb)/sizeof(TBBUTTON) + 1;
		}
	}
	TBADDBITMAP tb = {};
	toolBar.AddButtons(tbb, sizeof(tbb)/sizeof(TBBUTTON), tb);
	toolBar.SetButtons(_T("1,2"));

	m_cover.Create(m_hWnd);
	m_coverInfo.Create(m_hWnd);

	m_splitWnd.Create(m_hWnd);
	m_splitWnd.SetMode(true);
	m_splitWnd.SetLfWnd(NULL);
	m_splitWnd.SetRtWnd(m_listView);
	m_splitWnd.SetImgWnd(m_cover);
	m_splitWnd.SetInfWnd(m_coverInfo);

	RECT rc;
	GetClientRect(m_hWnd, &rc);
	setWindowSize(rc.right-rc.left, rc.bottom-rc.top);
	m_splitWnd.SetPosition(m_splitWnd.GetLeftMinWidth() + m_imageSize);
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);

	return true;
}

void CGrPanelSubtitleWnd::SetSplitPos(int pos)
{
	m_imageSize = pos;
	if(m_splitWnd){
		m_splitWnd.SetPosition(m_splitWnd.GetLeftMinWidth() + m_imageSize);
	}
}

int CGrPanelSubtitleWnd::GetSplitPos()
{
	return m_splitWnd.GetPosition() - m_splitWnd.GetLeftMinWidth();
}

void CGrSplitImgWnd::SetImgWnd(HWND hWnd)
{
	m_hImgWnd = hWnd;
}
void CGrSplitImgWnd::SetInfWnd(HWND hWnd)
{
	m_hInfWnd = hWnd;
}

void CGrSplitImgWnd::resizeRightWindow(RECT &rect)
{
	CGrSplitWnd::resizeRightWindow(rect);
	if(m_hImgWnd){
		int image_size = GetPosition() - 2 - m_leftMin;
		if(image_size < 0){
			image_size = 0;
		}
		MoveWindow(m_hImgWnd, 1, m_leftMin + 1, image_size, image_size, TRUE);
	}
	if(m_hInfWnd){
		int image_size = GetPosition() - 2 - m_leftMin;
		if(image_size < 0){
			image_size = 0;
		}
		MoveWindow(m_hInfWnd, image_size+1+5/*padding*/, m_leftMin + 1, rect.right-image_size-(1+5*2/*padding*/), image_size, TRUE);
	}
}

void CGrPanelSubtitleWnd::setWindowSize(int cx, int cy)
{
	if(cx <= 0 || cy <= 0){
		return;
	}
	if(!m_cover){
		return;
	}
	RECT client_rect = {0,0,cx,cy};
	if(m_coolBar){
		FORWARD_WM_SIZE(m_coolBar, SIZE_RESTORED, cx, cy, SendMessage);
		RECT rect;
		GetWindowRect(m_coolBar, &rect);
		client_rect.top += rect.bottom - rect.top + GetSystemMetrics(SM_CYEDGE);
	}
	if(m_splitWnd){
		m_splitWnd.SetLeftMinWidth(client_rect.top);
		m_splitWnd.SetWindowSize(&client_rect);
	}
}

// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
LRESULT CGrPanelSubtitleWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_COMMAND      , OnCommand      );
	case WM_SIZE:
		{
			auto state = static_cast<UINT>(wParam);
			auto cx = static_cast<int>(LOWORD(lParam));
			auto cy = static_cast<int>(HIWORD(lParam));
			if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
				return 0L;
			}
			setWindowSize(cx, cy);
			return 0L;
		}
		break;
	}
	return CGrPanelWnd::WndProc(hWnd, uMsg, wParam, lParam);
}

bool CGrPanelSubtitleWnd::openFile(int iItem)
{
	if(CGrTxtFunc::IsWaitting()){
		return false;
	}
	auto index = m_listView.GetParam(iItem);
	if(index < 0 || index >= static_cast<signed int>(m_subtitle_list.size())){
		return false;
	}
	const auto &subtitle = m_subtitle_list[index];
	if(subtitle.read_page > 0){
		m_booklistDlg.OpenFile(subtitle.url.c_str(), subtitle.read_page);
	} else {
		m_booklistDlg.OpenFile(subtitle.url.c_str(), subtitle.page);
	}
	return true;
}

bool CGrPanelSubtitleWnd::setUnRead(int iItem)
{
	if(CGrTxtFunc::IsWaitting()){
		return false;
	}
	auto index = m_listView.GetParam(iItem);
	if(index < 0 || index >= static_cast<signed int>(m_subtitle_list.size())){
		return false;
	}
	auto &subtitle = m_subtitle_list[index];
	bool bRet = true;
	do {
		CGrBookmarkDB db;
		if(!db.Open()){
			bRet = false;
			break;
		}
		Place place;
		bRet = db.GetPlace(place, subtitle.url.c_str());
		if(!bRet){
			break;
		}
		if(m_book.url == subtitle.url){
			int page = subtitle.page;
			page &= (-2);
			if(place.read_page > page){
				place.read_page = max(page-1,0);
				CGrDBFunc::GetSysDate(place.last_visit_date);
				m_book.read_page = place.read_page;
			} else {
				break;
			}
		} else {
			place.read_page       = 0;
			place.last_visit_date = _T("");
		}
		bRet = db.PutPlace(place);
		if(!bRet){
			break;
		}
	} while(0);

	return bRet;
}

bool CGrPanelSubtitleWnd::setRead(int iItem)
{
	if(CGrTxtFunc::IsWaitting()){
		return false;
	}
	int index = m_listView.GetParam(iItem);
	if(index < 0 || index >= static_cast<signed int>(m_subtitle_list.size())){
		return false;
	}
	const auto &subtitle = m_subtitle_list[index];
	bool bRet = true;
	do {
		CGrBookmarkDB db;
		if(!db.Open()){
			bRet = false;
			break;
		}
		Place place;
		bRet = db.GetPlace(place, subtitle.url.c_str());
		if(!bRet){
			if(m_book.url == subtitle.url){
				break;
			}
			int page = max(subtitle.page, 1);
			place.id          = NULL_INT_VALUE;
			place.url         = subtitle.url  ;
			place.title       = subtitle.title;
			place.author      = m_book.author ;
			place.page        = page          ;
			place.read_page   = page          ;
			place.visit_count = 0             ;
			CGrDBFunc::GetSysDate(place.last_visit_date);
			place.insert_date = place.last_visit_date;
		} else {
			if(m_book.url == subtitle.url){
				place.read_page = subtitle.page;
				CGrDBFunc::GetSysDate(place.last_visit_date);
				History history;
				history.id              = NULL_INT_VALUE       ;
				history.place_id        = place.id             ;
				history.url             = place.url            ;
				history.title           = place.title          ;
				history.author          = place.author         ;
				history.page            = place.page           ;
				history.read_page       = place.read_page      ;
				history.insert_date     = place.insert_date    ;
				history.last_visit_date = place.last_visit_date;
				bRet = db.PutHistory(history);
				if(!bRet){
					break;
				}
				m_book.read_page = place.read_page;
			} else {
				int page = max(subtitle.page, 1);
				place.read_page = page;
				CGrDBFunc::GetSysDate(place.last_visit_date);
			}
		}
		bRet = db.PutPlace(place);
		if(!bRet){
			break;
		}
	} while(0);

	return bRet;
}

void CGrPanelSubtitleWnd::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDM_SUBTITLE_OPEN:
		{
			auto iItem = GetFocusedItem();
			if(iItem >= 0){
				openFile(iItem);
			} else if(!CGrTxtFunc::IsWaitting() && !m_book.url.empty()){
				m_booklistDlg.OpenFile(m_book.url.c_str());
			}
		}
		break;
	case IDM_SUBTITLE_READ:
		{
			auto cnt = m_listView.GetSelectedCount();
			if(cnt > 0){
				int iItem = -1;
				m_listView.SetRedraw(FALSE);
				while((iItem=m_listView.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED))!=-1){
					LVITEM item = { LVIF_IMAGE };
					item.iItem = iItem;
					if(FALSE == m_listView.GetItem(&item)){
						continue;
					}
					if(/**/item.iImage == static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_bookmark_normal_update) || item.iImage == static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_bookmark_current_update)
					   ||  item.iImage == static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_link_normal_update    ) || item.iImage == static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_link_current_update    ) ){
						continue;
					}
					if(!setRead(iItem)){
						break;
					}
				}
				m_listView.SetRedraw(TRUE);
				m_booklistDlg.RefreshBookList();
				Refresh();
			} else {
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_ERROR_SUBTITLE, CGrTxtFunc::AppName());
			}
		}
		break;
	case IDM_SUBTITLE_NOREAD:
		{
			auto cnt = m_listView.GetSelectedCount();
			if(cnt > 0){
				int iItem = -1;
				m_listView.SetRedraw(FALSE);
				while((iItem=m_listView.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED))!=-1){
					LVITEM item = { LVIF_IMAGE };
					item.iItem = iItem;
					if(FALSE == m_listView.GetItem(&item)){
						continue;
					}
					if(/**/item.iImage == static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_bookmark_normal) || item.iImage == static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_bookmark_current)
					   ||  item.iImage == static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_link_normal    ) || item.iImage == static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_link_current    ) ){
						continue;
					}
					if(!setUnRead(iItem)){
						break;
					}
				}
				m_listView.SetRedraw(TRUE);
				m_booklistDlg.RefreshBookList();
				Refresh();
			} else {
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_ERROR_SUBTITLE, CGrTxtFunc::AppName());
			}
		}
		break;
	}
}

LRESULT CGrPanelSubtitleWnd::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	if(idFrom == listwnd_id){
		switch (lpnmhdr->code) {
		case NM_DBLCLK:
		{
			auto lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(lpnmhdr);
			openFile(lpnmitem->iItem);
			break;
		}
		case NM_RCLICK:
			if (!CGrTxtFunc::IsWaitting()) {
				POINT cursor;
				GetCursorPos(&cursor);
				auto lpnmitemactivate = reinterpret_cast<LPNMITEMACTIVATE>(lpnmhdr);
				auto hMenu = ::LoadMenu(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDR_MENU_POPUP_SUBTITLE));
				RemoveMenu(hMenu, IDM_MAKECACHE, FALSE); // ☆一時的
				CGrPanelWnd_SetTouchMenu(hMenu);
				::TrackPopupMenu(::GetSubMenu(hMenu, 0), TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, cursor.x, cursor.y, 0, hWnd, NULL);
				::DestroyMenu(hMenu);
			}
			break;
		case LVN_BEGINDRAG:
		{
			if (!m_booklistDlg.IsUpdating()) {
				auto pnmv = reinterpret_cast<LPNMLISTVIEW>(lpnmhdr);
				int cnt = m_listView.GetSelectedCount();
				UINT flags = (cnt <= 0) ? (LVNI_ALL | LVNI_FOCUSED) : (LVNI_ALL | LVNI_SELECTED);
				int iItem = -1;
				std::tstring url;
				while ((iItem = m_listView.GetNextItem(iItem, flags)) != -1) {
					LVITEM item = { LVIF_PARAM };
					item.iItem = iItem;
					if (FALSE == m_listView.GetItem(&item)) {
						break;
					}
					int index = static_cast<int>(item.lParam);
					if (index < 0 || index >= static_cast<signed int>(m_subtitle_list.size())) {
						break;
					}
					if (url.empty()) {
						url = m_subtitle_list[index].url;
					}
				}
				auto* pDropSource = new CDropSource;
				///////////////
				auto* pDataObject = new CDataObject(pDropSource);
				pDataObject->SetUrl(url.c_str());
				///////////////
				CDragSourceHelper dragSrcHelper;
				dragSrcHelper.InitializeFromWindow(m_listView, pnmv->ptAction, pDataObject);

				DWORD dwEffect;
				auto hr = ::DoDragDrop(pDataObject, pDropSource, DROPEFFECT_MOVE, &dwEffect);
				if (SUCCEEDED(hr)) {
					if (dwEffect == DROPEFFECT_MOVE) {
						DispList(true);
					}
				}
				pDropSource->Release();
				pDataObject->Release();
			}
			break;
		}
		case NM_CUSTOMDRAW:
		{
			auto lpCustomDraw = reinterpret_cast<LPNMTBCUSTOMDRAW>(lpnmhdr);
			break;
		}
		}
	}
	return CGrPanelWnd::OnNotify(hWnd, idFrom, lpnmhdr);
}

bool CGrPanelSubtitleWnd::Set(const TxtFuncBookmark::Book &book)
{
	if(m_book.place_id == book.place_id){
		return false;
	}
	m_book = book;
	//
	auto &&toolBar = m_coolBar.GetToolBar();
	TBBUTTONINFO ti = {sizeof(TBBUTTONINFO)};
	ti.dwMask = TBIF_TEXT;
	ti.pszText = const_cast<LPTSTR>(m_book.title.c_str());
	ti.cchText = m_book.title.size();
	SendMessage(toolBar, TB_SETBUTTONINFO, IDS_BMH_TITLE, reinterpret_cast<LPARAM>(&ti));
	//
	m_coverInfo.Set(m_book);
	//
	return DispList(false);
}

bool CGrPanelSubtitleWnd::Refresh(int book_id)
{
	if(book_id == -1){
		book_id = m_book.id;
	} else if(book_id != m_book.id){
		return false;
	}
	//
	auto &&toolBar = m_coolBar.GetToolBar();
	TBBUTTONINFO ti = {sizeof(TBBUTTONINFO)};
	ti.dwMask = TBIF_TEXT;
	ti.pszText = const_cast<LPTSTR>(m_book.title.c_str());
	ti.cchText = m_book.title.size();
	SendMessage(toolBar, TB_SETBUTTONINFO, IDS_BMH_TITLE, reinterpret_cast<LPARAM>(&ti));
	//
	return DispList(true);
}


int CGrPanelSubtitleWnd::GetSelectID() const
{
	return m_book.id;
}

static std::tstring getTitle(const std::tstring &title)
{
	const auto check = L" - 111";
	const auto check_end = check + sizeof(L" - 111") / sizeof(TCHAR) - 1;
	auto step = check;
	int len = 0;
	int target_len = 0;
	for (auto ch : title) {
		if (*step == L'1') {
			if (ch >= L'0' && ch <= L'9') {
				++step;
			} else {
				step = check;
			}
		} else if (*step == ch) {
			if (step == check) {
				target_len = len;
			}
			++step;
		} else if(step != check){
			step = check;
		}
		if (len > 0 && step == check_end) {
			return title.substr(0, target_len);
		}
		++len;
	}
	return title;
}

bool CGrPanelSubtitleWnd::DispList(bool bKeepIndex)
{
	bool bRet = true;
	m_listView.SetRedraw(FALSE);
	do {
		CGrBookmarkDB db;
		if(!db.Open()){
			bRet = false;
			break;
		}
		//
		m_cover.Load(m_book.id);
		//
		int index = -1;
		if(bKeepIndex){
			int iTopIndex = m_listView.GetTopIndex();
			int iCountPage = m_listView.GetCountPerPage();
			index = iTopIndex + iCountPage - 1;
		}
		int iCurrentImage = -1;
		int iCurrentTitle = -1;
		int iCurrentPage  = m_booklistDlg.GetCurrentPage();
		const auto &current_filename = m_booklistDlg.GetFilename();
		if(current_filename == m_book.url){
			if(iCurrentPage >= 0){
				iCurrentPage &= (-2);
				++iCurrentPage;
			}
		} else {
			iCurrentPage = -1;
		}
		SCROLLINFO si = {sizeof(SCROLLINFO)};
		si.fMask = SIF_POS;
		::GetScrollInfo(m_listView, SB_HORZ, &si);

		m_listView.DeleteAllItems();
		m_subtitle_list.clear();

		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			bRet = false;
			break;
		}
		if(CGrText::isMatchChar(m_book.url.c_str(), _T(":Visit")))
		{
			int count = _ttoi(m_book.url.c_str()+6);
			if(count == 0){
				count = 10;
			}
			bool bret = true;
			sqlite3_stmt *pstmt_txtmiru_st_latest = nullptr;
			do {
				int ret = 0;
				ret = sqlite3_prepare16(pSqlite3, _T("SELECT P.ID,P.URL,P.TITLE,P.AUTHOR,P.READ_PAGE,P.PAGE,P.LAST_VISIT_DATE")
										_T(" FROM TXTMIRU_PLACES P")
										_T(" ORDER BY P.VISIT_COUNT DESC"), -1, &pstmt_txtmiru_st_latest, nullptr);
				if(ret != SQLITE_OK || !pstmt_txtmiru_st_latest) {
					// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
					bRet = false;
					break;
				}
				// 実行
				bret = false;
				while(true){
					ret = sqlite3_step(pstmt_txtmiru_st_latest);
					if(ret != SQLITE_ROW){
						break;
					}
					TxtFuncBookmark::Subtitle subtitle;
					subtitle.id        = 0;
					subtitle.b_order   = 0;
					subtitle.page      = 0;
					subtitle.read_page = 0;
					subtitle.level     = 0;
					std::tstring author;
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 0, subtitle.id             );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 1, subtitle.url            );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 2, subtitle.title          );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 3, author                  );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 4, subtitle.read_page      );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 5, subtitle.page           );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 6, subtitle.last_visit_date);
					if(CGrText::isMatchChar(subtitle.url.c_str(), _T(":"))){
						continue;
					}
					if(m_subtitle_list.size() > static_cast<UINT>(count)){
						break;
					}

					std::tstring title;

					title = subtitle.title;
					title += _T(":");
					title += author;
					LVITEM item = {};
					item.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_INDENT;
					item.iItem    = m_listView.GetItemCount();
					item.iSubItem = 0;
					item.lParam   = m_subtitle_list.size();
					item.iImage   = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_normal);
					item.pszText  = const_cast<LPTSTR>(title.c_str());

					bool bRead = false;
					item.iItem    = m_listView.InsertItem(&item);
					item.mask     = LVIF_TEXT;

					m_subtitle_list.push_back(subtitle);

					TCHAR buf[512];
					if(subtitle.read_page > 0){
						_stprintf_s(buf, _T("%d"), subtitle.read_page);
						item.pszText  = buf;
						item.iSubItem = 1;
						m_listView.SetItem(&item);
					}
					if(subtitle.page > 0){
						_stprintf_s(buf, _T("%d"), subtitle.page);
						item.pszText  = buf;
						item.iSubItem = 2;
						m_listView.SetItem(&item);
					}

					item.pszText  = const_cast<LPTSTR>(subtitle.last_visit_date.c_str());
					item.iSubItem = 3;
					m_listView.SetItem(&item);

					item.pszText  = const_cast<LPTSTR>(subtitle.url.c_str());
					item.iSubItem = 4;
					m_listView.SetItem(&item);
				};
			} while(0);
			if(pstmt_txtmiru_st_latest){
				sqlite3_finalize(pstmt_txtmiru_st_latest);
			}
		} else if(CGrText::isMatchChar(m_book.url.c_str(), _T(":Latest")))
		{
			int count = _ttoi(m_book.url.c_str()+7);
			if(count == 0){
				count = 10;
			}
			bool bret = true;
			sqlite3_stmt *pstmt_txtmiru_st_latest = nullptr;
			do {
				int ret = sqlite3_prepare16(pSqlite3, _T("SELECT P.PLACE_ID,P.URL,P.TITLE,P.AUTHOR,P.READ_PAGE,P.PAGE,P.LAST_VISIT_DATE")
										_T(" FROM TXTMIRU_HISTORY P")
										_T(" ORDER BY P.LAST_VISIT_DATE DESC"), -1, &pstmt_txtmiru_st_latest, nullptr);
				if(ret != SQLITE_OK || !pstmt_txtmiru_st_latest) {
					// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
					bRet = false;
					break;
				}
				// 実行
				std::map<int,bool> place_id_map;
				bret = false;
				while(true){
					ret = sqlite3_step(pstmt_txtmiru_st_latest);
					if(ret != SQLITE_ROW){
						break;
					}
					TxtFuncBookmark::Subtitle subtitle;
					subtitle.id        = 0;
					subtitle.b_order   = 0;
					subtitle.page      = 0;
					subtitle.read_page = 0;
					subtitle.level     = 0;
					std::tstring author;
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 0, subtitle.id             );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 1, subtitle.url            );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 2, subtitle.title          );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 3, author                  );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 4, subtitle.read_page      );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 5, subtitle.page           );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_latest, 6, subtitle.last_visit_date);
					if(CGrText::isMatchChar(subtitle.url.c_str(), _T(":"))){
						continue;
					}
					if(place_id_map.find(subtitle.id) == place_id_map.end()){
						place_id_map[subtitle.id] = true;
					} else {
						continue;
					}
					if(place_id_map.size() > static_cast<UINT>(count)){
						break;
					}

					std::tstring title;

					title = subtitle.title;
					title += _T(":");
					title += author;
					LVITEM item = {};
					item.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_INDENT;
					item.iItem    = m_listView.GetItemCount();
					item.iSubItem = 0;
					item.lParam   = m_subtitle_list.size();
					item.iImage   = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_normal);
					item.pszText  = const_cast<LPTSTR>(title.c_str());

					bool bRead = false;
					item.iItem    = m_listView.InsertItem(&item);
					item.mask     = LVIF_TEXT;

					m_subtitle_list.push_back(subtitle);

					TCHAR buf[512];
					if(subtitle.read_page > 0){
						_stprintf_s(buf, _T("%d"), subtitle.read_page);
						item.pszText  = buf;
						item.iSubItem = 1;
						m_listView.SetItem(&item);
					}
					if(subtitle.page > 0){
						_stprintf_s(buf, _T("%d"), subtitle.page);
						item.pszText  = buf;
						item.iSubItem = 2;
						m_listView.SetItem(&item);
					}

					item.pszText  = const_cast<LPTSTR>(subtitle.last_visit_date.c_str());
					item.iSubItem = 3;
					m_listView.SetItem(&item);

					item.pszText  = const_cast<LPTSTR>(subtitle.url.c_str());
					item.iSubItem = 4;
					m_listView.SetItem(&item);
				};
			} while(0);
			if(pstmt_txtmiru_st_latest){
				sqlite3_finalize(pstmt_txtmiru_st_latest);
			}
		} else if(m_book.url == _T(":Siori"))
		{
			bool bret = true;
			sqlite3_stmt *pstmt_txtmiru_st_siori = nullptr;
			do {
				int ret = sqlite3_prepare16(pSqlite3, _T("SELECT P.URL,P.TITLE,P.AUTHOR,B.TITLE,B.PAGE")
										_T(" FROM TXTMIRU_BOOKMARKS B")
										_T(" INNER JOIN TXTMIRU_PLACES P ON P.ID=B.PLACE_ID")
										_T(" ORDER BY P.URL,B.PAGE,B.B_LINE,B.B_INDEX,B.B_POS"), -1, &pstmt_txtmiru_st_siori, nullptr);
				if(ret != SQLITE_OK || !pstmt_txtmiru_st_siori) {
					// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
					bRet = false;
					break;
				}
				// 実行
				bret = false;
				while(true){
					ret = sqlite3_step(pstmt_txtmiru_st_siori);
					if(ret != SQLITE_ROW){
						break;
					}
					TxtFuncBookmark::Subtitle subtitle;
					subtitle.id        = 0;
					subtitle.b_order   = 0;
					subtitle.page      = 0;
					subtitle.read_page = 0;
					subtitle.level     = 0;
					std::tstring author;
					std::tstring b_title;
					CGrDBFunc::GetValue(pstmt_txtmiru_st_siori, 0, subtitle.url  );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_siori, 1, subtitle.title);
					CGrDBFunc::GetValue(pstmt_txtmiru_st_siori, 2, author        );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_siori, 3, b_title       );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_siori, 4, subtitle.page );

					std::tstring title;

					title = subtitle.title;
					title += _T(":");
					title += author;
					title += _T(":");
					title += b_title;
					LVITEM item = {};
					item.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_INDENT;
					item.iItem    = m_listView.GetItemCount();
					item.iSubItem = 0;
					item.lParam   = m_subtitle_list.size();
					item.iImage   = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_bookmark);
					item.pszText  = const_cast<LPTSTR>(title.c_str());

					bool bRead = false;
					item.iItem    = m_listView.InsertItem(&item);
					item.mask     = LVIF_TEXT;

					m_subtitle_list.push_back(subtitle);

					TCHAR buf[512];
					if(subtitle.page > 0){
						_stprintf_s(buf, _T("%d"), subtitle.page);
						item.pszText  = buf;
						item.iSubItem = 2;
						m_listView.SetItem(&item);
					}

					item.pszText  = const_cast<LPTSTR>(subtitle.url.c_str());
					item.iSubItem = 4;
					m_listView.SetItem(&item);
				};
			} while(0);
			if(pstmt_txtmiru_st_siori){
				sqlite3_finalize(pstmt_txtmiru_st_siori);
			}
		} else {
			bool bret = true;
			sqlite3_stmt *pstmt_txtmiru_st_siori = nullptr;
			do {
				int ret = sqlite3_prepare16(pSqlite3, _T("SELECT P.URL,P.TITLE,P.AUTHOR,B.TITLE,B.PAGE")
										_T(" FROM TXTMIRU_BOOKMARKS B")
										_T(" INNER JOIN TXTMIRU_PLACES P ON P.ID=B.PLACE_ID")
										_T(" WHERE B.PLACE_ID=@PLACE_ID")
										_T(" ORDER BY P.URL,B.PAGE,B.B_LINE,B.B_INDEX,B.B_POS"), -1, &pstmt_txtmiru_st_siori, nullptr);
				if(ret != SQLITE_OK || !pstmt_txtmiru_st_siori) {
					// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
					bRet = false;
					break;
				}
				sqlite3_clear_bindings(pstmt_txtmiru_st_siori);
				CGrDBFunc::PutValue(pstmt_txtmiru_st_siori, "@PLACE_ID", m_book.place_id);
				// 実行
				bret = false;
				while(true){
					ret = sqlite3_step(pstmt_txtmiru_st_siori);
					if(ret != SQLITE_ROW){
						break;
					}
					TxtFuncBookmark::Subtitle subtitle;
					subtitle.id        = 0;
					subtitle.b_order   = 0;
					subtitle.page      = 0;
					subtitle.read_page = 0;
					subtitle.level     = 0;
					std::tstring author;
					std::tstring b_title;
					CGrDBFunc::GetValue(pstmt_txtmiru_st_siori, 0, subtitle.url  );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_siori, 1, subtitle.title);
					CGrDBFunc::GetValue(pstmt_txtmiru_st_siori, 2, author        );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_siori, 3, b_title       );
					CGrDBFunc::GetValue(pstmt_txtmiru_st_siori, 4, subtitle.page );

					std::tstring title;

					title = b_title;
					LVITEM item = {};
					item.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_INDENT;
					item.iItem    = m_listView.GetItemCount();
					item.iSubItem = 0;
					item.lParam   = m_subtitle_list.size();
					item.pszText  = const_cast<LPTSTR>(title.c_str());
					item.iImage   = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_bookmark);

					bool bRead = false;
					item.iItem    = m_listView.InsertItem(&item);
					item.mask     = LVIF_TEXT;

					m_subtitle_list.push_back(subtitle);

					TCHAR buf[512];
					if(subtitle.page > 0){
						_stprintf_s(buf, _T("%d"), subtitle.page);
						item.pszText  = buf;
						item.iSubItem = 2;
						m_listView.SetItem(&item);
					}

					item.pszText  = const_cast<LPTSTR>(subtitle.url.c_str());
					item.iSubItem = 4;
					m_listView.SetItem(&item);
				};
			} while(0);
			if(pstmt_txtmiru_st_siori){
				sqlite3_finalize(pstmt_txtmiru_st_siori);
			}
			//
			sqlite3_stmt *pstmt_txtmiru_st_link = nullptr;
			sqlite3_stmt *pstmt_txtmiru_st_bm   = nullptr;
			do {
				int ret = sqlite3_prepare16(pSqlite3, _T("SELECT S.ID,S.B_ORDER,S.TITLE,S.URL,")
										_T("P.PAGE,P.READ_PAGE,P.LAST_VISIT_DATE,S.PAGE,S.LEVEL")
										_T(" FROM TXTMIRU_SUBTITLES S LEFT JOIN TXTMIRU_PLACES P ON P.URL=S.URL ")
										_T(" WHERE S.PLACE_ID=@PLACE_ID AND S.URL!='' ORDER BY S.B_ORDER"), -1, &pstmt_txtmiru_st_link, nullptr);
				if(ret != SQLITE_OK || !pstmt_txtmiru_st_link) {
					// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
					bRet = false;
					break;
				}
				ret = sqlite3_prepare16(pSqlite3, _T("SELECT S.ID,S.B_ORDER,S.TITLE,S.URL,")
										_T("-1,-1,'',S.PAGE,S.LEVEL")
										_T(" FROM TXTMIRU_SUBTITLES S")
										_T(" WHERE S.PLACE_ID=@PLACE_ID AND S.URL='' ORDER BY S.B_ORDER"), -1, &pstmt_txtmiru_st_bm, nullptr);
				if(ret != SQLITE_OK || !pstmt_txtmiru_st_bm) {
					// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
					bRet = false;
					break;
				}
				sqlite3_clear_bindings(pstmt_txtmiru_st_link);
				CGrDBFunc::PutValue(pstmt_txtmiru_st_link, "@PLACE_ID", m_book.place_id);
				sqlite3_clear_bindings(pstmt_txtmiru_st_bm);
				CGrDBFunc::PutValue(pstmt_txtmiru_st_bm, "@PLACE_ID", m_book.place_id);
				auto *pstmt = pstmt_txtmiru_st_bm;
				// 実行
				bret = false;
				while(true){
					ret = sqlite3_step(pstmt);
					if(ret != SQLITE_ROW){
						if(pstmt == pstmt_txtmiru_st_bm){
							pstmt = pstmt_txtmiru_st_link;
							continue;
						}
						break;
					}
					TxtFuncBookmark::Subtitle subtitle;
					subtitle.read_page = 0;
					CGrDBFunc::GetValue(pstmt, 0, subtitle.id             );
					CGrDBFunc::GetValue(pstmt, 1, subtitle.b_order        );
					CGrDBFunc::GetValue(pstmt, 2, subtitle.title          );
					CGrDBFunc::GetValue(pstmt, 3, subtitle.url            );
					CGrDBFunc::GetValue(pstmt, 4, subtitle.page           );
					CGrDBFunc::GetValue(pstmt, 5, subtitle.read_page      );
					CGrDBFunc::GetValue(pstmt, 6, subtitle.last_visit_date);
					CGrDBFunc::GetValue(pstmt, 8, subtitle.level          );
					bret = true;

					if(!subtitle.last_visit_date.empty()){
						subtitle.read_page &= (-2);
						++subtitle.read_page;
					}
					auto title = getTitle(subtitle.title);

					LVITEM item = {};
					item.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_INDENT;
					item.iItem    = m_listView.GetItemCount();
					item.iSubItem = 0;
					item.lParam   = m_subtitle_list.size();
					item.pszText  = const_cast<LPTSTR>(title.c_str());

					if(subtitle.level > 1){
						item.iIndent = subtitle.level-1;
					}

					bool bRead = false;

					if(subtitle.url.empty()){
						item.iImage   = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_bookmark_normal);
						CGrDBFunc::GetValue(pstmt, 7, subtitle.page);
						int subtitle_page = subtitle.page;
						subtitle_page &= (-2);
						if(db.GetLastVisitDate(subtitle.last_visit_date, m_book.place_id, subtitle_page)){
							if(subtitle.page > m_book.read_page && m_book.last_visit_date > subtitle.last_visit_date){
								bRead = false;
							} else {
								bRead = true;
							}
						}
						if(!bKeepIndex){
							if(bRead){
								index = item.lParam + 1;
							}
						}
						if(iCurrentPage >= subtitle.page){
							iCurrentTitle = item.lParam;
							iCurrentImage = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_bookmark_current);
							if(bRead){
								iCurrentImage += 1;
							}
						}
						subtitle.url = m_book.url;
					} else {
						item.iImage   = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_link_normal);
						if((subtitle.page < 4 && subtitle.page > 0)|| subtitle.read_page > (subtitle.page / 4) && !subtitle.last_visit_date.empty()){
							bRead = true;
						}
						if(!bKeepIndex){
							if(subtitle.url == current_filename){
								index = item.lParam;
							} else if(bRead){
								index = item.lParam + 1;
							}
						}
						if(subtitle.url == current_filename){
							iCurrentTitle = item.lParam;
							iCurrentImage = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_link_current);
							if(bRead){
								iCurrentImage += 1;
							}
						}
					}
					if(bRead){
						item.iImage += 1;
					}
					item.iItem    = m_listView.InsertItem(&item);
					item.mask     = LVIF_TEXT;

					TCHAR buf[512];

					if(!subtitle.url.empty()){
						if(subtitle.read_page > 0){
							_stprintf_s(buf, _T("%d"), subtitle.read_page);
							item.pszText  = buf;
							item.iSubItem = 1;
							m_listView.SetItem(&item);
						}
					}
					m_subtitle_list.push_back(subtitle);
					if(subtitle.page > 0){
						_stprintf_s(buf, _T("%d"), subtitle.page);
						item.pszText  = buf;
						item.iSubItem = 2;
						m_listView.SetItem(&item);
					}

					item.pszText  = const_cast<LPTSTR>(subtitle.last_visit_date.c_str());
					item.iSubItem = 3;
					m_listView.SetItem(&item);

					item.pszText  = const_cast<LPTSTR>(subtitle.url.c_str());
					item.iSubItem = 4;
					m_listView.SetItem(&item);
				};
			} while(0);
			if(pstmt_txtmiru_st_link){
				sqlite3_finalize(pstmt_txtmiru_st_link);
			}
			if(pstmt_txtmiru_st_bm){
				sqlite3_finalize(pstmt_txtmiru_st_bm);
			}
		}
		if(index >= 0){
			int count = m_listView.GetItemCount();
			if(count <= index){
				index = count - 1;
			}
			if(!bKeepIndex){
				int next_index = min(index + 2, count - 1);
				if(next_index != index){
					m_listView.EnsureVisible(next_index, FALSE);
				}
			}
			if(iCurrentTitle >= 0){
				if(count <= iCurrentTitle){
					iCurrentTitle = count - 1;
				}
				LVITEM lvitem = {LVIF_IMAGE};
				lvitem.iItem = iCurrentTitle;
				lvitem.iImage = iCurrentImage;
				m_listView.SetItem(&lvitem);
			}
			m_listView.SetItemState(index, LVNI_FOCUSED, LVNI_FOCUSED);
			m_listView.EnsureVisible(index, FALSE);
		} else {
			::SetScrollInfo(m_listView, SB_HORZ, &si, TRUE);
		}
	} while(0);
	m_listView.SetRedraw(TRUE);

	return bRet;
}
