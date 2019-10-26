#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "LinkDlg.h"
#include "TxtFunc.h"
#include "Text.h"
#include "Image.h"
#include "PictRendererMgr.h"
#include "TxtFuncBookmarkDef.h"
#include "MessageBox.h"
#include "TxtMiruTheme.h"

union SubtitlePos {
	struct  {
		int iLinkShow;
		int iStayOn  ;
		int left     ;
		int top      ;
		int width    ;
		int height   ;
		int iAutoPos ;
	};
	int data[7];
} spos;

CGrLinkDlg::CGrLinkDlg(HWND hWnd, int mode, CGrTxtFuncIBookmark *pBook, CGrTxtFuncISubTitle *pSubtitle)
: CGrModelessWnd(hWnd), m_mode(mode),m_pBookmark(pBook), m_pSubTitle(pSubtitle)
{
}

CGrLinkDlg::~CGrLinkDlg()
{
}

void CGrLinkDlg::Show(HWND hWnd)
{
	switch(m_mode){
	case 1: ShowBookmark(hWnd, true); break;
	case 2: ShowSubtitle(hWnd, true); break;
	case 0: ShowNormal(hWnd); break;
	default:
		if(m_hWnd){
			ShowWindow(m_hWnd, SW_HIDE);
		}
		break;
	}
}

void CGrLinkDlg::drawItem(DRAWITEMSTRUCT *pdi, RECT &clinetRect)
{
	HDC hDC        = pdi->hDC   ;
	int itemID     = pdi->itemID;
	auto &&item_rc = pdi->rcItem;

	LV_ITEM lv_item = {LVIF_PARAM|LVIF_INDENT, itemID};
	m_listView.GetItem(&lv_item);
	int index = lv_item.lParam;
	if(index >= 0 && index < static_cast<signed int>(m_linkInfoList.size())){
		auto hfontOld = static_cast<HFONT>(SelectObject(hDC, m_titleFont));
		auto &&li = m_linkInfoList[index];
		RECT rc=item_rc;
		int height = item_rc.bottom - item_rc.top;
		int icon_width = height;
		rc.bottom = rc.top + height;
		//
		auto oldbkcolor = ::SetBkColor  (hDC, TxtMiruTheme_GetSysColor(COLOR_WINDOW    ));
		auto oldtxcolor = ::SetTextColor(hDC, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
		UINT iconFmt = ILD_NORMAL;
		int icon = 5;
		if(li.bBookmark){
			icon = 4;
		} else if(li.filename.empty()){
			icon = 5;
		} else {
			icon = 6;
		}
		SetBkMode(hDC, OPAQUE);
		item_rc.left += icon_width;
		if((pdi->itemState & ODS_FOCUS  ) == ODS_FOCUS ){
			iconFmt = ILD_FOCUS;
			::SetBkColor  (hDC, TxtMiruTheme_GetSysColor(COLOR_HIGHLIGHT    ));
			::SetTextColor(hDC, TxtMiruTheme_GetSysColor(COLOR_HIGHLIGHTTEXT));
		} else if((pdi->itemState & ODS_SELECTED) == ODS_SELECTED){
			iconFmt = ILD_FOCUS;
			::SetBkColor  (hDC, TxtMiruTheme_GetSysColor(COLOR_HIGHLIGHT    ));
			::SetTextColor(hDC, TxtMiruTheme_GetSysColor(COLOR_HIGHLIGHTTEXT));
		} else {
		}
		::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &item_rc, NULL, 0, NULL);
		SetBkMode(hDC, TRANSPARENT);
		height /= 2;
		rc.top += 2;
		rc.bottom = rc.top + height;
		ImageList_Draw(m_hImg, icon, hDC, rc.left, rc.top, iconFmt);
		rc.left += icon_width + 5;
		rc.left += lv_item.iIndent * icon_width / 2;
		if(li.top){
			DrawText(hDC, m_pBookmark->GetParamString(STR_PARAMTYPE_TITLE), -1, &rc, DT_LEFT | DT_SINGLELINE | DT_BOTTOM | DT_END_ELLIPSIS);
		} else if(li.filename.empty() && li.page <= 0){
			DrawText(hDC, li.name.c_str(), -1, &rc, DT_LEFT | DT_SINGLELINE | DT_BOTTOM | DT_END_ELLIPSIS);
		} else {
			DrawText(hDC, li.name.c_str(), -1, &rc, DT_LEFT | DT_SINGLELINE | DT_BOTTOM | DT_END_ELLIPSIS);
		}

		SelectObject(hDC, m_pageFont);
		rc.top += height;
		rc.bottom = rc.top + height;
		if(!li.filename.empty()){
			auto pos = li.filename.find(_T("|"));
			if(pos == std::string::npos){
				DrawText(hDC, li.filename.c_str(), -1, &rc, DT_LEFT | DT_SINGLELINE | DT_TOP | DT_END_ELLIPSIS);
			}
		} else if(!li.top && li.page > 0){
			std::tstring str;
			CGrText::FormatMessage(CGrTxtFunc::GetDllModuleHandle(), str, IDS_LIST_PAGE, li.page);
			DrawText(hDC, str.c_str(), -1, &rc, DT_LEFT | DT_SINGLELINE | DT_TOP | DT_END_ELLIPSIS);
		} else {
			std::tstring top_page_str;
			CGrText::LoadString(IDS_TOPPAGE, top_page_str); // 表紙
			DrawText(hDC, top_page_str.c_str(), -1, &rc, DT_LEFT | DT_SINGLELINE | DT_TOP | DT_END_ELLIPSIS);
		}
		::SetBkColor  (hDC, oldbkcolor);
		::SetTextColor(hDC, oldtxcolor);
		SelectObject(hDC, hfontOld);
	}
}

