#ifndef __OPENURLDLG_H__
#define __OPENURLDLG_H__

#include "WindowCtrl.h"
#include "ComonCtrl.h"
class CGrURLEditBox : public CGrComonCtrl
{
public:
	CGrURLEditBox();
	virtual ~CGrURLEditBox();

	void SetImage(UINT id);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
private:
	HIMAGELIST m_hImage = NULL;
	int m_imageCount = 0;
	int m_imageNo = 0;
	int m_iconSize = 0;
};

class CGrOpenURLDlg : public CGrComonCtrl
{
public:
	CGrOpenURLDlg();
	~CGrOpenURLDlg();
	int DoModal(HWND hWnd, LPCTSTR url);
	void Show(HWND hWnd, LPCTSTR url);
	void GetURL(std::tstring &url);
	bool IsRefresh();
	void Close();
	static bool IsWaitting();
protected:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void SetWindowSize();
private:
	bool m_doModal = false;
	bool m_refresh = false;;
	HWND m_hParentWnd = NULL;
	std::tstring m_url;
	CGrURLEditBox m_editBox;
};

#endif // __OPENURLDLG_H__
