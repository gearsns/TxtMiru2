// TxtMiruTheme.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"

#include <uxtheme.h>
#include <Vsstyle.h>
#include <Vssym32.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <vector>
#include <map>
#ifndef FACILITY_VISUALCPP
#define FACILITY_VISUALCPP  ((LONG)0x6d)
#endif
#include <DelayImp.h>
#include "stltchar.h"
#include "Win32Wrap.h"
#include "TxtMiruTheme.h"

#include "shell.h"

static const auto AppName = L"TxTMiru";
TCHAR l_data_path[MAX_PATH] = {};

static LPCTSTR GetDataPath()
{
	if (l_data_path[0] == '\0') {
		TCHAR curPath[MAX_PATH];
		CGrShell::GetExePath(curPath);
		TCHAR iniFileName[MAX_PATH];
		_stprintf_s(iniFileName, _T("%s/%s.ini"), curPath, AppName);
		int iUserApplicatinData = ::GetPrivateProfileInt(_T("TxtMiru"), _T("UseApplicationData"), 0, iniFileName);
		if (iUserApplicatinData == 1) {
			CGrShell::GetDataPath(l_data_path, _countof(l_data_path), AppName);
			CGrShell::CreateFolder(l_data_path);
		}
		else {
			_tcscpy_s(l_data_path, curPath);
		}
	}
	return l_data_path;
}

static bool IsDarkThemeEnabled()
{
	static int iEnableDarkTheme = -1;
	if (iEnableDarkTheme == -1) {
		TCHAR iniFileName[MAX_PATH];
		_stprintf_s(iniFileName, _T("%s/%s.ini"), GetDataPath(), AppName);

		iEnableDarkTheme = 0;
		TCHAR str[104];
		if (::GetPrivateProfileString(_T("points"), _T("EnableDarkTheme"), _T("0"), str, _countof(str), iniFileName)) {
			iEnableDarkTheme = _ttoi(str);
		}
	}
	return iEnableDarkTheme == 1;
}

////////////////////////////////////////////////////////////
// lib /def:sqlite3.def /machine:x86
static DWORD DelayLoadExFilter(DWORD ec, EXCEPTION_POINTERS *ep)
{
	DelayLoadInfo *dli = nullptr;
	DelayLoadProc *dlp = nullptr;

	if (ec == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND)) {
	}
	else if (ec == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND)) {
	}
	else {
		// 遅延読み込みの例外でない場合は上位の例外ハンドラへ
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// 遅延読み込み用の例外情報を取得
	dli = reinterpret_cast<DelayLoadInfo*>(ep->ExceptionRecord->ExceptionInformation[0]);
	dlp = &(dli->dlp);
	if (dlp->fImportByName) {
	}
	else {
	}

	// 処理続行
	return EXCEPTION_EXECUTE_HANDLER;
}

static bool IsDarkThemeActive()
{
	if (IsDarkThemeEnabled()) {
		DWORD type;
		DWORD value;
		DWORD count = 4;
		__try {
			auto st = RegGetValue(
				HKEY_CURRENT_USER,
				TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
				TEXT("AppsUseLightTheme"),
				RRF_RT_REG_DWORD,
				&type,
				&value,
				&count);
			if (st == ERROR_SUCCESS && type == REG_DWORD) {
				return value == 0;
			}
		}
		__except (DelayLoadExFilter(GetExceptionCode(), GetExceptionInformation())) {
		}
		return false;
	}
	else {
		return false;
	}
}

extern "C" {
bool __stdcall ShouldAppsUseDarkMode();
bool __stdcall IsDarkModeAllowedForWindow(HWND__* window);
bool __stdcall AllowDarkModeForWindow(HWND__* window, bool allow);
bool __stdcall AllowDarkModeForApp(bool allow);
}

