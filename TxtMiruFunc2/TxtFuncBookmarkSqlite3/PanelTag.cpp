#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "PanelTag.h"
#include "BookListDlg.h"
#include "Text.h"
#include "Shell.h"
#include "MessageBox.h"
#include "TagDlg.h"
#include "BookmarkDB.h"
#include "TxtMiruTheme.h"
//#define __DBG__
#include "Debug.h"

class CTagListropTarget : public CDropTarget
{
private:
	int m_tag_id = -1;
	CGrPanelTagWnd &m_tagWnd;
	CGrTreeListViewTxtMiru &m_listView;
public:
	CTagListropTarget(HWND hTargetWnd, CGrPanelTagWnd &tagWnd) : CDropTarget(hTargetWnd), m_tagWnd(tagWnd), m_listView(tagWnd.m_listView){}

	virtual HRESULT __stdcall DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
	{
		m_tag_id = -1;
		auto hr = CDropTarget::DragEnter(pDataObject, grfKeyState, ptl, pdwEffect);
		if(hr == S_OK && *pdwEffect != DROPEFFECT_NONE && m_bAllowDrop && m_pSupportedFrmt){
			STGMEDIUM medium;
			if(pDataObject->GetData(m_pSupportedFrmt, &medium) == S_OK){
				if(m_pSupportedFrmt->cfFormat == RegisterClipboardFormat(_T("TxtMiruBookInfo")) && medium.tymed == TYMED_HGLOBAL){
					m_listView.SetItemState(-1, 0, LVIS_SELECTED);
					auto *pBook = (TxtFuncBookmark::BookDragInfo*)GlobalLock(medium.hGlobal);
					if(pBook){
						m_tag_id = pBook->tag_id;
					}
					GlobalUnlock(medium.hGlobal);
				} else if(m_pSupportedFrmt->cfFormat == RegisterClipboardFormat(_T("TxtMiruTagInfo")) && medium.tymed == TYMED_HGLOBAL){
					m_tagWnd.m_listView.DragMode(true);
				}
				ReleaseStgMedium(&medium);
			}
		}
		return hr;
	}
	virtual HRESULT __stdcall DragLeave()
	{
		m_tagWnd.m_listView.DragMode(false);
		return CDropTarget::DragLeave();
	}
	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
	{
		CDropTarget::DragOver(grfKeyState, pt, pdwEffect);
		LVHITTESTINFO ht;
		ht.pt.x = pt.x;
		ht.pt.y = pt.y;
		ScreenToClient(m_hTargetWnd, &ht.pt);
		*pdwEffect = DROPEFFECT_NONE;
		if(m_tag_id < 0){
			int iItem = m_listView.HitTest(&ht);
			if(iItem < 0){
				*pdwEffect = DROPEFFECT_MOVE;
				iItem = m_listView.GetItemCount();
			} else {
				*pdwEffect = DROPEFFECT_MOVE;
				RECT rect;
				m_listView.GetItemRect(iItem, &rect, LVIR_BOUNDS);
				if(ht.pt.y > rect.top + (rect.bottom-rect.top) / 2){
					++iItem;
				}
			}
			int iInsertPos = iItem - 1;
			{
				/*         1  2  3   1  2  3   1  2  3   1  2  3  4   1  2  3  4
				       0  ○      | ○      | ○      | ○         | ○         |
				       1     ○   |    ○   | ■ □ ×|    ○      |    ○      |
				       2     ○   | ■ □ □|    ○   |       ○   |       ○   |
				       3  □ □ □|    ○   |       ○| □ □ □ □| ■ □ □ □|
				       4          |         |         |            |    ○      |
				  Parent  -1  0  2   X  0  1  X   0  X  -1  0  1  2   X  0  1  2
				  Inesrt   3  3  3   X  2  2  X   1  X   3  3  3  3   X  3  3  3
				  Prev     0  2 -1   X  1 -1  X  -1  X   0  1  2 -1   X  1  2 -1
				 */
				/*         1  2  3   1  2  3   1  2  3   1  2  3  4   1  2  3  4
				       0  ○      | ○      | ●      | ○         | ○         |
				       1     ○   |    ●   | ■ × ×|    ○      |    ○      |
				       2     ●   | ■ □ ×|    ○   |       ●   |       ●   |
				       3  □ □ ×|    ○   |       ○| □ □ □ ×| ■ □ □ ×|
				       4          |         |         |            |    ○      |
				  Parent  -1  0  2   X  0  1  X   0  X  -1  0  1  2   X  0  1  2
				  Inesrt   3  3  3   X  2  2  X   1  X   3  3  3  3   X  3  3  3
				  Prev     0  2 -1   X  1 -1  X  -1  X   0  1  2 -1   X  1  2 -1
				 */
				LVITEM item = {LVIF_PARAM};
				TreeListInfo *pStopInfo = nullptr;
				{
					// 親より上に挿入できないように位置を取得する [■]の部分
					// ※挿入行の 次の行と同じ親には移動できないようにする
					item.iItem  = iItem;
					m_listView.GetItem(&item);
					auto *pNextInfo = reinterpret_cast<TreeListInfo*>(item.lParam);
					if(pNextInfo){
						pStopInfo = pNextInfo->m_pParent;
					}
				}
				item.iItem  = iInsertPos;
				m_listView.GetItem(&item);
				int iPreParent = iInsertPos;
				if(item.lParam){
					RECT rect;
					m_listView.GetItemRect(iInsertPos, &rect, LVIR_LABEL);
					if(m_listView.GetItemState(item.iItem, LVIS_SELECTED) & LVIS_SELECTED){
						rect.left = ht.pt.x + 1;
					}
					auto *pinfo = reinterpret_cast<TreeListInfo*>(item.lParam);
					while(rect.left > ht.pt.x){
						if(pStopInfo == pinfo){
							*pdwEffect = DROPEFFECT_NONE;
							break;
						}
						auto *pPrev = pinfo;
						pinfo = pinfo->m_pParent;
						if(!pinfo){
							iPreParent = iItem -1;
							iInsertPos = iItem -1;
							break;
						}
						LVFINDINFO info = {LVFI_PARAM};
						info.lParam = reinterpret_cast<LPARAM>(pinfo);
						int iParent = m_listView.FindItem(&info);
						iInsertPos = iPreParent; // 同一レベルでの挿入位置
						iPreParent = iParent;
						if(iParent < 0){
							break;
						} else {
							m_listView.GetItemRect(iParent, &rect, LVIR_LABEL);
							if(m_listView.GetItemState(iParent, LVIS_SELECTED) & LVIS_SELECTED){
								rect.left = ht.pt.x + 1;
							}
						}
					}
				}
				m_listView.SetDrawInsertPos(iItem, iPreParent, iInsertPos+1);
			}
		} else {
			int iItem = m_listView.HitTest(&ht);
			m_listView.SetItemState(-1, 0, LVIS_SELECTED);
			if(iItem >= 0){
				auto *ptag = reinterpret_cast<TxtFuncBookmark::Tag *>(m_listView.GetParam(iItem));
				if(ptag){
					if(ptag->id != m_tag_id){
						*pdwEffect = DROPEFFECT_MOVE;
						m_listView.SetItemState(iItem, LVNI_SELECTED, LVNI_SELECTED);
					}
				}
			}
		}
		return S_OK;
	}
	virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
	{
		if(pFmtEtc->cfFormat == RegisterClipboardFormat(_T("TxtMiruBookInfo")) && medium.tymed == TYMED_HGLOBAL){
			do {
				int cnt = m_listView.GetSelectedCount();
				if(cnt <= 0){
					break;
				}
				int iItem = -1;
				while((iItem=m_listView.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED))!=-1){
					break;
				}
				if(iItem < 0){
					break;
				}
				auto *ptag = reinterpret_cast<TxtFuncBookmark::Tag *>(m_listView.GetParam(iItem));
				if(!ptag){
					break;
				}
				if(ptag->id == m_tag_id){
					break;
				}
				auto *pBook = static_cast<TxtFuncBookmark::BookDragInfo*>(GlobalLock(medium.hGlobal));
				if(pBook){
					*pdwEffect = DROPEFFECT_MOVE;
					m_tagWnd.moveBook(pBook->num, pBook->id_list, ptag->id);
				}
				GlobalUnlock(medium.hGlobal);
			} while(0);
		} else if(pFmtEtc->cfFormat == RegisterClipboardFormat(_T("TxtMiruTagInfo")) && medium.tymed == TYMED_HGLOBAL){
			do {
				std::vector<int> id_list;
				int cnt = m_listView.GetSelectedCount();
				UINT flags = (cnt <= 0) ? (LVNI_ALL | LVNI_FOCUSED) : (LVNI_ALL | LVNI_SELECTED);
				int iItem = -1;
				{
					int count = m_listView.GetItemCount();
					while((iItem=m_listView.GetNextItem(iItem, flags))!=-1){
						if(iItem >= count){
							break;
						}
						id_list.push_back(iItem); // 親だけを選択
						LVITEM item = { LVIF_INDENT };
						item.iItem = iItem;
						if(FALSE == m_listView.GetItem(&item)){
							break;
						}
						int iIndent = item.iIndent;
						++item.iItem;
						for(;item.iItem<count; ++item.iItem){
							if(FALSE == m_listView.GetItem(&item)){
								break;
							}
							if(item.iIndent <= iIndent){
								break;
							}
							iItem = item.iItem;
						}
					}
				}
				if(!id_list.empty()){
					std::reverse(id_list.begin(), id_list.end());
					m_listView.MoveItems(m_listView.GetInsertParent(), m_listView.GetTreeInsertPos(), &id_list[0], id_list.size());
				}
			} while(0);
		}
		m_listView.DragMode(false);
		return true; //let base free the medium
	}
};

