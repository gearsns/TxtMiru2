#ifndef __PROPPAGESIZE_H__
#define __PROPPAGESIZE_H__

#include "PropPage.h"
#define WM_SETPAGEFOCUS WM_APP+2
class CGrTxtConfigDlg;
class CGrPropPageSize : public CGrPropPage
{
public:
	CGrPropPageSize(CGrTxtConfigDlg *pDlg);
protected:
	int m_scroll_line = -1;
	SIZE m_minSize = {};
	SIZE m_minClientSize = {};
	POINT m_point = {};
	bool m_bCalc = false;
	CGrTxtConfigDlg *m_pConfigDlg = nullptr;
protected:
	int getScrollPos(int fnBar);
	void onxScroll(int fnBar, UINT code, int pos);
	void setScrollPos(int fnBar, SCROLLINFO &si);
	void setScrollRange(int cx, int cy);
	void setFocus(UINT id  );
	void setFocus(HWND hwnd);
public:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnSize(HWND hwnd, UINT nType, int cx, int cy);
	void OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
	void OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
	virtual void SetPropSheetPage(HINSTANCE hInst, HWND hWnd, PROPSHEETPAGE *psp){}
};

#endif // __PROPPAGESIZE_H__