class CGrWindowTheme
{
public:
	static CGrWindowTheme &theme();
	HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
	HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
	HRESULT CloseThemeData(HTHEME hTheme);
	HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc, const RECT *prc);
	HRESULT DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect);
	HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
	bool IsDarkThemeActive() { return m_bDarkMode;  }
	COLORREF GetSysColor(int iIndex) const;
	HBRUSH GetSysColorBrush(int iIndex) const;
	void RefreshThemeColor();
	bool ShouldAppsUseDarkMode();
	bool IsDarkModeAllowedForWindow(HWND__* window);
	bool AllowDarkModeForWindow(HWND__* window, bool allow);
	bool AllowDarkModeForApp(bool allow);
private:
	CGrWindowTheme();
	virtual ~CGrWindowTheme();
	HMODULE m_hDLL = NULL;
	const static int max_color_num = 40;
	COLORREF m_sysColor[max_color_num] = {CLR_NONE};
	HBRUSH m_sysBrush[max_color_num] = {};
	bool m_bDarkMode = false;
	decltype(::SetWindowTheme)* m_pfSetWindowTheme = nullptr;
	decltype(::OpenThemeData)* m_pfOpenThemeData = nullptr;
	decltype(::CloseThemeData)* m_pfCloseThemeData = nullptr;
	decltype(::DrawThemeParentBackground)* m_pfDrawThemeParentBackground = nullptr;
	decltype(::DrawThemeText)* m_pfDrawThemeText = nullptr;
	decltype(::DrawThemeBackground)* m_pfDrawThemeBackground = nullptr;
	//
	decltype(::ShouldAppsUseDarkMode)* m_pfShouldAppsUseDarkMode = nullptr;
	decltype(::IsDarkModeAllowedForWindow)* m_pfIsDarkModeAllowedForWindow = nullptr;
	decltype(::AllowDarkModeForWindow)* m_pfAllowDarkModeForWindow = nullptr;
	decltype(::AllowDarkModeForApp)* m_pfAllowDarkModeForApp = nullptr;

};


CGrWindowTheme &CGrWindowTheme::theme()
{
	static CGrWindowTheme theme; // <- (各Include先で実体が作成されるので)実体をヘッダに記述しないこと! ※確認するのであれば、コンストラクタが何回呼ばれているかをチェック
	return theme;
}

CGrWindowTheme::CGrWindowTheme()
{
	m_hDLL = LoadLibrary(_T("UxTheme.dll"));
	if (m_hDLL) {
		SetProcAddressPtr(m_pfSetWindowTheme, m_hDLL, "SetWindowTheme");
		SetProcAddressPtr(m_pfOpenThemeData, m_hDLL, "OpenThemeData");
		SetProcAddressPtr(m_pfCloseThemeData, m_hDLL, "CloseThemeData");
		SetProcAddressPtr(m_pfDrawThemeParentBackground, m_hDLL, "DrawThemeParentBackground");
		SetProcAddressPtr(m_pfDrawThemeText, m_hDLL, "DrawThemeText");
		SetProcAddressPtr(m_pfShouldAppsUseDarkMode, m_hDLL, MAKEINTRESOURCEA(132));
		SetProcAddressPtr(m_pfIsDarkModeAllowedForWindow, m_hDLL, MAKEINTRESOURCEA(137));
		SetProcAddressPtr(m_pfAllowDarkModeForWindow, m_hDLL, MAKEINTRESOURCEA(133));
		SetProcAddressPtr(m_pfAllowDarkModeForApp, m_hDLL, MAKEINTRESOURCEA(135));
	}
	RefreshThemeColor();
}

CGrWindowTheme::~CGrWindowTheme()
{
	for (auto hBrush : m_sysBrush) {
		if (hBrush) {
			::DeleteObject(hBrush);
		}
	}
	if (m_hDLL) {
		FreeLibrary(m_hDLL);
	}
}

HRESULT CGrWindowTheme::SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	if (m_pfSetWindowTheme) {
		return m_pfSetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
	}
	else {
		return 0;
	}
}

HTHEME CGrWindowTheme::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	if (m_pfOpenThemeData) {
		return m_pfOpenThemeData(hwnd, pszClassList);
	}
	else {
		return NULL;
	}
}

