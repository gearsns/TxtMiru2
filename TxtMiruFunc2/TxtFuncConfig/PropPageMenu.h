#ifndef __PROPPAGEMENU_H__
#define __PROPPAGEMENU_H__

#include <vector>
#include "PropPageSize.h"
#include "ListBox.h"
#include "TxtMiruFunc.h"

class CGrPropPageMenu : public CGrPropPageSize
{
public:
	static CGrPropPage* PASCAL CreateProp(CGrTxtConfigDlg *pDlg);
	virtual ~CGrPropPageMenu();
	virtual bool Apply();
private:
	CGrPropPageMenu(CGrTxtConfigDlg *pDlg);
	void setWindowSize(int cx, int cy);
protected:
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnSize(HWND hwnd, UINT nType, int cx, int cy);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	CGrListBox m_listFunc;
	CGrListBox m_listMenu;
};

#endif // __PROPPAGEMENU_H__
