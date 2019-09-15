#ifndef __VERSIONDLG_H__
#define __VERSIONDLG_H__

#include "WindowCtrl.h"

class CGrVersionDlg : public CGrWinCtrl
{
	HINSTANCE m_hInst = NULL;
	SIZE m_minSize = {};
	HANDLE m_hThread = NULL;
	unsigned int m_threadid = 0;
	std::tstring m_client_version;
	std::tstring m_server_version;

	void setWindowSize(int cx, int cy);
	static unsigned __stdcall GetVersionProc(void *lpParam);
public:
	CGrVersionDlg(HINSTANCE hInst);
	virtual ~CGrVersionDlg();
	int DoModal(HWND hWnd);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnSize(HWND hwnd, UINT nType, int cx, int cy);
	void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
};

#endif // __VERSIONDLG_H__