HRESULT CGrWindowTheme::CloseThemeData(HTHEME hTheme)
{
	if (m_pfCloseThemeData) {
		return m_pfCloseThemeData(hTheme);
	}
	else {
		return 0;
	}
}

HRESULT CGrWindowTheme::DrawThemeParentBackground(HWND hwnd, HDC hdc, const RECT *prc)
{
	if (m_pfDrawThemeParentBackground) {
		return m_pfDrawThemeParentBackground(hwnd, hdc, prc);
	}
	else {
		return 0;
	}
}

HRESULT CGrWindowTheme::DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect)
{
	if (m_pfDrawThemeText) {
		return m_pfDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
	}
	else {
		return 0;
	}
}

HRESULT CGrWindowTheme::DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect)
{
	if (m_pfDrawThemeBackground) {
		return m_pfDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
	}
	else {
		return 0;
	}
}

COLORREF CGrWindowTheme::GetSysColor(int iIndex) const
{
	return m_sysColor[iIndex];
}

HBRUSH CGrWindowTheme::GetSysColorBrush(int iIndex) const
{
	if (m_sysBrush[iIndex]) {
		return m_sysBrush[iIndex];
	}
	else {
		return ::GetSysColorBrush(iIndex);
	}
}

void CGrWindowTheme::RefreshThemeColor()
{
	auto bDarkMode = ::IsDarkThemeActive();
	if (m_bDarkMode == bDarkMode && m_sysColor[0] != CLR_NONE) {
		return;
	}
	m_bDarkMode = bDarkMode;
	const UINT index_list[] = { COLOR_WINDOW , COLOR_WINDOWTEXT, COLOR_WINDOWFRAME, COLOR_HOTLIGHT, COLOR_HIGHLIGHT, COLOR_HIGHLIGHTTEXT, COLOR_BTNFACE, COLOR_BTNHIGHLIGHT, COLOR_BTNTEXT, COLOR_BTNSHADOW };
	if (m_bDarkMode) {

		m_sysColor[COLOR_SCROLLBAR] = RGB(0x20, 0x20, 0x20);
		m_sysColor[COLOR_WINDOW] = RGB(0x20, 0x20, 0x20);
		m_sysColor[COLOR_WINDOWTEXT] = RGB(0xff, 0xff, 0xff);
		m_sysColor[COLOR_WINDOWFRAME] = RGB(0x20, 0x20, 0x20);
		m_sysColor[COLOR_HOTLIGHT] = RGB(0x40, 0x40, 0x40);
		m_sysColor[COLOR_HIGHLIGHT] = RGB(0x40, 0x40, 0x40);
		m_sysColor[COLOR_HIGHLIGHTTEXT] = RGB(0xff, 0xff, 0xff);
		m_sysColor[COLOR_BTNFACE] = RGB(0x33, 0x33, 0x33);
		m_sysColor[COLOR_BTNHIGHLIGHT] = RGB(0x45, 0x45, 0x45);
		m_sysColor[COLOR_BTNTEXT] = RGB(0xff, 0xff, 0xff);
		m_sysColor[COLOR_BTNSHADOW] = RGB(0xff, 0xff, 0xff);

		for (auto iIndex : index_list) {
			if (m_sysBrush[iIndex]) {
				DeleteObject(m_sysBrush[iIndex]);
			}
			m_sysBrush[iIndex] = CreateSolidBrush(m_sysColor[iIndex]);
		}
	}
	else {
		for (auto iIndex : index_list) {
			m_sysColor[iIndex] = ::GetSysColor(iIndex);
			if (m_sysBrush[iIndex]) {
				DeleteObject(m_sysBrush[iIndex]);
			}
			m_sysBrush[iIndex] = NULL;
		}
	}
}

bool CGrWindowTheme::ShouldAppsUseDarkMode()
{
	if (m_pfShouldAppsUseDarkMode) {
		return m_pfShouldAppsUseDarkMode();
	}
	else {
		return false;
	}
}

