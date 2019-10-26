#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>
#include <windowsx.h>
#include <ShlObj.h>
#include <wininet.h>
#include <memory>
#include <regex>
#include "resource.h"
#include "stdwin.h"
#include "TxtMiru.h"
#include "TxtDocument.h"
#include "KeyState.h"
#include "TxtGotoPageDlg.h"
#include "TxtFuncBookmark.h"
#include "FunctionKeyMap.h"
#include "WaitCursor.h"
#include "TxtMiruDef.h"
#include "VersionDlg.h"
#include "Tooltips.h"
#define TXTMIRUMENU_ID_H
#include "TxtMiruFunc.h"
#include "MessageBox.h"
#include "TxtDocInfoDlg.h"
#include "MessageWnd.h"
#include "LupeWnd.h"
#include "OpenUrlDlg.h"
#include "FileUpdateWatcher.h"
#include "Gesture.h"
#include "ImageFile.h"
#include "Menu.h"
#include "TxtMiruTheme.h"
#include "TxtMiruMenu.h"
#include "AozoraList.h"
#include "MouseEvent.h"
#include "Win32Wrap.h"

// ホイールマウス対応
#ifndef WM_MOUSEWHEEL
#include "zmouse.h"
#endif
// マウスサイドボタン対応
#ifndef HANDLE_WM_XBUTTONUP
#define HANDLE_WM_XBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_XBUTTONUP(hwnd, x, y, keyFlags, fn) \
    (void)(fn)((hwnd), WM_XBUTTONUP, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))
#define HANDLE_WM_XBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_XBUTTONDOWN(hwnd, fDoubleClick, x, y, keyFlags, fn) \
    (void)(fn)((hwnd), (fDoubleClick) ? WM_XBUTTONDBLCLK : WM_XBUTTONDOWN, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))
#endif

#include "Shell.h"
#include "TxtCanvas.h"
#include "Image.h"
#include "TxtFuncPict.h"

#define KEYBIND_FILE   _T("keybind.ini")
enum {
	ID_TIMER_PAGEFLIP = 100 ,
	ID_TIMER_LUPE           ,
	ID_TIMER_AUTOSAVE       , // Bookmark Auto Save
};
#define IDRPREPARSERMENU (IDPREPARSER_DEF+1)
#define IDINITSTYLEMENU  (IDINITSTYLE+1)
#define TXTMIRU_PROP_NAME _T("TxtMiru2.0-Server")

#endif // __MAIN_H__