LRESULT CGrLinkDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG   , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_COMMAND      , OnCommand      );
		HANDLE_MSG(hWnd, WM_TIMER        , OnTimer        );
		HANDLE_MSG(hWnd, WM_NOTIFY       , OnNotify       );
		HANDLE_MSG(hWnd, WM_SIZE         , OnSize         );
		HANDLE_MSG(hWnd, WM_GETMINMAXINFO, OnGetMinMaxInfo);
	case WM_SETTINGCHANGE:
		if (TxtMiruTheme_IsImmersiveColorSet(lParam)) {
			if (m_listView) {
				ListView_SetBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
				ListView_SetTextColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
				ListView_SetTextBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
			}
			TxtMiruTheme_UpdateDarkMode(m_hWnd);
			InvalidateRect(m_hWnd, nullptr, TRUE);
		}
		break;
	case WM_DESTROY:
		{
			auto &&param = CGrTxtFunc::Param();
			if(!param.GetBoolean(CGrTxtFuncIParam::PointsType::SaveWindowSize)){
				PostQuitMessage(0);
				break;
			}
			int window_pos[5];
			//
			param.GetPoints(CGrTxtFuncIParam::PointsType::BookmarkPos, window_pos, sizeof(window_pos)/sizeof(int));
			window_pos[0] = m_bBookmark ? 1 : 0;
			param.SetPoints(CGrTxtFuncIParam::PointsType::BookmarkPos, window_pos, sizeof(window_pos)/sizeof(int));
			//
			param.GetPoints(CGrTxtFuncIParam::PointsType::SubtitlePos, window_pos, sizeof(window_pos)/sizeof(int));
			window_pos[0] = m_bSubtitle ? 1 : 0;
			param.SetPoints(CGrTxtFuncIParam::PointsType::SubtitlePos, window_pos, sizeof(window_pos)/sizeof(int));
			//
			RECT rect;
			SubtitlePos spos;
			::GetWindowRect(m_hWnd, &rect);
			param.GetPoints(CGrTxtFuncIParam::PointsType::LinkPos, spos.data, sizeof(spos)/sizeof(int));
			spos.iStayOn  = m_bStayOn ? 1 : 0;
			spos.left     = rect.left   ;
			spos.top      = rect.top    ;
			spos.width    = rect.right  - rect.left;
			spos.height   = rect.bottom - rect.top;
			spos.iAutoPos = m_bAutoScroll ? 1 : 0;
			param.SetPoints(CGrTxtFuncIParam::PointsType::LinkPos, spos.data, sizeof(spos)/sizeof(int));
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
	case WM_DRAWITEM:
		{
			RECT rect;
			::GetClientRect(hWnd, &rect);
			auto *pdi = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
			drawItem(pdi, rect);
		}
		break;
	case WM_MEASUREITEM:
		{
			HDC   hdc;
			SIZE  size;
			HFONT hfontOld;
			auto lpmis = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);

			hdc = GetDC(hWnd);

			hfontOld = reinterpret_cast<HFONT>(SelectObject(hdc, GetWindowFont(hWnd)));
			GetTextExtentPoint32(hdc, _T("A"), 1, &size);

			lpmis->itemHeight = size.cy * 2 + 10;

			SelectObject(hdc, hfontOld);

			ReleaseDC(hWnd, hdc);
			return 0;
		}
		break;
	default:
		return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

