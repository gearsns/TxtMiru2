#include <windowsx.h>
#include "KeyBind.h"
#include "KeyState.h"

CGrKeyBind::CGrKeyBind() : CGrComonCtrl()
{
}

CGrKeyBind::~CGrKeyBind()
{
}

bool GetKeyName(UINT nVK, LPTSTR str, int len){
	auto nScanCode = MapVirtualKeyEx(nVK, 0, GetKeyboardLayout(0));
	switch(nVK) {
		// Keys which are "extended" (except for Return which is Numeric Enter as extended)
	case VK_INSERT:
	case VK_DELETE:
	case VK_HOME:
	case VK_END:
	case VK_NEXT:  // Page down
	case VK_PRIOR: // Page up
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
		nScanCode |= 0x100; // Add extended bit
	}
	// GetKeyNameText() expects the scan code to be on the same format as WM_KEYDOWN
	// Hence the left shift
	auto bResult = GetKeyNameText(nScanCode << 16, str, len);
	return bResult != FALSE;
}

LRESULT CGrKeyBind::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_GETDLGCODE: ImmAssociateContext(hWnd, NULL); return DLGC_WANTALLKEYS;
	case WM_IME_NOTIFY: return TRUE;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		{
			std::tstring key_name;
			CGrKeyboardState ks(wParam, m_ks.Delay());
			TCHAR buf[512];
			if(ks.Shift()){
				GetKeyName(VK_SHIFT, buf, sizeof(buf)/sizeof(TCHAR));
				key_name += buf;
				key_name += _T(" + ");
			}
			if(ks.Ctrl()){
				GetKeyName(VK_CONTROL, buf, sizeof(buf)/sizeof(TCHAR));
				key_name += buf;
				key_name += _T(" + ");
			}
			if(ks.Alt()){
				GetKeyName(VK_MENU, buf, sizeof(buf)/sizeof(TCHAR));
				key_name += buf;
				key_name += _T(" + ");
			}
			if(ks.Delay()){
				key_name += _T("Delay + ");
			}
			switch(ks.Key()){
			case VK_SHIFT:
			case VK_CONTROL:
			case VK_MENU:
			case VK_RMENU:
			case VK_LMENU:
				break;
			default:
				GetKeyNameText(lParam, buf, sizeof(buf)/sizeof(TCHAR));
				key_name += buf;
				break;
			}
			m_ks = ks;
			SetWindowText(hWnd, key_name.c_str());
		}
		return TRUE;
	case WM_SYSKEYUP:
	case WM_KEYUP:
	case WM_CHAR:
		return TRUE;
	}
	return CGrComonCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}
