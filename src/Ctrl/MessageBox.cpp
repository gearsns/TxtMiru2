#pragma warning( disable : 4786 )

#include "MessageBox.h"
#include "Text.h"

namespace CGrMessageBox {
	static HHOOK m_hHook = NULL;
	static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if(HCBT_ACTIVATE == nCode){
			RECT rc_parent ={};
			RECT rc_msg_box={};
			auto hwnd = reinterpret_cast<HWND>(wParam);
			::GetWindowRect(GetParent(hwnd), &rc_parent );
			::GetWindowRect(hwnd           , &rc_msg_box);
			int x = rc_parent.left + (rc_parent.right  - rc_parent.left) / 2 - (rc_msg_box.right  - rc_msg_box.left) / 2;
			int y = rc_parent.top  + (rc_parent.bottom - rc_parent.top ) / 2 - (rc_msg_box.bottom - rc_msg_box.top ) / 2;
			::SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			auto result = ::CallNextHookEx(m_hHook, nCode, wParam, lParam);
			::UnhookWindowsHookEx(m_hHook);
			m_hHook = NULL;
			return result;
		}
		return ::CallNextHookEx(m_hHook, nCode, wParam, lParam);
	}
#ifdef _USRDLL
	int Show(HINSTANCE hInst, HWND hwnd, UINT id, UINT title_id, UINT nType)
	{
		std::tstring str;
		CGrText::LoadString(hInst, id, str);
		std::tstring title;
		if(title_id == 0){
			title = _T("Error");
		} else {
			CGrText::LoadString(hInst, title_id, title);
		}
		return Show(hInst, hwnd, str.c_str(), title.c_str(), nType);
	}
	int Show(HINSTANCE hInst, HWND hwnd, UINT id, LPCTSTR lpszCaption, UINT nType)
	{
		std::tstring str;
		CGrText::LoadString(hInst, id, str);
		return Show(hInst, hwnd, str.c_str(), lpszCaption, nType);
	}
	int Show(HINSTANCE hInst, HWND hwnd, LPCTSTR lpszText, LPCTSTR lpszCaption, UINT nType)
	{
		int ret;
		m_hHook = ::SetWindowsHookEx(WH_CBT, CBTProc, hInst, ::GetCurrentThreadId());
		ret = ::MessageBox(hwnd, lpszText, lpszCaption, nType);
		m_hHook = NULL;
		return ret;
	}
#else
	int Show(HWND hwnd, UINT id, UINT title_id, UINT nType)
	{
		std::tstring str;
		CGrText::LoadString(id, str);
		std::tstring title;
		if(title_id == 0){
			title = _T("Error");
		} else {
			CGrText::LoadString(title_id, title);
		}
		return Show(hwnd, str.c_str(), title.c_str(), nType);
	}
	int Show(HWND hwnd, UINT id, LPCTSTR lpszCaption, UINT nType)
	{
		std::tstring str;
		CGrText::LoadString(id, str);
		return Show(hwnd, str.c_str(), lpszCaption, nType);
	}
	int Show(HWND hwnd, LPCTSTR lpszText, LPCTSTR lpszCaption, UINT nType)
	{
		int ret;
		m_hHook = ::SetWindowsHookEx(WH_CBT, CBTProc, reinterpret_cast<HINSTANCE>(::GetWindowLong(hwnd, GWL_HINSTANCE)), ::GetCurrentThreadId());
		ret = ::MessageBox(hwnd, lpszText, lpszCaption, nType);
		m_hHook = NULL;
		return ret;
	}
#endif
};
