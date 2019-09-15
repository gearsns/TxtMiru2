#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "WindowTheme.h"
#include "Win32Wrap.h"

CGrWindowTheme &CGrWindowTheme::theme()
{
	static CGrWindowTheme theme; // <- (各Include先で実体が作成されるので)実体をヘッダに記述しないこと! ※確認するのであれば、コンストラクタが何回呼ばれているかをチェック
	return theme;
}

CGrWindowTheme::CGrWindowTheme()
{
	m_hDLL = LoadLibrary(_T("UxTheme.dll"));
	if(m_hDLL){
		SetProcAddressPtr(m_pfSetWindowTheme           , m_hDLL, "SetWindowTheme");
		SetProcAddressPtr(m_pfOpenThemeData            , m_hDLL, "OpenThemeData");
		SetProcAddressPtr(m_pfCloseThemeData           , m_hDLL, "CloseThemeData");
		SetProcAddressPtr(m_pfDrawThemeParentBackground, m_hDLL, "DrawThemeParentBackground");
		SetProcAddressPtr(m_pfDrawThemeText            , m_hDLL, "DrawThemeText");
		SetProcAddressPtr(m_pfDrawThemeBackground      , m_hDLL, "DrawThemeBackground");
	}
	RefreshThemeColor();
}

CGrWindowTheme::~CGrWindowTheme()
{
	if(m_hDLL){
		FreeLibrary(m_hDLL);
	}
}

HRESULT CGrWindowTheme::SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	if(m_pfSetWindowTheme){
		return m_pfSetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
	} else {
		return 0;
	}
}

HTHEME CGrWindowTheme::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	if(m_pfOpenThemeData){
		return m_pfOpenThemeData(hwnd, pszClassList);
	} else {
		return NULL;
	}
}

HRESULT CGrWindowTheme::CloseThemeData(HTHEME hTheme)
{
	if(m_pfCloseThemeData){
		return m_pfCloseThemeData(hTheme);
	} else {
		return 0;
	}
}

HRESULT CGrWindowTheme::DrawThemeParentBackground(HWND hwnd, HDC hdc, const RECT *prc)
{
	if(m_pfDrawThemeParentBackground){
		return m_pfDrawThemeParentBackground(hwnd, hdc, prc);
	} else {
		return 0;
	}
}

HRESULT CGrWindowTheme::DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect)
{
	if(m_pfDrawThemeText){
		return m_pfDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
	} else {
		return 0;
	}
}

HRESULT CGrWindowTheme::DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect)
{
	if(m_pfDrawThemeBackground){
		return m_pfDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
	} else {
		return 0;
	}
}
COLORREF CGrWindowTheme::GetSysColor(int iIndex) const
{
	return m_sysColor[iIndex];
}
void CGrWindowTheme::RefreshThemeColor()
{
	m_sysColor[COLOR_WINDOW] = ::GetSysColor(COLOR_WINDOW);
	m_sysColor[COLOR_WINDOWTEXT] = ::GetSysColor(COLOR_WINDOWTEXT);
}