bool CGrWindowTheme::IsDarkModeAllowedForWindow(HWND__* window)
{
	if (m_pfIsDarkModeAllowedForWindow) {
		return m_pfIsDarkModeAllowedForWindow(window);
	}
	else {
		return false;
	}
}

bool CGrWindowTheme::AllowDarkModeForWindow(HWND__* window, bool allow)
{
	if (m_pfAllowDarkModeForWindow) {
		return m_pfAllowDarkModeForWindow(window, allow);
	}
	else {
		return false;
	}
}

bool CGrWindowTheme::AllowDarkModeForApp(bool allow)
{
	if (m_pfAllowDarkModeForApp) {
		return m_pfAllowDarkModeForApp(allow);
	}
	else {
		return false;
	}
}

TXTMIRUTHEME_API bool TxtMiruTheme_IsDarkThemeActive()
{
	return CGrWindowTheme::theme().IsDarkThemeActive();
}

TXTMIRUTHEME_API COLORREF TxtMiruTheme_GetSysColor(int iIndex)
{
	return CGrWindowTheme::theme().GetSysColor(iIndex);
}

TXTMIRUTHEME_API HBRUSH TxtMiruTheme_GetSysColorBrush(int iIndex)
{
	return CGrWindowTheme::theme().GetSysColorBrush(iIndex);
}

TXTMIRUTHEME_API HRESULT TxtMiruTheme_SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	return CGrWindowTheme::theme().SetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
}

TXTMIRUTHEME_API void TxtMiruTheme_RefreshThemeColor()
{
	CGrWindowTheme::theme().RefreshThemeColor();
}

TXTMIRUTHEME_API bool TxtMiruTheme_IsImmersiveColorSet(LPARAM lParam)
{
	if (lParam) {
		auto lParamString = reinterpret_cast<const wchar_t*>(lParam);
		if (!wcscmp(lParamString, L"ImmersiveColorSet")) {
			CGrWindowTheme::theme().RefreshThemeColor();
			return true;
		}
	}
	return false;
}

#define MOUSEEVENTF_FROMTOUCH 0xFF515700
TXTMIRUTHEME_API void TxtMiruTheme_SetTouchMenu(HMENU hMenu)
{
	auto extra = GetMessageExtraInfo();
	if ((extra & MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH) {
		if (extra & 0x80) {
			MENUITEMINFO mi = { sizeof(MENUITEMINFO) };
			mi.fMask = MIIM_FTYPE | MIIM_DATA;
			std::vector<HMENU> menu_list;
			menu_list.push_back(hMenu);
			for (UINT index = 0; menu_list.size() > index; ++index) {
				auto hCurMenu = menu_list[index];
				for (int i = GetMenuItemCount(hCurMenu) - 1; i >= 0; --i) {
					::GetMenuItemInfo(hCurMenu, i, TRUE, &mi);
					mi.fType |= MFT_OWNERDRAW;
					mi.dwItemData = reinterpret_cast<ULONG_PTR>(hCurMenu);
					auto hSubMenu = GetSubMenu(hCurMenu, i);
					if (hSubMenu) {
						menu_list.push_back(hSubMenu);
					}
					if ((mi.fType & MFT_SEPARATOR) != MFT_SEPARATOR || hSubMenu) {
						::SetMenuItemInfo(hCurMenu, i, TRUE, &mi);
					}
				}
			}
		}
	}
}

TXTMIRUTHEME_API void TxtMiruTheme_MenuMeasureItem(HWND hWnd, LPARAM lParam)
{
	auto lpMI = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);
	if (lpMI->CtlType == ODT_MENU && lpMI->itemData) {
		auto hMenu = reinterpret_cast<HMENU>(lpMI->itemData);
		TCHAR str[2048] = {};
		GetMenuString(hMenu, lpMI->itemID, str, sizeof(str) / sizeof(TCHAR), MF_BYCOMMAND);
		int len = _tcslen(str);
		if (len > 0) {
			SIZE sz;
			auto hdc = GetDC(hWnd);
			GetTextExtentPoint32(hdc, str, len - 1, &sz);
			lpMI->itemWidth = sz.cx;
			lpMI->itemHeight = sz.cy * 2; // タッチ操作でメニューを出したときは、OwnerDrawで幅を広げる
			ReleaseDC(hWnd, hdc);
			lpMI->itemWidth += GetSystemMetrics(SM_CXMENUCHECK) + GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CXMENUSIZE);
		}
	}
}

