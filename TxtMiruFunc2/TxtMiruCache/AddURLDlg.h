#pragma once
#include "WindowCtrl.h"
#include "EditBox.h"

class CGrAddURLDlg :
	public CGrWinCtrl
{
public:
	CGrAddURLDlg();
	virtual ~CGrAddURLDlg();
	int DoModal(HINSTANCE hinstance, HWND hWnd);
	void Get(std::tstring &outurl);
protected:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
private:
	CGrEditBox m_EditURL;
	std::tstring m_url;
};

