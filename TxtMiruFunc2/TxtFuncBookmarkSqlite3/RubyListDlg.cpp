#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "RubyListDlg.h"
#include "TxtFunc.h"
#include "Text.h"
#include "TxtFuncBookmarkDef.h"
#include "csvtext.h"
#include "MessageBox.h"
#include "TxtMiruTheme.h"
#include <algorithm>
#include <functional>

#define TIMERID_REFRESH         1
#define TIMERID_FILTER          2

CGrRubyListDlg::CGrRubyListDlg(HWND hWnd, void *pDoc) : CGrModelessWnd(hWnd), m_pDoc(pDoc)
{
}

CGrRubyListDlg::~CGrRubyListDlg()
{
}

void CGrRubyListDlg::Show(HWND hWnd)
{
	if(hWnd){
		m_parentWnd = hWnd;
	}
	if(m_hWnd){
		PostRefresh();
	} else {
		::CreateDialogParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_RUBY), m_parentWnd, (DLGPROC)CGrWinCtrl::WindowMapProc, (LPARAM)this);
	}
	if(m_hWnd){
		::ShowWindowAsync(m_hWnd, SW_SHOW);
		::BringWindowToTop(m_hWnd);
	}
}

LRESULT CGrRubyListDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
			std::tstring ini_filename;
			CGrTxtFunc::GetBookmarkFolder(&param, ini_filename);
			ini_filename += _T("/bookmark.ini");
			TCHAR val[2048];
			WINDOWPLACEMENT wndp = {sizeof(WINDOWPLACEMENT)};
			GetWindowPlacement(m_hWnd, &wndp);
			_stprintf_s(val, _T("%d,%d,%d,%d"),
						wndp.rcNormalPosition.left, wndp.rcNormalPosition.top, wndp.rcNormalPosition.right, wndp.rcNormalPosition.bottom
						);
			::WritePrivateProfileString(_T("RubyList"), _T("WindowPos"), val, ini_filename.c_str());
			//
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
	default:
		return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

static int CALLBACK listViewCompare(LPARAM lparam1, LPARAM lparam2, LPARAM lParamSort)
{
	auto *rubyInfoList = reinterpret_cast<std::vector<CGrRubyListDlg::RubyInfo>*>(lParamSort);
	int len = static_cast<signed int>(rubyInfoList->size());
	if(lparam1 < 0 || lparam1 >= len){
		return -1;
	}
	if(lparam2 < 0 || lparam2 >= len){
		return 1;
	}
	const auto &li1 = (*rubyInfoList)[lparam1];
	const auto &li2 = (*rubyInfoList)[lparam2];
	int rc = 0;
	if(rc == 0){
		rc = li1.str.compare(li2.str);
	}
	if(rc == 0){
		rc = li1.ruby.compare(li2.ruby);
	}
	return rc;
}

#include "Shell.h"

BOOL CGrRubyListDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	auto hInst = CGrTxtFunc::GetDllModuleHandle();
	SetWindowPosCenter();
	RECT rc_dialog = {};
	::GetWindowRect(hwnd, &rc_dialog);
	m_minWindowSize.x = rc_dialog.right - rc_dialog.left;
	m_minWindowSize.y = rc_dialog.bottom - rc_dialog.top;

	auto &&param = CGrTxtFunc::Param();
	std::tstring ini_filename;
	CGrTxtFunc::GetBookmarkFolder(&param, ini_filename);
	ini_filename += _T("/bookmark.ini");
	TCHAR val[2048];
	::GetPrivateProfileString(_T("RubyList"), _T("WindowPos"), _T(""), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
	WINDOWPLACEMENT wndp = {sizeof(WINDOWPLACEMENT)};
	GetWindowPlacement(m_hWnd, &wndp);
	CGrCSVText csv;
	csv.AddTail(val);
	int row = csv.GetRowSize() - 1;
	if(row >= 0 && csv.GetColmnSize(row) >= 4){
		RECT rect;
		rect.left   = csv.GetInteger(row, 0, 0);
		rect.top    = csv.GetInteger(row, 1, 0);
		rect.right  = csv.GetInteger(row, 2, 0);
		rect.bottom = csv.GetInteger(row, 3, 0);
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
			int x = w0 / 2 - w / 2;
			int y = h0 / 2 - h / 2;
			wndp.rcNormalPosition.left   = x    ;
			wndp.rcNormalPosition.right  = x + w;
			wndp.rcNormalPosition.top    = y    ;
			wndp.rcNormalPosition.bottom = y + h;
		}
		SetWindowPlacement(m_hWnd, &wndp);
	}
	//
	RECT client_rect;
	::GetClientRect(hwnd, &client_rect);
	m_filterEditBox.Attach(GetDlgItem(m_hWnd, IDC_EDIT_FILTER));
	std::tstring str_filter_name;
	CGrText::LoadString(IDS_FILTER_TEXT, str_filter_name);
	m_filterEditBox.SetBackgroundText(str_filter_name.c_str());
	//

	std::tstring str_text_name;
	CGrText::LoadString(IDS_TITLE_TEXT, str_text_name);
	auto hListWnd = GetDlgItem(m_hWnd, IDC_LIST_RUBY);
	m_listView.Attach(hListWnd);
	LVCOLUMN lvc = {};
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 100;
	lvc.pszText = const_cast<LPTSTR>(str_text_name.c_str());
	lvc.iSubItem = static_cast<int>(RubyListColumn::RLC_STR);
	m_listView.InsertColumn(lvc.iSubItem, &lvc);

	std::tstring str_ruby_name;
	CGrText::LoadString(IDS_TITLE_RUBY, str_ruby_name);
	lvc.pszText = const_cast<LPTSTR>(str_ruby_name.c_str());
	lvc.iSubItem = static_cast<int>(RubyListColumn::RLC_RUBY);
	m_listView.InsertColumn(lvc.iSubItem, &lvc);

	std::tstring str_page_name;
	CGrText::LoadString(IDS_BMH_TOTALPAGE, str_page_name);
	lvc.pszText = const_cast<LPTSTR>(str_page_name.c_str());
	lvc.iSubItem = static_cast<int>(RubyListColumn::RLC_PAGE);
	m_listView.InsertColumn(lvc.iSubItem, &lvc);

	auto styleex = m_listView.GetExtendedListViewStyle();
	styleex |= LVS_EX_FLATSB | LVS_EX_FULLROWSELECT;
	m_listView.SetExtendedListViewStyle(styleex);

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

