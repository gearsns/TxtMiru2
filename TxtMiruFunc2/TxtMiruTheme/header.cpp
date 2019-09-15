#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <uxtheme.h>
#include <Vsstyle.h>
#include <stdlib.h>
#include "header.h"
#include "TxtMiruTheme.h"

#define IsKeyDown(vk) (::GetAsyncKeyState(vk) & 0x8000)
static bool isMouseLButtonDown()
{
	return IsKeyDown(::GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON);
}

static void HeaderDrawItem(HWND hWnd, HDC hdc, INT iItem, BOOL bHotTrack, RECT& item_rect)
{
	wchar_t buf[2048];
	HDITEM hdi = { HDI_FORMAT | HDI_HEIGHT | HDI_IMAGE | HDI_LPARAM | HDI_ORDER | HDI_STATE | HDI_TEXT | HDI_WIDTH };
	hdi.pszText = buf;
	hdi.cchTextMax = _countof(buf);
	Header_GetItem(hWnd, iItem, &hdi);

	if (item_rect.right - item_rect.left == 0) {
		return;
	}
	SetTextColor(hdc, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
	SetBkColor(hdc, TxtMiruTheme_GetSysColor((bHotTrack ? COLOR_HOTLIGHT : COLOR_WINDOW)));
	FillRect(hdc, &item_rect, TxtMiruTheme_GetSysColorBrush(bHotTrack ? COLOR_HOTLIGHT : COLOR_WINDOW));
	if (((hdi.fmt & HDF_STRING)
		|| (!(hdi.fmt & (HDF_OWNERDRAW | HDF_STRING | HDF_BITMAP | HDF_BITMAP_ON_RIGHT | HDF_IMAGE))))
		&& (hdi.pszText)) {
		auto r = item_rect;
		if (bHotTrack && isMouseLButtonDown()) { // Press
			r.left += 2;
			r.top += 2;
		}
		const auto iMargin = 4;
		const auto iVertBorder = 4;
		int cw = 0;
		if (hdi.fmt & HDF_STRING) {
			RECT textRect = {};
			DrawText(hdc, hdi.pszText, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			cw = textRect.right - textRect.left + 2 * iMargin;
		}
		int sort_w = 0;
		if (hdi.fmt & (HDF_SORTUP | HDF_SORTDOWN)) {
			const auto nHeight = item_rect.bottom - item_rect.top;
			auto sort_h = MulDiv(nHeight - iVertBorder, 4, 13);
			sort_w = 2 * sort_h - 1 + iMargin * 2;
			cw += sort_w;
		}
		int cx;
		switch (hdi.fmt & HDF_JUSTIFYMASK) {
		case HDF_LEFT: cx = r.left; break;
		case HDF_CENTER: cx = r.left + (r.right - r.left) / 2 - cw / 2; break;
		default: cx = r.right - cw; break;
		}
		if (cx < r.left) {
			cx = r.left;
		}
		if (cx + cw > r.right) {
			cw = r.right - cx;
		}
		r.left = cx + iMargin;
		r.right = r.left + (cw - iMargin * 2 - sort_w);
		auto oldBkMode = SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, hdi.pszText, -1, &r, DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_SINGLELINE);
		if (oldBkMode != TRANSPARENT) {
			SetBkMode(hdc, oldBkMode);
		}
	}
}

static void Refresh(HWND hWnd, HDC hdc)
{
	RECT rect;
	GetClientRect(hWnd, &rect);

	auto hFont = GetWindowFont(hWnd);
	auto hOldFont = SelectObject(hdc, hFont);
	FillRect(hdc, &rect, TxtMiruTheme_GetSysColorBrush(COLOR_WINDOW));
	//
	POINT mouse_point;
	GetCursorPos(&mouse_point);
	ScreenToClient(hWnd, &mouse_point);
	HDHITTESTINFO HitTest;
	HitTest.pt.x = mouse_point.x;
	HitTest.pt.y = mouse_point.y;
	//
	auto uNumItem = Header_GetItemCount(hWnd);
	auto x = rect.left;
	for (auto i = 0; x <= rect.right && i < uNumItem; i++) {
		auto idx = Header_OrderToIndex(hWnd, i);
		RECT item_rect;
		Header_GetItemRect(hWnd, i, &item_rect);
		if (RectVisible(hdc, &item_rect)) {
			auto iHotItem = SendMessage(hWnd, HDM_HITTEST, 0, reinterpret_cast<LPARAM>(&HitTest));
			HeaderDrawItem(hWnd, hdc, idx, iHotItem == idx, item_rect);
		}
		x = item_rect.right;
	}
	auto rcRest = rect;
	rcRest.left = x;
	if ((x <= rect.right) && RectVisible(hdc, &rcRest) && (uNumItem > 0)) {
		FillRect(hdc, &rcRest, TxtMiruTheme_GetSysColorBrush(COLOR_WINDOW));
	}
	SelectObject(hdc, hOldFont);
}

namespace CGrHeader {
	LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		switch (uMsg) {
		case WM_PRINTCLIENT:
		case WM_PAINT:
			if (TxtMiruTheme_IsDarkThemeActive()) {
				PAINTSTRUCT ps;
				HDC hdc = wParam == 0 ? BeginPaint(hWnd, &ps) : reinterpret_cast<HDC>(wParam);
				auto nOldMode = SetBkMode(hdc, OPAQUE);
				Refresh(hWnd, hdc);
				SetBkMode(hdc, nOldMode);
				if (!wParam) {
					EndPaint(hWnd, &ps);
				}
				return FALSE;
			}
			break;
		default:
			break;

		}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
}