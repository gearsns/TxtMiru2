// stdwin.h
//
// メインウインドウ作成登録
//
#ifndef __STDWIN_H__
#define __STDWIN_H__

class CGrStdWin
{
public:
	static HINSTANCE GetInst() { return m_hInst; }
	static HWND      GetWnd()  { return m_hWnd;  }
private:
	static HINSTANCE m_hInst;
	static HWND      m_hWnd;
	int    ole_initialized;
public:
	CGrStdWin(HINSTANCE hInst);
	virtual ~CGrStdWin();
public:
	// ウィンドウ・クラスの登録
	BOOL InitApp(WNDPROC fnWndProc, LPCTSTR szClassNm, UINT idi_app, UINT idr_menu);
	// ウィンドウの生成
	HWND InitInstance(int nCmdShow, LPCTSTR szClassName, LPCTSTR szAppName, LPWINDOWPLACEMENT lpwndp);
};

#endif
