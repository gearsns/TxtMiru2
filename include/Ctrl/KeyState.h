#ifndef __KEYSTATE_H__
#define __KEYSTATE_H__

#ifndef VK_TILT_LBUTTON
#define VK_TILT_LBUTTON     0xff01
#define VK_TILT_RBUTTON     0xff02
#define VK_WHEEL_SCROLLUP   0xff03
#define VK_WHEEL_SCROLLDW   0xff04
#define VK_LBUTTON_DBL      0xff05
#define VK_RBUTTON_DBL      0xff06
#define VK_MBUTTON_DBL      0xff07
#define VK_XBUTTON1_DBL     0xff08
#define VK_XBUTTON2_DBL     0xff09
#define VK_TILT_LBUTTON_DBL 0xff0a
#define VK_TILT_RBUTTON_DBL 0xff0b
#endif

class CGrKeyboardState
{
	UINT m_vk;
	bool m_shift;
	bool m_ctrl;
	bool m_alt;
	bool m_delay;

public:
	CGrKeyboardState()
	{
		m_vk    = 0;
		m_shift = false;
		m_ctrl  = false;
		m_alt   = false;
		m_delay = false;
	}
	CGrKeyboardState(const CGrKeyboardState &key)
	{
		m_vk    = key.m_vk   ;
		m_shift = key.m_shift;
		m_ctrl  = key.m_ctrl ;
		m_alt   = key.m_alt  ;
		m_delay = key.m_delay;
	}
	CGrKeyboardState(const UINT vk, bool delay = false)
	{
		m_vk    = vk;
		m_shift = isKeyDown(VK_SHIFT  );
		m_ctrl  = isKeyDown(VK_CONTROL);
		m_alt   = isKeyDown(VK_MENU   );
		m_delay = delay;
	}
	CGrKeyboardState(const UINT vk, const bool shift, const bool ctrl, const bool alt, bool delay = false)
	{
		m_vk    = vk;
		m_shift = shift;
		m_ctrl  = ctrl ;
		m_alt   = alt  ;
		m_delay = delay;
	}
	CGrKeyboardState &operator =(const UINT vk)
	{
		m_vk    = vk;
		m_shift = isKeyDown(VK_SHIFT  );
		m_ctrl  = isKeyDown(VK_CONTROL);
		m_alt   = isKeyDown(VK_MENU   );
		m_delay = false;
		return *this;
	}
	CGrKeyboardState &operator =(const CGrKeyboardState &key)
	{
		m_vk    = key.m_vk   ;
		m_shift = key.m_shift;
		m_ctrl  = key.m_ctrl ;
		m_alt   = key.m_alt  ;
		m_delay = key.m_delay;
		return *this;
	}
	bool Shift() const { return m_shift; }
	bool Ctrl () const { return m_ctrl ; }
	bool Alt  () const { return m_alt  ; }
	bool Delay() const { return m_delay; }
	UINT Key  () const { return m_vk   ; }
	bool isKeyDown(const UINT key) const
	{
		return (GetKeyState(key) & 0x80) ? true : false;
	}
	bool operator ==(const UINT vk) const
	{
		return
			(m_vk == vk
			 && m_shift == isKeyDown(VK_SHIFT  )
			 && m_ctrl  == isKeyDown(VK_CONTROL)
			 && m_alt   == isKeyDown(VK_MENU   )
			 && m_delay == false);
	}
	bool operator ==(const CGrKeyboardState &key) const
	{
		return
			(m_vk == key.m_vk
			 && m_shift == key.m_shift
			 && m_ctrl  == key.m_ctrl
			 && m_alt   == key.m_alt
			 && m_delay == key.m_delay);
	}
	bool operator <(const CGrKeyboardState &key) const
	{
		if(m_shift != key.m_shift){
			return m_shift < key.m_shift;
		}
		if(m_ctrl != key.m_ctrl){
			return m_ctrl < key.m_ctrl;
		}
		if(m_alt != key.m_alt){
			return m_alt < key.m_alt;
		}
		if(m_delay != key.m_delay){
			return m_delay < key.m_delay;
		}
		return m_vk < key.m_vk;
	}
	void to_s(std::tstring &str) const
	{
		TCHAR buf[512];
		_stprintf_s(buf, _T("%s%s%s%s%X"),
				  m_shift ? _T("S-") : _T(""),
				  m_ctrl  ? _T("C-") : _T(""),
				  m_alt   ? _T("M-") : _T(""),
				  m_delay ? _T("D-") : _T(""),
				  m_vk);
		str = buf;
	}
	void GetKeyName(std::tstring &str) const {
		TCHAR buf[512];
		if(m_shift){
			CGrKeyboardState::GetKeyName(VK_SHIFT, buf, sizeof(buf)/sizeof(TCHAR));
			str += buf;
			str += _T(" + ");
		}
		if(m_ctrl){
			CGrKeyboardState::GetKeyName(VK_CONTROL, buf, sizeof(buf)/sizeof(TCHAR));
			str += buf;
			str += _T(" + ");
		}
		if(m_alt){
			CGrKeyboardState::GetKeyName(VK_MENU, buf, sizeof(buf)/sizeof(TCHAR));
			str += buf;
			str += _T(" + ");
		}
		if(m_delay){
			str += _T("Delay + ");
		}
		switch(m_vk){
		case VK_SHIFT:
		case VK_CONTROL:
		case VK_MENU:
		case VK_RMENU:
		case VK_LMENU:
			break;
		default:
			if(GetKeyName(m_vk, buf, sizeof(buf)/sizeof(TCHAR))){
				str += buf;
			}
			break;
		}
	}
	static bool GetKeyName(UINT nVK, LPTSTR str, int len){
		UINT nScanCode = MapVirtualKeyEx(nVK, 0, GetKeyboardLayout(0));
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
			break;
		case VK_LBUTTON         : _tcscpy_s(str, len, _T("Left mouse button"            )); return TRUE;
		case VK_RBUTTON         : _tcscpy_s(str, len, _T("Right mouse button"           )); return TRUE;
		case VK_MBUTTON         : _tcscpy_s(str, len, _T("Middle mouse button"          )); return TRUE;
		case VK_XBUTTON1        : _tcscpy_s(str, len, _T("Left Side mouse button"       )); return TRUE;
		case VK_XBUTTON2        : _tcscpy_s(str, len, _T("Right Side mouse button"      )); return TRUE;
		case VK_TILT_LBUTTON    : _tcscpy_s(str, len, _T("Tilt Left mouse button"       )); return TRUE;
		case VK_TILT_RBUTTON    : _tcscpy_s(str, len, _T("Tilt Right mouse button"      )); return TRUE;
		case VK_WHEEL_SCROLLUP  : _tcscpy_s(str, len, _T("Wheel scroll up"              )); return TRUE;
		case VK_WHEEL_SCROLLDW  : _tcscpy_s(str, len, _T("Wheel scroll down"            )); return TRUE;
		case VK_LBUTTON_DBL     : _tcscpy_s(str, len, _T("Left mouse double click"      )); return TRUE;
		case VK_RBUTTON_DBL     : _tcscpy_s(str, len, _T("Right mouse double click"     )); return TRUE;
		case VK_MBUTTON_DBL     : _tcscpy_s(str, len, _T("Middle mouse double click"    )); return TRUE;
		case VK_XBUTTON1_DBL    : _tcscpy_s(str, len, _T("Left Side mouse double click" )); return TRUE;
		case VK_XBUTTON2_DBL    : _tcscpy_s(str, len, _T("Right Side mouse double click")); return TRUE;
		case VK_TILT_LBUTTON_DBL: _tcscpy_s(str, len, _T("Tilt Left mouse double click" )); return TRUE;
		case VK_TILT_RBUTTON_DBL: _tcscpy_s(str, len, _T("Tilt Right mouse double click")); return TRUE;
		}
		// GetKeyNameText() expects the scan code to be on the same format as WM_KEYDOWN
		// Hence the left shift
		BOOL bResult = GetKeyNameText(nScanCode << 16, str, len);
		return bResult != FALSE;
	}
};

#endif // __KEYSTATE_H__
