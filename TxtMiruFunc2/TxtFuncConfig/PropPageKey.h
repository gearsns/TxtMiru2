#ifndef __PROPPAGEKEY_H__
#define __PROPPAGEKEY_H__

#include "PropPageSize.h"
#include "EditKeyBind.h"
#include "KeyState.h"
#include "ListBox.h"
#include "ListView.h"
#include "FunctionKeyMap.h"
#include <vector>

class CGrPropPageKey : public CGrPropPageSize
{
public:
	static CGrPropPage* PASCAL CreateProp(CGrTxtConfigDlg *pDlg);
	virtual ~CGrPropPageKey();
	virtual bool Apply();
private:
	CGrPropPageKey(CGrTxtConfigDlg *pDlg);
	void setWindowSize(int cx, int cy);
	int GetFocusedItem() const;
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
protected:
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnSize(HWND hwnd, UINT nType, int cx, int cy);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	std::tstring m_filename;
	CGrEditKeyBind m_editKeyBind;
	CGrListView m_listFunc;
	CGrListBox m_listKey;
	CGrFunctionKeyMap::FunctionMap m_funcMap;
	std::vector<CGrKeyboardState> m_keyStateList;
	bool m_bDelay = false;

	void updateKeyList();
};

#endif // __PROPPAGEKEY_H__
