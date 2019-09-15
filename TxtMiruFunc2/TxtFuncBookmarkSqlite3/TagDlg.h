#ifndef __TAGDLG_H__
#define __TAGDLG_H__

#include "WindowCtrl.h"
#include "TxtFuncBookmarkDef.h"
#include "EditBox.h"

class CGrTagDlg : public CGrWinCtrl
{
public:
	CGrTagDlg(TxtFuncBookmark::Tag &tag);
	virtual ~CGrTagDlg();
	int DoModal(HWND hWnd);
protected:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
private:
	TxtFuncBookmark::Tag &m_tag;
	CGrEditBox m_EditTag;
};

#endif // __TAGDLG_H__
