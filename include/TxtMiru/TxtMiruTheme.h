#pragma once

#ifdef TXTMIRUTHEME_EXPORTS
#define TXTMIRUTHEME_API __declspec(dllexport)
#else
#define TXTMIRUTHEME_API __declspec(dllimport)
#endif

#include <windows.h>

extern "C" {
	TXTMIRUTHEME_API bool TxtMiruTheme_IsDarkThemeActive();
	TXTMIRUTHEME_API COLORREF TxtMiruTheme_GetSysColor(int iIndex);
	TXTMIRUTHEME_API HBRUSH TxtMiruTheme_GetSysColorBrush(int iIndex);
	TXTMIRUTHEME_API HRESULT TxtMiruTheme_SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
	TXTMIRUTHEME_API void TxtMiruTheme_RefreshThemeColor();
	TXTMIRUTHEME_API bool TxtMiruTheme_IsImmersiveColorSet(LPARAM lParam);
	TXTMIRUTHEME_API void TxtMiruTheme_SetTouchMenu(HMENU hMenu);
	TXTMIRUTHEME_API void TxtMiruTheme_MenuMeasureItem(HWND hWnd, LPARAM lParam);
	TXTMIRUTHEME_API void TxtMiruTheme_MenuDrawItem(HWND hWnd, LPARAM lParam);
	TXTMIRUTHEME_API void TxtMiruTheme_Hook();
	TXTMIRUTHEME_API void TxtMiruTheme_Unhook();
	TXTMIRUTHEME_API void TxtMiruTheme_SetWindowSubclass(HWND hWnd);
	TXTMIRUTHEME_API void TxtMiruTheme_RemoveWindowSubclass(HWND hWnd);
	TXTMIRUTHEME_API void TxtMiruTheme_UpdateDarkMode(HWND hWnd);
}