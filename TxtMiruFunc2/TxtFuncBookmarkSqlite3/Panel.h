#ifndef __PANEL_H__
#define __PANEL_H__

#include "WindowCtrl.h"
#include "ToolBar.h"
#include "TxtFunc.h"
#include <vector>
#include "ListView.h"
#include "TreeListView.h"
#include "WindowTheme.h"

class CGrListViewTxtMiru : public CGrListView
{
public:
	CGrListViewTxtMiru();
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	bool m_bRedraw = true;
};

class CGrTreeListViewTxtMiru : public CGrTreeListView
{
public:
	CGrTreeListViewTxtMiru();
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

void CGrPanelWnd_SetTouchMenu(HMENU hMenu);
class CGrBookListDlg;
class CGrPanelWnd : public CGrWinCtrl
{
protected:
	CGrCoolBar m_coolBar;
	CGrBookListDlg &m_booklistDlg;
	const int listwnd_id = 3000;
	const int coolbar_id = 3001;
	const int toolbar_id = 3002;
	CGrListViewTxtMiru m_listView;
public:
	CGrPanelWnd(CGrBookListDlg &dlg);
	virtual ~CGrPanelWnd();
	virtual bool Create(HWND hParent, HIMAGELIST hImg);

	int GetColumnCount() const;
	int GetColumnWidth(int iCol) const;
	void SetColumnWidth(int iCol, int cx);
protected:
	BOOL InitApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm);
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	virtual void OnSize(HWND hwnd, UINT state, int cx, int cy);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
	int GetFocusedItem() const;
	void InsertColumn(UINT id, int iColumn, int iWidth, int iFormat);
};

class CGrTreePanelWnd : public CGrWinCtrl
{
protected:
	CGrCoolBar m_coolBar;
	CGrBookListDlg &m_booklistDlg;
	const int listwnd_id = 3000;
	const int coolbar_id = 3001;
	const int toolbar_id = 3002;
	CGrTreeListViewTxtMiru m_listView;
public:
	CGrTreePanelWnd(CGrBookListDlg &dlg);
	virtual ~CGrTreePanelWnd();
	virtual bool Create(HWND hParent, HIMAGELIST hImg, HIMAGELIST hTreeImg);

	int GetColumnCount() const;
	int GetColumnWidth(int iCol) const;
	void SetColumnWidth(int iCol, int cx);
protected:
	BOOL InitApp(HINSTANCE hInst, WNDPROC fnWndProc, LPCTSTR szClassNm);
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	virtual void OnSize(HWND hwnd, UINT state, int cx, int cy);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);

	int GetFocusedItem() const;
};

#endif // __PANEL_H__
