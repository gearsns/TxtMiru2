#ifndef __WINDOWTHEME_H__
#define __WINDOWTHEME_H__

#include <uxtheme.h>
#include <Vsstyle.h>
#include <Vssym32.h>
#include "stltchar.h"
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
	COLORREF GetSysColor(int iIndex) const;
	void RefreshThemeColor();
private:
	CGrWindowTheme();
	virtual ~CGrWindowTheme();
	HMODULE m_hDLL = NULL;
	const static int max_color_num = 40;
	COLORREF m_sysColor[max_color_num] = {};
	decltype(::SetWindowTheme           )* m_pfSetWindowTheme            = nullptr;
	decltype(::OpenThemeData            )* m_pfOpenThemeData             = nullptr;
	decltype(::CloseThemeData           )* m_pfCloseThemeData            = nullptr;
	decltype(::DrawThemeParentBackground)* m_pfDrawThemeParentBackground = nullptr;
	decltype(::DrawThemeText            )* m_pfDrawThemeText             = nullptr;
	decltype(::DrawThemeBackground      )* m_pfDrawThemeBackground       = nullptr;
};

#endif // __WINDOWTHEME_H__
