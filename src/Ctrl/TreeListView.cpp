#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "TreeListView.h"
#include "MemDC.h"

TreeListInfo::~TreeListInfo()
{
	DeleteAllItems();
}

void TreeListInfo::DeleteAllItems()
{
	for(auto &&item : m_children){
		if(item){
			delete item;
		}
	}
	m_children.clear();
}
void TreeListInfo::ResetLevel()
{
	int nLevel = m_nLevel;
	if(m_pParent){
		nLevel = m_pParent->m_nLevel + 1;
	} else {
		nLevel = 0;
	}
	if(nLevel != m_nLevel){
		m_nLevel = nLevel;
		for(auto &&pitem : m_children){
			if(pitem){
				pitem->ResetLevel();
			}
		}
	}
}
bool TreeListInfo::HasChild(TreeListInfo *pinfo)
{
	for(auto &&item : m_children){
		if(item == pinfo){
			return true;
		}
		if(item && item->HasChild(pinfo)){
			return true;
		}
	}
	return false;
}

bool TreeListInfo::AddDummyChild()
{
	if(m_children.empty()){
		m_bCollapsed = true; // 子有
		m_children.push_back(nullptr);
		return true;
	}
	return false;
}

bool TreeListInfo::isDummyChild()
{
	return !m_children.empty() && (m_children.back() == nullptr);
}


CGrTreeListView::CGrTreeListView()
{
}

CGrTreeListView::~CGrTreeListView()
{
}

void CGrTreeListView::SetCallback(CGrTreeListViewCallback *pCallback)
{
	m_pCallback = pCallback;
}

void CGrTreeListView::OnUpdateItemPosition(TreeListInfo *pinfo_p, TreeListInfo *pInsertItem, TreeListInfo *pinfo)
{
	if(m_pCallback){
		m_pCallback->OnUpdateItemPosition(pinfo_p, pInsertItem, pinfo);
	}
}

void CGrTreeListView::OnDeleteItem(TreeListInfo *pinfo)
{
	if(m_pCallback){
		m_pCallback->OnDeleteItem(pinfo);
	}
}

void CGrTreeListView::OnExpandItem(TreeListInfo *pinfo)
{
	if(m_pCallback){
		m_pCallback->OnExpandItem(pinfo);
	}
}

bool CGrTreeListView::OnDeletingItem(TreeListInfo *pinfo)
{
	if(m_pCallback){
		return m_pCallback->OnDeletingItem(pinfo);
	}
	return true;
}

BOOL CGrTreeListView::DeleteAllItems()
{
	m_root.DeleteAllItems();
	return CGrListView::DeleteAllItems();
}

BOOL CGrTreeListView::DeleteItem(int nItem)
{
	auto *pinfo = GetTreeListInfo(nItem);
	if(pinfo){
		OnDeleteItem(pinfo);
		Collapse(nItem, pinfo);
		if(pinfo->m_pParent){
			pinfo->m_pParent->m_children.remove(pinfo);
			delete pinfo;
		}
	}
	return CGrListView::DeleteItem(nItem);
}

