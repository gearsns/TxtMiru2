#ifndef __TXTDOCINFODLG_H__
#define __TXTDOCINFODLG_H__

#include "WindowCtrl.h"

class CGrTxtDocInfoDlg : public CGrWinCtrl
{
private:
	std::tstring m_title;
	std::tstring m_author;
	std::tstring m_filename;
	std::tstring m_text;
	std::tstring m_writetime;
	int m_page = 0;
	int m_maxpage = 0;
	SIZE m_minSize = {};
	void setWindowSize(int cx, int cy);
public:
	CGrTxtDocInfoDlg(LPCTSTR title, LPCTSTR author, LPCTSTR filename, LPCTSTR text, LPCTSTR writetime, int page, int maxpage);
	virtual ~CGrTxtDocInfoDlg();
	int DoModal(HWND hWnd);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnSize(HWND hwnd, UINT nType, int cx, int cy);
	void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
};

#endif // __TXTDOCINFODLG_H__
