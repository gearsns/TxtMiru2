#ifndef __LISTVIEW_H__
#define __LISTVIEW_H__

#include "WindowCtrl.h"

class CGrListView : public CGrWinCtrl
{
protected:
	bool m_bDragMode = false;
	int m_iInsertPos = -1;
public:
	CGrListView();
	~CGrListView();
	void DragMode(bool bDragMode);
	void SetDrawInsertPos(int iItem);
	int GetDrawInsertPos();
protected:
	virtual void OnKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
	virtual void OnChar(HWND hWnd, TCHAR ch, int cRepeat);
	// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	virtual BOOL DeleteItem(int nItem);
	int InsertItem(LVITEM *pItem);
	BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText);
	int GetNextItem(int nItem, UINT flag) const;
	int GetItem(LV_ITEM *pitem) const;
	BOOL SetItem(LV_ITEM *pitem);
	void SetRedraw(BOOL fRedraw);
	BOOL SetItemState(int nItem, UINT data, UINT mask);
	UINT GetItemState(int i, UINT mask) const;
	int FindItem(const LVFINDINFO *plvfi, int iStart = -1);
	int GetItemCount() const;
	void SetItemCount(int nCount);
	BOOL GetItemRect(int i, LPRECT prc, UINT code) const;
	HWND GetHeader() const;
	int HitTest(LVHITTESTINFO *lpInfo) const;
	BOOL DeleteAllItems();
	int GetSelectedCount() const;
	DWORD SetExtendedListViewStyle(DWORD dw);
	DWORD SetExtendedListViewStyleEx(DWORD dwMask, DWORD dw);
	DWORD GetExtendedListViewStyle() const;
	HIMAGELIST SetImageList(HIMAGELIST hImg, UINT iImageList);
	int InsertColumn(int iSubItem, LVCOLUMN *lplvc);
	int GetColumnWidth(int iCol) const;
	BOOL SetColumnWidth(int iCol, int cx);
	BOOL EnsureVisible(int i, BOOL fPartialOK) const;
	BOOL SortItems(PFNLVCOMPARE pfnCompare, LPARAM lPrm);
	int GetTopIndex() const;
	int GetCountPerPage() const;
	LPARAM GetParam(int iItem, LPARAM lParam = -1) const;
	bool SetParam(int iItem, LPARAM lparam);
};

#endif // __LISTVIEW_H__
