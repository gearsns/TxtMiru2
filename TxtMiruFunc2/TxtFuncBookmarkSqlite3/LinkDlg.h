#ifndef __LINKDLG_H__
#define __LINKDLG_H__

#include "WindowCtrl.h"
#include "ToolBar.h"
#include "TxtMiruDef.h"
#include "Font.h"
#include "ListView.h"
#include "TxtFuncIBookmark.h"
#include "TxtFuncBookmark.h"
#include "TxtFuncIParam.h"
#include "ModelessWnd.h"

class CGrLinkDlg : public CGrModelessWnd
{
protected:
	const int listwnd_id = 3000;
	const int coolbar_id = 3001;
	const int toolbar_id = 3002;
public:
	struct LinkInfo {
		bool bBookmark = false;
		int page = 0;
		int level = 0;
		bool top = false;
		std::tstring name;
		std::tstring filename;
	};
private:
	CGrTxtFuncIBookmark *m_pBookmark = nullptr;
	CGrTxtFuncISubTitle *m_pSubTitle = nullptr;
	std::vector<LinkInfo> m_linkInfoList;
	HIMAGELIST m_hImg = NULL;
	CGrListView m_listView;
public:
	CGrLinkDlg(HWND hWnd, int mode, CGrTxtFuncIBookmark *pBook, CGrTxtFuncISubTitle *pSubtitle);
	virtual ~CGrLinkDlg();
	virtual void Show(HWND hWnd);
	void Refresh();
	void PostRefresh();
	void PtstUpdatePage();

	bool isShowBookmark();
	bool isShowSubtitle();
	void ShowNormal(HWND hWnd);
	void ShowBookmark(HWND hWnd, bool bVisible);
	void ShowSubtitle(HWND hWnd, bool bVisible);
	virtual BOOL IsDialogMessage(HWND hwnd, MSG &msg);
	void RefreshCurrentPos();
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnTimer(HWND hwnd, UINT id);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
	void drawItem(DRAWITEMSTRUCT *pdi, RECT &clinetRect);
private:
	int m_nCurPage = 0;
	int m_mode = 0;
	bool m_bBookmark = true;
	bool m_bSubtitle = true;
	bool m_bStayOn = true;
	bool m_bAutoScroll = true;
	POINT m_minWindowSize = {};
	CGrCoolBar m_coolBar;
	CGrFont m_titleFont;
	CGrFont m_pageFont;

	void endDialog(UINT id);
	void setWindowSize(int cx, int cy);
};

#endif // __LINKDLG_H__
