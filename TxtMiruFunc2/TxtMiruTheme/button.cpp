#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdlib.h>
#include <climits>
#include "button.h"
#include "TxtMiruTheme.h"
#include "stltchar.h"

#define ButtonType(window_style) (window_style & BS_TYPEMASK)

static UINT BStoDT(DWORD style, DWORD ex_style)
{
	if (style & BS_PUSHLIKE) {
		style &= ~BS_TYPEMASK;
	}
	UINT dtStyle = DT_NOCLIP | ((style & BS_MULTILINE) ? DT_WORDBREAK : DT_SINGLELINE);
	const UINT btStyle = ButtonType(style);

	switch (style & BS_CENTER)
	{
	case BS_LEFT:   /* DT_LEFT is 0 */    break;
	case BS_RIGHT:  dtStyle |= DT_RIGHT;  break;
	case BS_CENTER: dtStyle |= DT_CENTER; break;
	default:
		if (btStyle == BS_DEFPUSHBUTTON || btStyle == BS_PUSHBUTTON) {
			dtStyle |= DT_CENTER;
		}
	}
	if (ex_style & WS_EX_RIGHT) {
		dtStyle = DT_RIGHT | (dtStyle & ~(DT_LEFT | DT_CENTER));
	}
	if (btStyle != BS_GROUPBOX)
	{
		switch (style & BS_VCENTER)
		{
		case BS_TOP:     /* DT_TOP is 0 */      break;
		case BS_BOTTOM:  dtStyle |= DT_BOTTOM;  break;
		case BS_VCENTER: /* fall through */
		default:         dtStyle |= DT_VCENTER; break;
		}
		return dtStyle;
	}
	return dtStyle | DT_SINGLELINE;
}

HRGN setClipping(HDC hdc, RECT rc)
{
	auto hrgn = CreateRectRgn(0, 0, 0, 0);
	if (GetClipRgn(hdc, hrgn) != 1)
	{
		DeleteObject(hrgn);
		hrgn = NULL;
	}
	DPtoLP(hdc, reinterpret_cast<POINT *>(&rc), 2);
	if (GetLayout(hdc) & LAYOUT_RTL)
	{
		rc.left++;
		rc.right++;
	}
	IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
	return hrgn;
}

static std::tstring getWindowText(HWND hwnd)
{
	auto len = GetWindowTextLength(hwnd);
	std::tstring str(len, TCHAR());
	if (len > 0) {
		GetWindowText(hwnd, &str[0], len + 1);
	}
	return str;
}

static UINT CalcLabelRect(HWND hwnd, HDC hdc, RECT *rc)
{
	auto style = GetWindowLong(hwnd, GWL_STYLE);
	auto dtStyle = BStoDT(style, GetWindowLong(hwnd, GWL_EXSTYLE));
	auto r = *rc;

	bool bRet = true;
	if (BS_TEXT == (style & (BS_ICON | BS_BITMAP))) {
		std::tstring text = getWindowText(hwnd);
		if (!text.empty()) {
			bRet = false;
			HGDIOBJ hPrevFont = NULL;
			auto hFont = GetWindowFont(hwnd);
			if(hFont){
				hPrevFont = SelectObject(hdc, hFont);
			}
			DrawTextW(hdc, text.c_str(), -1, &r, dtStyle | DT_CALCRECT);
			if (hPrevFont) {
				SelectObject(hdc, hPrevFont);
			}
		}
	}
	if (bRet) {
		rc->right = r.left;
		rc->bottom = r.top;
		return UINT_MAX;
	}

	switch (dtStyle & (DT_CENTER | DT_RIGHT))
	{
	case DT_LEFT:
		r.left++;
		r.right++;
		break;
	case DT_CENTER:
	{
		auto width = r.right - r.left;
		r.left = rc->left + (rc->right - rc->left - width) / 2;
		r.right = r.left + width;
	}
		break;
	case DT_RIGHT:
	{
		auto width = r.right - r.left;
		r.right = rc->right - 1;
		r.left = r.right - width;
	}
		break;
	}

	switch (dtStyle & (DT_VCENTER | DT_BOTTOM))
	{
	case DT_TOP:
		r.top++;
		r.bottom++;
		break;
	case DT_VCENTER:
	{
		auto height = r.bottom - r.top;
		r.top = rc->top + (rc->bottom - 1 - rc->top - height) / 2 + 1;
		r.bottom = r.top + height;
	}
		break;
	case DT_BOTTOM:
	{
		auto height = r.bottom - r.top;
		r.bottom = rc->bottom - 1;
		r.top = r.bottom - height;
	}
		break;
	}
	*rc = r;
	return dtStyle;
}

