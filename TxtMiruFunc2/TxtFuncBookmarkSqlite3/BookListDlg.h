#ifndef __BOOKLISTDLG_H__
#define __BOOKLISTDLG_H__

#include "WindowCtrl.h"
#include "SplitWnd.h"
#include "TxtFuncIBookmark.h"
#include "PanelTag.h"
#include "PanelBook.h"
#include "PanelSubtitle.h"
#include "ModelessWnd.h"
#include "FunctionKeyMap.h"

class CGrBookListDlg : public CGrModelessWnd
{
public:
	CGrBookListDlg(HWND hWnd, LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark);
	virtual ~CGrBookListDlg();
	void Show(HWND hWnd);

	void ShowBookList(const TxtFuncBookmark::Tag &tag);
	void ShowSubtitleList(const TxtFuncBookmark::Book &book);
	void RefreshBookList(int parent = -1);
	void RefreshSubtitleList(int book_id = -1);
	int GetCurBookIDSubtitleList();
	int GetCurrentPage();

	void PostRefresh();
	void PostRefreshSubtitleList();
	void PostSelect(int tag_id, int book_id);
	//
	bool OpenFile(LPCTSTR lpUrl, int page=-1);
	//
	bool UpdateCheck(LPCTSTR lpUrl);
	bool AddUpdateList(int iItem);
	bool UpdateCheck();
	bool IsUpdating();
	void SetWorking(bool bWorking);
	bool AbortUpdate();
	void ClearUpdateList();
	bool IsShowBookmark(){ return m_bBookmark; }
	bool IsSubtitle(){ return m_bSubtitle; }
	bool IsStayOn(){ return m_bStayOn; }
	void SetGhowBookmark(bool b){ m_bBookmark = b; }
	void SetSubtitle(bool b){ m_bSubtitle = b; }
	void SetStayOn(bool b){ m_bStayOn = b; }
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void OnSize(HWND hwnd, UINT state, int cx, int cy);
	void OnTimer(HWND hwnd, UINT id);
	void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
private:
	static void __stdcall CGrDocOpenProcCallbak(int ret, LPARAM lParam);
	void setWindowSize(int cx, int cy);

	CGrSplitWnd m_splitLWnd;
	CGrSplitWnd m_splitRWnd;
	CGrPanelTagWnd      m_tagWnd;
	CGrPanelBookWnd     m_bookWnd;
	CGrPanelSubtitleWnd m_subtitleWnd;
	bool m_bBookmark = true;
	bool m_bSubtitle = true;
	bool m_bStayOn   = false;
	
	bool bWaitting = false;
	int m_iCurrentTag  = -1;
	int m_iCurrentBook = -1;
	HIMAGELIST m_hImg = NULL;
	HIMAGELIST m_hTreeImg = NULL;

	std::tstring m_filename;
	CGrTxtFuncIBookmark *m_pBookmark = nullptr;
	HANDLE m_hBackgroundPrc = NULL;
	unsigned int m_BackgroundThreadID = 0;
	std::vector<int> m_updateCheckList;
	int m_curUpdateItem = -1;
public:
	std::tstring &GetFilename(){ return m_filename; }
	bool SetFilename(LPCTSTR lpFilename);
};

#endif // __BOOKLISTDLG_H__
