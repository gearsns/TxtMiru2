#ifndef __PAPERSIZEDLG_H__
#define __PAPERSIZEDLG_H__

#include "WindowCtrl.h"

class CGrPaperSizeDlg : public CGrWinCtrl
{
private:
	SIZE m_paperSize = {};
public:
	CGrPaperSizeDlg();
	virtual ~CGrPaperSizeDlg();
public:
	virtual int DoModal(HWND hWnd, const SIZE &paperSize);
	const SIZE GetPaperSize() const;
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
};

#endif // __PAPERSIZEDLG_H__
