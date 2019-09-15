#ifndef __TXTGOTOPAGEDLG_H__
#define __TXTGOTOPAGEDLG_H__

#include "WindowCtrl.h"

class CGrTxtGotoPageDlg : CGrWinCtrl
{
public:
	CGrTxtGotoPageDlg(int maxnum, int cur_page);
	virtual ~CGrTxtGotoPageDlg();
	int DoModal(HWND hWnd);
	int GetPage(){ return m_nCurPage; }
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
private:
	int m_nMaxPage;
	int m_nCurPage;
};

#endif // __TXTGOTOPAGEDLG_H__