TreeListInfo *CGrTreeListView::AddItem(LPCTSTR lpszItem, int nImage, int nLevel, LPARAM lParam)
{
	auto *pinfo = new TreeListInfo();
	pinfo->m_nLevel     = nLevel;
	pinfo->m_nImage     = nImage;
	pinfo->m_bCollapsed = true;
	pinfo->m_lParam     = lParam;
	pinfo->m_lstCols.resize(Header_GetItemCount(GetHeader()));
	pinfo->m_lstCols[0] = lpszItem;

	if(nLevel == 0 || m_root.m_children.empty()){
		m_root.m_bCollapsed = false;
		pinfo->m_pParent = &m_root;
		m_root.m_children.push_back(pinfo);
		nLevel = 0;
	} else {
		auto pitem = m_root.m_children.back();
		if(pitem){
			if(pitem->m_nLevel >= nLevel){
				pinfo->m_pParent = &m_root;
				m_root.m_children.push_back(pinfo);
				nLevel = 0;
			} else {
				do {
					auto &&children = pitem->m_children;
					if(children.empty()){
						break;
					}
					auto pitem_next = children.back();
					if(pitem_next && pitem_next->m_nLevel >= nLevel){
						break;
					}
					pitem = pitem_next;
					if (!pitem) {
						break;
					}
				} while(true);
				if (pitem) {
					pitem->m_children.push_back(pinfo);
					pinfo->m_pParent = pitem;
				}
			}
		}
	}

	int cnt = GetItemCount();
	if(nLevel == 0){
		LVITEM item = {LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT | LVIF_STATE | LVIF_PARAM};
		item.iItem      = cnt;
		item.iSubItem   = 0;
		item.pszText    = const_cast<LPTSTR>(pinfo->m_lstCols[0].c_str());
		item.iImage     = pinfo->m_nImage;
		item.iIndent    = pinfo->m_nLevel;
		item.state      = INDEXTOSTATEIMAGEMASK(3);
		item.stateMask  = LVIS_STATEIMAGEMASK;
		item.lParam     = reinterpret_cast<LPARAM>(pinfo);
		CGrListView::InsertItem(&item);
	} else {
		if(pinfo->m_pParent && !pinfo->m_pParent->m_children.empty()){
			SetItemState(cnt-1, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
		}
	}

	return pinfo;
}

TreeListInfo *CGrTreeListView::InsertItemPos(TreeListInfo *pinfo_p, TreeListInfo *pinfo_i, LPCTSTR lpszItem, int nImage, LPARAM lParam)
{
	if(!pinfo_p){
		pinfo_p = &m_root;
	}
	auto *pinfo = new TreeListInfo();
	pinfo->m_pParent    = pinfo_p;
	pinfo->m_nLevel     = pinfo_p->m_nLevel + 1;
	pinfo->m_nImage     = nImage;
	pinfo->m_bCollapsed = true;
	pinfo->m_lParam     = lParam;
	pinfo->m_lstCols.resize(Header_GetItemCount(GetHeader()));
	pinfo->m_lstCols[0] = lpszItem;

	bool bCollapsed = pinfo_p->m_bCollapsed;
	int nParent = -1;
	if(pinfo_p != &m_root){
		LVFINDINFO info = {LVFI_PARAM};
		info.lParam = reinterpret_cast<LPARAM>(pinfo_p);
		nParent = FindItem(&info);
		if(!pinfo_p->m_bCollapsed){
			Collapse(nParent, pinfo_p); // 下位構成の削除
		}
	}
	auto it = std::find(pinfo_p->m_children.begin(), pinfo_p->m_children.end(), pinfo_i);
	if(pinfo_p->m_children.empty()){
		bCollapsed = false;
	}
	pinfo_p->m_children.insert(it, pinfo);
	if(pinfo_p == &m_root){
		int cnt = GetItemCount();
		LVFINDINFO info = {LVFI_PARAM};
		info.lParam = reinterpret_cast<LPARAM>(pinfo_i);
		int nItem = FindItem(&info);
		if(nItem < 0 || nItem > cnt){
			nItem = cnt;
		}
		LVITEM item = {LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT | LVIF_STATE | LVIF_PARAM};
		item.iItem      = nItem;
		item.iSubItem   = 0;
		item.pszText    = const_cast<LPTSTR>(pinfo->m_lstCols[0].c_str());
		item.iImage     = pinfo->m_nImage;
		item.iIndent    = pinfo->m_nLevel;
		item.state      = INDEXTOSTATEIMAGEMASK(3);
		item.stateMask  = LVIS_STATEIMAGEMASK;
		item.lParam     = reinterpret_cast<LPARAM>(pinfo);
		CGrListView::InsertItem(&item);
	} else {
		if(!bCollapsed){
			int nCounterInsert = nParent;
			Expand(false, nCounterInsert, pinfo_p);
			SetItemState(nParent, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
		} else {
			SetItemState(nParent, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
		}
	}

	return pinfo;
}

void CGrTreeListView::CollapseItem(int nItem)
{
	Collapse(nItem);
}

void CGrTreeListView::ExpandItem(int nItem, bool bRecursive)
{
	auto *pinfo = reinterpret_cast<TreeListInfo*>(CGrListView::GetParam(nItem));
	if(pinfo && pinfo->m_bCollapsed){
		int nCounter = nItem;
		Expand(bRecursive, nCounter, pinfo);
		if(itemHasChildren(pinfo)){
			if(pinfo->m_bCollapsed){
				SetItemState(nItem, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
			} else {
				SetItemState(nItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
			}
		} else {
			SetItemState(nItem, INDEXTOSTATEIMAGEMASK(3), LVIS_STATEIMAGEMASK);
		}
	}
}


BOOL CGrTreeListView::MoveItems(int nParent, int nItem, int nMoveItems[], int nMoveItemCnt)
{
	auto *pinfo_p = GetTreeListInfo(nParent);
	if(!pinfo_p){
		pinfo_p = &m_root;
	}
	//
	if(pinfo_p && pinfo_p->m_bCollapsed){
		LVFINDINFO info = {LVFI_PARAM};
		info.lParam = reinterpret_cast<LPARAM>(pinfo_p);
		int nParentTmp = FindItem(&info);
		if(nParentTmp >= 0){
			int nInsert = nParentTmp;
			Expand(false, nInsert, pinfo_p);
			Collapse(nParentTmp, pinfo_p);
		}
	}
	//
	// 挿入位置の取得
	auto *pinfo_i = GetTreeListInfo(nItem);
	if(pinfo_i){
		if(pinfo_p->m_children.empty()){
			;
		} else {
			bool bNotFound = true;
			for(auto &&item : pinfo_p->m_children){
				if(!item){
					continue;
				}
				if(!bNotFound){
					pinfo_i = item;
					break;
				}
				if(item == pinfo_i){
					bNotFound = false;
					break;
				} else if(item->HasChild(pinfo_i)){
					pinfo_i = nullptr;
					bNotFound = false;
				}
			}
		}
	}
	// 移動準備(下位構成を一旦閉じる)
	std::vector<TreeListInfo *> tli_list;
	for(int i=0; i<nMoveItemCnt; ++i){
		auto *pinfo = GetTreeListInfo(nMoveItems[i]);
		if(pinfo){
			if(!pinfo->m_bCollapsed){
				Collapse(nMoveItems[i], pinfo); // 下位構成の削除
				pinfo->m_bCollapsed = false; // 後でExpandするため
			}
			if(pinfo->m_pParent){
				pinfo->m_pParent->m_children.remove(pinfo);
				if(pinfo->m_pParent->m_children.empty()){
					pinfo->m_pParent->m_bCollapsed = false;
					LVFINDINFO info = {LVFI_PARAM};
					info.lParam = reinterpret_cast<LPARAM>(pinfo->m_pParent);
					int nParentTmp = FindItem(&info);
					SetItemState(nParentTmp, INDEXTOSTATEIMAGEMASK(3), LVIS_STATEIMAGEMASK);
				}
			}
			tli_list.push_back(pinfo);
		}
	}
	for(int i=0; i<nMoveItemCnt; ++i){
		CGrListView::DeleteItem(nMoveItems[i]); // 移動する項目を現在の親から一旦削除
	}
	bool bCollapsed = pinfo_p->m_bCollapsed;
	if(pinfo_p != &m_root){
		LVFINDINFO info = {LVFI_PARAM};
		info.lParam = reinterpret_cast<LPARAM>(pinfo_p);
		nParent = FindItem(&info);
		if(!pinfo_p->m_bCollapsed){
			Collapse(nParent, pinfo_p); // 下位構成の削除
		}
	}
	// 移動 + フルレベルで各項目のレベルを振りなおす(挿入位置のレベルが変わる場合があるので)
	std::reverse(tli_list.begin(), tli_list.end());
	auto it = std::find(pinfo_p->m_children.begin(), pinfo_p->m_children.end(), pinfo_i);
	int nLevel = pinfo_p->m_nLevel;
	for(auto &&pinfo : tli_list){
		if(pinfo){
			pinfo->m_pParent = pinfo_p; // 移動
			pinfo->ResetLevel();        // レベルの振りなおし
		}
	}
	if(pinfo_p->m_children.empty()){
		bCollapsed = false;
	}
	// 移動
	pinfo_p->m_children.insert(it, tli_list.begin(), tli_list.end());
	{
		// Callback用の処理
		TreeListInfo *pInsertItem = nullptr;
		if(it != pinfo_p->m_children.end()){
			pInsertItem = *it;
		}
		for(auto &&pinfo : tli_list){
			if(pinfo){
				OnUpdateItemPosition(pinfo_p, pInsertItem, pinfo);
				pInsertItem = pinfo;
			}
		}
	}
	if(pinfo_p == &m_root){
		int cnt = GetItemCount();
		LVFINDINFO info = {LVFI_PARAM};
		info.lParam = reinterpret_cast<LPARAM>(pinfo_i);
		nItem = FindItem(&info);
		if(nItem < 0 || nItem > cnt){
			nItem = cnt;
		}
		for(auto &&pinfo : tli_list){
			if(!pinfo){
				continue;
			}
			LVITEM item = {LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT | LVIF_STATE | LVIF_PARAM};
			item.iItem      = nItem;
			item.iSubItem   = 0;
			item.pszText    = const_cast<LPTSTR>(pinfo->m_lstCols[0].c_str());
			item.iImage     = pinfo->m_nImage;
			item.iIndent    = pinfo->m_nLevel;
			if(pinfo->m_children.empty()){
				item.state      = INDEXTOSTATEIMAGEMASK(3);
			} else {
				item.state      = INDEXTOSTATEIMAGEMASK(pinfo->m_bCollapsed ? 1 : 2);
			}
			item.stateMask  = LVIS_STATEIMAGEMASK;
			item.lParam     = reinterpret_cast<LPARAM>(pinfo);
			CGrListView::InsertItem(&item);
			if(pinfo->m_bCollapsed){
				++nItem;
			} else {
				Expand(false, nItem, pinfo);
			}
		}
	} else {
		if(!bCollapsed){
			int nCounterInsert = nParent;
			Expand(false, nCounterInsert, pinfo_p);
			SetItemState(nParent, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
		} else {
			SetItemState(nParent, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
		}
	}
	// 再選択
	SetItemState(-1, 0, LVIS_FOCUSED|LVIS_SELECTED);
	int iFocusItem = -1;
	int count = GetItemCount();
	for(auto &&pinfo : tli_list){
		if(!pinfo){
			continue;
		}
		LVFINDINFO info = {LVFI_PARAM};
		info.lParam = reinterpret_cast<LPARAM>(pinfo);
		int iSelectItem = FindItem(&info);
		if(iSelectItem >= 0){
			iFocusItem = iSelectItem;
			SetItemState(iSelectItem, LVIS_SELECTED, LVIS_SELECTED);
			LVITEM item = { LVIF_INDENT };
			item.iItem = iSelectItem;
			int iIndent = pinfo->m_nLevel;
			++item.iItem;
			for(;item.iItem<count; ++item.iItem){
				if(FALSE == GetItem(&item)){
					break;
				}
				if(item.iIndent <= iIndent){
					break;
				}
				SetItemState(item.iItem, LVIS_SELECTED, LVIS_SELECTED);
			}
		}
	}
	if(iFocusItem >= 0){
		SetItemState(iFocusItem, LVIS_FOCUSED, LVIS_FOCUSED);
	}
	//
	return TRUE;
}

TreeListInfo *CGrTreeListView::GetTreeListInfo(int nItem) const
{
	LVITEM item = {LVIF_PARAM};
	item.iItem = nItem;
	if(GetItem(&item)){
		return reinterpret_cast<TreeListInfo*>(item.lParam);
	}
	return nullptr;
}

const TreeListInfo *CGrTreeListView::GetRootTreeListInfo() const
{
	return &m_root;
}

LPARAM CGrTreeListView::GetParam(int iItem, LPARAM lParam) const
{
	auto *pinfo = GetTreeListInfo(iItem);
	if(pinfo){
		return pinfo->m_lParam;
	}
	return lParam;
}

bool CGrTreeListView::SetParam(int iItem, LPARAM lparam)
{
	auto *pinfo = GetTreeListInfo(iItem);
	if(!pinfo){
		return false;
	}
	pinfo->m_lParam = lparam;
	return true;
}

BOOL CGrTreeListView::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
	auto *pinfo = GetTreeListInfo(nItem);
	if(pinfo){
		pinfo->m_lstCols[nSubItem] = lpszText;
		return CGrListView::SetItemText(nItem, nSubItem, lpszText);
	}
	return FALSE;
}

BOOL CGrTreeListView::SetItemText(TreeListInfo *pinfo, int nSubItem, LPCTSTR lpszText)
{
	if(pinfo){
		pinfo->m_lstCols[nSubItem] = lpszText;
		LVFINDINFO info = {LVFI_PARAM};
		info.lParam = reinterpret_cast<LPARAM>(pinfo);
		int nItem = FindItem(&info);
		if(nItem >= 0){
			return CGrListView::SetItemText(nItem, nSubItem, lpszText);
		} else {
			return TRUE;
		}
	}
	return FALSE;
}

bool CGrTreeListView::ItemHasChildren(int iItem)
{
	return itemHasChildren(GetTreeListInfo(iItem));
}

void CGrTreeListView::SetDrawInsertPos(int iItem, int iTreeInsertParent, int iTreeInsertPos)
{
	if(m_iInsertPos != iItem || m_iInsertParent != iTreeInsertParent || m_iTreeInsertPos != iTreeInsertPos){
		m_iInsertPos     = iItem;
		m_iTreeInsertPos = iTreeInsertPos;
		m_iInsertParent  = iTreeInsertParent;
		InvalidateRect(m_hWnd, NULL, FALSE);
	}
}

int CGrTreeListView::GetInsertParent() const
{
	return m_iInsertParent;
}

int CGrTreeListView::GetTreeInsertPos() const
{
	return m_iTreeInsertPos;
}

void CGrTreeListView::Collapse(int nItem, TreeListInfo *pinfo)
{
	if(!pinfo){
		pinfo = GetTreeListInfo(nItem);
	}
	if(!pinfo){
		return;
	}
	pinfo->m_bCollapsed = true;

	SetRedraw(FALSE);
	int nLevel = pinfo->m_nLevel;
	int nLast  = GetItemCount();

	int nCounter = nItem + 1;
	if(nCounter < nLast){
		do {
			pinfo = GetTreeListInfo(nCounter);
			if(pinfo && pinfo->m_nLevel > nLevel){
				CGrListView::DeleteItem(nItem + 1);
			}
			if (!pinfo) {
				break;
			}
		} while(pinfo->m_nLevel > nLevel && GetItemCount() > nCounter);
	}

	SetRedraw(TRUE);
}

int CGrTreeListView::Expand(bool bRecursive, int& nCounterInsert, TreeListInfo *pinfo)
{
	OnExpandItem(pinfo);
	pinfo->m_bCollapsed = false;

	++nCounterInsert;
	int nCounter = nCounterInsert + 1;
	for(auto &&pitem : pinfo->m_children){
		if(!pitem){
			continue;
		}
		if(bRecursive){
			pitem->m_bCollapsed = false;
		}
		InsertItem(nCounterInsert, *pitem);
		if(bRecursive || !pitem->m_bCollapsed){
			nCounter = Expand(bRecursive, nCounterInsert, pitem);
		} else {
			++nCounterInsert;
		}
	}

	return nCounter;
}

int CGrTreeListView::InsertItem(int nItem, TreeListInfo& info)
{
	LVITEM item = {LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT | LVIF_STATE | LVIF_PARAM};
	item.iItem      = nItem;
	item.iSubItem   = 0;
	item.pszText    = const_cast<LPTSTR>(info.m_lstCols[0].c_str());
	item.iImage     = info.m_nImage;
	item.iIndent    = info.m_nLevel;
	if(itemHasChildren(&info)){
		item.state      = INDEXTOSTATEIMAGEMASK(info.m_bCollapsed ? 1 : 2);
	} else {
		item.state      = INDEXTOSTATEIMAGEMASK(3);
	}
	item.stateMask  = LVIS_STATEIMAGEMASK;
	item.lParam     = reinterpret_cast<LPARAM>(&info);

	int nItemRes = CGrListView::InsertItem(&item);
	int col_size = static_cast<signed int>(info.m_lstCols.size());
	for(int i=1; i < col_size; i++){
		CGrListView::SetItemText(nItemRes, i, const_cast<LPTSTR>(info.m_lstCols[i].c_str()));
	}
	return nItemRes;
}

bool CGrTreeListView::itemHasChildren(TreeListInfo *pinfo)
{
	return pinfo && !pinfo->m_children.empty();
}

void CGrTreeListView::OnKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);

	if(nItem == -1){
		CGrListView::OnKeyDown(hWnd, vk, fDown, cRepeat, flags);
		return;
	}
	LVITEM item = {LVIF_INDENT | LVIF_PARAM};
	item.iItem    = nItem;
	item.iSubItem = 0;
	GetItem(&item);

	auto *pinfo = reinterpret_cast<TreeListInfo*>(item.lParam);

	switch(vk){
	case VK_DELETE:
		{
			if(OnDeletingItem(pinfo)){
				DeleteItem(nItem);
			}
		}
		break;
	case VK_ADD:
		if(pinfo && pinfo->m_bCollapsed){
			if(itemHasChildren(pinfo)){
				int nInsert = nItem;
				SetRedraw(FALSE);
				Expand(false, nInsert, pinfo);
				SetRedraw(TRUE);
				SetItemState(nItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
			}
		}
		break;
	case VK_SUBTRACT:
		if(pinfo && !pinfo->m_bCollapsed){
			if(itemHasChildren(pinfo)){
				SetRedraw(FALSE);
				Collapse(nItem, pinfo);
				SetRedraw(TRUE);
				SetItemState(nItem, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
			}
		}
		break;
	case VK_RIGHT:
		if(pinfo){
			if(pinfo->m_bCollapsed){
				if(itemHasChildren(pinfo)){
					int nInsert = nItem;
					SetRedraw(FALSE);
					Expand(false, nInsert, pinfo);
					SetRedraw(TRUE);
					SetItemState(nItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
				}
			} else {
				SetItemState(-1, 0, LVIS_FOCUSED|LVIS_SELECTED);
				SetItemState(nItem + 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}
		break;
	case VK_LEFT:
		if(pinfo){
			if(!pinfo->m_bCollapsed){
				if(itemHasChildren(pinfo)){
					SetRedraw(FALSE);
					Collapse(nItem, pinfo);
					SetRedraw(TRUE);
					SetItemState(nItem, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
				}
			} else {
				if(pinfo->m_pParent){
					LVFINDINFO info = {LVFI_PARAM};
					info.lParam = reinterpret_cast<LPARAM>(pinfo->m_pParent);
					SetItemState(-1, 0, LVIS_FOCUSED|LVIS_SELECTED);
					SetItemState(FindItem(&info), LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
			}
		}
		break;
	default:
		CGrListView::OnKeyDown(hWnd, vk, fDown, cRepeat, flags);
		break;
	}
}

void CGrTreeListView::OnChar(HWND hwnd, TCHAR ch, int cRepeat)
{
	int nItem = GetNextItem(-1, LVNI_SELECTED);

	if(nItem != -1){
		LVITEM item = {LVIF_INDENT | LVIF_PARAM};
		item.iItem    = nItem;
		item.iSubItem = 0;
		GetItem(&item);
		if(ch == '*'){
			auto *pinfo = reinterpret_cast<TreeListInfo*>(item.lParam);
			if(pinfo && pinfo->m_bCollapsed && itemHasChildren(pinfo)){
				int nInsert = nItem;
				SetRedraw(FALSE);
				Expand(true, nInsert, pinfo);
				SetRedraw(TRUE);
				SetItemState(nItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
			}
		}
	}
}

LRESULT CGrTreeListView::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_ERASEBKGND: return 1;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = ::BeginPaint(hWnd, &ps);
			{
				if (m_bDragMode && m_iInsertPos >= 0 && m_iInsertParent >= 0) {
					SetItemState(m_iInsertParent, LVIS_SELECTED, LVIS_SELECTED);
				}
				CGrMemDC memdc(hdc, ps.rcPaint);
				auto hHeader = GetHeader();
				if(hHeader){
					RECT headerRect;
					GetWindowRect(hHeader, &headerRect);
					POINT pos;
					pos.x = headerRect.left;
					ScreenToClient(hWnd, &pos);
					SetWindowOrgEx(memdc, -pos.x, 0, NULL);
					SendMessage(hHeader, WM_ERASEBKGND, reinterpret_cast<WPARAM>(static_cast<HDC>(memdc)), 0);
					SendMessage(hHeader, WM_PAINT, reinterpret_cast<WPARAM>(static_cast<HDC>(memdc)), 0);
					ValidateRect(hHeader, &ps.rcPaint);
					SetWindowOrgEx(memdc, 0,0, NULL);
					RECT rcHeader;
					GetClientRect(hHeader, &rcHeader);
					ExcludeClipRect(memdc, rcHeader.left, rcHeader.top, rcHeader.right, rcHeader.bottom);
					CGrListView::WndProc(hWnd, uMsg, reinterpret_cast<WPARAM>(static_cast<HDC>(memdc)), 0);
				}
				if (m_bDragMode && m_iInsertPos >= 0 && m_iInsertParent >= 0) {
					SetItemState(m_iInsertParent, 0, LVIS_SELECTED);
				}
			}
			if(m_bDragMode){
				if(m_iInsertPos >= 0){
					auto oldclr = ::SetBkColor(hdc, ::GetSysColor(COLOR_WINDOW));
					RECT clinentRect;
					GetClientRect(m_hWnd, &clinentRect);
					RECT rect;
					int cnt = GetItemCount();
					if(m_iInsertPos >= cnt){
						GetItemRect(cnt-1, &rect, LVIR_BOUNDS);
						rect.top = rect.bottom;
					} else if(m_iInsertPos > 0){
						GetItemRect(m_iInsertPos-1, &rect, LVIR_ICON);
						rect.top = rect.bottom;
					} else {
						GetItemRect(0, &rect, LVIR_BOUNDS);
						rect.bottom = rect.top;
					}
					if(m_iInsertParent < 0){
						RECT rect_parent;
						GetItemRect(m_iTreeInsertPos, &rect_parent, LVIR_LABEL);
						rect.left = 0;
					} else {
						RECT rect_parent;
						GetItemRect(m_iInsertParent, &rect_parent, LVIR_LABEL);
						rect.left = rect_parent.left;
					}
					rect.right = clinentRect.right;
					rect.bottom += 2;
					::SetBkColor(hdc, ::GetSysColor(COLOR_WINDOWFRAME));
					::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

					::SetBkColor(hdc, oldclr);
				}
			}
			::EndPaint(hWnd, &ps);
			return 0;
		}
		break;
	case WM_LBUTTONDBLCLK:
	{
		auto x = static_cast<int>(static_cast<short>(LOWORD(lParam)));
		auto y = static_cast<int>(static_cast<short>(HIWORD(lParam)));
		auto keyFlags = static_cast<UINT>(wParam);
		LVHITTESTINFO info;
		info.pt.x = x;
		info.pt.y = y;
		info.flags = 0;
		info.iItem = 0;
		info.iSubItem = 0;
		HitTest(&info);
		if (info.flags & LVHT_ONITEMLABEL) {
			LVITEM item = { };
			item.mask = LVIF_INDENT | LVIF_PARAM;
			item.iItem = info.iItem;
			item.iSubItem = 0;
			GetItem(&item);
			auto *pinfo = reinterpret_cast<TreeListInfo*>(item.lParam);
			if (!pinfo) {
				break;
			}
			if (pinfo->m_bCollapsed) {
				if (itemHasChildren(pinfo)) {
					int nInsert = info.iItem;
					SetRedraw(FALSE);
					Expand(false, nInsert, pinfo);
					SetRedraw(TRUE);
					SetItemState(info.iItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
				}
			}
			else {
				SetRedraw(FALSE);
				Collapse(info.iItem, pinfo);
				SetRedraw(TRUE);
				SetItemState(info.iItem, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
			}
			SetItemState(info.iItem, LVIS_FOCUSED, LVIS_FOCUSED);
		}
		break;
	}
	case WM_LBUTTONDOWN:
		{
			auto x = static_cast<int>(static_cast<short>(LOWORD(lParam)));
			auto y = static_cast<int>(static_cast<short>(HIWORD(lParam)));
			auto keyFlags = static_cast<UINT>(wParam);
			LVHITTESTINFO info;
			info.pt.x = x;
			info.pt.y = y;
			info.flags = 0;
			info.iItem = 0;
			info.iSubItem = 0;
			HitTest(&info);
			if(info.flags & LVHT_ONITEMSTATEICON){
				LVITEM item = {};
				item.mask     = LVIF_INDENT | LVIF_PARAM;
				item.iItem    = info.iItem;
				item.iSubItem = 0;
				GetItem(&item);
				auto *pinfo = reinterpret_cast<TreeListInfo*>(item.lParam);
				if(!pinfo){
					break;
				}
				if(pinfo->m_bCollapsed){
					if(itemHasChildren(pinfo)){
						int nInsert = info.iItem;
						SetRedraw(FALSE);
						Expand(false, nInsert, pinfo);
						SetRedraw(TRUE);
						SetItemState(info.iItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
					}
				} else {
					SetRedraw(FALSE);
					Collapse(info.iItem, pinfo);
					SetRedraw(TRUE);
					SetItemState(info.iItem, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
				}
				SetItemState(info.iItem, LVIS_FOCUSED, LVIS_FOCUSED);
			}
		}
		break;
	}
	return CGrListView::WndProc(hWnd, uMsg, wParam, lParam);
}
