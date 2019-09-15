#pragma once
#include "WindowCtrl.h"
#include "EditBox.h"

class CGrImportDlg :
	public CGrWinCtrl
{
public:
	CGrImportDlg();
	virtual~CGrImportDlg();
	int DoModal(HINSTANCE hinstance, HWND hWnd);
protected:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
private:
	CGrEditBox m_EditURL;
	CGrEditBox m_EditFolder;
	std::tstring m_url;
	std::tstring m_folder;
};
