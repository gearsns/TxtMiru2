#ifndef __PANELTAG_H__
#define __PANELTAG_H__

#include "Panel.h"
#include "TxtFuncBookmarkDef.h"
#include "DragDropImpl.h"

class CTagListropTarget;
class CGrPanelTagWnd : public CGrTreePanelWnd, CGrTreeListViewCallback
{
	friend CTagListropTarget;
public:
	CGrPanelTagWnd(CGrBookListDlg &dlg);
	virtual ~CGrPanelTagWnd();
	virtual bool Create(HWND hParent, HIMAGELIST hImg, HIMAGELIST hTreeImg);
	bool DispList(bool bKeepIndex = false);
	bool Select(int id);
	void SetWorking(bool bWork);
private:
	bool addItem(TreeListInfo *pParentInfo, TreeListInfo *pPosInfo, LPCTSTR lpURL, LPCTSTR lpTitle);
	bool updateItem(int id, LPCTSTR lpTitle);
	bool deleteItem(int id);
	bool moveBook(int num, int book_list[], int tag_id);
	void deleteSelectItem();
	void deleteAllTagList();
	bool setSelectTree(const std::vector<int> &tag_id_list);
	bool getSelectTree(std::vector<int> &tag_id_list, int id);
protected:
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr);
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	virtual void OnUpdateItemPosition(TreeListInfo *pinfo_p, TreeListInfo *pInsertItem, TreeListInfo *pinfo);
	virtual void OnDeleteItem(TreeListInfo *pinfo);
	virtual void OnExpandItem(TreeListInfo *pinfo);
	virtual bool OnDeletingItem(TreeListInfo *pinfo);
private:
	std::list<TxtFuncBookmark::Tag *> m_tag_list;
	CDropTarget *m_pDropTarget = nullptr;
	HIMAGELIST m_hTreeImg = NULL;
};

#endif // __PANELTAG_H__
