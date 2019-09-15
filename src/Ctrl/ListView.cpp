#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "ListView.h"

CGrListView::CGrListView() : CGrWinCtrl(){}

CGrListView::~CGrListView()
{
}

void CGrListView::DragMode(bool bDragMode)
{
	if(!bDragMode){
		m_iInsertPos = -1;
	}
	m_bDragMode = true;
	InvalidateRect(m_hWnd, nullptr, FALSE);
	UpdateWindow(m_hWnd);
	m_bDragMode = bDragMode;
}

void CGrListView::SetDrawInsertPos(int iItem)
{
	if(m_iInsertPos != iItem){
		m_iInsertPos = iItem;
		InvalidateRect(m_hWnd, nullptr, FALSE);
	}
}
int CGrListView::GetDrawInsertPos()
{
	return m_iInsertPos;
}

void CGrListView::OnKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	CGrWinCtrl::WndProc(hWnd, WM_KEYDOWN, static_cast<WPARAM>(vk), MAKELPARAM((cRepeat), (flags)));
}

void CGrListView::OnChar(HWND hWnd, TCHAR ch, int cRepeat)
{
	CGrWinCtrl::WndProc(hWnd, WM_CHAR, static_cast<WPARAM>(ch), MAKELPARAM((cRepeat),0));
}

LRESULT CGrListView::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_KEYDOWN:
		{
			OnKeyDown((hWnd), static_cast<UINT>(wParam), TRUE, static_cast<int>(static_cast<short>(LOWORD(lParam))), static_cast<UINT>(HIWORD(lParam)));
		}
		return 0;
		HANDLE_MSG(hWnd, WM_CHAR         , OnChar         );
	default:
		return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

BOOL CGrListView::DeleteItem(int nItem)
{
	return ListView_DeleteItem(m_hWnd, nItem);
}

int CGrListView::InsertItem(LVITEM *pItem)
{
	return ListView_InsertItem(m_hWnd, pItem);
}

BOOL CGrListView::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
	LV_ITEM _ms_lvi;
	_ms_lvi.iSubItem = nSubItem;
	_ms_lvi.pszText = const_cast<LPTSTR>(lpszText);
	return SNDMSG(m_hWnd, LVM_SETITEMTEXT, static_cast<WPARAM>(nItem), reinterpret_cast<LPARAM>(&_ms_lvi));
}

int CGrListView::GetNextItem(int nItem, UINT flag) const
{
	return ListView_GetNextItem(m_hWnd, nItem, flag);
}

int CGrListView::GetItem(LV_ITEM *pitem)  const
{
	return ListView_GetItem(m_hWnd, pitem);
}

BOOL CGrListView::SetItem(LV_ITEM *pitem)
{
	return ListView_SetItem(m_hWnd, pitem);
}

void CGrListView::SetRedraw(BOOL fRedraw)
{
	SetWindowRedraw(m_hWnd, fRedraw);
}

BOOL CGrListView::SetItemState(int nItem, UINT data, UINT mask)
{
	LV_ITEM _ms_lvi;
	_ms_lvi.stateMask = mask;
	_ms_lvi.state = data;
	return SNDMSG(m_hWnd, LVM_SETITEMSTATE, static_cast<WPARAM>(nItem), reinterpret_cast<LPARAM>(&_ms_lvi));
}

UINT CGrListView::GetItemState(int i, UINT mask) const
{
	return ListView_GetItemState(m_hWnd, i, mask);
}

int CGrListView::FindItem(const LVFINDINFO *plvfi, int iStart)
{
	return ListView_FindItem(m_hWnd, iStart, plvfi);
}

int CGrListView::GetItemCount() const
{
	return ListView_GetItemCount(m_hWnd);
}

void CGrListView::SetItemCount(int nCount)
{
	ListView_SetItemCountEx(m_hWnd, nCount, LVSICF_NOINVALIDATEALL);
}

BOOL CGrListView::GetItemRect(int i, LPRECT prc, UINT code) const
{
	return ListView_GetItemRect(m_hWnd, i, prc, code);
}

HWND CGrListView::GetHeader() const
{
	return ListView_GetHeader(m_hWnd);
}

int CGrListView::HitTest(LVHITTESTINFO *lpInfo) const
{
	return ListView_HitTest(m_hWnd, lpInfo);
}

BOOL CGrListView::DeleteAllItems()
{
	return ListView_DeleteAllItems(m_hWnd);
}

int CGrListView::GetSelectedCount() const
{
	return ListView_GetSelectedCount(m_hWnd);
}

DWORD CGrListView::SetExtendedListViewStyle(DWORD dw)
{
	return ListView_SetExtendedListViewStyle(m_hWnd, dw);
}

DWORD CGrListView::SetExtendedListViewStyleEx(DWORD dwMask, DWORD dw)
{
	return ListView_SetExtendedListViewStyleEx(m_hWnd, dwMask, dw);
}

DWORD CGrListView::GetExtendedListViewStyle() const
{
	return ListView_GetExtendedListViewStyle(m_hWnd);
}

HIMAGELIST CGrListView::SetImageList(HIMAGELIST hImg, UINT iImageList)
{
	return ListView_SetImageList(m_hWnd, hImg, iImageList);
}

int CGrListView::InsertColumn(int iSubItem, LVCOLUMN *lplvc)
{
	return ListView_InsertColumn(m_hWnd, iSubItem, lplvc);
}

int CGrListView::GetColumnWidth(int iCol) const
{
	return ListView_GetColumnWidth(m_hWnd, iCol);
}

BOOL CGrListView::SetColumnWidth(int iCol, int cx)
{
	return ListView_SetColumnWidth(m_hWnd, iCol, cx);
}

BOOL CGrListView::EnsureVisible(int i, BOOL fPartialOK) const
{
	return ListView_EnsureVisible(m_hWnd, i, fPartialOK);
}

BOOL CGrListView::SortItems(PFNLVCOMPARE pfnCompare, LPARAM lPrm)
{
	return ListView_SortItems(m_hWnd, pfnCompare, lPrm);
}

int CGrListView::GetTopIndex() const
{
	return ListView_GetTopIndex(m_hWnd);
}

int CGrListView::GetCountPerPage() const
{
	return ListView_GetCountPerPage(m_hWnd);
}

LPARAM CGrListView::GetParam(int iItem, LPARAM lParam) const
{
	LVITEM item = {LVIF_PARAM};
	item.iItem = iItem;
	if(GetItem(&item)){
		return item.lParam;
	}
	return lParam;
}

bool CGrListView::SetParam(int iItem, LPARAM lparam)
{
	LVITEM item = {LVIF_PARAM};
	item.iItem = iItem;
	item.lParam = lparam;
	SetItem(&item);
	return true;
}
