#include <windows.h>
#include <winuser.h>
#include <commctrl.h>
#include <stdlib.h>
#include "WindowCtrl.h"
#include "Win32Wrap.h"

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

// ホイールマウス対応
#include "zmouse.h"

CGrWinCtrl::CGrWinCtrl() : m_hWnd(NULL), m_popupParent(GetDesktopWindow()), m_bDialog(FALSE), m_lpfnOldWndProc(nullptr)
{
}

CGrWinCtrl::~CGrWinCtrl()
{
	Detach();
}

// 上につけます。
//   hWnd       [in]: HWND
//   lprect [in,out]: クライアントのサイズ
//                    上につけた後のクライアントサイズを返します。
void CGrWinCtrl::SetWindowPosTop(HWND hWnd, LPRECT lprect)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int nWidth  = RECT_GET_WIDTH(lprect);
	int nHeight = RECT_GET_HEIGHT(&rect);
	// 常に最上部に表示されるようにします。
	MoveWindow(hWnd, lprect->left, lprect->top, nWidth, nHeight, TRUE);
	// 高さ分クライアント領域を減らします。
	lprect->top += nHeight;
}

// 下につけます。
//   hWnd       [in]: HWND
//   lprect [in,out]: クライアントのサイズ
//                    下につけた後のクライアントサイズを返します。
void CGrWinCtrl::SetWindowPosBottom(HWND hWnd, LPRECT lprect)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int nWidth  = RECT_GET_WIDTH(lprect);
	int nHeight = RECT_GET_HEIGHT(&rect);
	// ステータスバーの高さ分クライアント領域を減らします。
	lprect->bottom -= nHeight;
	// サイズと位置調整（常に一番下にくるように設定します。）
	MoveWindow(hWnd, lprect->left, lprect->bottom, nWidth, nHeight, TRUE);
}

// 左につけます。
//   hWnd       [in]: HWND
//   lprect [in,out]: クライアントのサイズ
//                    左につけた後のクライアントサイズを返します。
void CGrWinCtrl::SetWindowPosLeft(HWND hWnd, LPRECT lprect)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int nWidth  = RECT_GET_WIDTH(&rect);
	int nHeight = RECT_GET_HEIGHT(lprect);
	// 常に左に表示されるようにします。
	MoveWindow(hWnd, lprect->left, lprect->top, nWidth, nHeight, TRUE);
	// 幅分クライアント領域を減らします。
	lprect->left += nWidth;
}

// 右につけます。
//   hWnd       [in]: HWND
//   lprect [in,out]: クライアントのサイズ
//                    右につけた後のクライアントサイズを返します。
void CGrWinCtrl::SetWindowPosRight(HWND hWnd, LPRECT lprect)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int nWidth  = RECT_GET_WIDTH(&rect);
	int nHeight = RECT_GET_HEIGHT(lprect);
	// 幅分クライアント領域を減らします。
	lprect->left -= nWidth;
	// 常に右に表示されるようにします。
	MoveWindow(hWnd, lprect->left, lprect->top, nWidth, nHeight, TRUE);
}

// ウインドウ全体に配置します。
//   hWnd       [in]: HWND
//   lprect [in,out]: クライアントのサイズ
void CGrWinCtrl::SetWindowPosAllClient(HWND hWnd, LPRECT lprect)
{
	int nWidth  = RECT_GET_WIDTH(lprect);
	int nHeight = RECT_GET_HEIGHT(lprect);
	MoveWindow(hWnd, lprect->left, lprect->top, nWidth, nHeight, TRUE);
}

// ウインドウをlprectの下に配置します。
//   lprect [in]: クライアントのサイズ
//   ※ウインドウが表示しきれない場合は、上に配置します。
//       ****** <- lprect |           | <- desktop       +--------+ | <- desktop
//       +--------+       |   ******* |                  |        | |
//       |        |       |   --------+              ->  +--------+ |
//       +--------+       |        の時                     ******* |
void CGrWinCtrl::SetWindowPosPopup(LPRECT lprect)
{
	RECT  rc, topRc;
	POINT pos;

	ClientToScreenRect(m_hWnd, lprect);
	pos.x = lprect->left;
	pos.y = lprect->bottom;
	GetWindowRect(m_hWnd, &rc);
	// 指定位置にウインドウを移動します。
	int width  = RECT_GET_WIDTH(&rc);
	int height = RECT_GET_HEIGHT(&rc);
	SetRect(&rc, pos.x, pos.y, pos.x+width, pos.y+height);
	// ウインドウが画面からはみ出ないようにします。
	GetWindowRect(m_popupParent, &topRc);
	long style = GetWindowLong(m_popupParent, GWL_STYLE);
	if(style & ~WS_HSCROLL){
		topRc.right -= GetSystemMetrics(SM_CXHSCROLL);
	}
	if(style & ~WS_VSCROLL){
		topRc.bottom -= GetSystemMetrics(SM_CYVSCROLL);
	}
	//   右にはみ出た
	if(rc.right > topRc.right){
		pos.x = topRc.right - width;
		if(pos.x < 0){
			pos.x = 0;
		}
	}
	//   下にはみ出た
	if(rc.bottom > topRc.bottom){
		pos.y = lprect->top - height;
		if(pos.y < 0){
			pos.y = 0;
		}
	}
	ScreenToClient(m_hWnd, &pos);
	MoveWindow(m_hWnd, pos.x, pos.y, width, height, TRUE);
}

