#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <Vsstyle.h>
#include <Vssym32.h>
#include <stdlib.h>
#include "TxtMiruTheme.h"
#include "TxtMiruWindow.h"

namespace CGrWindow {
	LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		switch (uMsg) {
		case WM_ERASEBKGND:
		{
			return 1;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			auto hDC = BeginPaint(hWnd, &ps);

			RECT rect;
			GetClientRect(hWnd, &rect);
			auto oldColor = GetBkColor(hDC);
			SetBkColor(hDC, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
			::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, nullptr, 0, nullptr);

			auto ret = DefSubclassProc(hWnd, uMsg, reinterpret_cast<WPARAM>(hDC), lParam);
			EndPaint(hWnd, &ps);
			return ret;
		}
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORSTATIC:
		{
			auto hDC = reinterpret_cast<HDC>(wParam);
			auto hCtrl = reinterpret_cast<HWND>(lParam);
			SetBkMode(hDC, OPAQUE);
			SetTextColor(hDC, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor(hDC, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
			return reinterpret_cast<LRESULT>(TxtMiruTheme_GetSysColorBrush(COLOR_WINDOW));
		}
		case WM_SETTINGCHANGE:
			if (TxtMiruTheme_IsImmersiveColorSet(lParam)) {
				InvalidateRect(hWnd, nullptr, TRUE);
			}
			break;
		default:
			break;

		}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
}