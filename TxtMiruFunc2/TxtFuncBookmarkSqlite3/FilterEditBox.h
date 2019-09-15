#ifndef __FILTEREDITBOX_H__
#define __FILTEREDITBOX_H__

#include "WindowCtrl.h"

class CGrFilterEditBox : public CGrWinCtrl
{
public:
	CGrFilterEditBox();
	virtual ~CGrFilterEditBox();

	void SetBackgroundText(LPCTSTR lpstr);
	int GetText(std::tstring &outstr);

protected:
	virtual BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
	virtual void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	virtual void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
	virtual void OnPaint(HWND hwnd);
	virtual BOOL OnEraseBkgnd(HWND hwnd, HDC hdc);
	virtual void OnChar(HWND hWnd, TCHAR ch, int cRepeat);
	virtual void OnImeChar(HWND hWnd, TCHAR ch, int cRepeat);
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	std::tstring m_bg_text;
	bool m_empty = true;
	bool m_barrow = true;
	void UpdateRect();
};

#endif // __FILTEREDITBOX_H__