static int CALLBACK listViewCompare(LPARAM lparam1, LPARAM lparam2, LPARAM lParamSort)
{
	auto *linkInfoList = reinterpret_cast<std::vector<CGrLinkDlg::LinkInfo>*>(lParamSort);
	auto len = static_cast<signed int>(linkInfoList->size());
	if(lparam1 < 0 || lparam1 >= len){
		return -1;
	}
	if(lparam2 < 0 || lparam2 >= len){
		return 1;
	}
	auto &&li1 = (*linkInfoList)[lparam1];
	auto &&li2 = (*linkInfoList)[lparam2];
	int page1 = li1.page;
	int page2 = li2.page;
	if(page1 < 0){
		page1 = page2 + 1;
	}
	if(page2 < 0){
		page2 = page1 + 1;
	}
	int rc = page1 - page2;
	if(rc == 0){
		int b1 = li1.bBookmark ? 1 : 0;
		int b2 = li2.bBookmark ? 1 : 0;
		rc = b1 - b2;
		if(rc == 0){
			if(b1){
				rc = li1.name.compare(li2.name);
			} else {
				rc = lparam1 - lparam2;
			}
		}
	}
	return rc;
}

#include "Shell.h"

BOOL CGrLinkDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	auto hInst = CGrTxtFunc::GetDllModuleHandle();
	SetWindowPosCenter();
	RECT rc_dialog = {};
	::GetWindowRect(hwnd, &rc_dialog);
	m_minWindowSize.x = rc_dialog.right - rc_dialog.left;
	m_minWindowSize.y = rc_dialog.bottom - rc_dialog.top;
	auto &&toolBar = m_coolBar.GetToolBar();
	toolBar.Create(hwnd, hInst, toolbar_id);
	{
		TBBUTTON tbb[] = {
			//iBitmap  idCommand                                                  fsState          fsStyle                         bReserved[2]  dwData iString
			{2       , static_cast<int>(TxtFuncBookmark::EventID::IDGOTOPAGE   ), TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_IDGOTOPAGE      },
			{0       , 0                                                        , 0              , TBSTYLE_SEP                   , 0, 0        , 0    , 0                   },
			{0       , static_cast<int>(TxtFuncBookmark::EventID::IDADDBOOKMARK), TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_IDADDBOOKMARK   },
			{1       , static_cast<int>(TxtFuncBookmark::EventID::IDDELBOOKMARK), TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_IDDELBOOKMARK   },
			{3       , static_cast<int>(TxtFuncBookmark::EventID::IDLINKSTAY   ), TBSTATE_ENABLED, TBSTYLE_CHECK  | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_IDLINKSTAY      },
			{4       , static_cast<int>(TxtFuncBookmark::EventID::IDBOOKMARK   ), TBSTATE_ENABLED, TBSTYLE_CHECK  | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_IDBOOKMARK      },
			{5       , static_cast<int>(TxtFuncBookmark::EventID::IDSUBTITLE   ), TBSTATE_ENABLED, TBSTYLE_CHECK  | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_IDSUBTITLE      },
			{7       , static_cast<int>(TxtFuncBookmark::EventID::IDAUTOSCROLL ), TBSTATE_ENABLED, TBSTYLE_CHECK  | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TITLE_AUTOSCROLL},
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
		SendMessage(toolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);
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
		const int icon_num = 8;
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
		bmp.Create(height*icon_num, height);
		{
			{
				auto color = GetSysColor(COLOR_WINDOW);
				int r = GetRValue(color);
				int g = GetGValue(color);
				int b = GetBValue(color);
				{
					int l = height*icon_num * height;
					auto lpRGB = bmp.GetBits();
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
			for(int i=0; i<static_cast<int>(TxtFuncBookmark::ImageIcon::ii_max); ++i){
				TCHAR path[_MAX_PATH] = {};
				_stprintf_s(path, _T("%s\\toolbar_icon\\toolbar_btn%d"), lpDir, i);
				std::tstring image_filename;
				if(pictMgr.GetSupportedFile(image_filename, path)){
					pictMgr.Draw(bmp, i*height, 0, height, height, image_filename.c_str());
				} else {
					_stprintf_s(path, _T("%s\\toolbar_icon\\toolbar_btn%d"), curPath, i);
					if(pictMgr.GetSupportedFile(image_filename, path)){
						pictMgr.Draw(bmp, i*height, 0, height, height, image_filename.c_str());
					}
				}
			}
			ImageList_Add(m_hImg, bmp, NULL);
		}
		SendMessage(toolBar, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(m_hImg));
		TBADDBITMAP tb = {};
		tb.hInst = NULL;
		tb.nID   = NULL;
		toolBar.AddButtons(tbb, sizeof(tbb)/sizeof(TBBUTTON), tb);
		toolBar.SetButtons(_T("0,1,2,3,1,4,1,5,6,7"));
		//
		m_coolBar.Create(m_hWnd, CGrTxtFunc::GetDllModuleHandle(), coolbar_id, toolbar_id);
	}
	auto &&param = CGrTxtFunc::Param();
	SubtitlePos spos;
	int window_pos[5];
	param.GetPoints(CGrTxtFuncIParam::PointsType::BookmarkPos, window_pos, sizeof(window_pos)/sizeof(int));
	m_bBookmark = (window_pos[0] == 1);
	param.GetPoints(CGrTxtFuncIParam::PointsType::SubtitlePos, window_pos, sizeof(window_pos)/sizeof(int));
	m_bSubtitle = (window_pos[0] == 1);
	if(!m_bBookmark && !m_bSubtitle){
		m_bBookmark = true;
		m_bSubtitle = true;
	}
	param.GetPoints(CGrTxtFuncIParam::PointsType::LinkPos, spos.data, sizeof(spos)/sizeof(int));
	m_bStayOn = (spos.iStayOn == 1);
	m_bAutoScroll = (spos.iAutoPos == 1);
	//
	SendMessage(toolBar, TB_CHECKBUTTON, static_cast<WPARAM>(TxtFuncBookmark::EventID::IDBOOKMARK  ), MAKELPARAM(m_bBookmark  , 0));
	SendMessage(toolBar, TB_CHECKBUTTON, static_cast<WPARAM>(TxtFuncBookmark::EventID::IDSUBTITLE  ), MAKELPARAM(m_bSubtitle  , 0));
	SendMessage(toolBar, TB_CHECKBUTTON, static_cast<WPARAM>(TxtFuncBookmark::EventID::IDLINKSTAY  ), MAKELPARAM(m_bStayOn    , 0));
	SendMessage(toolBar, TB_CHECKBUTTON, static_cast<WPARAM>(TxtFuncBookmark::EventID::IDAUTOSCROLL), MAKELPARAM(m_bAutoScroll, 0));

	// モニタ内でウインドウを移動
	MoveWindowInMonitor(spos.left, spos.top, spos.width, spos.height);
	//
	RECT client_rect;
	::GetClientRect(hwnd, &client_rect);

	std::tstring str_page_name;
	CGrText::LoadString(IDS_TITLE_PAGE_NAME, str_page_name);
	auto hListWnd = GetDlgItem(m_hWnd, IDC_LINK_LIST);
	m_listView.Attach(hListWnd);
	LVCOLUMN lvc = {};
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_RIGHT;
	lvc.cx = 0;
	lvc.pszText = const_cast<LPTSTR>(str_page_name.c_str());
	lvc.iSubItem = 0;
	m_listView.InsertColumn(0, &lvc);

	auto styleex = m_listView.GetExtendedListViewStyle();
	styleex |= LVS_EX_FLATSB | LVS_EX_FULLROWSELECT;
	m_listView.SetExtendedListViewStyle(styleex);
	TxtMiruTheme_SetWindowTheme(m_listView, _T("explorer"), NULL);
	ListView_SetBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
	ListView_SetTextColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
	ListView_SetTextBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);
	TxtMiruTheme_UpdateDarkMode(m_hWnd);

	PostRefresh();

	setWindowSize(client_rect.right, client_rect.bottom);

	return TRUE;
}

void CGrLinkDlg::Refresh()
{
	if(!m_hWnd){ return; }
	auto hListWnd = GetDlgItem(m_hWnd, IDC_LINK_LIST);
	if(!hListWnd){ return; }
	int top = m_listView.GetTopIndex();
	m_listView.DeleteAllItems();
	m_linkInfoList.clear();
	m_linkInfoList.shrink_to_fit();
	//
	if(m_bSubtitle){
		int subtitle_len=m_pSubTitle->Count();
		//
		bool bTopPage = true;
		for(int idx=0; idx<subtitle_len; ++idx){
			const auto *pLeaf = m_pSubTitle->GetLeaf(idx);
			int level = pLeaf->GetLevel();
			if(level >= 999){ // ID ページ内リンク用(非表示:NAMEタグ)
				continue;
			}
			if(level < 0 && pLeaf->GetOrder() == 0){
				continue; // カレントファイルへのリンク
			}
			LinkInfo li;
			li.top       = false;
			li.bBookmark = false;
			li.level     = level;
			if(level < 0){
				li.page      = -1;
				li.name      = pLeaf->GetName();
				li.filename  = pLeaf->GetFileName();
			} else {
				li.page = pLeaf->GetPage();
				li.name = pLeaf->GetName();
			}
			if(bTopPage){
				if(li.page != 0){
					LinkInfo li_top;
					li_top.top       = true;
					li_top.bBookmark = false;
					li_top.level     = 0;
					li_top.page      = 0;
					m_linkInfoList.push_back(li_top);
					LVITEM item = {0};
					item.mask     = LVIF_PARAM;
					item.iItem    = m_listView.GetItemCount();
					item.lParam   = m_linkInfoList.size()-1;
					m_listView.InsertItem(&item);
				}
				bTopPage = false;
			}
			m_linkInfoList.push_back(li);
			LVITEM item = {};
			item.mask     = LVIF_PARAM | LVIF_INDENT;
			item.iItem    = m_listView.GetItemCount();
			item.iIndent  = level;
			item.lParam   = m_linkInfoList.size()-1;
			m_listView.InsertItem(&item);
		}
	}
	//
	if(m_bBookmark){
		int bookmark_len = m_pBookmark->Count();
		for(int idx=0; idx<bookmark_len; ++idx){
			auto pleaf = m_pBookmark->GetLeaf(idx);
			LinkInfo li;
			li.bBookmark = true;
			li.page      = pleaf->GetPage();
			li.level     = 0;
			li.name      = pleaf->GetName();
			if(li.page <= 0){
				li.top = true;
			} else {
				li.top = false;
			}
			m_linkInfoList.push_back(li);
			LVITEM item = {};
			item.mask     = LVIF_PARAM;
			item.iItem    = m_listView.GetItemCount();
			item.lParam   = m_linkInfoList.size()-1;
			m_listView.InsertItem(&item);
		}
	}
	//
	top += m_listView.GetCountPerPage()-1;
	m_listView.EnsureVisible(top, FALSE);
	::InvalidateRect(m_listView, NULL, FALSE);
	::UpdateWindow(m_listView);
	m_listView.SortItems(listViewCompare, reinterpret_cast<LPARAM>(&m_linkInfoList));
}

void CGrLinkDlg::endDialog(UINT id)
{
	switch(id){
	case static_cast<int>(TxtFuncBookmark::EventID::IDGOTOPAGE):
	case IDOK:
		{
			LVITEM lv = {LVIF_TEXT};
			lv.iItem = m_listView.GetNextItem(-1, LVNI_SELECTED);
			if(lv.iItem >= 0){
				LV_ITEM lv_item = {LVIF_PARAM, lv.iItem};
				m_listView.GetItem(&lv_item);
				int index = lv_item.lParam;
				if(index >= 0 && index < static_cast<signed int>(m_linkInfoList.size())){
					auto &li = m_linkInfoList[index];
					if(m_bStayOn){
						;
					} else {
						::ShowWindow(m_hWnd, SW_HIDE);
					}
					if(!li.filename.empty()){
						FORWARD_WM_COMMAND(m_parentWnd, TxtDocMessage::OPEN_FILE/*id*/,li.filename.c_str()/*hwndCtl*/, 0/*codeNotify*/, SendMessage);
					} else {
						FORWARD_WM_COMMAND(m_parentWnd, TxtDocMessage::GOTO_NPAGE, m_hWnd, li.page, SendMessage);
					}
					if(m_bStayOn){
						;
					} else {
						::SendMessage(m_parentWnd, WM_COMMAND, TxtDocMessage::UPDATE_CMD_UI, 0);
					}
				}
			}
		}
		break;
	}
}

void CGrLinkDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case static_cast<int>(TxtFuncBookmark::EventID::IDGOTOPAGE):
		endDialog(id);
		break;
	case static_cast<int>(TxtFuncBookmark::EventID::IDLINKSTAY):
		m_bStayOn = (SendMessage(m_coolBar.GetToolBar(), TB_ISBUTTONCHECKED, static_cast<WPARAM>(TxtFuncBookmark::EventID::IDLINKSTAY), 0) != 0);
		break;
	case static_cast<int>(TxtFuncBookmark::EventID::IDBOOKMARK):
		m_bBookmark = (SendMessage(m_coolBar.GetToolBar(), TB_ISBUTTONCHECKED, static_cast<WPARAM>(TxtFuncBookmark::EventID::IDBOOKMARK), 0) != 0);
		Refresh();
		break;
	case static_cast<int>(TxtFuncBookmark::EventID::IDSUBTITLE):
		m_bSubtitle = (SendMessage(m_coolBar.GetToolBar(), TB_ISBUTTONCHECKED, static_cast<WPARAM>(TxtFuncBookmark::EventID::IDSUBTITLE), 0) != 0);
		Refresh();
		break;
	case static_cast<int>(TxtFuncBookmark::EventID::IDAUTOSCROLL):
		m_bAutoScroll = (SendMessage(m_coolBar.GetToolBar(), TB_ISBUTTONCHECKED, static_cast<WPARAM>(TxtFuncBookmark::EventID::IDAUTOSCROLL), 0) != 0);
		break;
	case static_cast<int>(TxtFuncBookmark::EventID::IDADDBOOKMARK):
		CGrTxtFunc::AddBookmark(m_parentWnd);
		break;
	case IDOK:
	case IDCANCEL:
		endDialog(id);
		break;
	case static_cast<int>(TxtFuncBookmark::EventID::IDDELBOOKMARK):
		{
			LVITEM lv = {LVIF_PARAM};
			lv.iItem = m_listView.GetNextItem(-1, LVNI_SELECTED);
			if(lv.iItem < 0){
				break;
			}
			LV_ITEM lv_item = {LVIF_PARAM, lv.iItem};
			m_listView.GetItem(&lv_item);
			int index = lv_item.lParam;
			if(index < 0 || index >= static_cast<signed int>(m_linkInfoList.size())){
				break;
			}
			auto &li = m_linkInfoList[index];

			int top = m_listView.GetTopIndex();
			int page_count = m_listView.GetCountPerPage();
			int row = top + page_count - 1;

			if(!li.bBookmark){
				break;
			}
			int bookmark_len = m_pBookmark->Count();
			if(bookmark_len <= 0){
				break;
			}
			std::tstring message;
			CGrText::FormatMessage(CGrTxtFunc::GetDllModuleHandle(), message, IDS_DELETE_SIORI);
			if(IDYES != CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, message.c_str(), CGrTxtFunc::AppName(), MB_YESNO)){
				break;
			}
			for(int idx=0; idx<bookmark_len; ++idx){
				auto pleaf = m_pBookmark->GetLeaf(idx);
				if(li.page == pleaf->GetPage()){
					CGrTxtFunc::DeleteBookmark(m_parentWnd, idx);
				}
			}
			Refresh();
			m_listView.SetItemState(lv.iItem, LVNI_SELECTED, LVNI_SELECTED);
			int count = m_listView.GetItemCount();
			if(count < row){
				row = count - 1;
			}
			m_listView.EnsureVisible(row, FALSE);
		}
		break;
	}
}

