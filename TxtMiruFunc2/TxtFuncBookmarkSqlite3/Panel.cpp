#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "Panel.h"
#include "MemDC.h"
#include "TxtFuncIParam.h"
#include "TxtMiruTheme.h"
#include "Text.h"

#include <vector>
#define MOUSEEVENTF_FROMTOUCH 0xFF515700
void CGrPanelWnd_SetTouchMenu(HMENU hMenu)
{
	const auto &param = CGrTxtFunc::Param();
	if(param.GetBoolean(CGrTxtFuncIParam::PointsType::TouchMenu)){
		TxtMiruTheme_SetTouchMenu(hMenu);
	}
}

CGrListViewTxtMiru::CGrListViewTxtMiru() : CGrListView(){}
LRESULT CGrListViewTxtMiru::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_UPDATEUISTATE:
		{
			// Disable focus rectangles by setting or masking out the flag where appropriate.
			switch (LOWORD(wParam))
			{
			case UIS_SET:
				wParam |= UISF_HIDEFOCUS << 16;
				break;
			case UIS_CLEAR:
			case UIS_INITIALIZE:
				wParam &= ~(UISF_HIDEFOCUS << 16);
				break;
			}
		}
		break;
	case WM_ERASEBKGND: return 1;
	case WM_SETREDRAW:
		m_bRedraw = (wParam == 1);
		break;
	case WM_PAINT:
		if(m_bRedraw){
			PAINTSTRUCT ps;
			auto hdc = ::BeginPaint(hWnd, &ps);
			{
				CGrMemDC memdc(hdc, ps.rcPaint);
				auto hHeader = GetHeader();
				if(hHeader){
					RECT headerRect;
					GetWindowRect(hHeader, &headerRect);
					POINT pos;
					pos.x = headerRect.left;
					ScreenToClient(hWnd, &pos);
					SetWindowOrgEx(memdc, -pos.x, 0, NULL);
					SendMessage(hHeader, WM_ERASEBKGND, reinterpret_cast<WPARAM>(static_cast<HDC>(memdc)), 0);
					SendMessage(hHeader, WM_PAINT, reinterpret_cast<WPARAM>(static_cast<HDC>(memdc)), 0);
					ValidateRect(hHeader, &ps.rcPaint);
					SetWindowOrgEx(memdc, 0,0, NULL);
					RECT rcHeader;
					GetClientRect(hHeader, &rcHeader);
					ExcludeClipRect(memdc, rcHeader.left, rcHeader.top, rcHeader.right, rcHeader.bottom);
					CGrListView::WndProc(hWnd, uMsg, reinterpret_cast<WPARAM>(static_cast<HDC>(memdc)), 0);
				}
			}
			if(m_bDragMode){
				RECT rect;
				GetClientRect(hWnd, &rect);
				auto oldclr = ::SetBkColor(hdc, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
				if(m_iInsertPos >= 0){
					int cnt = GetItemCount();
					if(m_iInsertPos >= cnt){
						GetItemRect(cnt-1, &rect, LVIR_BOUNDS);
						rect.top = rect.bottom;
					} else {
						GetItemRect(m_iInsertPos, &rect, LVIR_BOUNDS);
						rect.bottom = rect.top;
					}
					rect.bottom += 2;
					::SetBkColor(hdc, TxtMiruTheme_GetSysColor(COLOR_WINDOWFRAME));
					::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
					::SetBkColor(hdc, oldclr);
				}
			}
			::EndPaint(hWnd, &ps);
			return 0;
		}
		break;
	}
	return CGrListView::WndProc(hWnd, uMsg, wParam, lParam);
}