using CGrDocEachAllProcCallbak = void (__stdcall *)(CGrTxtFuncIEachAllTool *pTool, LPARAM lParam);
using TxtMiruFunc_EachAll = bool (cdecl *)(void *pDocParam, CGrDocEachAllProcCallbak func, LPARAM lParam);

static bool RubyListCompare(const CGrRubyListDlg::RubyInfo& left, const CGrRubyListDlg::RubyInfo& right)
{
	return (left.ruby == right.ruby) ? (left.str < right.str) : (left.ruby < right.ruby);
}
static bool isEqualRubyListCompare(const CGrRubyListDlg::RubyInfo& left, const CGrRubyListDlg::RubyInfo& right)
{
	return (left.str == right.str) && (left.ruby == right.ruby);
}

void __stdcall CGrRubyListDlg::DocEachAllProcCallbak(CGrTxtFuncIEachAllTool *pTool, LPARAM lParam)
{
	if(pTool && pTool->TextType() == static_cast<WORD>(TxtMiru::TextType::RUBY)){
		auto *pDlg = reinterpret_cast<CGrRubyListDlg*>(lParam);
		if (!pDlg) {
			return;
		}
		CGrRubyListDlg::RubyInfo info;
		info.str = pTool->RangeString();
		info.ruby = pTool->String();

		auto lpStr = info.ruby.c_str();
		try {
			if (std::regex_match(lpStr, pDlg->m_ignore_pattern)) {
				return;
			}
		}
		catch (...) {
			return;
		}

		auto &&ruby_list = pDlg->m_rubyInfoList;
		auto it = std::find_if(ruby_list.begin(), ruby_list.end(), [&info](CGrRubyListDlg::RubyInfo &left){
			return isEqualRubyListCompare(left, info);
		});
		if(it == ruby_list.end()){
			info.page_list.push_back(pTool->Page());
			ruby_list.push_back(info);
			std::sort(ruby_list.begin(), ruby_list.end(), RubyListCompare);
		} else {
			int page = pTool->Page();
			const auto &page_list = it->page_list;
			auto page_it = std::find(page_list.begin(), page_list.end(), page);
			if(page_it == page_list.end()){
				it->page_list.push_back(page);
			}
		}
	}
}

