#ifndef __TXTCONFIGDLG_H__
#define __TXTCONFIGDLG_H__

#include <map>
class CGrPropPage;
typedef std::map<UINT, CGrPropPage *> PROPPAGE_LIST;

#include "WindowCtrl.h"
#include "MenuButton.h"
class CGrTxtConfigDlg : public CGrWinCtrl
{
public:
	CGrTxtConfigDlg();
	virtual ~CGrTxtConfigDlg();

	int DoModal(HWND hWnd, LPCTSTR lpkeyBindFileName);

	LPCTSTR GetKeyBindFileName() const { return m_keyBindFileName.c_str(); }
private:
	void setWindowSize(int cx, int cy);
	void showPage(int page);
	PROPPAGE_LIST m_list;
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
	BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	void OnSize(HWND hwnd, UINT nType, int cx, int cy);
	void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
protected:
	std::tstring m_keyBindFileName;
	SIZE m_minSize = {};
	enum { MBTN_STYLE, MBTN_BOOKMARK, MBTN_HTML, MBTN_OPTION, MBTN_MENU, MBTN_KEY, MBTN_RUBY, MBTN_MaxNum };
	CGrMenuButton m_menuButton[MBTN_MaxNum];
};

#endif // __TXTCONFIGDLG_H__
