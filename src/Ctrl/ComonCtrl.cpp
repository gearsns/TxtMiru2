#include <windows.h>
#include <windowsx.h>
#include "ComonCtrl.h"

CGrComonCtrl::CGrComonCtrl() : m_exitKeyCode(0), m_bShift(FALSE), m_bPopup(FALSE)
{
}

CGrComonCtrl::~CGrComonCtrl()
{
}

// メインウィンドウ
LRESULT CGrComonCtrl::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!m_bPopup){
		return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
	}
	switch(uMsg){
	case WM_DESTROY:
		m_exitKeyCode = 0;
		m_bShift  = FALSE;
		return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	case WM_CHAR:
		if(wParam == VK_ESCAPE || wParam == VK_RETURN ||
		   wParam == VK_TAB){
			m_exitKeyCode = wParam;
			m_bShift = HIBYTE(GetKeyState(VK_SHIFT));
			SetFocus(GetParent(hWnd));
			break;
		}
		return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_KILLFOCUS:
		CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
		break;
	default:
		return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
	}

	return FALSE;
}