static BOOL CALLBACK DrawTextCallback(HDC hdc, LPARAM lp, WPARAM wp, int cx, int cy)
{
	RECT rc = { 0,0,cx,cy };
	DrawText(hdc, reinterpret_cast<LPCWSTR>(lp), -1, &rc, static_cast<UINT>(wp));
	return TRUE;
}

static void DrawLabel(HWND hwnd, HDC hdc, UINT dtFlags, const RECT *rc)
{
	HBRUSH hbr = NULL;
	UINT flags = IsWindowEnabled(hwnd) ? DSS_NORMAL : DSS_DISABLED;
	auto style = GetWindowLong(hwnd, GWL_STYLE);

	if ((style & BS_PUSHLIKE) && (Button_GetState(hwnd) & BST_INDETERMINATE))
	{
		hbr = TxtMiruTheme_GetSysColorBrush(COLOR_GRAYTEXT);
		flags |= DSS_MONO;
	}
	if(!(style & (BS_ICON | BS_BITMAP)))
	{
		std::tstring text = getWindowText(hwnd);
		if (!text.empty()) {
			LPARAM lp = reinterpret_cast<LPARAM>(text.c_str());
			WPARAM wp = dtFlags;
			DrawState(hdc, hbr, DrawTextCallback, lp, wp, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, flags);
		}
	}
}

static void PaintPushButton(HWND hwnd, HDC hDC, UINT action)
{
	auto hFont = GetWindowFont(hwnd);
	if (hFont) {
		SelectObject(hDC, hFont);
	}
	RECT rc;
	GetClientRect(hwnd, &rc);
	auto hrgn = setClipping(hDC, rc);
	auto hpen = CreatePen(PS_SOLID, 1, TxtMiruTheme_GetSysColor(COLOR_WINDOWFRAME));
	auto hOldPen = SelectObject(hDC, hpen);
	auto hOldBrush = SelectObject(hDC, TxtMiruTheme_GetSysColorBrush(COLOR_BTNFACE));
	auto oldBkMode = SetBkMode(hDC, TRANSPARENT);

	auto state = Button_GetState(hwnd);
	FillRect(hDC, &rc, TxtMiruTheme_GetSysColorBrush(COLOR_BTNSHADOW));
	InflateRect(&rc, -2, -2);
	FillRect(hDC, &rc, TxtMiruTheme_GetSysColorBrush((state & BST_HOT) ? COLOR_BTNHIGHLIGHT: COLOR_BTNFACE));

	auto r = rc;
	auto dtFlags = CalcLabelRect(hwnd, hDC, &r);
	if (dtFlags != UINT_MAX) {
		if ((state & BST_PUSHED)) { // pushedState
			OffsetRect(&r, 1, 1);
		}
		auto oldTxtColor = SetTextColor(hDC, TxtMiruTheme_GetSysColor(COLOR_BTNTEXT));
		DrawLabel(hwnd, hDC, dtFlags, &r);
		SetTextColor(hDC, oldTxtColor);
		if (action == ODA_FOCUS || (state & BST_FOCUS))
		{
			DrawFocusRect(hDC, &rc);
		}
	}
	SelectObject(hDC, hOldPen);
	SelectObject(hDC, hOldBrush);
	SetBkMode(hDC, oldBkMode);
	SelectClipRgn(hDC, hrgn);
	if (hrgn) {
		DeleteObject(hrgn);
	}
	DeleteObject(hpen);
}

