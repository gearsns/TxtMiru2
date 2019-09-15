#ifndef __PROPPAGERUBYLIST_H__
#define __PROPPAGERUBYLIST_H__

#include "PropPageSize.h"

class CGrPropPageRubyList : public CGrPropPageSize
{
public:
	static CGrPropPage* PASCAL CreateProp(CGrTxtConfigDlg *pDlg);
	virtual ~CGrPropPageRubyList();
	virtual bool Apply();
private:
	CGrPropPageRubyList(CGrTxtConfigDlg *pDlg);
protected:
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // __PROPPAGERUBYLIST_H__
