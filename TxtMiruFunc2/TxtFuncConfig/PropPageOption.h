#ifndef __PROPPAGEOPTION_H__
#define __PROPPAGEOPTION_H__

#include "PropPageSize.h"

class CGrPropPageOption : public CGrPropPageSize
{
public:
	static CGrPropPage* PASCAL CreateProp(CGrTxtConfigDlg *pDlg);
	virtual ~CGrPropPageOption();
	virtual bool Apply();
private:
	CGrPropPageOption(CGrTxtConfigDlg *pDlg);
protected:
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // __PROPPAGEOPTION_H__