static void PaintCheckBox(HWND hwnd, HDC hDC, UINT action)
{
	auto style = GetWindowLong(hwnd, GWL_STYLE);
	if (style & BS_PUSHLIKE)
	{
		PaintPushButton(hwnd, hDC, action);
		return;
	}
	RECT client;
	GetClientRect(hwnd, &client);
	auto rbox = client;
	auto rtext = client;
	auto checkBoxWidth = 12 * GetDeviceCaps(hDC, LOGPIXELSX) / 96 + 1;

	auto hFont = GetWindowFont(hwnd);
	if (hFont) {
		SelectObject(hDC, hFont);
	}
	int text_offset;
	GetCharWidth(hDC, '0', '0', &text_offset);
	text_offset /= 2;

	auto hBrush = TxtMiruTheme_GetSysColorBrush(COLOR_WINDOW);
	auto hrgn = setClipping(hDC, client);
	if (style & BS_LEFTTEXT || GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_RIGHT)
	{
		rtext.right -= checkBoxWidth + text_offset;
		rbox.left = rbox.right - checkBoxWidth;
	}
	else
	{
		rtext.left += checkBoxWidth + text_offset;
		rbox.right = checkBoxWidth;
	}

	if (action == ODA_SELECT) {
		FillRect(hDC, &rbox, hBrush);
	} else if (action == ODA_DRAWENTIRE) {
		FillRect(hDC, &client, hBrush);
	}
	auto dtFlags = CalcLabelRect(hwnd, hDC, &rtext);
	if (dtFlags != UINT_MAX)
	{
		rbox.top = rtext.top;
		rbox.bottom = rtext.bottom;
	}
	auto state = Button_GetState(hwnd);
	if (action == ODA_DRAWENTIRE || action == ODA_SELECT)
	{
		UINT flags = DFCS_BUTTONCHECK;
		auto tmp_style = ButtonType(style);
		if ((tmp_style == BS_RADIOBUTTON) || (tmp_style == BS_AUTORADIOBUTTON)) {
			flags = DFCS_BUTTONRADIO;
		}
		else if (state & BST_INDETERMINATE) {
			flags = DFCS_BUTTON3STATE;
		}
		if (state & (BST_CHECKED | BST_INDETERMINATE)) {
			flags |= DFCS_CHECKED;
		}
		if (state & BST_PUSHED) {
			flags |= DFCS_PUSHED;
		}
		if (style & WS_DISABLED) {
			flags |= DFCS_INACTIVE;
		}
		auto checkBoxHeight = 12 * GetDeviceCaps(hDC, LOGPIXELSY) / 96 + 1;
		auto delta = rbox.bottom - rbox.top - checkBoxHeight;

		if (style & BS_TOP) {
			if (delta <= 0) {
				rbox.top -= -delta / 2 + 1;
			}
			rbox.bottom = rbox.top + checkBoxHeight;
		}
		else if (style & BS_BOTTOM) {
			if (delta <= 0) {
				rbox.bottom += -delta / 2 + 1;
			}
			rbox.top = rbox.bottom - checkBoxHeight;
		}
		else if (delta > 0) {
			rbox.bottom -= (delta / 2) + 1;
			rbox.top = rbox.bottom - checkBoxHeight;
		}
		else if (delta < 0) {
			rbox.top -= (-delta / 2) + 1;
			rbox.bottom = rbox.top + checkBoxHeight;
		}
		DrawFrameControl(hDC, &rbox, DFC_BUTTON, flags | DFCS_FLAT);
	}
	if (dtFlags != UINT_MAX) {
		if (action == ODA_DRAWENTIRE) {
			auto oldBkMode = SetBkMode(hDC, TRANSPARENT);
			auto oldTxtColor = SetTextColor(hDC, TxtMiruTheme_GetSysColor(COLOR_BTNTEXT));
			DrawLabel(hwnd, hDC, dtFlags, &rtext);
			SetTextColor(hDC, oldTxtColor);
			SetBkMode(hDC, oldBkMode);
		}
		if (action == ODA_FOCUS || (state & BST_FOCUS))
		{
			rtext.left--;
			rtext.right++;
			IntersectRect(&rtext, &rtext, &client);
			DrawFocusRect(hDC, &rtext);
		}
	}
	SelectClipRgn(hDC, hrgn);
	if (hrgn) {
		DeleteObject(hrgn);
	}
}

namespace CGrButton {
	LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		switch (uMsg) {
		case WM_PAINT:
			if (TxtMiruTheme_IsDarkThemeActive() && IsWindowVisible(hWnd)) {
				PAINTSTRUCT ps;
				BeginPaint(hWnd, &ps);
				auto nOldMode = SetBkMode(ps.hdc, OPAQUE);
				switch (ButtonType(GetWindowLong(hWnd, GWL_STYLE))) {
				case BS_PUSHBUTTON:
				case BS_DEFPUSHBUTTON:
				case BS_SPLITBUTTON:
				case BS_DEFSPLITBUTTON:
				case BS_COMMANDLINK:
				case BS_DEFCOMMANDLINK:
					PaintPushButton(hWnd, ps.hdc, ODA_DRAWENTIRE);
					break;
				case BS_CHECKBOX:
				case BS_AUTOCHECKBOX:
				case BS_RADIOBUTTON:
				case BS_3STATE:
				case BS_AUTO3STATE:
				case BS_AUTORADIOBUTTON:
					PaintCheckBox(hWnd, ps.hdc, ODA_DRAWENTIRE);
					break;
				}
				SetBkMode(ps.hdc, nOldMode);
				EndPaint(hWnd, &ps);
				return FALSE;
			}
			break;
		default:
			break;

		}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
}
