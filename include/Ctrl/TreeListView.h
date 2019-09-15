#ifndef __TREELISTVIEW_H__
#define __TREELISTVIEW_H__

#include "ListView.h"

#include <vector>
#include <list>

class TreeListInfo
{
public:
	int m_nLevel = -1;
	int m_nImage = 0;
	LPARAM m_lParam = 0;
	std::vector<std::tstring> m_lstCols;
	bool m_bCollapsed = false;
	std::list<TreeListInfo *> m_children;
	TreeListInfo *m_pParent = nullptr;

	virtual ~TreeListInfo();
	void DeleteAllItems();
	void ResetLevel();
	bool HasChild(TreeListInfo *pinfo);
	virtual bool isDummy(){ return false; }
	bool AddDummyChild();
	bool isDummyChild();
};

class CGrTreeListViewCallback
{
public:
	CGrTreeListViewCallback(){};
	virtual ~CGrTreeListViewCallback(){};
	virtual void OnUpdateItemPosition(TreeListInfo *pinfo_p, TreeListInfo *pInsertItem, TreeListInfo *pinfo){};
	virtual void OnDeleteItem(TreeListInfo *pinfo){};
	virtual void OnExpandItem(TreeListInfo *pinfo){};
	virtual bool OnDeletingItem(TreeListInfo *pinfo){ return true; };
};
class CGrTreeListView : public CGrListView
{
protected:
	CGrTreeListViewCallback *m_pCallback = nullptr;;
public:
	CGrTreeListView();
	virtual ~CGrTreeListView();
	virtual BOOL DeleteAllItems();
	virtual BOOL DeleteItem(int nItem);
	void SetCallback(CGrTreeListViewCallback *pCallback);
	TreeListInfo *AddItem(LPCTSTR lpszItem, int nImage, int nLevel, LPARAM lParam = 0);
	TreeListInfo *InsertItemPos(TreeListInfo *pinfo_p, TreeListInfo *pinfo_i, LPCTSTR lpszItem, int nImage, LPARAM lParam = 0);
	void CollapseItem(int nItem);
	void ExpandItem(int nItem, bool bRecursive = false);
	BOOL MoveItems(int nParent, int nItem, int nMoveItems[], int nMoveItemCnt);
	BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText);
	BOOL SetItemText(TreeListInfo *pinfo, int nSubItem, LPCTSTR lpszText);
	LPARAM GetParam(int iItem, LPARAM lParam = -1) const;
	bool SetParam(int iItem, LPARAM lparam);
	bool ItemHasChildren(int nItem);
	void SetDrawInsertPos(int iItem, int iTreeInsertParent = -1, int iTreeInsertPos = -1);
	int GetInsertParent() const;
	int GetTreeInsertPos() const;
	TreeListInfo *GetTreeListInfo(int nItem) const;
	const TreeListInfo *GetRootTreeListInfo() const;

	void OnUpdateItemPosition(TreeListInfo *pinfo_p, TreeListInfo *pInsertItem, TreeListInfo *pinfo);
	void OnDeleteItem(TreeListInfo *pinfo);
	void OnExpandItem(TreeListInfo *pinfo);
	bool OnDeletingItem(TreeListInfo *pinfo);
private:
	TreeListInfo m_root;
protected:
	int m_iInsertParent = -1;
	int m_iTreeInsertPos = -1;
	void Collapse(int nItem, TreeListInfo *pinfo = nullptr);
	virtual int Expand(bool bRecursive, int& nCounterInsert, TreeListInfo *pinfo);
	int InsertItem(int nItem, TreeListInfo& info);
	bool itemHasChildren(TreeListInfo *pinfo);
	virtual void OnKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
	virtual void OnChar(HWND hwnd, TCHAR ch, int cRepeat);
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // __TREELISTVIEW_H__
