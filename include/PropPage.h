//
// オプション設定
//
#ifndef __PROP_PAGE_H__
#define __PROP_PAGE_H__

#include "WindowCtrl.h"

class CGrPropPage : public CGrWinCtrl
{
public:
	CGrPropPage();

	// ページの設定
	//   hWnd [in]: HWND
	//   psp  [in]: PROPSHEETPAGE
	virtual void SetPropSheetPage(HINSTANCE hInst, HWND hWnd, PROPSHEETPAGE *psp);

	virtual bool Apply() { return true; }
	bool IsApply() { return bApply; }
protected:
	// メッセージを振り分けるウィンドウ(ダイアログ)プロシージャ
	static LRESULT CALLBACK PropWindowMapProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
protected:
	TCHAR m_title[512];
	bool bApply;
};

#endif