// InitCommonControlsEx
//   dwICC [in]: dwICC
void CGrWinCtrl::InitCommonControlsEx(DWORD dwICC)
{
	// CommonControlの初期化を行う
	InitCommonControls();
	INITCOMMONCONTROLSEX iccex = { sizeof(INITCOMMONCONTROLSEX), dwICC };
	::InitCommonControlsEx(&iccex);
}

void CGrWinCtrl::ClientToScreenRect(HWND hWnd, LPRECT lprc)
{
	::ClientToScreen(hWnd, reinterpret_cast<LPPOINT>(lprc)  );
	::ClientToScreen(hWnd, reinterpret_cast<LPPOINT>(lprc)+1);
}

void CGrWinCtrl::ScreenToClientRect(HWND hWnd, LPRECT lprc)
{
	::ScreenToClient(hWnd, reinterpret_cast<LPPOINT>(lprc)  );
	::ScreenToClient(hWnd, reinterpret_cast<LPPOINT>(lprc)+1);
}

// pgm form Visual C TechTips
// ウィンドウハンドルとCGrWinCtrlオブジェクトを結び付ける
BOOL CGrWinCtrl::Attach(HWND hWnd)
{
	// ウィンドウハンドル hWnd を m_hWnd に保存
	if(!hWnd){
		return FALSE;
	}
	m_hWnd = hWnd;
	// ダイアログかウィンドウかを判定する
	m_bDialog = (GetWindowLong(hWnd, DWL_DLGPROC) != 0);
	int nIndex = m_bDialog ? DWL_DLGPROC : GWL_WNDPROC;

	// ウィンドウハンドルとCGrWinCtrlオブジェクトを結びつける
	SetProp(m_hWnd, _T("CPP_CLASS"), this);

	// 既存のウィンドウをサブクラス化する場合は、ウィンドウ(ダイアログ)
	// プロシージャをWindowMapProcに置き換える
	if(GetWindowLong(m_hWnd, nIndex) != reinterpret_cast<LONG>(WindowMapProc)){
		m_lpfnOldWndProc = reinterpret_cast<WNDPROC>(SetWindowLong(m_hWnd, nIndex, reinterpret_cast<LONG>(WindowMapProc)));
	}

	return TRUE;
}

// ウィンドウハンドルをCWndBaseオブジェクトから切り離す
BOOL CGrWinCtrl::Detach()
{
	if(!m_hWnd){
		return FALSE;
	}

	// ウィンドウがサブクラス化されている場合は、ウィンドウ(ダイアログ)
	// プロシージャを元に戻す。
	if(m_lpfnOldWndProc){
		SetWindowLong(m_hWnd, (m_bDialog ? DWL_DLGPROC : GWL_WNDPROC), reinterpret_cast<DWORD>(m_lpfnOldWndProc));
	}
	// ウィンドウハンドルをCWndBaseオブジェクトから切り離す
	RemoveProp(m_hWnd, _T("CPP_CLASS"));

	m_hWnd = NULL;

	return TRUE;
}

bool isSendChildNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(hWnd){
		auto *pChildWnd = static_cast<CGrWinCtrl*>(GetProp(hWnd, _T("CPP_CLASS")));
		return (pChildWnd && pChildWnd->OnChildNotify(uMsg, wParam, lParam, nullptr)) == TRUE;
	}
	return false;
}

