// WindowsCtrl.h
//
// サイズと位置をを調整します。
//   追加した後のクライアントウインドウを
//  lprectに格納します。
//
#ifndef __WINDOWCTRL_H__
#define __WINDOWCTRL_H__
#pragma warning(disable:4786)

#include "stltchar.h"
#include <windows.h>
#include <string>

class CGrWinCtrl
{
protected:
	HWND m_hWnd;
	HWND m_popupParent;
	BOOL m_bDialog;            // ダイアログか否か
	WNDPROC m_lpfnOldWndProc;  // ウィンドウプロシージャアドレス
public:
	CGrWinCtrl();
	virtual ~CGrWinCtrl();

	// InitCommonControlsEx
	//   dwICC [in]: dwICC
	void InitCommonControlsEx(DWORD dwICC);
	// HWNDを取得します。
	//   戻り値 [out]: HWND
	inline HWND GetWnd() { return m_hWnd; }
	// 上につけます。
	//   hWnd       [in]: HWND
	//   lprect [in,out]: クライアントのサイズ
	//                    上につけた後のクライアントサイズを返します。
	void SetWindowPosTop(LPRECT lprect) { SetWindowPosTop(m_hWnd, lprect); }
	void SetWindowPosTop(HWND hWnd, LPRECT lprect);
	// 下につけます。
	//   hWnd       [in]: HWND
	//   lprect [in,out]: クライアントのサイズ
	//                    下につけた後のクライアントサイズを返します。
	void SetWindowPosBottom(LPRECT lprect) { SetWindowPosBottom(m_hWnd, lprect); }
	void SetWindowPosBottom(HWND hWnd, LPRECT lprect);
	// 左につけます。
	//   hWnd       [in]: HWND
	//   lprect [in,out]: クライアントのサイズ
	//                    左につけた後のクライアントサイズを返します。
	void SetWindowPosLeft(LPRECT lprect) { SetWindowPosLeft(m_hWnd, lprect); }
	void SetWindowPosLeft(HWND hWnd, LPRECT lprect);
	// 右につけます。
	//   hWnd       [in]: HWND
	//   lprect [in,out]: クライアントのサイズ
	//                    右につけた後のクライアントサイズを返します。
	void SetWindowPosRight(LPRECT lprect) { SetWindowPosRight(m_hWnd, lprect); }
	void SetWindowPosRight(HWND hWnd, LPRECT lprect);
	// ウインドウ全体に配置します。
	//   hWnd       [in]: HWND
	//   lprect [in,out]: クライアントのサイズ
	void SetWindowPosAllClient(LPRECT lprect) { SetWindowPosAllClient(m_hWnd, lprect); }
	void SetWindowPosAllClient(HWND hWnd, LPRECT lprect);
	// ウインドウをlprectの下に配置します。
	//   lprect [in]: クライアントのサイズ
	//   ※ウインドウが表示しきれない場合は、上に配置します。
	//       ****** <- lprect |           | <- desktop       +--------+ | <- desktop
	//       +--------+       |   ******* |                  |        | |
	//       |        |       |   --------+              ->  +--------+ |
	//       +--------+       |        の時                     ******* |
	void SetWindowPosPopup(LPRECT lprect);
	void SetPopupParent(HWND hWnd) { m_popupParent = hWnd; }
	// ウインドウをセンターに表示
	void SetWindowPosCenter();
	// モニタ内でウインドウを移動
	void MoveWindowInMonitor(int left, int top, int width, int height);
	// スタイルの変更
	//   dwRemove [in]: スタイルの変更中に削除されるウィンドウ スタイルを指定します。
	//   dwAdd    [in]: スタイルの変更中に追加されるウィンドウ スタイルを指定します。
	void ModifyStyle(DWORD dwRemove, DWORD dwAdd)
	{
		SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) & ~(dwRemove) | (dwAdd));
	}
	void ClientToScreenRect(LPRECT lprc) { ClientToScreenRect(m_hWnd, lprc); }
	void ClientToScreenRect(HWND hWnd, LPRECT lprc);
	void ScreenToClientRect(LPRECT lprc) { ScreenToClientRect(m_hWnd, lprc); }
	void ScreenToClientRect(HWND hWnd, LPRECT lprc);
	// ホイールマウスのスクロール行数取得
	UINT GetMouseScrollLines();

	// 再描画
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
public:
	operator HWND() { return m_hWnd; }
	// ウィンドウハンドルとCWndBaseオブジェクトを結び付ける
	BOOL Attach(HWND hWnd);
	// ウィンドウハンドルをCWndBaseオブジェクトから切り離す
	BOOL Detach();

protected:
	// メッセージを振り分けるウィンドウ(ダイアログ)プロシージャ
	static LRESULT CALLBACK WindowMapProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
};

#define RECT_GET_WIDTH(__lprc__)        ((__lprc__)->right - (__lprc__)->left)
#define RECT_GET_HEIGHT(__lprc__)       ((__lprc__)->bottom - (__lprc__)->top)

#endif