LRESULT CGrLinkDlg::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	if ((idFrom == coolbar_id || idFrom == toolbar_id) && lpnmhdr->code == NM_CUSTOMDRAW && TxtMiruTheme_IsDarkThemeActive()) {
		auto lpNMCustomDraw = reinterpret_cast<LPNMTBCUSTOMDRAW>(lpnmhdr);
		if (lpNMCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT) {
			RECT rect;
			GetClientRect(hWnd, &rect);
			auto hdc = lpNMCustomDraw->nmcd.hdc;
			SetBkColor(hdc, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
			::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, nullptr, 0, nullptr);
			return CDRF_NOTIFYITEMDRAW;
		}
		else if (lpNMCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
			lpNMCustomDraw->clrText = TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT);
			lpNMCustomDraw->clrTextHighlight = TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT);
			if (lpNMCustomDraw->nmcd.uItemState & CDIS_SELECTED) {
				lpNMCustomDraw->clrHighlightHotTrack = TxtMiruTheme_GetSysColor(COLOR_HIGHLIGHT);
				return TBCDRF_HILITEHOTTRACK | TBCDRF_NOEDGES | TBCDRF_USECDCOLORS;
			}
			else if (lpNMCustomDraw->nmcd.uItemState & CDIS_HOT) {
				lpNMCustomDraw->clrHighlightHotTrack = TxtMiruTheme_GetSysColor(COLOR_HOTLIGHT);
				return TBCDRF_HILITEHOTTRACK | TBCDRF_NOEDGES | TBCDRF_USECDCOLORS;
			}
			return TBCDRF_USECDCOLORS;
		}
		return 0;
	} else if(lpnmhdr->hwndFrom == GetDlgItem(m_hWnd, IDC_LINK_LIST)){
		switch(lpnmhdr->code){
		case NM_DBLCLK:
			endDialog(IDOK);
			break;
		}
	}
	return FALSE;
}

