#ifndef __BOOKDLG_H__
#define __BOOKDLG_H__

#include "WindowCtrl.h"
#include "TxtFuncBookmarkDef.h"
#include "Cover.h"
#include "EditBox.h"
#include "DragDropImpl.h"

struct TagInfo {
	int id = 0;
	std::tstring title;
	int position = 0;
	int parent = 0;
};
class CGrBookDlg : public CGrWinCtrl
{
public:
	CGrBookDlg(TxtFuncBookmark::Book &book, int parent, int pre_id = -1);
	virtual ~CGrBookDlg();
	int DoModal(HWND hWnd);
protected:
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
private:
	void getImage();
	bool getInfo();
	bool getInfoUpdate();
	int addItem();
	bool updateItem();
	void getUrl();
	bool AbortUpdate();
	static void __stdcall CGrDocOpenProcCallbak(int ret, LPARAM lParam);
	void openSPFunc();
	void closePFunc();
private:
	HANDLE m_hBackgroundPrc = NULL;
	unsigned int m_BackgroundThreadID = 0;
	TxtFuncBookmark::Book &m_book;
	bool m_bOpenSPFunc = false;
	int m_parent = -1;
	int m_pre_id = -1;
	std::tstring m_imgFilename;
	std::vector<TagInfo> m_tag_list;
	CGrCover m_cover;
	CGrEditBox m_EditTitle ;
	CGrEditBox m_EditURL   ;
	CGrEditBox m_EditAuthor;
	CGrEditBox m_EditNum   ;
	CDropTarget *m_pDropImageTarget = nullptr;
	CDropTarget *m_pDropURLTarget = nullptr;
};

#endif // __BOOKDLG_H__