CGrPanelTagWnd::CGrPanelTagWnd(CGrBookListDlg &dlg) : CGrTreePanelWnd(dlg)
{
}

CGrPanelTagWnd::~CGrPanelTagWnd()
{
	deleteAllTagList();
}

bool CGrPanelTagWnd::Create(HWND hParent, HIMAGELIST hImg, HIMAGELIST hTreeImg)
{
	if(!CGrTreePanelWnd::Create(hParent, hImg, hTreeImg)){
		return false;
	}
	//
	std::tstring str_name;
	SetWindowFont(m_listView, GetWindowFont(hParent), TRUE);
	LVCOLUMN lvc = {};
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	//
	CGrText::LoadString(IDS_BMH_TITLE, str_name);
	lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
	lvc.iSubItem = 0;
	lvc.cx = 300;
	m_listView.InsertColumn(lvc.iSubItem, &lvc);
	//
	auto &&toolBar = m_coolBar.GetToolBar();
	TBBUTTON tbb[] = {
		//iBitmap                                                      idCommand       fsState          fsStyle                         bReserved[2]  dwData iString
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stay_on_top), IDM_STAYONTOP , TBSTATE_ENABLED, TBSTYLE_CHECK  | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_IDLINKSTAY     },
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_tag_add    ), IDM_TAG_ADD   , TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_TAG_ADD   },
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_tag_modify ), IDM_TAG_MODIFY, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_TAG_MODIFY},
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_tag_delete ), IDM_TAG_DELETE, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_TAG_DELETE},
	};
	for(auto &&item : tbb){
		if(item.iString != 0){
			std::tstring tips;
			CGrText::LoadString(item.iString, tips);
			item.iString = SendMessage(toolBar, TB_ADDSTRING, 0, reinterpret_cast<LPARAM>(tips.c_str()));
		} else {
			item.iString = sizeof(tbb)/sizeof(TBBUTTON) + 1;
		}
	}
	TBADDBITMAP tb = {};
	toolBar.AddButtons(tbb, sizeof(tbb)/sizeof(TBBUTTON), tb);
	toolBar.SetButtons(_T("1,2,3,4,5"));
	SendMessage(toolBar, TB_CHECKBUTTON, IDM_STAYONTOP, MAKELPARAM(m_booklistDlg.IsStayOn(), 0));
	//
	m_listView.SetCallback(this);
	m_listView.SetRedraw(FALSE);
	DispList();
	if(m_listView.GetItemCount() == 0){
		std::tstring name;
		CGrText::LoadString(IDS_TAG_FAVORITE, name);
		addItem(nullptr, nullptr, NULL, name.c_str());
	}
	m_listView.SetRedraw(TRUE);
	//
	m_pDropTarget = new CTagListropTarget(m_listView, *this);
	if(FAILED(RegisterDragDrop(m_listView, m_pDropTarget))){
		m_pDropTarget = nullptr;
	} else {
		FORMATETC ftetc = {};
		ftetc.cfFormat = RegisterClipboardFormat(_T("TxtMiruTagInfo"));
		ftetc.dwAspect = DVASPECT_CONTENT;
		ftetc.lindex   = -1;
		ftetc.tymed    = TYMED_HGLOBAL;
		m_pDropTarget->AddSuportedFormat(ftetc);
		ftetc.cfFormat = RegisterClipboardFormat(_T("TxtMiruBookInfo"));
		m_pDropTarget->AddSuportedFormat(ftetc);
	}
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);
	//
	return true;
}

// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
LRESULT CGrPanelTagWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_COMMAND      , OnCommand      );
	case WM_SIZE:
		{
			auto state = static_cast<UINT>(wParam);
			auto cx = static_cast<int>(static_cast<short>(LOWORD(lParam)));
			auto cy = static_cast<int>(static_cast<short>(HIWORD(lParam)));
		}
		break;
	}
	return CGrTreePanelWnd::WndProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CGrPanelTagWnd::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	if(idFrom == listwnd_id){
		switch(lpnmhdr->code){
		case NM_RCLICK:
			if(!m_booklistDlg.IsUpdating()){
				POINT cursor;
				GetCursorPos(&cursor);
				auto lpnmitemactivate = reinterpret_cast<LPNMITEMACTIVATE>(lpnmhdr);
				auto hMenu = ::LoadMenu(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDR_MENU_POPUP_TAG));
				CGrPanelWnd_SetTouchMenu(hMenu);
				::TrackPopupMenu(::GetSubMenu(hMenu, 0), TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL, cursor.x, cursor.y, 0, hWnd, NULL);
				::DestroyMenu(hMenu);
			}
			break;
		case LVN_ITEMCHANGED:
			if(!m_booklistDlg.IsUpdating()){
				auto pnmv = reinterpret_cast<LPNMLISTVIEW>(lpnmhdr);
				if(pnmv->uChanged & LVIF_STATE && pnmv->uNewState & LVIS_FOCUSED){
					if(m_listView.GetItemState(pnmv->iItem, LVIS_SELECTED) & LVIS_SELECTED){
						auto *pInfo = reinterpret_cast<TreeListInfo*>(pnmv->lParam);
						if(pInfo){
							auto *ptag = reinterpret_cast<TxtFuncBookmark::Tag *>(pInfo->m_lParam);
							if(ptag){
								m_booklistDlg.ShowBookList(*ptag);
							}
						}
					}
				}
			}
			break;
		case LVN_BEGINDRAG:
			if(!m_booklistDlg.IsUpdating()){
				auto pnmv = reinterpret_cast<LPNMLISTVIEW>(lpnmhdr);
				std::vector<int> id_list;
				std::vector<int> select_id_list;
				int cnt = m_listView.GetSelectedCount();
				UINT flags = (cnt <= 0) ? (LVNI_ALL | LVNI_FOCUSED) : (LVNI_ALL | LVNI_SELECTED);
				int iItem = -1;
				{
					int count = m_listView.GetItemCount();
					while((iItem=m_listView.GetNextItem(iItem, flags))!=-1){
						if(iItem >= count){
							break;
						}
						id_list.push_back(iItem); // 親だけを選択
						select_id_list.push_back(iItem);
						LVITEM item = { LVIF_INDENT };
						item.iItem = iItem;
						if(FALSE == m_listView.GetItem(&item)){
							break;
						}
						int iIndent = item.iIndent;
						++item.iItem;
						for(;item.iItem<count; ++item.iItem){
							if(FALSE == m_listView.GetItem(&item)){
								break;
							}
							if(item.iIndent <= iIndent){
								break;
							}
							if(m_listView.GetItemState(item.iItem, LVIS_SELECTED) & LVIS_SELECTED){
								select_id_list.push_back(item.iItem);
							} else {
								m_listView.SetItemState(item.iItem, LVIS_SELECTED, LVIS_SELECTED);
							}
							iItem = item.iItem;
						}
					}
				}
				if(id_list.size() <= 0){
					break;
				}
				auto *pDropSource = new CDropSource;

				///////////////
				auto *pDataObject = new CDataObject(pDropSource);

				FORMATETC fmtetc = {0};
				fmtetc.cfFormat = RegisterClipboardFormat(_T("TxtMiruTagInfo"));
				fmtetc.dwAspect = DVASPECT_CONTENT;
				fmtetc.lindex   = -1;
				fmtetc.tymed    = TYMED_HGLOBAL;
				STGMEDIUM medium = {0};
				medium.tymed   = TYMED_HGLOBAL;
				medium.hGlobal = GlobalAlloc(GHND|GMEM_SHARE, sizeof(TxtFuncBookmark::TagDragInfo)+sizeof(int)*id_list.size());
				if (medium.hGlobal) {
					auto* pTag = static_cast<TxtFuncBookmark::TagDragInfo*>(GlobalLock(medium.hGlobal));
					if (pTag) {
						pTag->num = id_list.size();
						int* p = pTag->id_list;
						for (const auto& id : id_list) {
							*p = id;
							++p;
						}
					}
					GlobalUnlock(medium.hGlobal);
				}
				pDataObject->SetData(&fmtetc,&medium,TRUE);
				///////////////
				CDragSourceHelper dragSrcHelper;
				dragSrcHelper.InitializeFromWindow(m_listView, pnmv->ptAction, pDataObject);

				DWORD dwEffect = DROPEFFECT_MOVE;
				auto hr = ::DoDragDrop(pDataObject, pDropSource, DROPEFFECT_MOVE, &dwEffect);
				m_listView.DragMode(false);
				bool bCancel = true;
				if(SUCCEEDED(hr)){
					if(dwEffect == DROPEFFECT_MOVE){
						bCancel = false;
					}
				}
				if(bCancel){
					m_listView.SetItemState(-1, 0, LVIS_FOCUSED|LVIS_SELECTED);
					for(auto i : select_id_list){
						m_listView.SetItemState(i, LVIS_SELECTED , LVIS_SELECTED);
					}
					if(select_id_list.size() > 0){
						m_listView.SetItemState(select_id_list[0], LVIS_FOCUSED|LVIS_SELECTED , LVIS_FOCUSED|LVIS_SELECTED);
					}
				}
				pDropSource->Release();
				pDataObject->Release();
			}
			break;
		}
	}
	return CGrTreePanelWnd::OnNotify(hWnd, idFrom, lpnmhdr);
}