CGrTreeListViewTxtMiru::CGrTreeListViewTxtMiru() : CGrTreeListView(){}
LRESULT CGrTreeListViewTxtMiru::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_UPDATEUISTATE:
		{
			// Disable focus rectangles by setting or masking out the flag where appropriate.
			switch (LOWORD(wParam))
			{
			case UIS_SET:
				wParam |= UISF_HIDEFOCUS << 16;
				break;
			case UIS_CLEAR:
			case UIS_INITIALIZE:
				wParam &= ~(UISF_HIDEFOCUS << 16);
				break;
			}
		}
		break;
	}
	return CGrTreeListView::WndProc(hWnd, uMsg, wParam, lParam);
}

CGrPanelWnd::CGrPanelWnd(CGrBookListDlg &dlg) : m_booklistDlg(dlg)
{
}

CGrPanelWnd::~CGrPanelWnd()
{
}

// ウィンドウ・クラスの登録
BOOL CGrPanelWnd::InitApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm)
{
	WNDCLASSEX wc = {sizeof(WNDCLASSEX)};

	wc.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
	wc.style         = CS_DBLCLKS;
	wc.lpszClassName = szClassNm;
	wc.hInstance     = hInst;
	wc.lpfnWndProc   = fnWndProc;

	return RegisterClassEx(&wc);
}
bool CGrPanelWnd::Create(HWND hParent, HIMAGELIST hImg)
{
	auto hInst = CGrTxtFunc::GetDllModuleHandle();
	InitApp(hInst, CGrWinCtrl::WindowMapProc, _T("Panel"));
	auto hWnd = CreateWindowEx(
		WS_EX_CONTROLPARENT,
		_T("Panel"),
		NULL,
		WS_CHILD | WS_BORDER,
		0, 0, 100, 100,
		hParent,
		NULL,
		hInst,
		this);
	if(!hWnd){
		return false;
	}
	if(!Attach(hWnd)){
		return false;
	}
	auto hListWnd = CreateWindow(_T("syslistview32"),
								 NULL,
								 WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ALIGNLEFT | WS_TABSTOP,
								 0,0,10,10,m_hWnd,reinterpret_cast<HMENU>(listwnd_id), hInst,NULL);
	m_listView.Attach(hListWnd);
	m_listView.SetImageList(hImg, LVSIL_SMALL);
	m_listView.SetExtendedListViewStyle(m_listView.GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);

	auto &toolBar = m_coolBar.GetToolBar();
	toolBar.Create(m_hWnd, hInst, toolbar_id);
	toolBar.ModifyStyle(TBSTYLE_ALTDRAG|CCS_ADJUSTABLE, 0);
	SendMessage(toolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);
	SendMessage(toolBar, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(hImg));
	TBBUTTON tbb[] = {
		//iBitmap  idCommand      fsState          fsStyle                         bReserved[2]  dwData iString
		{0       , 0            , 0              , TBSTYLE_SEP                   , 0, 0        , 0    , 0                },
	};
	TBADDBITMAP tb = {0};
	toolBar.AddButtons(tbb, sizeof(tbb)/sizeof(TBBUTTON), tb);
	toolBar.SetButtons(_T("0"));
	m_coolBar.Create(m_hWnd, CGrTxtFunc::GetDllModuleHandle(), coolbar_id, toolbar_id);


	// LVS_EX_DOUBLEBUFFER にすると focus時のアイコンがおかしくなるので
	//  LVS_EX_DOUBLEBUFFER をやめるか SetWindowTheme で Theme を変更のどちらかになる?
	//  取りあえず SetWindowTheme で変更にしてみる
	SendMessage(m_listView, WM_CHANGEUISTATE, MAKELONG(UIS_SET, UISF_HIDEFOCUS), 0);
	TxtMiruTheme_SetWindowTheme(m_listView, _T("explorer"), NULL);
	ListView_SetBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
	ListView_SetTextColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
	ListView_SetTextBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));

	return true;
}

// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
LRESULT CGrPanelWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_COMMAND      , OnCommand      );
		HANDLE_MSG(hWnd, WM_NOTIFY       , OnNotify       );
		HANDLE_MSG(hWnd, WM_SIZE         , OnSize         );
	case WM_MEASUREITEM:
		TxtMiruTheme_MenuMeasureItem(hWnd, lParam);
		break;
	case WM_DRAWITEM:
		TxtMiruTheme_MenuDrawItem(hWnd, lParam);
		break;
	case WM_SETTINGCHANGE:
		if (TxtMiruTheme_IsImmersiveColorSet(lParam)) {
			ListView_SetBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
			ListView_SetTextColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
			ListView_SetTextBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
			TxtMiruTheme_UpdateDarkMode(m_hWnd);
			InvalidateRect(m_hWnd, nullptr, TRUE);
		}
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CGrPanelWnd::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
}
LRESULT CGrPanelWnd::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
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
	}
	return FALSE;
}
void CGrPanelWnd::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
		return;
	}
	RECT client_rect = {0,0,cx,cy};
	if(m_coolBar){
		FORWARD_WM_SIZE(m_coolBar, SIZE_RESTORED, cx, cy, SendMessage);
		RECT rect;
		GetWindowRect(m_coolBar, &rect);
		client_rect.top += rect.bottom - rect.top + GetSystemMetrics(SM_CYEDGE);
	}
	if(m_listView){
		SetWindowPosAllClient(m_listView, &client_rect);
	}
}

int CGrPanelWnd::GetFocusedItem() const
{
	LVITEM item = { LVIF_PARAM };
	item.iItem = -1;
	while((item.iItem=m_listView.GetNextItem(item.iItem, LVNI_ALL | LVNI_FOCUSED))!=-1){
		break;
	}
	return item.iItem;
}

int CGrPanelWnd::GetColumnCount() const
{
	return Header_GetItemCount(m_listView.GetHeader());
}
int CGrPanelWnd::GetColumnWidth(int iCol) const
{
	return m_listView.GetColumnWidth(iCol);
}
void CGrPanelWnd::SetColumnWidth(int iCol, int cx)
{
	m_listView.SetColumnWidth(iCol, cx);
}

void CGrPanelWnd::InsertColumn(UINT id, int iColumn, int iWidth, int iFormat)
{
	LVCOLUMN lvc = {};
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = iFormat;
	//
	std::tstring str_name;
	CGrText::LoadString(id, str_name);
	lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
	lvc.iSubItem = iColumn;
	lvc.cx = iWidth;
	m_listView.InsertColumn(lvc.iSubItem, &lvc);
}
//////////////////////////////////////////////////////////////////////////////////////////
CGrTreePanelWnd::CGrTreePanelWnd(CGrBookListDlg &dlg) : m_booklistDlg(dlg)
{
}

CGrTreePanelWnd::~CGrTreePanelWnd()
{
}