void CGrRubyListDlg::SetFilter(std::tstring &filtertext)
{
	m_listView.DeleteAllItems();
	LVITEM item = {};
	for(const auto &info : m_rubyInfoList){
		if (!filtertext.empty()
			&& info.str .find(filtertext) == std::string::npos
			&&  info.ruby.find(filtertext) == std::string::npos
			) {
			continue;
		}

		item.iItem    = m_listView.GetItemCount();
		item.mask     = LVIF_TEXT | LVIF_PARAM;
		item.iSubItem = static_cast<int>(RubyListColumn::RLC_STR);
		item.lParam   = reinterpret_cast<LPARAM>(&info);
		item.pszText  = const_cast<LPTSTR>(info.str.c_str());
		item.iItem    = m_listView.InsertItem(&item);
		item.mask     = LVIF_TEXT;
		item.iSubItem = static_cast<int>(RubyListColumn::RLC_RUBY);
		item.pszText  = const_cast<LPTSTR>(info.ruby.c_str());
		m_listView.SetItem(&item);
		{
			TCHAR sep[2] = _T("");
			std::tstring page_str;
			for(const auto &page : info.page_list){
				TCHAR buf[100];
				_stprintf_s(buf, _T("%s%d"), sep, page);
				page_str += buf;
				sep[0] = _T(',');
			}
			item.iSubItem = static_cast<int>(RubyListColumn::RLC_PAGE);
			item.pszText  = const_cast<LPTSTR>(page_str.c_str());
			m_listView.SetItem(&item);
		}
	}
}

void CGrRubyListDlg::Refresh()
{
	if(!m_hWnd){ return; }

	try {
		const auto &param = CGrTxtFunc::Param();
		TCHAR val[2048];
		param.GetText(CGrTxtFuncIParam::TextType::RubyListIgnore, val, _countof(val));
		m_ignore_pattern = std::wregex(val, std::regex_constants::icase);
	} catch(...){}
	
	auto hListWnd = GetDlgItem(m_hWnd, IDC_LIST_RUBY);
	if(!hListWnd){ return; }
	int top = m_listView.GetTopIndex();
	m_listView.DeleteAllItems();
	m_rubyInfoList.clear();
	m_rubyInfoList.shrink_to_fit();
	//
	auto func = reinterpret_cast<TxtMiruFunc_EachAll>(GetProcAddress(GetModuleHandle(NULL), "TxtMiruFunc_EachAll"));
	if(!func){
		return;
	}
	func(m_pDoc, DocEachAllProcCallbak, reinterpret_cast<LPARAM>(this));

	LVITEM item = {};
	for(const auto &info : m_rubyInfoList){
		item.iItem    = m_listView.GetItemCount();
		item.mask     = LVIF_TEXT | LVIF_PARAM;
		item.iSubItem = static_cast<int>(RubyListColumn::RLC_STR);
		item.lParam   = reinterpret_cast<LPARAM>(&info);
		item.pszText  = const_cast<LPTSTR>(info.str.c_str());
		item.iItem    = m_listView.InsertItem(&item);
		item.mask     = LVIF_TEXT;
		item.iSubItem = static_cast<int>(RubyListColumn::RLC_RUBY);
		item.pszText  = const_cast<LPTSTR>(info.ruby.c_str());
		m_listView.SetItem(&item);
		{
			TCHAR sep[2] = _T("");
			std::tstring page_str;
			for(const auto &page : info.page_list){
				TCHAR buf[100];
				_stprintf_s(buf, _T("%s%d"), sep, page);
				page_str += buf;
				sep[0] = _T(',');
			}
			item.iSubItem = static_cast<int>(RubyListColumn::RLC_PAGE);
			item.pszText  = const_cast<LPTSTR>(page_str.c_str());
			m_listView.SetItem(&item);
		}
	}
	//
	top += m_listView.GetCountPerPage()-1;
	m_listView.EnsureVisible(top, FALSE);
	::InvalidateRect(hListWnd, NULL, FALSE);
	::UpdateWindow(hListWnd);
}

void CGrRubyListDlg::endDialog(UINT id)
{
	::ShowWindow(m_hWnd, SW_HIDE);
	::SendMessage(m_parentWnd, WM_COMMAND, TxtDocMessage::UPDATE_CMD_UI, 0);
}

void CGrRubyListDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if(m_filterEditBox == hwndCtl){
		switch(codeNotify){
		case EN_CHANGE:
			KillTimer(m_hWnd, TIMERID_FILTER);
			SetTimer(m_hWnd, TIMERID_FILTER, 100, nullptr);
			break;
		}
	}
	switch(id){
	case IDOK:
	case IDCANCEL:
		endDialog(id);
		break;
	}
}