void CGrPanelTagWnd::SetWorking(bool bWork)
{
	int id_list[] = {
		IDM_TAG_ADD   ,
		IDM_TAG_MODIFY,
		IDM_TAG_DELETE,
	};
	BOOL bEnabled = bWork ? FALSE : TRUE;
	auto &&toolBar = m_coolBar.GetToolBar();
	for(int i=0; i<sizeof(id_list)/sizeof(int); ++i){
		SendMessage(toolBar, TB_ENABLEBUTTON, id_list[i], bEnabled);
	}
}

void CGrPanelTagWnd::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDM_STAYONTOP:
		{
			m_booklistDlg.SetStayOn(SendMessage(m_coolBar.GetToolBar(), TB_ISBUTTONCHECKED, IDM_STAYONTOP, 0) != 0);
		}
		break;
	case IDM_TAG_ADD:
		{
			TxtFuncBookmark::Tag tag;
			CGrTagDlg dlg(tag);
			if(IDOK == dlg.DoModal(m_hWnd)){
				int iItem = GetFocusedItem();
				TreeListInfo *pInfo = nullptr;
				if(iItem >= 0){
					pInfo = m_listView.GetTreeListInfo(iItem);
				}
				if(pInfo && pInfo->m_pParent){
					auto pInfoParent = pInfo->m_pParent;
					{
						const auto &children = pInfoParent->m_children;
						auto it = std::find(children.begin(), children.end(), pInfo);
						if(it == children.end()){
							break;
						}
						++it; // 次の行
						if(it == children.end()){
							pInfo = nullptr;
						} else {
							pInfo = *it;
						}
					}
					addItem(pInfoParent, pInfo, nullptr, tag.title.c_str());
				} else {
					// Topの最後に追加
					addItem(nullptr, pInfo, nullptr, tag.title.c_str());
				}
			}
		}
		break;
	case IDM_TAG_MODIFY:
		if(!m_booklistDlg.IsUpdating()){
			int iItem = GetFocusedItem();
			if(iItem < 0){
				break;
			}
			auto *pTagInfo = reinterpret_cast<TxtFuncBookmark::Tag*>(m_listView.GetParam(iItem));
			if(!pTagInfo){
				break;
			}
			auto &&cur_tag = *pTagInfo;
			auto tag = cur_tag;
			CGrTagDlg dlg(tag);
			if(IDOK == dlg.DoModal(m_hWnd)){
				cur_tag.title = tag.title;
				updateItem(cur_tag.id, cur_tag.title.c_str());
				m_listView.SetItemText(iItem, 0, const_cast<LPTSTR>(cur_tag.title.c_str()));
			}
		}
		break;
	case IDM_TAG_DELETE:
		if(!m_booklistDlg.IsUpdating()){
			deleteSelectItem();
		}
		break;
	}
}