TXTMIRUTHEME_API void TxtMiruTheme_MenuDrawItem(HWND hWnd, LPARAM lParam)
{
	// wParam : If the message was sent by a menu, this parameter is zero.
	auto lpDI = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
	auto rc = lpDI->rcItem;
	auto hdc = lpDI->hDC;
	auto hMenu = reinterpret_cast<HMENU>(lpDI->itemData);
	TCHAR str[2048] = {};
	GetMenuString(hMenu, lpDI->itemID, str, sizeof(str) / sizeof(TCHAR), MF_BYCOMMAND);
	auto text_rc = rc;
	text_rc.left += GetSystemMetrics(SM_CXMENUCHECK) + GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CXMENUSIZE);
	auto check_rc = lpDI->rcItem;
	check_rc.right = text_rc.left - 1;

	auto &&theme = CGrWindowTheme::theme();
	auto hTheme = theme.OpenThemeData(hWnd, _T("MENU"));
	if (hTheme) {
		theme.DrawThemeParentBackground(hWnd, hdc, &rc);
		theme.DrawThemeBackground(hTheme, hdc, MENU_POPUPBACKGROUND, MCB_NORMAL, &rc, nullptr);
		SetBkMode(hdc, TRANSPARENT);
		int iStateId = 0;
		if ((lpDI->itemState & ODS_SELECTED) || (lpDI->itemState & ODS_FOCUS)) {
			if (lpDI->itemState & (ODS_DISABLED | ODS_GRAYED)) {
				iStateId = MPI_DISABLEDHOT;
			}
			else {
				iStateId = MPI_HOT;
			}
		}
		else {
			if (lpDI->itemState & (ODS_DISABLED | ODS_GRAYED)) {
				iStateId = MPI_DISABLED;
			}
			else {
				iStateId = MPI_NORMAL;
			}
		}
		theme.DrawThemeBackground(hTheme, hdc, MENU_POPUPITEM, iStateId, &rc, nullptr);
		theme.DrawThemeText(hTheme, hdc, MENU_POPUPITEM, iStateId, str, -1, DT_VCENTER | DT_SINGLELINE, 0, &text_rc);
		if (lpDI->itemState & ODS_CHECKED) {
			theme.DrawThemeBackground(hTheme, hdc, MENU_POPUPCHECKBACKGROUND, MCB_NORMAL, &check_rc, nullptr);
			theme.DrawThemeBackground(hTheme, hdc, MENU_POPUPCHECK, MC_CHECKMARKNORMAL, &check_rc, nullptr);
		}
		theme.CloseThemeData(hTheme);
	}
	else {
		if ((lpDI->itemState & ODS_SELECTED) || (lpDI->itemState & ODS_FOCUS)) {
			if (lpDI->itemState & (ODS_DISABLED | ODS_GRAYED)) {
				FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNSHADOW));
				SetBkColor(hdc, GetSysColor(COLOR_BTNSHADOW));
				SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
			}
			else {
				FillRect(hdc, &rc, GetSysColorBrush(COLOR_MENUHILIGHT));
				SetBkColor(hdc, GetSysColor(COLOR_MENUHILIGHT));
				SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
			}
		}
		else {
			FillRect(hdc, &rc, GetSysColorBrush(COLOR_MENU));
			SetBkColor(hdc, GetSysColor(COLOR_MENU));
			if (lpDI->itemState & (ODS_DISABLED | ODS_GRAYED)) {
				SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
			}
			else {
				SetTextColor(hdc, GetSysColor(COLOR_MENUTEXT));
			}
		}
		DrawText(hdc, str, -1, &text_rc, DT_VCENTER | DT_SINGLELINE);
		if (lpDI->itemState & ODS_CHECKED) {
			DrawFrameControl(hdc, &check_rc, DFC_MENU, DFCS_MENUCHECK);
		}
	}
}

