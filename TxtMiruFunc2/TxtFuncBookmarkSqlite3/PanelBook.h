#ifndef __PANELBOOK_H__
#define __PANELBOOK_H__

#include "Panel.h"
#include "TxtFuncBookmarkDef.h"
#include "DragDropImpl.h"

class CBookListDropTarget;
class CGrPanelBookWnd : public CGrPanelWnd
{
	friend CBookListDropTarget;
public:
	CGrPanelBookWnd(CGrBookListDlg &dlg);
	virtual ~CGrPanelBookWnd();
	virtual bool Create(HWND hParent, HIMAGELIST hImg);
	bool DispList(bool bKeepIndex = false, int parent = -1);
	bool Set(const TxtFuncBookmark::Tag &tag);
	int GetSelectID() const;
	bool Select(int id);
	bool SetUpdating(int iItem);
	bool RefreshItem(int iItem);
	bool GetBook(int iItem, TxtFuncBookmark::Book &book);
	void SetWorking(bool bWork);
private:
	bool refreshItem(int iItem);
	bool updateItem(TxtFuncBookmark::Book &book);
	bool deleteItem(int iItem);
	bool moveSelectionItem(bool bUp);
	bool moveBook(int num, int tag_list[], int position);
	bool addBook(std::vector<TxtFuncBookmark::Book> &book_list, int pos);
	bool addBookDlg(LPCTSTR lpURL, int itemPos);
	bool addDelayBookDlg(LPCTSTR lpURL, int itemPos);

	bool getBookByUrl(TxtFuncBookmark::Book &book, LPCTSTR lpURL);
	bool saveOrder();
	bool dispListSub(LPCTSTR lpSql, bool bKeepIndex, int in_iItem, int in_index, int in_parent);

	bool openFile(int iItem);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
private:
	std::vector<TxtFuncBookmark::Book> m_book_list;
	TxtFuncBookmark::Tag m_tag;
	CDropTarget *m_pDropTarget = nullptr;
	std::tstring m_dropFileName;
	int m_dropItemPos = -1;
};

#endif // __PANELBOOK_H__