static void setDlgItemPos(HWND hWnd, UINT id, int x, int y)
{
	auto childWindow = GetDlgItem(hWnd, id);
	RECT rect;
	GetWindowRect(childWindow, &rect);
	::ScreenToClient(hWnd, reinterpret_cast<LPPOINT>(&rect));
	if(y < 0){ y = rect.top; }
	if(x < 0){ x = rect.left; }
	// 常に右に表示されるようにします。
	SetWindowPos(childWindow, NULL, x, y, 0, 0, SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING|SWP_NOSIZE|SWP_NOZORDER);
}
#define PADDING 10
void CGrLinkDlg::setWindowSize(int cx, int cy)
{
	if(cx <= 0 || cy <= 0){
		return;
	}
	RECT client_rect = {0,0,cx,cy};
	FORWARD_WM_SIZE(m_coolBar, SIZE_RESTORED, cx, cy, SendMessage);
	RECT rect;
	GetWindowRect(m_coolBar, &rect);
	client_rect.top += rect.bottom - rect.top;
	SetWindowPosAllClient(m_listView, &client_rect);
	RECT list_rc;
	GetClientRect(m_listView, &list_rc);
	m_listView.SetColumnWidth(0, list_rc.right-list_rc.left);
}

void CGrLinkDlg::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
		return;
	}
	if(!GetDlgItem(m_hWnd, IDC_LINK_LIST)){
		return;
	}
	setWindowSize(cx, cy);
}