#include "TxtMiruWindow.h"
#include "button.h"
#include "header.h"

TXTMIRUTHEME_API void TxtMiruTheme_Hook()
{
}

TXTMIRUTHEME_API void TxtMiruTheme_Unhook()
{
}

struct TxtMiruSubclassData
{
	bool bAttach = true;
};
static BOOL CALLBACK SubclassEnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	auto *lpData = reinterpret_cast<TxtMiruSubclassData*>(lParam);
	TCHAR classname[2048];
	GetClassName(hwnd, classname, _countof(classname));
	if (lpData->bAttach) {
		if (_tcsicmp(classname, L"BUTTON") == 0) {
			SetWindowSubclass(hwnd, CGrButton::SubclassProc, 0, 0);
		}
		else if (_tcsicmp(classname, L"SYSLISTVIEW32") == 0) {
		}
		else if (_tcsicmp(classname, L"SYSHEADER32") == 0) {
			SetWindowSubclass(hwnd, CGrHeader::SubclassProc, 0, 0);
		}
	}
	else {
		if (_tcsicmp(classname, L"BUTTON") == 0) {
			RemoveWindowSubclass(hwnd, CGrButton::SubclassProc, 0);
		}
		else if (_tcsicmp(classname, L"SYSLISTVIEW32") == 0) {
		}
		else if (_tcsicmp(classname, L"SYSHEADER32") == 0) {
			SetWindowSubclass(hwnd, CGrHeader::SubclassProc, 0, 0);
		}
	}
	return TRUE;
}

TXTMIRUTHEME_API void TxtMiruTheme_SetWindowSubclass(HWND hWnd)
{
	TxtMiruSubclassData data = { true };
	EnumChildWindows(hWnd, SubclassEnumWindowsProc, LPARAM(&data));
	SetWindowSubclass(hWnd, CGrWindow::SubclassProc, 0, 0);
}

TXTMIRUTHEME_API void TxtMiruTheme_RemoveWindowSubclass(HWND hWnd)
{
	TxtMiruSubclassData data = { false };
	EnumChildWindows(hWnd,SubclassEnumWindowsProc, LPARAM(&data));
	RemoveWindowSubclass(hWnd, CGrWindow::SubclassProc, 0);
}

struct TxtMiruSetWindowThemeData
{
	bool bDarkMode = true;
};
static BOOL CALLBACK SetWindowThemeEnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	auto* lpData = reinterpret_cast<TxtMiruSetWindowThemeData*>(lParam);
	TCHAR classname[2048];
	GetClassName(hwnd, classname, _countof(classname));
	if (lpData->bDarkMode) {
		if (_tcsicmp(classname, L"BUTTON") == 0) {
		}
		else if (_tcsicmp(classname, L"SYSLISTVIEW32") == 0) {
			CGrWindowTheme::theme().SetWindowTheme(hwnd, L"DarkMode_Explorer", nullptr);
		}
		else if (_tcsicmp(classname, L"SYSHEADER32") == 0) {
		}
	}
	else {
		if (_tcsicmp(classname, L"BUTTON") == 0) {
		}
		else if (_tcsicmp(classname, L"SYSLISTVIEW32") == 0) {
			CGrWindowTheme::theme().SetWindowTheme(hwnd, L"Explorer", nullptr);
		}
		else if (_tcsicmp(classname, L"SYSHEADER32") == 0) {
		}
	}
	return TRUE;
}

TXTMIRUTHEME_API void TxtMiruTheme_UpdateDarkMode(HWND hWnd)
{
	bool allow = TxtMiruTheme_IsDarkThemeActive();
	TxtMiruSetWindowThemeData data = { allow };
	EnumChildWindows(hWnd, SetWindowThemeEnumWindowsProc, LPARAM(&data));
}
