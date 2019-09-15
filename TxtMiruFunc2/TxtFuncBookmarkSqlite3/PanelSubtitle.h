#ifndef __PANELSUBTITLE_H__
#define __PANELSUBTITLE_H__

#include "Panel.h"
#include "TxtFuncBookmarkDef.h"
#include "Cover.h"
#include "CoverInfo.h"
#include "SplitWnd.h"

class CGrSplitImgWnd : public CGrSplitWnd
{
public:
	void SetImgWnd(HWND hWnd);
	void SetInfWnd(HWND hWnd);
protected:
	virtual void resizeRightWindow(RECT &rect);
private:
	HWND m_hImgWnd = NULL;
	HWND m_hInfWnd = NULL;
};

class CGrPanelSubtitleWnd : public CGrPanelWnd
{
public:
	CGrPanelSubtitleWnd(CGrBookListDlg &dlg);
	virtual ~CGrPanelSubtitleWnd();
	virtual bool Create(HWND hParent, HIMAGELIST hImg);
	bool DispList(bool bKeepIndex = false);
	bool Refresh(int book_id = -1);
	bool Set(const TxtFuncBookmark::Book &book);
	void SetSplitPos(int pos);
	int GetSplitPos();
	int GetSelectID() const;
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void setWindowSize(int cx, int cy);
private:
	std::vector<TxtFuncBookmark::Subtitle> m_subtitle_list;
	TxtFuncBookmark::Book m_book;
	int m_imageSize = 128;
	CGrSplitImgWnd m_splitWnd;
	CGrCover m_cover;
	CGrCoverInfo m_coverInfo;

	bool openFile(int iItem);
	bool setRead(int iItem);
	bool setUnRead(int iItem);
};

#endif // __PANELSUBTITLE_H__