void CGrPanelTagWnd::deleteSelectItem()
{
	do {
		int cnt = m_listView.GetSelectedCount();
		int refresh_index = -1;
		if(cnt > 0){
			std::tstring message;
			if(cnt == 1){
				int iItem = m_listView.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
				auto *pTagInfo = reinterpret_cast<TxtFuncBookmark::Tag*>(m_listView.GetParam(iItem));
				if(pTagInfo){
					CGrText::FormatMessage(CGrTxtFunc::GetDllModuleHandle(), message, IDS_DELETE_TAG, pTagInfo->title.c_str());
				}
			} else {
				CGrText::FormatMessage(CGrTxtFunc::GetDllModuleHandle(), message, IDS_DELETE_TAG_MULTI, cnt);
			}
			if(IDYES != CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, message.c_str(), CGrTxtFunc::AppName(), MB_YESNO)){
				break;
			}
			std::vector<int> id_list;
			{
				int iItem = -1;
				while((iItem=m_listView.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED))!=-1){
					id_list.push_back(iItem);
				}
			}
			// 下から順に削除
			std::reverse(id_list.begin(), id_list.end());
			for(auto iItem : id_list){
				if(!m_listView.DeleteItem(iItem)){
					break;
				}
			}
		}
	} while(0);
}

bool CGrPanelTagWnd::OnDeletingItem(TreeListInfo *pinfo)
{
	if(pinfo){
		deleteSelectItem();
	}
	return false;
}


bool CGrPanelTagWnd::Select(int id)
{
	m_listView.SetRedraw(FALSE);
	m_listView.SetItemState(-1, 0, LVNI_FOCUSED|LVIS_SELECTED);
	int index = -1;
	for(int i=m_listView.GetItemCount()-1; i>=0; --i){
		auto *pTagInfo = reinterpret_cast<TxtFuncBookmark::Tag*>(m_listView.GetParam(i));
		if(pTagInfo){
			if(id == pTagInfo->id){
				m_listView.SetItemState(i, LVNI_FOCUSED|LVNI_SELECTED, LVNI_FOCUSED|LVNI_SELECTED);
				index = i;
			}
		}
	}
	if(index < 0){
		std::vector<int> tag_id_list;
		if(getSelectTree(tag_id_list, id)){
			setSelectTree(tag_id_list);
		}
	} else {
		m_listView.EnsureVisible(index, FALSE);
	}
	m_listView.SetRedraw(TRUE);
	return true;
}

bool CGrPanelTagWnd::setSelectTree(const std::vector<int> &tag_id_list)
{
	int iSelectItem = -1;
	LVFINDINFO info = {LVFI_PARAM};
	auto pitem = m_listView.GetRootTreeListInfo();
	for(auto id : tag_id_list){
		bool bNotFind = true;
		for(const auto pitem_c : pitem->m_children){
			auto *pTagInfo = reinterpret_cast<TxtFuncBookmark::Tag*>(pitem_c->m_lParam);
			if(pTagInfo){
				if(id == pTagInfo->id){
					pitem = pitem_c;
					info.lParam = reinterpret_cast<LPARAM>(pitem);
					int nItem = m_listView.FindItem(&info);
					if(nItem >= 0){
						m_listView.ExpandItem(nItem, false);
						iSelectItem = nItem;
						bNotFind = false;
					}
					break;
				}
			}
		}
		if(bNotFind){
			break;
		}
	}
	if(iSelectItem >= 0){
		m_listView.SetItemState(iSelectItem, LVNI_FOCUSED|LVNI_SELECTED, LVNI_FOCUSED|LVNI_SELECTED);
		m_listView.EnsureVisible(iSelectItem, FALSE);
	}
	return true;
}

void CGrPanelTagWnd::deleteAllTagList()
{
	for(auto &item : m_tag_list){
		if(item){
			delete item;
		}
	}
	m_tag_list.clear();
}

bool CGrPanelTagWnd::addItem(TreeListInfo *pParentInfo, TreeListInfo *pPosInfo, LPCTSTR lpURL, LPCTSTR lpTitle)
{
	CGrBookmarkDB db;
	if(!db.Open()){
		return false;
	}
	bool bret = false;
	do {
		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			return false;
		}
		CGrDBBooks books;
		if(lpURL){
			if(_tcslen(lpURL) > 0){
				::Book book;
				if(books.GetByBaseURL(pSqlite3, book, lpURL, 1)){
					return false;
				}
			}
		} else {
			lpURL = _T("");
		}
		std::tstring sysdate;
		CGrDBFunc::GetSysDate(sysdate);
		int position = 0;
		int insert_id = NULL_INT_VALUE;
		TxtFuncBookmark::Tag *pTagInfo_i = nullptr;
		if(pPosInfo){
			pTagInfo_i = reinterpret_cast<TxtFuncBookmark::Tag*>(pPosInfo->m_lParam);
			if(pTagInfo_i){
				insert_id = pTagInfo_i->id;
			}
		} else {
			books.MaxPosition(pSqlite3, NULL_INT_VALUE, position);
		}
		int parent_id = -1;
		if(pParentInfo){
			auto *pTagParentInfo = reinterpret_cast<TxtFuncBookmark::Tag *>(pParentInfo->m_lParam);
			if(pTagParentInfo){
				parent_id = pTagParentInfo->id;
			}
		}
		if(parent_id < 0){
			parent_id = NULL_INT_VALUE;
		}
		int ret = 0;
		if(SQLITE_OK != db.BeginSession()){
			bret = false;
			break;
		}
		::Book book;
		book.id          = NULL_INT_VALUE;
		book.place_id    = NULL_INT_VALUE;
		book.parent      = parent_id     ;
		book.type        = 1             ;
		book.position    = position+1    ;
		book.base_url    = lpURL         ;
		book.title       = lpTitle       ;
		book.author      = _T("")        ;
		book.insert_date = sysdate       ;
		bret = books.Put(pSqlite3, book);
		if(!bret){
			break;
		}
		bret = books.GetLastInsertRow(pSqlite3, book);
		if(!bret){
			break;
		}
		do {
			bret = books.SortPosition(pSqlite3, parent_id, insert_id, book.id);
			if(!bret){
				break;
			}
		} while(0);
		if(bret){
			if(SQLITE_OK != db.Commit()){
				bret = false;
				break;
			}
		} else {
			db.Rollback();
			break;
		}
		//////////////////////////// TreeView 画面への反映
		auto *pTagInfo = new TxtFuncBookmark::Tag();
		auto &&tag = *pTagInfo;
		tag.id       = book.id      ;
		tag.title    = book.title   ;
		tag.position = book.position;
		//
		m_tag_list.push_back(pTagInfo);
		if(pParentInfo){
			m_listView.InsertItemPos(pParentInfo, pPosInfo, lpTitle, static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_tag_favorite), reinterpret_cast<LPARAM>(pTagInfo));
		} else {
			m_listView.AddItem(lpTitle, static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_tag_favorite), 0, reinterpret_cast<LPARAM>(pTagInfo));
		}
	} while(0);

	return bret;
}

