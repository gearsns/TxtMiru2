#ifndef __TXTSEARCHDLG_H__
#define __TXTSEARCHDLG_H__

#include "WindowCtrl.h"
#include <vector>
#include "TxtMiruDef.h"

class CGrTxtSearchDlg : public CGrWinCtrl
{
	void *m_pDoc = nullptr;
public:
	CGrTxtSearchDlg(void *pDoc, const TxtMiru::TextPoint &point);
	virtual ~CGrTxtSearchDlg();
	int DoModal(HWND hWnd);
	const TxtMiru::TextPoint &GetTextPoint() const { return m_curPoint; }
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
private:
	void refreshList();
	TxtMiru::TextPoint m_curPoint;
};

#endif // __TXTSEARCHDLG_H__