void CGrLinkDlg::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	lpMinMaxInfo->ptMinTrackSize = m_minWindowSize;
}

bool CGrLinkDlg::isShowBookmark()
{
	return m_bBookmark;
}
bool CGrLinkDlg::isShowSubtitle()
{
	return m_bSubtitle;
}

void CGrLinkDlg::ShowNormal(HWND hWnd)
{
	if(hWnd){
		m_parentWnd = hWnd;
	}
	if(m_hWnd){
		PostRefresh();
	} else {
		::CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_LINK), m_parentWnd, reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
	}
	if(m_hWnd){
		::ShowWindowAsync(m_hWnd, SW_SHOW);
		::BringWindowToTop(m_hWnd);
	}
}

void CGrLinkDlg::ShowBookmark(HWND hWnd, bool bVisible)
{
	if(bVisible){
		ShowNormal(hWnd);
	}
	if(m_bBookmark != bVisible){
		m_bBookmark = bVisible;
		SendMessage(m_coolBar.GetToolBar(), TB_CHECKBUTTON, static_cast<WPARAM>(TxtFuncBookmark::EventID::IDBOOKMARK), MAKELPARAM(m_bBookmark, 0));
		PostRefresh();
	}
}
void CGrLinkDlg::ShowSubtitle(HWND hWnd, bool bVisible)
{
	if(bVisible){
		ShowNormal(hWnd);
	}
	if(m_bSubtitle != bVisible){
		m_bSubtitle = bVisible;
		SendMessage(m_coolBar.GetToolBar(), TB_CHECKBUTTON, static_cast<WPARAM>(TxtFuncBookmark::EventID::IDSUBTITLE), MAKELPARAM(m_bSubtitle, 0));
		PostRefresh();
	}
}