bool CGrPanelTagWnd::updateItem(int id, LPCTSTR lpTitle)
{
	bool bret = false;
	do {
		CGrBookmarkDB db;
		if(!db.Open()){
			break;
		}
		sqlite3 *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			break;
		}
		CGrDBBooks books;
		std::tstring sysdate;
		CGrDBFunc::GetSysDate(sysdate);

		::Book book;
		if(!books.Get(pSqlite3, book, id)){
			break;
		}
		book.title       = lpTitle;
		book.insert_date = sysdate;
		bret = books.Put(pSqlite3, book);
		if(!bret){
			break;
		}
	} while(0);

	return bret;
}

bool CGrPanelTagWnd::DispList(bool bKeepIndex)
{
	bool bRet = true;

	m_listView.SetRedraw(FALSE);
	do {
		auto &param = CGrTxtFunc::Param();
		// 栞フォルダ
		std::tstring str;
		CGrTxtFunc::GetBookmarkFolder(&param, str);
		std::tstring bookmark_db_file;
		if(!CGrShell::GetSearchDir(str.c_str())){
			if(!CGrShell::CreateFolder(str.c_str())){
				bRet = false;
				break;
			}
		}
		int index = -1;
		int focus_item = -1;
		if(bKeepIndex){
			int iTopIndex = m_listView.GetTopIndex();
			int iCountPage = m_listView.GetCountPerPage();
			index = iTopIndex + iCountPage - 1;
			focus_item = m_listView.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED);
		}
		SCROLLINFO si = {sizeof(SCROLLINFO)};
		si.fMask = SIF_POS;
		::GetScrollInfo(m_listView, SB_HORZ, &si);
		CGrBookmarkDB db;
		if(!db.Open()){
			bRet = false;
			break;
		}
		deleteAllTagList();
		m_listView.DeleteAllItems();

		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			bRet = false;
			break;
		}
		bool bret = true;
		sqlite3_stmt *pstmt = nullptr;
		do {
			int ret = sqlite3_prepare16(pSqlite3, _T("SELECT B.ID,B.TITLE,B.POSITION,EXISTS(SELECT 1 FROM TXTMIRU_BOOKS A WHERE A.PARENT=B.ID AND A.TYPE=1)")
										_T(" FROM TXTMIRU_BOOKS B ")
										_T(" WHERE B.PARENT IS NULL ORDER BY B.POSITION"), -1, &pstmt, nullptr);
			if(ret != SQLITE_OK || !pstmt) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt);
			// 実行
			bret = false;
			while(true){
				ret = sqlite3_step(pstmt);
				if(ret != SQLITE_ROW){
					break;
				}
				auto *pTagInfo = new TxtFuncBookmark::Tag();
				auto &&tag = *pTagInfo;

				int iChildExists = 0;
				CGrDBFunc::GetValue(pstmt, 0, tag.id             );
				CGrDBFunc::GetValue(pstmt, 1, tag.title          );
				CGrDBFunc::GetValue(pstmt, 2, tag.position       );
				CGrDBFunc::GetValue(pstmt, 3, iChildExists       );
				m_tag_list.push_back(pTagInfo);
				bret = true;
				LVITEM item = {};
				item.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_STATE;
				item.iItem     = m_listView.GetItemCount();
				item.iSubItem  = 0;
				item.lParam    = reinterpret_cast<LPARAM>(pTagInfo);
				item.pszText   = const_cast<LPTSTR>(tag.title.c_str());
				item.iImage    = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_tag_favorite);
				item.stateMask = LVIS_STATEIMAGEMASK;
				item.state     = INDEXTOSTATEIMAGEMASK(2);
				auto *pItemInfo = m_listView.AddItem(item.pszText, item.iImage, 0, item.lParam);
				if(pItemInfo && iChildExists > 0){
					pItemInfo->AddDummyChild(); // 子有
					m_listView.SetItemState(item.iItem, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
				}
			};
		} while(0);

		if(pstmt){
			sqlite3_finalize(pstmt);
		}
		if(index >= 0){
			int iLastItem = m_listView.GetItemCount() - 1;
			if(iLastItem < index){
				index = iLastItem;
			}
			if(focus_item >= 0){
				m_listView.SetItemState(focus_item, LVNI_FOCUSED|LVNI_SELECTED, LVNI_FOCUSED|LVNI_SELECTED);
			} else {
				m_listView.SetItemState(index, LVNI_FOCUSED|LVNI_SELECTED, LVNI_FOCUSED|LVNI_SELECTED);
			}
			m_listView.EnsureVisible(index, FALSE);
		}
		::SetScrollInfo(m_listView, SB_HORZ, &si, TRUE);
	} while(0);
	m_listView.SetRedraw(TRUE);

	return bRet;
}
bool CGrPanelTagWnd::getSelectTree(std::vector<int> &tag_id_list, int id)
{
	bool bRet = true;

	do {
		auto &param = CGrTxtFunc::Param();
		// 栞フォルダ
		std::tstring str;
		CGrTxtFunc::GetBookmarkFolder(&param, str);
		std::tstring bookmark_db_file;
		if(!CGrShell::GetSearchDir(str.c_str())){
			if(!CGrShell::CreateFolder(str.c_str())){
				bRet = false;
				break;
			}
		}
		CGrBookmarkDB db;
		if(!db.Open()){
			bRet = false;
			break;
		}
		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			bRet = false;
			break;
		}
		bool bret = true;
		CGrDBBooks m_books;
		do {
			Book book;
			if(m_books.Get(pSqlite3, book, id)){
				tag_id_list.push_back(id);
				if(tag_id_list.size() < 20 && book.parent >= 0 && book.parent != id){
					id = book.parent;
					continue;
				}
			}
			break;
		} while(true);
		std::reverse(tag_id_list.begin(), tag_id_list.end());
	} while(0);

	return bRet;
}

