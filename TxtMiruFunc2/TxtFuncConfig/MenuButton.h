#ifndef __MENUBUTTON_H__
#define __MENUBUTTON_H__

#include "ComonCtrl.h"

class CGrMenuButton : public CGrComonCtrl
{
public:
	CGrMenuButton();
	virtual ~CGrMenuButton();

	virtual void DrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItemStruct);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);

	BOOL Attach(HWND hWnd);
	void SetCheck(bool checked);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	bool m_bHover = false;
	bool m_bChecked = false;
};

#endif // __MENUBUTTON_H__
