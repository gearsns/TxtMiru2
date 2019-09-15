#ifndef __MESSAGEWND_H__
#define __MESSAGEWND_H__

#include "WindowCtrl.h"
#include "Font.h"

class CGrMessageWnd : public CGrWinCtrl
{
public:
	CGrMessageWnd();
	virtual ~CGrMessageWnd();
	void Show(HWND hWnd, LPCTSTR lpStr);
	void Hide();
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	void OnPaint(HWND hwnd);
	BOOL InitApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm);
private:
	void setWindowSize(int cx, int cy);
	CGrFont m_font;
	bool m_bHide;
	std::tstring m_message;
};

#endif // __MESSAGEWND_H__