bool CGrPanelTagWnd::deleteItem(int iItem)
{
	auto *pInfo = (TxtFuncBookmark::Tag*)m_listView.GetParam(iItem);
	if(!pInfo){
		return false;
	}
	auto &&tag = *pInfo;
	CGrBookmarkDB db;
	if(!db.Open()){
		return false;
	}
	bool bret = false;
	do {
		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			return false;
		}
		CGrDBBooks books;
		::Book book;
		if(books.Get(pSqlite3, book, tag.id)){
			if(!book.base_url.empty()){
				bret = true;
				break;
			}
		}
		bret = books.Delete(pSqlite3, tag.id);
		if(!bret){
			break;
		}
		if(bret){
			m_tag_list.remove(pInfo);
			if(SQLITE_OK != db.Commit()){
				bret = false;
				break;
			}
		} else {
			db.Rollback();
			break;
		}
	} while(0);

	return bret;
}
//
// pinfo_p     : 親
// pInsertItem : 挿入位置
// pinfo       : 移動対象
void CGrPanelTagWnd::OnUpdateItemPosition(TreeListInfo *pinfo_p, TreeListInfo *pInsertItem, TreeListInfo *pinfo)
{
	if(!pinfo_p || !pinfo){
		return;
	}
	CGrBookmarkDB db;
	if(!db.Open()){
		return;
	}
	bool bret = false;
	do {
		bool bRet = true;

		auto *pTagInfoP = reinterpret_cast<TxtFuncBookmark::Tag*>(pinfo_p->m_lParam);
		int parent_id = NULL_INT_VALUE;
		if(pTagInfoP){
			parent_id = pTagInfoP->id;
		} else {
			// Top
		}
		int insert_id = NULL_INT_VALUE;
		if(pInsertItem){
			auto *pTagInsertInfo = reinterpret_cast<TxtFuncBookmark::Tag*>(pInsertItem->m_lParam);
			if(pTagInsertInfo){
				insert_id = pTagInsertInfo->id;
			}
		} else {
		}
		auto *pTagInfo = reinterpret_cast<TxtFuncBookmark::Tag*>(pinfo->m_lParam);
		if(!pTagInfo){
			break;
		}
		int id = pTagInfo->id;
		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			return;
		}
		CGrDBBooks books;
		std::tstring sysdate;
		CGrDBFunc::GetSysDate(sysdate);

		sqlite3_stmt *pstmt = nullptr;
		LPCTSTR lpSql = nullptr;
		int ret = 0;
		if(SQLITE_OK != db.BeginSession()){
			bRet = false;
			break;
		}
		if(parent_id == NULL_INT_VALUE){
			lpSql = _T("UPDATE TXTMIRU_BOOKS SET PARENT=NULL WHERE ID=@ID");
		} else {
			lpSql = _T("UPDATE TXTMIRU_BOOKS SET PARENT=@PARENT WHERE ID=@ID");
		}
		ret = sqlite3_prepare16(pSqlite3, lpSql, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bRet = false;
			break;
		}
		bRet = true;
		do {
			sqlite3_reset(pstmt);
			sqlite3_clear_bindings(pstmt);
			CGrDBFunc::PutValue(pstmt, "@PARENT", parent_id);
			CGrDBFunc::PutValue(pstmt, "@ID"    , id       );
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_DONE){
				bRet = false;
				break;
			}
		} while(0);
		if(pstmt){
			sqlite3_finalize(pstmt);
		}
		do {
			std::vector<::Book> id_list;
			::Book book;
			book.position = 0;
			for(const auto item : pinfo_p->m_children){
				if(!item){
					continue;
				}
				auto *pTagInfoItem = reinterpret_cast<TxtFuncBookmark::Tag*>(item->m_lParam);
				if(!pTagInfoItem){
					continue;
				}
				++book.position;
				book.id = pTagInfoItem->id;
				id_list.push_back(book);
			}
			bRet = books.SetPositionList(pSqlite3, id_list);
			if(!bRet){
				break;
			}
		} while(0);
		if(bRet){
			if(SQLITE_OK != db.Commit()){
				bRet = false;
				break;
			}
		} else {
			db.Rollback();
			break;
		}
	} while(0);
}