// メッセージを振り分けるウィンドウ(ダイアログ)プロシージャ
LRESULT CALLBACK CGrWinCtrl::WindowMapProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// ウィンドウのプロパティリストからCGrWinCtrlへのポインタの取得を試みる
	auto* pTargetWnd = static_cast<CGrWinCtrl*>(GetProp(hWnd, _T("CPP_CLASS")));

	// CGrWinCtrlオブジェクトとウィンドウが結び付けられていない場合は
	// CGrWinCtrlオブジェクトとウィンドウを結び付ける
	if(!pTargetWnd){
		// CGrWinCtrlへのポインタを取得
		if((uMsg == WM_CREATE) || (uMsg == WM_NCCREATE)){
			pTargetWnd = static_cast<CGrWinCtrl*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
		} else if(uMsg == WM_INITDIALOG){
			pTargetWnd = reinterpret_cast<CGrWinCtrl*>(lParam);
		}
		// ウィンドウハンドルとCGrWinCtrlオブジェクトを結び付ける
		if(pTargetWnd){
			pTargetWnd->Attach(hWnd);
		}
	}

	// pTargetWnd の取得に成功した場合は、CGrWinCtrlのWndProcを呼び出す
	if(pTargetWnd){
		if(uMsg == WM_DRAWITEM){
			auto dw = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
			if(dw && dw->hwndItem != hWnd && isSendChildNotify(dw->hwndItem, uMsg, wParam, lParam)){
				SendMessage(dw->hwndItem, uMsg, wParam, lParam);
				return TRUE;
			}
		} else if(uMsg == WM_CTLCOLORSTATIC ||
				  uMsg == WM_CTLCOLOREDIT   ){
			auto cdlg = reinterpret_cast<HWND>(lParam);
			if(cdlg != hWnd && isSendChildNotify(cdlg, uMsg, wParam, lParam)){
				return SendMessage(cdlg, uMsg, wParam, lParam);
			}
		}
		// CGrWinCtrlのWndProcを呼び出す
		auto lResult = pTargetWnd->WndProc(hWnd, uMsg, wParam, lParam);
		// WM_DESTROYメッセージでウィンドウとCGrWinCtrlオブジェクトを切り離す
		// このWindowMapProcで行うことにより、派生クラスからウィンドウと
		// クラスの結びつきを意識せずにすむ
		if(uMsg == WM_DESTROY){
			pTargetWnd->Detach();
		} else if(uMsg == WM_NOTIFY){
			auto pnmh = reinterpret_cast<LPNMHDR>(lParam);
			if(pnmh && pnmh->hwndFrom != hWnd && isSendChildNotify(pnmh->hwndFrom, uMsg, wParam, lParam)){
				SendNotifyMessage(pnmh->hwndFrom, uMsg, wParam, lParam);
			}
		}
		return lResult;
	}

	// ダイアログの場合、FALSEを返す
	if(GetWindowLong(hWnd, DWL_DLGPROC)){
		return FALSE;
	}

	// デフォルトウィンドウプロシージャを呼び出す
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
LRESULT CGrWinCtrl::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// サブクラス化している場合は、古いウィンドウプロシージャに後の処理を任せる
	if(m_lpfnOldWndProc){
		return CallWindowProc(m_lpfnOldWndProc, hWnd, uMsg, wParam, lParam);
	}
	// ダイアログの場合、FALSEを返す
	if(m_bDialog){
		return FALSE;
	}
	// サブクラス化で無い場合は、デフォルトウィンドウプロシージャを呼び出す
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
// ホイールマウスのスクロール行数取得
UINT CGrWinCtrl::GetMouseScrollLines()
{
	static UINT uCachedScrollLines = 0;
	if(uCachedScrollLines != 0){
		return uCachedScrollLines;
	}
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uCachedScrollLines, 0);
	return uCachedScrollLines;
}

BOOL CGrWinCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	return FALSE;
}

BOOL CGrWinCtrl::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	return FALSE;
}

void CGrWinCtrl::SetWindowPosCenter()
{
	RECT rc_parent ={};
	RECT rc_dialog ={};
	::GetWindowRect(GetParent(m_hWnd), &rc_parent);
	::GetWindowRect(m_hWnd           , &rc_dialog);
	int x = rc_parent.left + (rc_parent.right  - rc_parent.left) / 2 - (rc_dialog.right  - rc_dialog.left) / 2;
	int y = rc_parent.top  + (rc_parent.bottom - rc_parent.top ) / 2 - (rc_dialog.bottom - rc_dialog.top ) / 2;
	::SetWindowPos(m_hWnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CGrWinCtrl::MoveWindowInMonitor(int left, int top, int width, int height)
{
	if(left >= 0 && top >= 0 && width >= 0 && height >= 0){
		// モニタ内に収まっているかチェック
		POINT point = {left, top};
		auto hMonitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
		if(hMonitor){
			MONITORINFO mi = {sizeof(MONITORINFO)};
			if(::GetMonitorInfo(hMonitor, &mi) && PtInRect(&mi.rcMonitor, point)){
				::MoveWindow(m_hWnd, left, top, width, height, TRUE);
			}
		}
	}
}