struct RubyListCompareParam {
	int column;
	bool bAsc;
};
static int CALLBACK rubyListCompare(LPARAM lp1, LPARAM lp2, LPARAM param)
{
	auto *pParam = reinterpret_cast<RubyListCompareParam*>(param);
	const auto &info1 = *reinterpret_cast<const CGrRubyListDlg::RubyInfo*>(lp1);
	const auto &info2 = *reinterpret_cast<const CGrRubyListDlg::RubyInfo*>(lp2);
	int ret = 0;
	switch(static_cast<CGrRubyListDlg::RubyListColumn>(pParam->column)){
	case CGrRubyListDlg::RubyListColumn::RLC_STR:
		ret = _tcscmp(info1.str.c_str(), info2.str.c_str());
		if(ret == 0){
			ret = _tcscmp(info1.ruby.c_str(), info2.ruby.c_str());
		}
		if(ret == 0){
			ret = info1.page_list[0] - info2.page_list[0];
		}
		break;
	case CGrRubyListDlg::RubyListColumn::RLC_RUBY:
		ret = _tcscmp(info1.ruby.c_str(), info2.ruby.c_str());
		if(ret == 0){
			ret = _tcscmp(info1.str.c_str(), info2.str.c_str());
		}
		if(ret == 0){
			ret = info1.page_list[0] - info2.page_list[0];
		}
		break;
	case CGrRubyListDlg::RubyListColumn::RLC_PAGE:
		ret = info1.page_list[0] - info2.page_list[0];
		if(ret == 0){
			ret = _tcscmp(info1.str.c_str(), info2.str.c_str());
		}
		if(ret == 0){
			ret = _tcscmp(info1.ruby.c_str(), info2.ruby.c_str());
		}
		break;
	}
	if(pParam->bAsc){
		return ret;
	} else {
		return -ret;
	}
}

LRESULT CGrRubyListDlg::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	if(idFrom == IDC_LIST_RUBY){
		switch(lpnmhdr->code){
		case NM_DBLCLK:
			{
				auto lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(lpnmhdr);
				if (lpnmitem) {
					auto* pruby_info = reinterpret_cast<RubyInfo*>(m_listView.GetParam(lpnmitem->iItem));
					if (pruby_info) {
						FORWARD_WM_COMMAND(m_parentWnd, TxtDocMessage::GOTO_NPAGE, m_hWnd, pruby_info->page_list[0], SendMessage);
					}
				}
			}
			break;
		case LVN_COLUMNCLICK:
			{
				auto lpnmlist = reinterpret_cast<LPNMLISTVIEW>(lpnmhdr);
				if (lpnmlist && lpnmlist->iSubItem >= 0 && lpnmlist->iSubItem < _countof(m_bAsc)) {
					m_bAsc[lpnmlist->iSubItem] = !m_bAsc[lpnmlist->iSubItem];
					RubyListCompareParam param = {
						lpnmlist->iSubItem, m_bAsc[lpnmlist->iSubItem]
					};
					m_listView.SortItems(rubyListCompare, reinterpret_cast<LPARAM>(&param));
				}
				break;
			}
		}
	}
	return TRUE;
}

static void setDlgItemPos(HWND hWnd, UINT id, int x, int y)
{
	auto childWindow = GetDlgItem(hWnd, id);
	if (childWindow) {
		RECT rect;
		GetWindowRect(childWindow, &rect);
		::ScreenToClient(hWnd, reinterpret_cast<LPPOINT>(&rect));
		if (y < 0) { y = rect.top; }
		if (x < 0) { x = rect.left; }
		// 常に右に表示されるようにします。
		SetWindowPos(childWindow, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOZORDER);
	}
}
#define PADDING 10
void CGrRubyListDlg::setWindowSize(int cx, int cy)
{
	if(cx <= 0 || cy <= 0){
		return;
	}
	if (m_filterEditBox.GetWnd()) {
		RECT client_rect = { 1,1,cx - 1,cy - 1 }; // 1px 隙間を追加
		SetWindowPosTop(m_filterEditBox, &client_rect);
		client_rect.top += 1;

		auto hListWnd = GetDlgItem(m_hWnd, IDC_LIST_RUBY);
		SetWindowPosAllClient(hListWnd, &client_rect);
	}
}

void CGrRubyListDlg::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
		return;
	}
	if(!GetDlgItem(m_hWnd, IDC_LIST_RUBY)){
		return;
	}
	setWindowSize(cx, cy);
}

void CGrRubyListDlg::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	lpMinMaxInfo->ptMinTrackSize = m_minWindowSize;
}

BOOL CGrRubyListDlg::IsDialogMessage(HWND hwnd, MSG &msg)
{
	if(::IsDialogMessage(m_hWnd, &msg)){
		return TRUE;
	}
	return FALSE;
}

void CGrRubyListDlg::PostRefresh()
{
	KillTimer(m_hWnd, TIMERID_REFRESH);
	SetTimer(m_hWnd, TIMERID_REFRESH, 100, nullptr);
}

void CGrRubyListDlg::OnTimer(HWND hwnd, UINT id)
{

	switch(id){
	case TIMERID_REFRESH:
		{
			KillTimer(m_hWnd, TIMERID_REFRESH);
			Refresh();
		}
		break;
	case TIMERID_FILTER:
		{
			KillTimer(m_hWnd, TIMERID_FILTER);
			std::tstring str;
			m_filterEditBox.GetText(str);
			SetFilter(str);
		}
		break;
	}
}
