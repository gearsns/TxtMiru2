#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "stltchar.h"
#include "stlutil.h"
#include "TxtConfigDlg.h"
#include "Text.h"
#include "Shell.h"
#include "TxtFunc.h"
#include "PropPageStyle.h"
#include "PropPageBookmark.h"
#include "PropPageHtml.h"
#include "PropPageOption.h"
#include "PropPageKey.h"
#include "PropPageMenu.h"
#include "PropPageRubyList.h"
#include "TxtConfigFunc.h"

//#define __DBG__
#include "Debug.h"

///////////////////////////////////////////////////
//
CGrTxtConfigDlg::CGrTxtConfigDlg()
{
}

CGrTxtConfigDlg::~CGrTxtConfigDlg()
{
}

using CreatePropFunc = CGrPropPage *(PASCAL *)(CGrTxtConfigDlg *pDlg);
static int g_padding = -1;

LRESULT CGrTxtConfigDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG    , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_COMMAND       , OnCommand      );
		HANDLE_MSG(hWnd, WM_CTLCOLOREDIT  , OnCtlColor     );
		HANDLE_MSG(hWnd, WM_CTLCOLORSTATIC, OnCtlColor     );
		HANDLE_MSG(hWnd, WM_SIZE          , OnSize         );
		HANDLE_MSG(hWnd, WM_GETMINMAXINFO , OnGetMinMaxInfo);
	case PSM_CHANGED:
		{
			auto hBtn = GetDlgItem(hWnd, ID_APPLY);
			if(hBtn){
				EnableWindow(hBtn, TRUE);
			}
		}
		break;
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

int CGrTxtConfigDlg::DoModal(HWND hWnd, LPCTSTR lpkeyBindFileName)
{
	m_keyBindFileName = lpkeyBindFileName;
	int ret = DialogBoxParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_CONFIG), hWnd, reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
	BOOL bRet = FALSE;
	for(auto &&item : m_list){
		auto *page = item.second;
		if(!page){ continue; }
		if(page->IsApply()){
			bRet = TRUE;
		}
		delete page;
	}
	return bRet;
}

BOOL CGrTxtConfigDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	//
	auto hIcon = static_cast<HICON>(LoadImage(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 32, 32, LR_SHARED));
	SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
	auto hIconSm = reinterpret_cast<HICON>(LoadImage(CGrTxtFunc::GetDllModuleHandle(),MAKEINTRESOURCE(IDI_APP),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR));
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSm));

	RECT win_rect;
	::GetWindowRect(m_hWnd, &win_rect);
	m_minSize.cx = win_rect.right - win_rect.left;
	m_minSize.cy = win_rect.bottom - win_rect.top;

	{
		RECT rect = {0,0,7,7};
		MapDialogRect(hwnd, &rect);
		g_padding = rect.right;
	}
	struct PropPageInfo {
		CreatePropFunc Create;
	} propTable[] = {
		{&CGrPropPageStyle   ::CreateProp},
		{&CGrPropPageBookmark::CreateProp},
		{&CGrPropPageHtml    ::CreateProp},
		{&CGrPropPageOption  ::CreateProp},
		{&CGrPropPageMenu    ::CreateProp},
		{&CGrPropPageKey     ::CreateProp},
		{&CGrPropPageRubyList::CreateProp},
	};

	//
	int max_height = 0;
	int max_width  = 0;
	for(int index=0; index<sizeof(propTable) / sizeof(PropPageInfo); ++index){
		auto *prop = propTable[index].Create(this);
		m_list[index] = prop;
		RECT rect;
		GetWindowRect(prop->GetWnd(), &rect);
		int height = rect.bottom - rect.top;
		int width  = rect.right - rect.left;
		if(height > max_height){
			max_height = height;
		}
		if(width > max_width){
			max_width = width;
		}
	}
	//
	m_menuButton[MBTN_STYLE   ].Attach(GetDlgItem(m_hWnd, IDC_BUTTON_STYLE   ));
	m_menuButton[MBTN_BOOKMARK].Attach(GetDlgItem(m_hWnd, IDC_BUTTON_BOOKMARK));
	m_menuButton[MBTN_HTML    ].Attach(GetDlgItem(m_hWnd, IDC_BUTTON_HTML    ));
	m_menuButton[MBTN_OPTION  ].Attach(GetDlgItem(m_hWnd, IDC_BUTTON_OPTION  ));
	m_menuButton[MBTN_MENU    ].Attach(GetDlgItem(m_hWnd, IDC_BUTTON_MENU    ));
	m_menuButton[MBTN_KEY     ].Attach(GetDlgItem(m_hWnd, IDC_BUTTON_KEY     ));
	m_menuButton[MBTN_RUBY    ].Attach(GetDlgItem(m_hWnd, IDC_BUTTON_RUBY    ));
	ShowWindow(m_list[0]->GetWnd(), SW_SHOW);
	m_menuButton[MBTN_STYLE].SetCheck(true);
	auto hBtn = GetDlgItem(m_hWnd, ID_APPLY);
	if(hBtn){
		EnableWindow(hBtn, FALSE);
	}
	//
	setWindowSize(-1, -1);
	{
		auto hFrame = GetDlgItem(m_hWnd, IDC_STATIC_FRAME);
		if(hFrame){
			RECT rect;
			GetWindowRect(hFrame, &rect);
			int width  = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			if(height > max_height){
				max_height = height;
			}
			if(width > max_width){
				max_width = width;
			}
			width  = (win_rect.right - win_rect.left) + (max_width - width);
			height = (win_rect.bottom - win_rect.top) + (max_height - height);
			//
			POINT point = {win_rect.left, win_rect.top};
			auto hMonitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST); // 指定した点に最も近い位置にあるディスプレイモニタのハンドルが返ります。
			MONITORINFO mi = {sizeof(MONITORINFO)};
			RECT desktop_rc = {0};
			if(hMonitor && ::GetMonitorInfo(hMonitor, &mi)){
				desktop_rc = mi.rcWork;
			} else {
				::SystemParametersInfo(SPI_GETWORKAREA, 0, &desktop_rc, 0);
			}
			max_width = (desktop_rc.right - win_rect.left);
			max_height = (desktop_rc.bottom - win_rect.top);
			if(width > max_width){
				width = max_width;
			}
			if(width < m_minSize.cx){
				width = m_minSize.cx;
			}
			if(height > max_height){
				height = max_height;
			}
			if(height < m_minSize.cy){
				height = m_minSize.cy;
			}
			MoveWindow(m_hWnd, win_rect.left, win_rect.top, width, height, TRUE);
		}
	}
	return TRUE;
}

