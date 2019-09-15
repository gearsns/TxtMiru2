#include <windowsx.h>
#include "EditKeyBind.h"
#include "KeyState.h"

CGrEditKeyBind::CGrEditKeyBind() : CGrComonCtrl()
{
}

CGrEditKeyBind::~CGrEditKeyBind()
{
}

LRESULT CGrEditKeyBind::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_GETDLGCODE: ImmAssociateContext(hWnd, NULL); return DLGC_WANTALLKEYS;
	case WM_IME_NOTIFY: return TRUE;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		{
			std::tstring key_name;
			CGrKeyboardState ks(wParam);
			TCHAR buf[512];
			if(ks.Shift()){
				CGrKeyboardState::GetKeyName(VK_SHIFT, buf, sizeof(buf)/sizeof(TCHAR));
				key_name += buf;
				key_name += _T(" + ");
			}
			if(ks.Ctrl()){
				CGrKeyboardState::GetKeyName(VK_CONTROL, buf, sizeof(buf)/sizeof(TCHAR));
				key_name += buf;
				key_name += _T(" + ");
			}
			if(ks.Alt()){
				CGrKeyboardState::GetKeyName(VK_MENU, buf, sizeof(buf)/sizeof(TCHAR));
				key_name += buf;
				key_name += _T(" + ");
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
			::SetWindowText(hWnd, key_name.c_str());
		}
		return TRUE;
	case WM_SYSKEYUP:
	case WM_KEYUP:
	case WM_CHAR:
		return TRUE;
	}
	return CGrComonCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

void CGrEditKeyBind::SetKeyboardState(const CGrKeyboardState &ks)
{
	std::tstring key_name;
	ks.GetKeyName(key_name);
	m_ks = ks;
	::SetWindowText(m_hWnd, key_name.c_str());
}
