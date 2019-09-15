#ifndef __TXTSEARCHFILES_H__
#define __TXTSEARCHFILES_H__

#include "WindowCtrl.h"
#include "ToolBar.h"
#include "TxtMiruDef.h"
#include "Font.h"
#include "TxtFuncIBookmark.h"
#include "TxtFuncBookmark.h"
#include "TxtFuncIParam.h"
#include "ModelessWnd.h"
#include "BookmarkDB.h"
#include "ListView.h"
#include <regex>

#include "ComonCtrl.h"
class CGrComboBoxFile : public CGrComonCtrl
{
public:
	CGrComboBoxFile() : CGrComonCtrl(){}
	virtual ~CGrComboBoxFile(){}
protected:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
class CGrTxtSearchFiles : public CGrModelessWnd
{
	struct SearchResult {
		std::tstring url   ;
		std::tstring title ;
		std::tstring author;
		int          page = 0;
		std::tstring text  ;
	};
protected:
	const int listwnd_id = 3000;
public:
	CGrTxtSearchFiles(HWND hWnd);
	virtual ~CGrTxtSearchFiles();
	virtual void Show(HWND hWnd);
	virtual BOOL IsDialogMessage(HWND hwnd, MSG &msg);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnTimer(HWND hwnd, UINT id);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
	static void __stdcall CGrDocOpenProcCallbak(int page, LPCTSTR lpSrc, LPARAM lParam);
	bool Search();
	void AddResult(int page, LPCTSTR lpSrc);
	bool Abort();
private:
	bool loadSetteing();
	bool saveSetteing();
	void setState(bool bStart = true);
	//
	CGrListView m_listview;
	CGrComboBoxFile m_path;
	std::vector<SearchResult> m_search_list;
	HANDLE m_hBackgroundPrc = NULL;
	unsigned int m_BackgroundThreadID = 0;
	std::vector<std::tstring> m_filname_list;
	int m_curSearchItem = -1;
	std::tstring m_cond;
	bool m_bUseRegExp = false;
	SearchResult m_curSearchresult;

	HANDLE m_hFindFile = INVALID_HANDLE_VALUE;
	std::wregex m_regexFileCond;
	std::tstring m_searchPath;
	//
	HIMAGELIST m_hImg = NULL;
	CGrFont m_titleFont;
	CGrFont m_pageFont;
	POINT m_minWindowSize = {};
	bool addSearchFile(LPCTSTR lpFilePath = nullptr);
	void endDialog(UINT id);
	void setWindowSize(int cx, int cy);
};

#endif // __TXTSEARCHFILES_H__