void CGrTxtConfigDlg::showPage(int page)
{
	setWindowSize(-1, -1);
	ShowWindow(m_list[page]->GetWnd(), SW_SHOW);
	m_menuButton[page].SetCheck(true);
	for(const auto &prop : m_list){
		auto hProp = prop.second->GetWnd();
		if(prop.first != page){
			ShowWindow(hProp, SW_HIDE);
			m_menuButton[prop.first].SetCheck(false);
		}
	}
}

void CGrTxtConfigDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDC_BUTTON_STYLE   : showPage(MBTN_STYLE   ); break;
	case IDC_BUTTON_BOOKMARK: showPage(MBTN_BOOKMARK); break;
	case IDC_BUTTON_HTML    : showPage(MBTN_HTML    ); break;
	case IDC_BUTTON_OPTION  : showPage(MBTN_OPTION  ); break;
	case IDC_BUTTON_MENU    : showPage(MBTN_MENU    ); break;
	case IDC_BUTTON_KEY     : showPage(MBTN_KEY     ); break;
	case IDC_BUTTON_RUBY    : showPage(MBTN_RUBY    ); break;
	case IDOK:
		for(const auto &prop : m_list){
			auto *pProp = prop.second;
			if(!pProp->IsApply()){
				pProp->Apply();
			}
		}
		::EndDialog(m_hWnd, id);
		break;
	case IDCANCEL:
		::EndDialog(m_hWnd, id);
		break;
	case ID_APPLY:
		for(const auto &prop : m_list){
			auto *pProp = prop.second;
			if(!pProp->IsApply()){
				pProp->Apply();
			}
		}
		{
			auto hBtn = GetDlgItem(m_hWnd, ID_APPLY);
			if(hBtn){
				EnableWindow(hBtn, FALSE);
			}
		}
		break;
	}
}

HBRUSH CGrTxtConfigDlg::OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
	return reinterpret_cast<HBRUSH>(CGrWinCtrl::WndProc(hwnd, GET_WM_CTLCOLOR_MSG(type), GET_WM_CTLCOLOR_MPS(hdc, hwnd, type)));
}

BOOL CGrTxtConfigDlg::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	return TRUE;
}

static void MoveWindowBottomRight(HWND hWnd, UINT id, int &right, int &bottom)
{
	auto hBtn = GetDlgItem(hWnd, id);
	if(hBtn){
		RECT rect;
		GetWindowRect(hBtn, &rect);
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;
		right -= (w+g_padding);
		bottom -= (h+g_padding);
		MoveWindow(hBtn, right, bottom, w, h, TRUE);
	}
}
void CGrTxtConfigDlg::setWindowSize(int cx, int cy)
{
	if(cx < 0 || cy < 0){
		RECT win_rect;
		::GetClientRect(m_hWnd, &win_rect);
		cx = win_rect.right - win_rect.left;
		cy = win_rect.bottom - win_rect.top;
	}
	if(cx > 0 && cy > 0){
		int right = cx;
		int bottom = cy;
		bottom = cy; MoveWindowBottomRight(m_hWnd, ID_APPLY, right, bottom);
		bottom = cy; MoveWindowBottomRight(m_hWnd, IDCANCEL, right, bottom);
		bottom = cy; MoveWindowBottomRight(m_hWnd, IDOK    , right, bottom);
		auto hFrame = GetDlgItem(m_hWnd, IDC_STATIC_FRAME);
		if(hFrame){
			RECT rect;
			GetWindowRect(hFrame, &rect);
			ScreenToClientRect(&rect);
			int w = (cx - g_padding) - (rect.left);
			int h = (bottom - g_padding) - (rect.top);
			ShowWindow(hFrame, SW_HIDE);
			MoveWindow(hFrame, rect.left, rect.top, w, h, TRUE);
			for(const auto &prop : m_list){
				auto hProp = prop.second->GetWnd();
				if(hProp == NULL){
					continue;
				}
				MoveWindow(hProp, rect.left, rect.top, w, h, TRUE);
			}
			InvalidateRect(m_hWnd, NULL, FALSE);
			UpdateWindow(m_hWnd);
		}
		InvalidateRect(GetDlgItem(m_hWnd, ID_APPLY), NULL, FALSE);
		InvalidateRect(GetDlgItem(m_hWnd, IDCANCEL), NULL, FALSE);
		InvalidateRect(GetDlgItem(m_hWnd, IDOK    ), NULL, FALSE);
	}
}

void CGrTxtConfigDlg::OnSize(HWND hwnd, UINT nType, int cx, int cy)
{
	setWindowSize(cx, cy);
}

void CGrTxtConfigDlg::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	lpMinMaxInfo->ptMinTrackSize.x = m_minSize.cx;
	lpMinMaxInfo->ptMinTrackSize.y = m_minSize.cy;
}