void CGrPanelTagWnd::OnDeleteItem(TreeListInfo *pinfo)
{
	if(pinfo){
		auto *pTagInfo = reinterpret_cast<TxtFuncBookmark::Tag*>(pinfo->m_lParam);
		if(pTagInfo){
			CGrBookmarkDB db;
			if(!db.Open()){
				return;
			}
			bool bret = false;
			do {
				bool bRet = true;
				auto *pSqlite3 = db.GetSqlite3();
				if(!pSqlite3){
					return;
				}
				int id = pTagInfo->id;
				//////
				sqlite3_stmt *pstmt = nullptr;
				LPCTSTR lpSql = nullptr;
				int ret = 0;
				if(SQLITE_OK != db.BeginSession()){
					bRet = false;
					break;
				}
				ret = sqlite3_prepare16(pSqlite3, _T("UPDATE TXTMIRU_BOOKS SET PARENT=-1 WHERE ID=@ID"), -1, &pstmt, nullptr);
				if(ret != SQLITE_OK || !pstmt) {
					// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
					bRet = false;
					break;
				}
				bRet = true;
				do {
					sqlite3_reset(pstmt);
					sqlite3_clear_bindings(pstmt);
					CGrDBFunc::PutValue(pstmt, "@ID", id);
					ret = sqlite3_step(pstmt);
					if(ret != SQLITE_DONE){
						bRet = false;
						break;
					}
				} while(0);
				if(pstmt){
					sqlite3_finalize(pstmt);
				}
				if(bRet){
					if(SQLITE_OK != db.Commit()){
						bRet = false;
						break;
					}
					m_tag_list.remove(pTagInfo);
					delete pTagInfo;
				} else {
					db.Rollback();
					break;
				}
			} while(0);
		}
	}
}

void CGrPanelTagWnd::OnExpandItem(TreeListInfo *pParentInfo)
{
	bool bExpand = false;
	if(pParentInfo->isDummyChild()){
		bExpand = true;
		pParentInfo->DeleteAllItems();
	}
	if(bExpand){
		auto *pParentTagInfo = reinterpret_cast<TxtFuncBookmark::Tag*>(pParentInfo->m_lParam);
		if(pParentTagInfo){
			bool bRet = true;

			do {
				CGrBookmarkDB db;
				if(!db.Open()){
					bRet = false;
					break;
				}
				auto *pSqlite3 = db.GetSqlite3();
				if(!pSqlite3){
					bRet = false;
					break;
				}
				bool bret = true;
				sqlite3_stmt *pstmt = nullptr;
				do {
					int ret = sqlite3_prepare16(pSqlite3, _T("SELECT B.ID,B.TITLE,B.POSITION,EXISTS(SELECT 1 FROM TXTMIRU_BOOKS A WHERE A.PARENT=B.ID AND A.TYPE=1)")
												_T(" FROM TXTMIRU_BOOKS B ")
												_T(" WHERE B.PARENT=@PARENT AND B.TYPE=1 ORDER BY B.POSITION"), -1, &pstmt, nullptr);
					if(ret != SQLITE_OK || !pstmt) {
						// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
						bret = false;
						break;
					}
					sqlite3_clear_bindings(pstmt);
					CGrDBFunc::PutValue(pstmt, "@PARENT", pParentTagInfo->id);
					// 実行
					bret = false;
					while(true){
						ret = sqlite3_step(pstmt);
						if(ret != SQLITE_ROW){
							break;
						}
						auto *pTagInfo = new TxtFuncBookmark::Tag();
						auto &&tag = *pTagInfo;

						int iChildExists = 0;
						CGrDBFunc::GetValue(pstmt, 0, tag.id             );
						CGrDBFunc::GetValue(pstmt, 1, tag.title          );
						CGrDBFunc::GetValue(pstmt, 2, tag.position       );
						CGrDBFunc::GetValue(pstmt, 3, iChildExists       );
						m_tag_list.push_back(pTagInfo);
						bret = true;
						LVITEM item = {0};
						item.mask      = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_STATE;
						item.iItem     = m_listView.GetItemCount();
						item.iSubItem  = 0;
						item.lParam    = reinterpret_cast<LPARAM>(pTagInfo);
						item.pszText   = const_cast<LPTSTR>(tag.title.c_str());
						item.iImage    = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_tag_favorite);
						item.stateMask = LVIS_STATEIMAGEMASK;
						item.state     = INDEXTOSTATEIMAGEMASK(2);
						auto *pItemInfo = new TreeListInfo();
						{
							pItemInfo->m_pParent    = pParentInfo;
							pItemInfo->m_nLevel     = pParentInfo->m_nLevel + 1;
							pItemInfo->m_nImage     = item.iImage;
							pItemInfo->m_bCollapsed = false;
							pItemInfo->m_lParam     = item.lParam;
							pItemInfo->m_lstCols.resize(Header_GetItemCount(m_listView.GetHeader()));
							pItemInfo->m_lstCols[0] = item.pszText;
							pParentInfo->m_children.push_back(pItemInfo);
						}
						if(pItemInfo && iChildExists > 0){
							pItemInfo->AddDummyChild(); // 子有
						}
					};
				} while(0);

				if(pstmt){
					sqlite3_finalize(pstmt);
				}
			} while(0);
		}
	}
}

bool CGrPanelTagWnd::moveBook(int num, int book_list[], int tag_id)
{
	CGrBookmarkDB db;
	if(!db.Open()){
		return false;
	}
	bool bret = false;
	do {
		sqlite3 *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			return false;
		}
		CGrDBBooks books;
		std::tstring sysdate;
		CGrDBFunc::GetSysDate(sysdate);
		int position = 0;
		books.MaxPosition(pSqlite3, tag_id, position);
		bret = true;
		for(int i=0; i<num; ++i){
			::Book book;
			if(books.Get(pSqlite3, book, book_list[i])){
				if(book.parent != tag_id){
					++position;
					book.parent      = tag_id  ;
					book.position    = position;
					book.insert_date = sysdate ;
					bret = books.Put(pSqlite3, book);
					if(!bret){
						break;
					}
				}
			}
		} /* for i */
	} while(0);

	return bret;
}