// ウィンドウ・クラスの登録
BOOL CGrTreePanelWnd::InitApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm)
{
	WNDCLASSEX wc = {sizeof(WNDCLASSEX)};

	wc.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
	wc.style         = CS_DBLCLKS;
	wc.lpszClassName = szClassNm;
	wc.hInstance     = hInst;
	wc.lpfnWndProc   = fnWndProc;

	return RegisterClassEx(&wc);
}
bool CGrTreePanelWnd::Create(HWND hParent, HIMAGELIST hImg, HIMAGELIST hTreeImg)
{
	auto hInst = CGrTxtFunc::GetDllModuleHandle();
	InitApp(hInst, CGrWinCtrl::WindowMapProc, _T("TreePanel"));
	auto hWnd = CreateWindowEx(
		WS_EX_CONTROLPARENT,
		_T("TreePanel"),
		NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		0, 0, 100, 100,
		hParent,
		NULL,
		hInst,
		this);
	if(!hWnd){
		return false;
	}
	if(!Attach(hWnd)){
		return false;
	}
	auto hListWnd = CreateWindow(_T("syslistview32"),
								 NULL,
								 WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ALIGNLEFT | WS_TABSTOP,
								 0,0,10,10,m_hWnd,reinterpret_cast<HMENU>(listwnd_id), hInst,NULL);
	m_listView.Attach(hListWnd);
	m_listView.SetImageList(hImg    , LVSIL_SMALL);
	m_listView.SetImageList(hTreeImg, LVSIL_STATE);
	m_listView.SetExtendedListViewStyle(m_listView.GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);

	auto &&toolBar = m_coolBar.GetToolBar();
	toolBar.Create(m_hWnd, hInst, toolbar_id);
	toolBar.ModifyStyle(TBSTYLE_ALTDRAG|CCS_ADJUSTABLE, 0);
	SendMessage(toolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);
	SendMessage(toolBar, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(hImg));
	TBBUTTON tbb[] = {
		//iBitmap  idCommand      fsState          fsStyle                         bReserved[2]  dwData iString
		{0       , 0            , 0              , TBSTYLE_SEP                   , 0, 0        , 0    , 0                },
	};
	TBADDBITMAP tb = {};
	toolBar.AddButtons(tbb, sizeof(tbb)/sizeof(TBBUTTON), tb);
	toolBar.SetButtons(_T("0"));
	m_coolBar.Create(m_hWnd, CGrTxtFunc::GetDllModuleHandle(), coolbar_id, toolbar_id);

	// LVS_EX_DOUBLEBUFFER にすると focus時のアイコンがおかしくなるので
	//  LVS_EX_DOUBLEBUFFER をやめるか SetWindowTheme で Theme を変更のどちらかになる?
	//  取りあえず SetWindowTheme で変更にしてみる
	SendMessage(m_listView, WM_CHANGEUISTATE, MAKELONG(UIS_SET, UISF_HIDEFOCUS), 0);
	TxtMiruTheme_SetWindowTheme(m_listView, _T("explorer"), NULL);
	ListView_SetBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
	ListView_SetTextColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
	ListView_SetTextBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));

	return true;
}

// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
LRESULT CGrTreePanelWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_COMMAND      , OnCommand      );
		HANDLE_MSG(hWnd, WM_NOTIFY       , OnNotify       );
		HANDLE_MSG(hWnd, WM_SIZE         , OnSize         );
	case WM_MEASUREITEM:
		TxtMiruTheme_MenuMeasureItem(hWnd, lParam);
		break;
	case WM_DRAWITEM:
		TxtMiruTheme_MenuDrawItem(hWnd, lParam);
		break;
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
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CGrTreePanelWnd::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
}
LRESULT CGrTreePanelWnd::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	if (lpnmhdr->code == NM_CUSTOMDRAW && TxtMiruTheme_IsDarkThemeActive()) {
		if ((idFrom == coolbar_id || idFrom == toolbar_id)) {
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
		}
	}
	return FALSE;
}
void CGrTreePanelWnd::OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
		return;
	}
	RECT client_rect = {0,0,cx,cy};
	if(m_coolBar){
		FORWARD_WM_SIZE(m_coolBar, SIZE_RESTORED, cx, cy, SendMessage);
		RECT rect;
		GetWindowRect(m_coolBar, &rect);
		client_rect.top += rect.bottom - rect.top + GetSystemMetrics(SM_CYEDGE);
	}
	if(m_listView){
		SetWindowPosAllClient(m_listView, &client_rect);
	}
}

int CGrTreePanelWnd::GetFocusedItem() const
{
	LVITEM item = { LVIF_PARAM };
	item.iItem = -1;
	while((item.iItem=m_listView.GetNextItem(item.iItem, LVNI_ALL | LVNI_FOCUSED))!=-1){
		break;
	}
	return item.iItem;
}

int CGrTreePanelWnd::GetColumnCount() const
{
	return Header_GetItemCount(m_listView.GetHeader());
}
int CGrTreePanelWnd::GetColumnWidth(int iCol) const
{
	return m_listView.GetColumnWidth(iCol);
}
void CGrTreePanelWnd::SetColumnWidth(int iCol, int cx)
{
	m_listView.SetColumnWidth(iCol, cx);
}