void CGrLinkDlg::RefreshCurrentPos()
{
	if(!m_bAutoScroll
	   || (!m_bBookmark && !m_bSubtitle)
	   || m_linkInfoList.size() == 0){
		return;
	}
	int page = CGrTxtFunc::GetCurrentDisplayPage(m_parentWnd);
	bool bSinglePape = CGrTxtFunc::GetSinglePage(m_parentWnd);
	if(!bSinglePape){
		page += 1;
	}

	int cnt = m_listView.GetItemCount();
	int current_pos = 0;
	for(int i=0; i<cnt; ++i){
		LV_ITEM lv_item = {LVIF_PARAM|LVIF_INDENT, i};
		m_listView.GetItem(&lv_item);
		int index = lv_item.lParam;
		if(index >= 0 && index < static_cast<signed int>(m_linkInfoList.size())){
			const auto &li = m_linkInfoList[i];
			if(page < li.page){
				break;
			}
		}
		current_pos = i;
	}
	m_listView.SetItemState(-1, 0, LVNI_FOCUSED|LVIS_SELECTED);
	m_listView.SetItemState(current_pos, LVIS_FOCUSED|LVIS_SELECTED, LVNI_FOCUSED|LVIS_SELECTED);
	m_listView.EnsureVisible(current_pos+1, TRUE);
	m_listView.EnsureVisible(current_pos, TRUE);
}

BOOL CGrLinkDlg::IsDialogMessage(HWND hwnd, MSG &msg)
{
	if(::IsDialogMessage(m_hWnd, &msg)){
		return TRUE;
	}
	return FALSE;
}

#define TIMERID_REFRESH         1
#define TIMERID_UPDATE_PAGE     2

void CGrLinkDlg::PostRefresh()
{
	KillTimer(m_hWnd, TIMERID_REFRESH);
	SetTimer(m_hWnd, TIMERID_REFRESH, 100, nullptr);
}

void CGrLinkDlg::PtstUpdatePage()
{
	KillTimer(m_hWnd, TIMERID_UPDATE_PAGE);
	SetTimer(m_hWnd, TIMERID_UPDATE_PAGE, 100, nullptr);
}

void CGrLinkDlg::OnTimer(HWND hwnd, UINT id)
{

	switch(id){
	case TIMERID_REFRESH:
		{
			KillTimer(m_hWnd, TIMERID_REFRESH);
			Refresh();
		}
		break;
	case TIMERID_UPDATE_PAGE:
		{
			KillTimer(m_hWnd, TIMERID_UPDATE_PAGE);
			RefreshCurrentPos();
		}
		break;
	}
}
