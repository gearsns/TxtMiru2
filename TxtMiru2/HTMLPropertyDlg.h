#ifndef __HTMLPROPERTYDLG_H__
#define __HTMLPROPERTYDLG_H__

#include "WindowCtrl.h"
#include "TxtDocument.h"

class CGrHTMLPropertyDlg : CGrWinCtrl
{
public:
	CGrHTMLPropertyDlg(const CGrTxtDocument *pDoc);
	virtual ~CGrHTMLPropertyDlg();
	int DoModal(HWND hWnd);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
private:
	const CGrTxtDocument *m_pDoc;
};

#endif // __HTMLPROPERTYDLG_H__
