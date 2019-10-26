#ifndef __RUBYLISTDLG_H__
#define __RUBYLISTDLG_H__

#include <regex>

#include "WindowCtrl.h"
#include "TxtMiruDef.h"
#include "Font.h"
#include "TxtFuncIBookmark.h"
#include "TxtFuncBookmark.h"
#include "TxtFuncIParam.h"
#include "ModelessWnd.h"
#include "ListView.h"
#include "FilterEditBox.h"

class CGrRubyListDlg : public CGrModelessWnd
{
public:
	enum class RubyListColumn {
		RLC_STR ,
		RLC_RUBY,
		RLC_PAGE,
		RLC_MAX ,
	};
public:
	struct RubyInfo {
		std::tstring str;
		std::tstring ruby;
		std::vector<int> page_list;
	};
private:
	std::wregex m_ignore_pattern;
	std::vector<RubyInfo> m_rubyInfoList;
	void *m_pDoc = nullptr;
	bool m_bAsc[static_cast<int>(RubyListColumn::RLC_MAX)] = {true, false, false};
	CGrListView m_listView;
	CGrFilterEditBox m_filterEditBox;
public:
	CGrRubyListDlg(HWND hWnd, void *pDoc);
	virtual ~CGrRubyListDlg();
	virtual void Show(HWND hWnd);
	virtual BOOL IsDialogMessage(HWND hwnd, MSG &msg);
	void Refresh();
	void PostRefresh();
	void SetFilter(std::tstring &filtertext);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnTimer(HWND hwnd, UINT id);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
private:
	POINT m_minWindowSize = {};

	void endDialog(UINT id);
	void setWindowSize(int cx, int cy);
	
	static void __stdcall DocEachAllProcCallbak(CGrTxtFuncIEachAllTool *pTool, LPARAM lParam);
};

#endif // __RUBYLISTDLG_H__
