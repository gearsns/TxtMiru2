#pragma warning( disable : 4786 )
#pragma warning( disable : 4091 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <algorithm>
#include "resource.h"
#include "PanelBook.h"
#include "BookListDlg.h"
#include "Text.h"
#include "shell.h"
#include "MessageBox.h"
#include "BookDlg.h"
#include "BookmarkDB.h"
#include "TxtMiruTheme.h"
//#define __DBG__
#include "Debug.h"

#define ID_TIMER_SHOWSUBTITLELIST 100
namespace TxtBookMessage
{
	enum {
		ADD_BOOK     ,
	};
};
class CBookListDropTarget : public CDropTarget
{
private:
	CGrPanelBookWnd &m_bookWnd;
	CGrListView &m_listview;
	CLIPFORMAT CF_TXTMIRUBOOKINFO        = 0;
	CLIPFORMAT CF_TXTMIRUAOZORABOOKSINFO = 0;
	CLIPFORMAT CF_UNIFORMRESOURCELOCATOR = 0;
	CLIPFORMAT CF_NETSCAPEBOOKMARK       = 0;
	CLIPFORMAT CF_FILEDESCRIPTOR         = 0;
	CLIPFORMAT CF_FILECONTENTS           = 0;
	CLIPFORMAT CF_SHELLURL               = 0;
private:
	LPCTSTR getNextData(LPCTSTR lpSrc, LPCTSTR lpEnd)
	{
		for(;*lpSrc; ++lpSrc){
			if(lpSrc >= lpEnd){
				return nullptr;
			}
		}
		return lpSrc + 1;
	}
public:
	CBookListDropTarget(CGrPanelBookWnd &bookWnd, CGrListView &listView)
	: CDropTarget(listView), m_bookWnd(bookWnd), m_listview(listView)
	{
	}

	void AddSuportedFormats()
	{
		FORMATETC ftetc = {0};
		ftetc.dwAspect = DVASPECT_CONTENT;
		ftetc.lindex   = -1;
		ftetc.tymed    = TYMED_HGLOBAL;
		CF_TXTMIRUBOOKINFO        = RegisterClipboardFormat(_T("TxtMiruBookInfo"       )); ftetc.cfFormat = CF_TXTMIRUBOOKINFO       ; AddSuportedFormat(ftetc);
		CF_TXTMIRUAOZORABOOKSINFO = RegisterClipboardFormat(_T("TxtMiruAozoraBooksInfo")); ftetc.cfFormat = CF_TXTMIRUAOZORABOOKSINFO; AddSuportedFormat(ftetc);
		CF_UNIFORMRESOURCELOCATOR = RegisterClipboardFormat(_T("UniformResourceLocator")); ftetc.cfFormat = CF_UNIFORMRESOURCELOCATOR; AddSuportedFormat(ftetc);
		CF_NETSCAPEBOOKMARK       = RegisterClipboardFormat(_T("Netscape Bookmark"     )); ftetc.cfFormat = CF_NETSCAPEBOOKMARK      ; AddSuportedFormat(ftetc);
		/*                                                                              */ ftetc.cfFormat = CF_HDROP                 ; AddSuportedFormat(ftetc);
		CF_SHELLURL               = RegisterClipboardFormat(CFSTR_SHELLURL              ); ftetc.cfFormat = CF_SHELLURL              ; AddSuportedFormat(ftetc);
		CF_FILEDESCRIPTOR         = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR        ); ftetc.cfFormat = CF_FILEDESCRIPTOR        ; AddSuportedFormat(ftetc);
	}

	virtual HRESULT __stdcall DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
	{
		m_listview.DragMode(true);
		return CDropTarget::DragEnter(pDataObject, grfKeyState, ptl, pdwEffect);
	}
	virtual HRESULT __stdcall DragLeave()
	{
		m_listview.DragMode(false);
		return CDropTarget::DragLeave();
	}
	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
	{
		CDropTarget::DragOver(grfKeyState, pt, pdwEffect);
		if(*pdwEffect != DROPEFFECT_NONE){
			// サポートされている形式の時のみ
			LVHITTESTINFO ht;
			ht.pt.x = pt.x;
			ht.pt.y = pt.y;
			ScreenToClient(m_hTargetWnd, &ht.pt);
			*pdwEffect = DROPEFFECT_NONE;
			int iItem = ListView_HitTest(m_hTargetWnd, &ht);
			if(iItem < 0){
				*pdwEffect = DROPEFFECT_MOVE;
				iItem = ListView_GetItemCount(m_hTargetWnd);
			} else {
				*pdwEffect = DROPEFFECT_MOVE;
				RECT rect;
				ListView_GetItemRect(m_hTargetWnd, iItem, &rect, LVIR_BOUNDS);
				if(ht.pt.y > rect.top + (rect.bottom-rect.top) / 2){
					++iItem;
				}
			}
			m_listview.SetDrawInsertPos(iItem);
		}
		return S_OK;
	}
	virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
	{
		if(medium.tymed == TYMED_HGLOBAL){
			if(pFmtEtc->cfFormat == CF_TXTMIRUBOOKINFO){
				do {
					auto *pBook = static_cast<TxtFuncBookmark::BookDragInfo*>(GlobalLock(medium.hGlobal));
					if(pBook){
						*pdwEffect = DROPEFFECT_NONE;
						m_bookWnd.moveBook(pBook->num, pBook->id_list, m_listview.GetDrawInsertPos());
					}
					GlobalUnlock(medium.hGlobal);
				} while(0);
			} else if(pFmtEtc->cfFormat == CF_TXTMIRUAOZORABOOKSINFO){
				do {
					int text_size = GlobalSize(medium.hGlobal) / sizeof(TCHAR);
					auto lpText = static_cast<LPCTSTR>(GlobalLock(medium.hGlobal));
					if(lpText){
						std::vector<TxtFuncBookmark::Book> book_list;
						*pdwEffect = DROPEFFECT_NONE;
						auto lpEnd = &lpText[text_size];
						while(true){
							auto lpUrl    = lpText; lpText = getNextData(lpText, lpEnd); if(!lpText){ break; }
							auto lpTitle  = lpText; lpText = getNextData(lpText, lpEnd); if(!lpText){ break; }
							auto lpAuthor = lpText; lpText = getNextData(lpText, lpEnd); if(!lpText){ break; }
							if(lpText >= lpEnd){
								break;
							}
							TxtFuncBookmark::Book book;
							book.url    = lpUrl;
							book.title  = lpTitle;
							book.author = lpAuthor;
							book_list.push_back(book);
						}
						if(book_list.size() > 0){
							m_bookWnd.addBook(book_list, m_listview.GetDrawInsertPos());
						}
					}
					GlobalUnlock(medium.hGlobal);
				} while(0);
			} else if(/**/pFmtEtc->cfFormat == CF_UNIFORMRESOURCELOCATOR
					  ||  pFmtEtc->cfFormat == CF_NETSCAPEBOOKMARK      ){
				// ブラウザからLINK D&D
				std::tstring url;
				if(CGrText::MultiByteToTString(CP_THREAD_ACP, static_cast<LPCSTR>(GlobalLock(medium.hGlobal)), -1, url)){
					m_bookWnd.addDelayBookDlg(url.c_str(), m_listview.GetDrawInsertPos());
				}
				GlobalUnlock(medium.hGlobal);
			} else if(pFmtEtc->cfFormat ==CF_FILECONTENTS){
				// ZipフォルダなどからDropされた仮想ファイル
				// NOP
			} else if(pFmtEtc->cfFormat == CF_FILEDESCRIPTOR
					  ||  pFmtEtc->cfFormat == CF_SHELLURL
					  ||  pFmtEtc->cfFormat == CF_HDROP         ){
				// ドロップされたものがURL
				TCHAR fileName[_MAX_PATH];
				auto len = ::DragQueryFile(static_cast<HDROP>(medium.hGlobal), 0, fileName, _countof(fileName));
				if(len == 0){
				} else {
					// InternetShortcut対応
					TCHAR buf[_MAX_PATH];
					if(GetPrivateProfileString(_T("InternetShortcut"), _T("URL"), _T(""), buf, sizeof(buf)/sizeof(TCHAR), fileName)){
						// buf
						m_bookWnd.addDelayBookDlg(buf, m_listview.GetDrawInsertPos());
					} else {
						m_bookWnd.addDelayBookDlg(fileName, m_listview.GetDrawInsertPos());
					}
				}
			} else if(pFmtEtc->cfFormat ==CF_UNICODETEXT){
				// NOP
			}
		}
		m_listview.DragMode(false);
		return true; //let base free the medium
	}
};

CGrPanelBookWnd::CGrPanelBookWnd(CGrBookListDlg &dlg) : CGrPanelWnd(dlg)
{
	m_tag.id = -1;
}

CGrPanelBookWnd::~CGrPanelBookWnd()
{
	if(m_pDropTarget){
		delete m_pDropTarget;
	}
}

bool CGrPanelBookWnd::Create(HWND hParent, HIMAGELIST hImg)
{
	if(!CGrPanelWnd::Create(hParent, hImg)){
		return false;
	}
	SetWindowFont(m_listView, GetWindowFont(hParent), TRUE);
	InsertColumn(IDS_BMH_TITLE, 0, 200, LVCFMT_LEFT);
	InsertColumn(IDS_BMH_URL, 1, 200, LVCFMT_LEFT);
	InsertColumn(IDS_BMH_AUTHOR, 2, 100, LVCFMT_LEFT);
	InsertColumn(IDS_BMH_READCNT, 3, 40, LVCFMT_RIGHT);
	InsertColumn(IDS_BMH_TOTAL, 4, 40, LVCFMT_RIGHT);
	InsertColumn(IDS_BMH_VISIT_DATE, 5, 150, LVCFMT_LEFT);
	//
	auto styleex = m_listView.GetExtendedListViewStyle();
	styleex |= LVS_EX_FLATSB | LVS_EX_FULLROWSELECT;
	m_listView.SetExtendedListViewStyle(styleex);
	//
	auto &&toolBar = m_coolBar.GetToolBar();
	TBBUTTON tbb[] = {
		//iBitmap                                                          idCommand           fsState          fsStyle                         bReserved[2]  dwData iString
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_book_open      ), IDM_BOOK_OPEN     , TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_BOOK_OPEN      },
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_book_add       ), IDM_BOOK_ADD      , TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_BOOK_ADD       },
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_book_modify    ), IDM_BOOK_MODIFY   , TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_BOOK_MODIFY    },
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_book_delete    ), IDM_BOOK_DELETE   , TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_BOOK_DELETE    },
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_book_update    ), IDM_UPDATECHECK   , TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_BOOK_UPDATE    },
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_book_update_all), IDM_UPDATECHECKALL, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_BOOK_UPDATE_ALL},
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_book_up        ), IDM_BOOK_UP       , TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_BOOK_UP        },
		{static_cast<int>(TxtFuncBookmark::ImageIcon::ii_book_down      ), IDM_BOOK_DOWN     , TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, 0, 0        , 0    , IDS_TIPS_BOOK_DOWN      },
		{I_IMAGENONE                                                     , IDS_BMH_TITLE     , TBSTATE_ENABLED, BTNS_AUTOSIZE  | BTNS_SHOWTEXT, 0, 0        , 0    , IDS_ERROR_FOLDER        },
	};
	for(auto &item : tbb){
		if(item.iString != 0){
			std::tstring tips;
			CGrText::LoadString(item.iString, tips);
			item.iString = SendMessage(toolBar, TB_ADDSTRING, 0, reinterpret_cast<LPARAM>(tips.c_str()));
		} else {
			item.iString = sizeof(tbb)/sizeof(TBBUTTON) + 1;
		}
	}
	TBADDBITMAP tb = {0};
	toolBar.AddButtons(tbb, sizeof(tbb)/sizeof(TBBUTTON), tb);
	toolBar.SetButtons(_T("1,2,3,4,5,6,7,8,9"));

	//
	m_pDropTarget = new CBookListDropTarget(*this, m_listView);
	if(FAILED(RegisterDragDrop(m_listView, m_pDropTarget))){
		delete m_pDropTarget;
		m_pDropTarget = nullptr;
	} else {
		static_cast<CBookListDropTarget*>(m_pDropTarget)->AddSuportedFormats();
	}
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);
	//
	return true;
}

// オーバーライド可能なウィンドウ(ダイアログ)プロシージャ
LRESULT CGrPanelBookWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_COMMAND      , OnCommand      );
	case WM_TIMER:
		{
			if(ID_TIMER_SHOWSUBTITLELIST == wParam){
				KillTimer(hWnd, ID_TIMER_SHOWSUBTITLELIST);
				LVITEM item = { LVIF_PARAM };
				item.iItem = GetFocusedItem();
				m_listView.GetItem(&item);
				int index = item.lParam;
				if(index >= 0 && index < static_cast<signed int>(m_book_list.size())){
					m_booklistDlg.ShowSubtitleList(m_book_list[index]);
				}
			}
		}
		break;
	case WM_SIZE:
		{
			auto state = static_cast<UINT>(wParam);
			auto cx = static_cast<int>(static_cast<short>(LOWORD(lParam)));
			auto cy = static_cast<int>(static_cast<short>(HIWORD(lParam)));
		}
		break;
	case WM_DESTROY:
		RevokeDragDrop(m_hWnd);
		break;
	}
	return CGrPanelWnd::WndProc(hWnd, uMsg, wParam, lParam);
}
struct BookCompareParam {
	int column;
	std::vector<TxtFuncBookmark::Book> *p_book_list;
};
static int CALLBACK bookCompare(LPARAM lp1, LPARAM lp2, LPARAM param)
{
	auto *pParam = reinterpret_cast<BookCompareParam*>(param);
	const auto &book1 = (*pParam->p_book_list)[lp1];
	const auto &book2 = (*pParam->p_book_list)[lp2];
	int ret = 0;
	switch(pParam->column){
	case 0: // IDS_BMH_TITLE
		ret = _tcscmp(book1.title.c_str(), book2.title.c_str());
		if(ret == 0){
			ret = book1.position - book2.position;
		}
		break;
	case 1: // IDS_BMH_URL
		ret = _tcscmp(book1.url.c_str(), book2.url.c_str());
		if(ret == 0){
			ret = book1.position - book2.position;
		}
		break;
	case 2: // IDS_BMH_AUTHOR
		ret = _tcscmp(book1.author.c_str(), book2.author.c_str());
		if(ret == 0){
			ret = book1.position - book2.position;
		}
		break;
	case 3: // IDS_BMH_READCNT
		ret = (book1.total_cnt == 0 ? ((book1.read_page % 2) + 1) : book1.read_cnt)
			- (book2.total_cnt == 0 ? ((book2.read_page % 2) + 1) : book2.read_cnt);
		if(ret == 0){
			ret = book1.position - book2.position;
		}
		break;
	case 4: // IDS_BMH_TOTAL
		ret = (book1.total_cnt == 0 ? book1.page : book1.total_cnt)
			- (book2.total_cnt == 0 ? book2.page : book2.total_cnt);
		if(ret == 0){
			ret = book1.position - book2.position;
		}
		break;
	case 5: // IDS_BMH_VISIT_DATE
		ret = _tcscmp(book1.last_visit_date.c_str(), book2.last_visit_date.c_str());
		if(ret == 0){
			ret = book1.position - book2.position;
		}
		break;
	}
	return ret;
}

LRESULT CGrPanelBookWnd::OnNotify(HWND hWnd, int idFrom, NMHDR FAR *lpnmhdr)
{
	if(idFrom == listwnd_id){
		switch(lpnmhdr->code){
		case NM_DBLCLK:
			{
				auto lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(lpnmhdr);
				openFile(lpnmitem->iItem);
			}
			break;
		case NM_RCLICK:
			if(!m_booklistDlg.IsUpdating()){
				POINT cursor;
				GetCursorPos(&cursor);
				auto lpnmitemactivate = reinterpret_cast<LPNMITEMACTIVATE>(lpnmhdr);
				auto hMenu = ::LoadMenu(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDR_MENU_POPUP_BOOK));
				CGrPanelWnd_SetTouchMenu(hMenu);
				::TrackPopupMenu(::GetSubMenu(hMenu, 0), TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL, cursor.x, cursor.y, 0, hWnd, nullptr);
				::DestroyMenu(hMenu);
			}
			break;
		case LVN_ITEMCHANGED:
			{
				auto pnmv = reinterpret_cast<LPNMLISTVIEW>(lpnmhdr);
				if(pnmv->uNewState & LVIS_FOCUSED){
					KillTimer(hWnd, ID_TIMER_SHOWSUBTITLELIST);
					::SetTimer(m_hWnd, ID_TIMER_SHOWSUBTITLELIST, 100, nullptr);
				}
			}
			break;
		case LVN_COLUMNCLICK:
			if(!m_booklistDlg.IsUpdating()){
				auto lpnmlist = reinterpret_cast<LPNMLISTVIEW>(lpnmhdr);
				BookCompareParam param;
				param.p_book_list = &m_book_list;
				param.column      = lpnmlist->iSubItem;
				m_listView.SortItems(bookCompare, reinterpret_cast<LPARAM>(&param));
				saveOrder();
			}
			break;
		case LVN_BEGINDRAG:
			if (!m_booklistDlg.IsUpdating()) {
				auto pnmv = reinterpret_cast<LPNMLISTVIEW>(lpnmhdr);
				std::vector<int> id_list;
				int cnt = m_listView.GetSelectedCount();
				UINT flags = (cnt <= 0) ? (LVNI_ALL | LVNI_FOCUSED) : (LVNI_ALL | LVNI_SELECTED);
				int iItem = -1;
				std::tstring url;
				while ((iItem = m_listView.GetNextItem(iItem, flags)) != -1) {
					LVITEM item = { LVIF_PARAM };
					item.iItem = iItem;
					if (FALSE == m_listView.GetItem(&item)) {
						break;
					}
					int index = static_cast<int>(item.lParam);
					if (index < 0 || index >= static_cast<signed int>(m_book_list.size())) {
						break;
					}
					id_list.push_back(m_book_list[index].id);
					if (url.empty()) {
						url = m_book_list[index].url;
					}
				}
				if (id_list.size() <= 0) {
					break;
				}
				auto* pDropSource = new CDropSource;
				///////////////
				auto* pDataObject = new CDataObject(pDropSource);
				pDataObject->SetUrl(url.c_str());
				FORMATETC fmtetc = {};
				fmtetc.cfFormat = RegisterClipboardFormat(_T("TxtMiruBookInfo"));
				fmtetc.dwAspect = DVASPECT_CONTENT;
				fmtetc.lindex = -1;
				fmtetc.tymed = TYMED_HGLOBAL;
				STGMEDIUM medium = {};
				medium.tymed = TYMED_HGLOBAL;
				medium.hGlobal = GlobalAlloc(GHND | GMEM_SHARE, sizeof(TxtFuncBookmark::BookDragInfo) + sizeof(int) * id_list.size());
				if (medium.hGlobal) {
					auto* pBook = static_cast<TxtFuncBookmark::BookDragInfo*>(GlobalLock(medium.hGlobal));
					if (pBook) {
						pBook->num = id_list.size();
						pBook->tag_id = m_tag.id;
						int* p = pBook->id_list;
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

				DWORD dwEffect;
				auto hr = ::DoDragDrop(pDataObject, pDropSource, DROPEFFECT_MOVE, &dwEffect);
				if(SUCCEEDED(hr)){
					if(dwEffect == DROPEFFECT_MOVE){
						DispList(true);
					}
				}
				pDropSource->Release();
				pDataObject->Release();
			}
			break;
		}
	}
	return CGrPanelWnd::OnNotify(hWnd, idFrom, lpnmhdr);
}

bool CGrPanelBookWnd::openFile(int iItem)
{
	if(CGrTxtFunc::IsWaitting()){
		return false;
	}
	LVITEM item = { LVIF_PARAM };
	item.iItem = iItem;
	m_listView.GetItem(&item);
	int index = (int)item.lParam;
	if(index < 0 || index >= static_cast<signed int>(m_book_list.size())){
		return false;
	}
	const auto &book = m_book_list[index];
	m_booklistDlg.OpenFile(book.url.c_str());
	return true;
}

bool CGrPanelBookWnd::addDelayBookDlg(LPCTSTR lpURL, int itemPos)
{
	CGrShell::ToPrettyFileName(lpURL, m_dropFileName);
	m_dropItemPos = itemPos;
	FORWARD_WM_COMMAND(m_hWnd, TxtBookMessage::ADD_BOOK/*id*/, NULL/*hwndCtl*/, 0/*codeNotify*/, PostMessage);
	return true;
}

bool CGrPanelBookWnd::addBookDlg(LPCTSTR lpURL, int itemPos)
{
	if(m_tag.id != -1){
		TxtFuncBookmark::Book book;
		book.id          = -1;
		book.place_id    = -1;
		book.page        = -1;
		book.read_page   = -1;
		book.visit_count = -1;
		book.read_cnt    = -1;
		book.total_cnt   = -1;
		book.position    = -1;
		getBookByUrl(book, lpURL);
		if(book.url.empty()){
			book.url = lpURL;
		}
		int iItem = itemPos;
		int pre_id = -1;
		if(iItem >= 0){
			int index = m_listView.GetParam(iItem) + 1;
			if(index < 0 || index >= static_cast<signed int>(m_book_list.size())){

			} else {
				pre_id = m_book_list[index].id;
			}
		}
		//
		CGrBookDlg dlg(book, m_tag.id, pre_id);
		if(IDOK == dlg.DoModal(m_booklistDlg)){
			DispList(true);
			auto it = std::find_if(m_book_list.begin(), m_book_list.end(), [&book](TxtFuncBookmark::Book &x) -> bool { return x.id == book.id; });
			int iLastItem = m_listView.GetItemCount() - 1;
			if(m_book_list.end() != it){
				iLastItem = std::distance(m_book_list.begin(), it);
			}
			m_listView.SetItemState(-1, 0, LVIS_SELECTED);
			m_listView.SetItemState(iLastItem, LVNI_FOCUSED|LVNI_SELECTED, LVNI_FOCUSED|LVNI_SELECTED);
			m_listView.EnsureVisible(iLastItem, FALSE);
		}
	} else {
		CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, IDS_ERROR_FOLDER, CGrTxtFunc::AppName());
	}
	return true;
}

void CGrPanelBookWnd::SetWorking(bool bWork)
{
	int id_list[] = {
		IDM_BOOK_OPEN     ,
		IDM_BOOK_ADD      ,
		IDM_BOOK_MODIFY   ,
		IDM_BOOK_DELETE   ,
		IDM_UPDATECHECKALL,
		IDM_BOOK_UP       ,
		IDM_BOOK_DOWN     ,
	};
	BOOL bEnabled = bWork ? FALSE : TRUE;
	auto &&toolBar = m_coolBar.GetToolBar();
	for(int i=0; i<sizeof(id_list)/sizeof(int); ++i){
		SendMessage(toolBar, TB_ENABLEBUTTON, id_list[i], bEnabled);
	}
	std::tstring tips;
	TBBUTTONINFO ti = {sizeof(TBBUTTONINFO)};
	ti.dwMask = TBIF_IMAGE | TBIF_TEXT;
	if(bWork){
		ti.iImage = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_book_abort);
		CGrText::LoadString(IDS_TIPS_BOOK_UPDATE, tips);
	} else {
		ti.iImage = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_book_update);
		CGrText::LoadString(IDS_TIPS_BOOK_UPDATE, tips);
	}
	ti.pszText = const_cast<LPTSTR>(tips.c_str());
	ti.cchText = tips.size();
	SendMessage(toolBar, TB_SETBUTTONINFO, IDM_UPDATECHECK, (LPARAM)&ti);
}

void CGrPanelBookWnd::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case TxtBookMessage::ADD_BOOK:
		if(!m_booklistDlg.IsUpdating()){
			addBookDlg(m_dropFileName.c_str(), m_dropItemPos);
		}
		break;
	case IDM_BOOK_OPEN: openFile(GetFocusedItem()); break;
	case IDM_UPDATECHECKALL:
		if(m_booklistDlg.IsUpdating()){
			m_booklistDlg.AbortUpdate();
		} else {
			m_booklistDlg.SetWorking(true);
			m_booklistDlg.ClearUpdateList();
			for(int iItem=0,i=m_book_list.size(); i>0; --i, ++iItem){
				m_booklistDlg.AddUpdateList(iItem);
			}
			m_booklistDlg.UpdateCheck();
		};
		break;
	case IDM_UPDATECHECK:
		if(m_booklistDlg.IsUpdating()){
			m_booklistDlg.AbortUpdate();
		} else {
			m_booklistDlg.SetWorking(true);
			m_booklistDlg.ClearUpdateList();
			int cnt = m_listView.GetSelectedCount();
			if(cnt > 0){
				int iItem = -1;
				while((iItem=m_listView.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED))!=-1){
					m_booklistDlg.AddUpdateList(iItem);
				}
			} else {
				for(int iItem=0,n=m_book_list.size(); n>0; --n, ++iItem){
					m_booklistDlg.AddUpdateList(iItem);
				}
			}
			m_booklistDlg.UpdateCheck();
		}
		break;
	case IDM_BOOK_ADD:
		if(!m_booklistDlg.IsUpdating()){
			addBookDlg(m_booklistDlg.GetFilename().c_str(), GetFocusedItem());
		}
		break;
	case IDM_BOOK_MODIFY:
		if(!m_booklistDlg.IsUpdating()){
			LVITEM item = { LVIF_PARAM };
			item.iItem = GetFocusedItem();
			if(item.iItem < 0){
				int cnt = m_listView.GetSelectedCount();
				if(cnt == 1){
					int iItem = -1;
					while((iItem=m_listView.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED))!=-1){
						item.iItem = iItem;
						break;
					}
				} else {
					break;
				}
			}
			if(!m_listView.GetItem(&item)){
				break;
			}
			int index = item.lParam;
			if(index < 0 || index >= static_cast<signed int>(m_book_list.size())){
				break;
			}
			auto book = m_book_list[index];
			CGrBookDlg dlg(book, m_tag.id);
			if(IDOK == dlg.DoModal(m_hWnd)){
				DispList(true);
				m_booklistDlg.RefreshSubtitleList();
			}
		}
		break;
	case IDM_BOOK_DELETE:
		if(!m_booklistDlg.IsUpdating()){
			int cnt = m_listView.GetSelectedCount();
			int refresh_index = -1;
			if(cnt > 0){
				std::tstring message;
				if(cnt == 1){
					LVITEM item = { LVIF_PARAM };
					item.iItem = m_listView.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
					if(m_listView.GetItem(&item)){
						int index = item.lParam;
						if(index >= 0 && index < static_cast<signed int>(m_book_list.size())){
							CGrText::FormatMessage(CGrTxtFunc::GetDllModuleHandle(), message, IDS_DELETE_BOOK, m_book_list[index].title.c_str());
						}
					}
				} else {
					CGrText::FormatMessage(CGrTxtFunc::GetDllModuleHandle(), message, IDS_DELETE_BOOK_MULTI, cnt);
				}
				if(IDYES != CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, message.c_str(), CGrTxtFunc::AppName(), MB_YESNO)){
					break;
				}
				int iItem = -1;
				while((iItem=m_listView.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED))!=-1){
					if(!deleteItem(iItem)){
						break;
					}
				}
				CGrTxtFunc::RemoveBackupImgFolder(m_hWnd);
				DispList(true);
			}
		}
		break;
	case IDM_BOOK_UP  :
		if(!m_booklistDlg.IsUpdating()){
			moveSelectionItem(true );
		}
		break;
	case IDM_BOOK_DOWN:
		if(!m_booklistDlg.IsUpdating()){
			moveSelectionItem(false);
		}
		break;
	case IDM_PROPERTY:
		{
			int iItem = GetFocusedItem();
			if(iItem < 0){
				break;
			}
			int index = m_listView.GetParam(iItem);
			if(index < 0 || index >= static_cast<signed int>(m_book_list.size())){
				break;
			}
			auto &&url = m_book_list[index].url;
			if(!url.empty()){
				auto lpFileName = url.c_str();
				if(CGrShell::IsURI(lpFileName) || CGrText::Find(lpFileName, _T("|"))/* アーカイブファイル */){
					;
				} else {
					SHELLEXECUTEINFO si = { sizeof(SHELLEXECUTEINFO) };
					si.fMask   = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_NO_UI;
					si.hwnd    = m_hWnd;
					si.lpVerb  = _T("properties";);
					si.lpFile  = lpFileName;
					::ShellExecuteEx(&si);
				}
			}
		}
		break;
	case IDM_FOLDER_OPEN:
		{
			int iItem = GetFocusedItem();
			if(iItem < 0){
				break;
			}
			int index = m_listView.GetParam(iItem);
			if(index < 0 || index >= static_cast<signed int>(m_book_list.size())){
				break;
			}
			auto &&url = m_book_list[index].url;
			if(!url.empty()){
				int size = url.size()+1;
				auto *dir = new TCHAR[size];
				if(dir){
					CGrShell::GetParentDir(const_cast<TCHAR*>(url.c_str()), dir);
					ShellExecute(HWND_DESKTOP, _T("open"), dir, NULL, NULL, SW_SHOWNORMAL);
				}
				delete [] dir;
			}
		}
		break;
	}
}

bool CGrPanelBookWnd::Set(const TxtFuncBookmark::Tag &tag)
{
	if(m_tag.id == tag.id){
		return true;
	}
	m_tag = tag;
	//
	auto &&toolBar = m_coolBar.GetToolBar();
	TBBUTTONINFO ti = {sizeof(TBBUTTONINFO)};
	ti.dwMask = TBIF_TEXT;
	ti.pszText = const_cast<LPTSTR>(m_tag.title.c_str());
	ti.cchText = m_tag.title.size();
	SendMessage(toolBar, TB_SETBUTTONINFO, IDS_BMH_TITLE, reinterpret_cast<LPARAM>(&ti));
	//
	return DispList(false, m_tag.id);
}

int CGrPanelBookWnd::GetSelectID() const
{
	return m_tag.id;
}

bool CGrPanelBookWnd::Select(int id)
{
	m_listView.SetRedraw(FALSE);
	m_listView.SetItemState(-1, 0, LVNI_FOCUSED|LVIS_SELECTED);
	int index = -1;
	for(int i=m_book_list.size()-1; i>=0; --i){
		LVITEM item = { LVIF_PARAM };
		item.iItem = i;
		if(FALSE == m_listView.GetItem(&item)){
			break;
		}
		const auto &book = m_book_list[static_cast<int>(item.lParam)];
		if(id == book.id){
			index = i;
			m_listView.SetItemState(i, LVNI_FOCUSED|LVNI_SELECTED, LVNI_FOCUSED|LVNI_SELECTED);
			break;
		}
	}
	if(index >= 0){
		m_listView.EnsureVisible(index, FALSE);
	}
	m_listView.SetRedraw(TRUE);
	return true;
}

bool CGrPanelBookWnd::DispList(bool bKeepIndex, int parent)
{
	if(parent < 0){
		parent = m_tag.id;
	}
	m_listView.SetRedraw(FALSE);
	bool bRet = dispListSub(_T("SELECT B.ID,B.PLACE_ID,P.URL,B.TITLE,B.AUTHOR,P.PAGE,P.READ_PAGE,P.VISIT_COUNT,B.POSITION,B.INSERT_DATE,P.LAST_VISIT_DATE")
							_T(" FROM TXTMIRU_BOOKS B ")
							_T(" INNER JOIN TXTMIRU_PLACES P ON P.ID=B.PLACE_ID WHERE B.PARENT=@PARENT AND B.TYPE=0 ORDER BY B.POSITION"),
							bKeepIndex, -1, -1, parent);
	m_listView.SetRedraw(TRUE);
	return bRet;
}

bool CGrPanelBookWnd::GetBook(int iItem, TxtFuncBookmark::Book &book)
{
	bool bRet = false;
	do {
		LVITEM item = { LVIF_PARAM | LVIF_IMAGE };
		item.iItem = iItem;
		if(FALSE == m_listView.GetItem(&item)){
			break;
		}
		int index = static_cast<int>(item.lParam);
		if(index < 0 || index >= static_cast<signed int>(m_book_list.size())){
			break;
		}
		book = m_book_list[index];
		bRet = true;
	} while(0);

	return bRet;
}

bool CGrPanelBookWnd::SetUpdating(int iItem)
{
	do {
		if(iItem < 0){
			break;
		}
		LVITEM item = { LVIF_IMAGE };
		item.iItem = iItem;
		item.iImage = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_check);
		if(FALSE == m_listView.SetItem(&item)){
			break;
		}
	} while(0);

	return true;
}

bool CGrPanelBookWnd::RefreshItem(int iItem)
{
	bool bRet = true;
	int book_id = -1;
	do {
		LVITEM item = { LVIF_PARAM | LVIF_IMAGE };
		item.iItem = iItem;
		if(FALSE == m_listView.GetItem(&item)){
			break;
		}
		int index = static_cast<int>(item.lParam);
		if(index < 0 || index >= static_cast<signed int>(m_book_list.size())){
			break;
		}
		InvalidateRect(m_listView, NULL, FALSE);
		UpdateWindow(m_listView);
		auto &&book = m_book_list[index];
		book_id = book.id;
		bRet = dispListSub(_T("SELECT B.ID,B.PLACE_ID,P.URL,B.TITLE,B.AUTHOR,P.PAGE,P.READ_PAGE,P.VISIT_COUNT,B.POSITION,B.INSERT_DATE,P.LAST_VISIT_DATE")
						   _T(" FROM TXTMIRU_BOOKS B ")
						   _T(" INNER JOIN TXTMIRU_PLACES P ON P.ID=B.PLACE_ID WHERE B.ID=@ID ORDER BY B.POSITION"),
						   false, iItem, index, -1);
	} while(0);
	if(m_booklistDlg.GetCurBookIDSubtitleList() == book_id || iItem == GetFocusedItem()){
		m_booklistDlg.RefreshSubtitleList();
	}

	return bRet;
}

bool CGrPanelBookWnd::refreshItem(int iItem)
{
	int iImage = -1;
	bool bRet = true;
	int book_id = -1;
	do {
		LVITEM item = { LVIF_PARAM | LVIF_IMAGE };
		item.iItem = iItem;
		if(FALSE == m_listView.GetItem(&item)){
			break;
		}
		int index = static_cast<int>(item.lParam);
		if(index < 0 || index >= static_cast<signed int>(m_book_list.size())){
			break;
		}
		iImage = item.iImage;
		item.iImage = static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_check);
		item.mask   = LVIF_IMAGE;
		if(FALSE == m_listView.SetItem(&item)){
			break;
		}
		InvalidateRect(m_listView, NULL, FALSE);
		UpdateWindow(m_listView);
		auto &&book = m_book_list[index];
		if(!m_booklistDlg.UpdateCheck(book.url.c_str())){
			bRet = false;
			break;
		}
		book_id = book.id;
		bRet = dispListSub(_T("SELECT B.ID,B.PLACE_ID,P.URL,B.TITLE,B.AUTHOR,P.PAGE,P.READ_PAGE,P.VISIT_COUNT,B.POSITION,B.INSERT_DATE,P.LAST_VISIT_DATE")
						   _T(" FROM TXTMIRU_BOOKS B ")
						   _T(" INNER JOIN TXTMIRU_PLACES P ON P.ID=B.PLACE_ID WHERE B.ID=@ID ORDER BY B.POSITION"),
						   false, iItem, index, -1);
	} while(0);
	if(!bRet && iImage >= 0){
		LVITEM item = { LVIF_IMAGE };
		item.iItem  = iItem;
		item.iImage = iImage;
		m_listView.SetItem(&item);
	}
	if(m_booklistDlg.GetCurBookIDSubtitleList() == book_id || iItem == GetFocusedItem()){
		m_booklistDlg.RefreshSubtitleList();
	}

	return bRet;
}

bool CGrPanelBookWnd::dispListSub(LPCTSTR lpSql, bool bKeepIndex, int in_iItem, int in_index, int in_parent)
{
	bool bRet = false;

	SCROLLINFO si = {sizeof(SCROLLINFO)};
	sqlite3_stmt *pstmt = nullptr;
	sqlite3_stmt *pstmt_txtmiru_st_link = nullptr;
	sqlite3_stmt *pstmt_txtmiru_st_bm   = nullptr;
	int iImage = -1;
	int focus_item = -1;
	int index = in_index;

	if(in_iItem < 0){
		if(bKeepIndex){
			int iTopIndex = m_listView.GetTopIndex();
			int iCountPage = m_listView.GetCountPerPage();
			index = iTopIndex + iCountPage - 1;
			focus_item = m_listView.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED);
		}
		si.fMask = SIF_POS;
		::GetScrollInfo(m_listView, SB_HORZ, &si);
		m_book_list.clear();
		m_listView.DeleteAllItems();
	}
	CGrBookmarkDB db; // CGrBookmarkDBのデストラクタで sqlite3_finalizeより先にCloseされるのでここに移動
	do {
		if(!db.Open()){
			bRet = false;
			break;
		}
		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			bRet = false;
			break;
		}
		int ret = sqlite3_prepare16(pSqlite3, lpSql, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bRet = false;
			break;
		}
		ret = sqlite3_prepare16(pSqlite3, _T("SELECT S.B_ORDER,S.URL,S.PAGE,P.PAGE,P.READ_PAGE,P.LAST_VISIT_DATE")
								_T(" FROM TXTMIRU_SUBTITLES S LEFT JOIN TXTMIRU_PLACES P ON P.URL=S.URL ")
								_T(" WHERE S.PLACE_ID=@PLACE_ID AND S.URL!='' ORDER BY S.B_ORDER"), -1, &pstmt_txtmiru_st_link, nullptr);
		if(ret != SQLITE_OK || !pstmt_txtmiru_st_link) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bRet = false;
			break;
		}
		ret = sqlite3_prepare16(pSqlite3, _T("SELECT S.B_ORDER,S.URL,S.PAGE,-1,-1,''")
								_T(" FROM TXTMIRU_SUBTITLES S ")
								_T(" WHERE S.PLACE_ID=@PLACE_ID AND S.URL='' ORDER BY S.B_ORDER"), -1, &pstmt_txtmiru_st_bm, nullptr);
		if(ret != SQLITE_OK || !pstmt_txtmiru_st_bm) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bRet = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		if(in_index >= 0){
			CGrDBFunc::PutValue(pstmt, "@ID", m_book_list[in_index].id);
		}
		if(in_parent >= 0){
			CGrDBFunc::PutValue(pstmt, "@PARENT", in_parent);
		}
		// 実行
		bRet = false;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}

			TxtFuncBookmark::Book book;
			int read_cnt_b  = 0;
			int total_cnt_b = 0;
			CGrDBFunc::GetValue(pstmt, 0, book.id             );
			CGrDBFunc::GetValue(pstmt, 1, book.place_id       );
			CGrDBFunc::GetValue(pstmt, 2, book.url            );
			CGrDBFunc::GetValue(pstmt, 3, book.title          );
			CGrDBFunc::GetValue(pstmt, 4, book.author         );
			CGrDBFunc::GetValue(pstmt, 5, book.page           );
			CGrDBFunc::GetValue(pstmt, 6, book.read_page      );
			CGrDBFunc::GetValue(pstmt, 7, book.visit_count    );
			CGrDBFunc::GetValue(pstmt, 8, book.position       );
			CGrDBFunc::GetValue(pstmt, 9, book.insert_date    );
			CGrDBFunc::GetValue(pstmt,10, book.last_visit_date);
			bool bCurrent = (book.url == m_booklistDlg.GetFilename());
			int i_current_index = -1;
			auto *pstmt_txtmiru_subtitles = pstmt_txtmiru_st_link;
			do {
				book.read_cnt  = 0;
				book.total_cnt = 0;
				sqlite3_clear_bindings(pstmt_txtmiru_subtitles);
				CGrDBFunc::PutValue(pstmt_txtmiru_subtitles, "@PLACE_ID", book.place_id);
				bRet = false;
				int b_subtitle_idx = 0;
				int book_read_page = book.read_page;
				book_read_page &= (-2);
				++book_read_page;
				while(true){
					ret = sqlite3_step(pstmt_txtmiru_subtitles);
					if(ret != SQLITE_ROW){
						break;
					}
					++b_subtitle_idx;
					int b_order;
					std::tstring url;
					int page      = -1;
					int page_p    = -1;
					int read_page = -1;
					std::tstring last_visit_date;
					CGrDBFunc::GetValue(pstmt_txtmiru_subtitles, 0, b_order        );
					CGrDBFunc::GetValue(pstmt_txtmiru_subtitles, 1, url            );
					CGrDBFunc::GetValue(pstmt_txtmiru_subtitles, 2, page           );
					CGrDBFunc::GetValue(pstmt_txtmiru_subtitles, 3, page_p         );
					CGrDBFunc::GetValue(pstmt_txtmiru_subtitles, 4, read_page      );
					CGrDBFunc::GetValue(pstmt_txtmiru_subtitles, 5, last_visit_date);
					read_page &= (-2);
					++read_page;
					if(url.empty()){
						++total_cnt_b;
					} else {
						++book.total_cnt;
						if(!bCurrent){
							bCurrent = (url == m_booklistDlg.GetFilename());
						}
						if(!last_visit_date.empty()){
							i_current_index = book.total_cnt;
						}
					}
					if(page <= book_read_page){
						++read_cnt_b;
					}
					if(page_p > 0 && read_page >= (page_p/4) && book.read_cnt < b_subtitle_idx && !last_visit_date.empty()){
						book.read_cnt = b_subtitle_idx;
					}
					bRet = true;
				};
				sqlite3_reset(pstmt_txtmiru_subtitles);
				if(b_subtitle_idx > 0){
					break;
				}
				if(pstmt_txtmiru_subtitles == pstmt_txtmiru_st_bm){
					break;
				}
				pstmt_txtmiru_subtitles = pstmt_txtmiru_st_bm;
			} while(true);

			bRet = true;
			LVITEM item = {0};
			if(in_iItem < 0){
				item.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
				item.iItem    = m_listView.GetItemCount();
				item.lParam   = m_book_list.size();
			} else {
				item.mask     = LVIF_TEXT | LVIF_IMAGE;
				item.iItem    = in_iItem;
			}
			item.iSubItem = 0;
			item.pszText  = const_cast<LPTSTR>(book.title.c_str());
			if(book.total_cnt == 0){
				book.read_cnt  = read_cnt_b ;
				book.total_cnt = total_cnt_b;
			} else {
				book.read_cnt = i_current_index;
			}
			if(book.total_cnt == 0){
				book.read_page &= (-2);
				if(book.read_page + 1 < book.page){
					item.iImage = bCurrent ? static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_current_update) : static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_normal_update);
				} else {
					item.iImage = bCurrent ? static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_current       ) : static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_normal       );
				}
			} else {
				if(book.read_cnt < book.total_cnt){
					item.iImage = bCurrent ? static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_current_update) : static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_normal_update);
				} else {
					item.iImage = bCurrent ? static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_current       ) : static_cast<int>(TxtFuncBookmark::ImageIcon::ii_stat_book_normal       );
				}
			}
			if(in_iItem < 0){
				if(bCurrent){
					if(!bKeepIndex){
						index = item.lParam;
					}
				}
				item.iItem    = m_listView.InsertItem(&item);
			} else {
				m_listView.SetItem(&item);
			}

			item.mask     = LVIF_TEXT;

			item.pszText  = const_cast<LPTSTR>(book.url.c_str());
			item.iSubItem = 1;
			m_listView.SetItem( &item);

			item.pszText  = const_cast<LPTSTR>(book.author.c_str());
			item.iSubItem = 2;
			m_listView.SetItem(&item);

			TCHAR buf[512] = {};
			if(book.total_cnt == 0){
				book.read_page &= (-2);
				_stprintf_s(buf, _T("%d"), book.read_page + 1);
			} else {
				_stprintf_s(buf, _T("%d"), book.read_cnt);
			}
			item.pszText  = buf;
			item.iSubItem = 3;
			m_listView.SetItem(&item);

			if(book.total_cnt == 0){
				_stprintf_s(buf, _T("%d"), book.page);
			} else {
				_stprintf_s(buf, _T("%d"), book.total_cnt);
			}

			item.pszText  = buf;
			item.iSubItem = 4;
			m_listView.SetItem(&item);

			item.pszText  = const_cast<LPTSTR>(book.last_visit_date.c_str());
			item.iSubItem = 5;
			m_listView.SetItem(&item);

			if(in_iItem < 0){
				m_book_list.push_back(book);
			} else {
				m_book_list[in_index] = book;
			}
			bRet = true;
		};
	} while(0);
	if(in_iItem < 0){
		if(index >= 0){
			int iLastItem = m_listView.GetItemCount() - 1;
			if(iLastItem < index){
				index = iLastItem;
			}
			bool bSetFocus = false;
			int iBookID = m_booklistDlg.GetCurBookIDSubtitleList();
			if(iBookID >= 0){
				LVITEM item = { LVIF_PARAM };
				item.iItem = focus_item;
				m_listView.GetItem(&item);
				int index = item.lParam;
				if(index >= 0 && index < static_cast<signed int>(m_book_list.size())){
					if(iBookID == m_book_list[index].id){
						bSetFocus = true;
					}
				}
			}
			if(bSetFocus){
				if(focus_item >= 0){
					m_listView.SetItemState(focus_item, LVNI_FOCUSED|LVNI_SELECTED, LVNI_FOCUSED|LVNI_SELECTED);
				} else {
					m_listView.SetItemState(index, LVNI_FOCUSED|LVNI_SELECTED, LVNI_FOCUSED|LVNI_SELECTED);
				}
			}
			m_listView.EnsureVisible(index, FALSE);
		}
		::SetScrollInfo(m_listView, SB_HORZ, &si, TRUE);
	}

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	if(pstmt_txtmiru_st_link){
		sqlite3_finalize(pstmt_txtmiru_st_link);
	}
	if(pstmt_txtmiru_st_bm){
		sqlite3_finalize(pstmt_txtmiru_st_bm);
	}

	return bRet;
}

bool CGrPanelBookWnd::getBookByUrl(TxtFuncBookmark::Book &book, LPCTSTR lpURL)
{
	return CGrTxtFunc::GetBookByUrl(book, lpURL);
}
//
bool CGrPanelBookWnd::deleteItem(int iItem)
{
	LVITEM item = { LVIF_PARAM };
	item.iItem = iItem;
	if(FALSE == m_listView.GetItem(&item)){
		return false;
	}
	int index = (int)item.lParam;
	if(index < 0 || index >= (signed int)m_book_list.size()){
		return false;
	}
	auto &&book = m_book_list[index];
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
		bret = books.Delete(pSqlite3, book.id);
		if(!bret){
			break;
		}
	} while(0);

	CGrTxtFunc::BackupImgFile(book.id);
	return bret;
}

bool CGrPanelBookWnd::moveSelectionItem(bool bUp)
{
	bool bRet = false;
	do {
		int cnt = m_listView.GetSelectedCount();
		if(cnt <= 0){
			break;
		}
		int iItem = -1;
		bRet = true;
		struct Position {
			int  id          ;
			int  position    ;
			int  position_org;
			UINT state       ;
			static bool LessID(const Position &a1, const Position &a2) { return a1.id < a2.id; }
			static bool LessPosition(const Position &a1, const Position &a2) { return a1.position < a2.position; }
			static bool GreaterPosition(const Position &a1, const Position &a2) { return a1.position > a2.position; }
		};
		std::vector<Position> position_list;
		for(const auto &book : m_book_list){
			position_list.push_back(Position{book.id,book.position,book.position,0});
		}
		std::sort(position_list.begin(),position_list.end(),Position::LessID);
		while((iItem=m_listView.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED))!=-1){
			LVITEM item = { LVIF_PARAM | LVIF_STATE };
			item.iItem     = iItem;
			item.stateMask = LVNI_FOCUSED|LVNI_SELECTED;
			if(FALSE == m_listView.GetItem(&item)){
				bRet = false;
				break;
			}
			auto &&book = m_book_list[(int)item.lParam];
			auto position_it = std::lower_bound(position_list.begin(),position_list.end(),Position{book.id,0,0},Position::LessID);
			if(position_it != position_list.end() && position_it->id == book.id){
				position_it->state = item.state;
			}
		}
		if(!bRet){
			break;
		}
		if(bUp){
			std::sort(position_list.begin(),position_list.end(),Position::LessPosition);
		} else {
			std::sort(position_list.begin(),position_list.end(),Position::GreaterPosition);
		}
		Position *p_pre_item = nullptr;
		for(auto &&item : position_list){
			if(item.state){
				if(!p_pre_item){
					bRet = false;
					break;
				}
				int position = item.position;
				item.position = p_pre_item->position;
				p_pre_item->position = position;
			} else {
				p_pre_item = &item;
			}
		}
		if(!bRet){
			break;
		}
		CGrBookmarkDB db;
		if(!db.Open()){
			break;
		}
		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			break;
		}
		if(SQLITE_OK != db.BeginSession()){
			bRet = false;
			break;
		}
		sqlite3_stmt *pstmt = nullptr;
		int ret = sqlite3_prepare16(pSqlite3, _T("UPDATE TXTMIRU_BOOKS SET POSITION=@POSITION")
									_T(" WHERE ID=@ID"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bRet = false;
			break;
		}
		bRet = true;
		for(auto &&item : position_list){
			if(item.position == item.position_org){
				continue;
			}
			sqlite3_reset(pstmt);
			sqlite3_clear_bindings(pstmt);
			CGrDBFunc::PutValue(pstmt, "@ID"      , item.id      );
			CGrDBFunc::PutValue(pstmt, "@POSITION", item.position);
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_DONE){
				bRet = false;
				break;
			}
		}
		if(pstmt){
			sqlite3_finalize(pstmt);
		}
		if(bRet){
			if(SQLITE_OK != db.Commit()){
				bRet = false;
				break;
			}
		} else {
			db.Rollback();
			break;
		}
		m_listView.SetRedraw(FALSE);
		DispList(true);
		m_listView.SetItemState(-1, 0, LVNI_FOCUSED|LVIS_SELECTED);
		std::sort(position_list.begin(),position_list.end(),Position::LessID);
		int index = -1;
		for(int i=m_book_list.size()-1; i>=0; --i){
			LVITEM item = { LVIF_PARAM };
			item.iItem = i;
			if(FALSE == m_listView.GetItem(&item)){
				bRet = false;
				break;
			}
			auto &&book = m_book_list[static_cast<int>(item.lParam)];
			auto position_it = std::lower_bound(position_list.begin(),position_list.end(),Position{book.id,0,0},Position::LessID);
			if(position_it != position_list.end() && position_it->id == book.id){
				if(position_it->state){
					m_listView.SetItemState(i, position_it->state, LVNI_FOCUSED|LVNI_SELECTED);
					index = i;
				}
			}
		}
		if(index >= 0){
			m_listView.EnsureVisible(index, FALSE);
		}
		m_listView.SetRedraw(TRUE);
	} while(0);

	return bRet;
}

bool CGrPanelBookWnd::addBook(std::vector<TxtFuncBookmark::Book> &book_list, int pos)
{
	CGrBookmarkDB db;
	if(!db.Open()){
		return false;
	}
	bool bret = true;
	do {
		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			bret = false;
			break;
		}
		if(SQLITE_OK != db.BeginSession()){
			bret = false;
			break;
		}
		int parent = m_tag.id;
		int pre_id = NULL_INT_VALUE;
		{
			int max_size = m_book_list.size();
			if(pos >= 0 && pos < max_size){
				pre_id = m_book_list[pos].id;
			} else if(max_size > 0){
				pre_id = -1;
			}
		}
		std::tstring sysdate;
		CGrDBFunc::GetSysDate(sysdate);
		for(const auto &item : book_list){
			{
				bool bSkip = false;
				for(const auto &check_item : m_book_list){
					if(item.url == check_item.url){
						bSkip = true;
						break;
					}
				}
				if(bSkip){
					continue;
				}
			}
			std::tstring url;
			CGrShell::ToPrettyFileName(item.url.c_str(), url);

			Place place;
			bret = db.GetPlace(place, url.c_str());
			if(!bret){
				place.id              = NULL_INT_VALUE;
				place.url             = url           ;
				place.title           = item.title    ;
				place.author          = item.author   ;
				place.page            = -1;
				place.read_page       = -1;
				place.visit_count     = 0;
				place.insert_date     = _T("");
				place.last_visit_date = _T("");
				bret = db.PutPlace(place);
				if(bret){
					bret = db.GetPlace(place, url.c_str());
					if(!bret){
						break;
					}
				} else {
					break;
				}
			}
			CGrDBBooks books;
			int position = 0;
			books.MaxPosition(pSqlite3, parent, position);

			::Book book;
			book.id          = NULL_INT_VALUE;
			book.place_id    = place.id      ;
			book.parent      = parent        ;
			book.type        = 0             ;
			book.position    = position+1    ;
			book.base_url    = place.url     ;
			book.title       = item.title    ;
			book.author      = item.author   ;
			book.insert_date = sysdate       ;
			bret = books.Put(pSqlite3, book);
			if(!bret){
				break;
			}
			int id = static_cast<int>(sqlite3_last_insert_rowid(pSqlite3));
			if(pre_id >= 0){
				do {
					bret = books.SortPosition(pSqlite3, parent, pre_id, id, 0);
					if(!bret){
						break;
					}
				} while(0);
				pre_id = id;
			}
		} /* for book_list */
		if(bret){
			if(SQLITE_OK != db.Commit()){
				bret = false;
				break;
			}
		} else {
			db.Rollback();
			break;
		}
		m_listView.SetRedraw(FALSE);
		DispList();
		m_listView.SetRedraw(TRUE);
	} while(0);
	return bret;
}

bool CGrPanelBookWnd::moveBook(int num, int book_list[], int position)
{
	bool bRet = false;
	do {
		int iItem = -1;
		bRet = true;
		struct Position {
			int  id          ;
			int  position    ;
			int  position_org;
			UINT state       ;
			static bool LessID(const Position &a1, const Position &a2) { return a1.id < a2.id; }
			static bool LessPosition(const Position &a1, const Position &a2) { return a1.position < a2.position; }
			static bool GreaterPosition(const Position &a1, const Position &a2) { return a1.position > a2.position; }
		};
		std::vector<Position> position_list;
		int i=0;
		for(const auto &book : m_book_list){
			position_list.push_back(Position{book.id,i,book.position,0});
			++i;
		}
		std::sort(position_list.begin(),position_list.end(),Position::LessID);
		if(num > 0){
			int *p = book_list;
			int i=position;
			for(; num>0; --num, ++p, ++i){
				auto position_it = std::lower_bound(position_list.begin(),position_list.end(),Position{*p,0,0},Position::LessID);
				if(position_it != position_list.end() && position_it->id == *p){
					position_it->position = i;
					position_it->state    = LVIS_SELECTED;
					++i;
				}
			}
			std::sort(position_list.begin(),position_list.end(),Position::LessPosition);
			int size = position_list.size();
			for(int j=position; j<size; ++j){
				if(position_list[j].state == 0){
					position_list[j].position = i;
					++i;
				}
			}
		}
		std::sort(position_list.begin(),position_list.end(),Position::LessPosition);
		CGrBookmarkDB db;
		if(!db.Open()){
			break;
		}
		sqlite3 *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			break;
		}
		sqlite3_stmt *pstmt = nullptr;
		int ret = sqlite3_prepare16(pSqlite3, _T("UPDATE TXTMIRU_BOOKS SET POSITION=@POSITION")
									_T(" WHERE ID=@ID"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bRet = false;
			break;
		}
		if(SQLITE_OK != db.BeginSession()){
			bRet = false;
			break;
		}
		bRet = true;
		for(const auto &item : position_list){
			if(item.position == item.position_org){
				continue;
			}
			sqlite3_reset(pstmt);
			sqlite3_clear_bindings(pstmt);
			CGrDBFunc::PutValue(pstmt, "@ID"      , item.id      );
			CGrDBFunc::PutValue(pstmt, "@POSITION", item.position);
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_DONE){
				bRet = false;
				break;
			}
		}
		if(pstmt){
			sqlite3_finalize(pstmt);
		}
		if(bRet){
			if(SQLITE_OK != db.Commit()){
				bRet = false;
				break;
			}
		} else {
			db.Rollback();
			break;
		}
		m_listView.SetRedraw(FALSE);
		DispList();
		m_listView.SetItemState(-1, 0, LVNI_FOCUSED|LVIS_SELECTED);
		std::sort(position_list.begin(),position_list.end(),Position::LessID);
		int index = -1;
		for(int i=m_book_list.size()-1; i>=0; --i){
			LVITEM item = { LVIF_PARAM };
			item.iItem = i;
			if(FALSE == m_listView.GetItem(&item)){
				bRet = false;
				break;
			}
			auto &&book = m_book_list[static_cast<int>(item.lParam)];
			auto position_it = std::lower_bound(position_list.begin(),position_list.end(),Position{book.id,0,0},Position::LessID);
			if(position_it != position_list.end() && position_it->id == book.id){
				if(position_it->state){
					m_listView.SetItemState(i, position_it->state, LVNI_FOCUSED|LVNI_SELECTED);
					index = i;
				}
			}
		}
		if(index >= 0){
			m_listView.EnsureVisible(index, FALSE);
		}
		m_listView.SetRedraw(TRUE);
	} while(0);

	return bRet;
}

bool CGrPanelBookWnd::saveOrder()
{
	bool bRet = false;
	do {
		bRet = true;
		struct Position {
			int  id          ;
			int  position    ;
			int  position_org;
			UINT state       ;
			static bool LessID(const Position &a1, const Position &a2) { return a1.id < a2.id; }
			static bool LessPosition(const Position &a1, const Position &a2) { return a1.position < a2.position; }
			static bool GreaterPosition(const Position &a1, const Position &a2) { return a1.position > a2.position; }
		};
		std::vector<Position> position_list;
		int i=0;
		int cnt = m_listView.GetItemCount();
		for(i=0; i<cnt; ++i){
			LVITEM item = { LVIF_PARAM };
			item.iItem = i;
			if(FALSE == m_listView.GetItem(&item)){
				bRet = false;
				break;
			}
			auto &&book = m_book_list[static_cast<int>(item.lParam)];
			position_list.push_back(Position{book.id,i,book.position,0});
		}
		if(!bRet){
			break;
		}
		CGrBookmarkDB db;
		if(!db.Open()){
			break;
		}
		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			break;
		}
		sqlite3_stmt *pstmt = nullptr;
		int ret = sqlite3_prepare16(pSqlite3, _T("UPDATE TXTMIRU_BOOKS SET POSITION=@POSITION")
									_T(" WHERE ID=@ID"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bRet = false;
			break;
		}
		if(SQLITE_OK != db.BeginSession()){
			bRet = false;
			break;
		}
		bRet = true;
		for(const auto &item : position_list){
			if(item.position == item.position_org){
				continue;
			}
			sqlite3_reset(pstmt);
			sqlite3_clear_bindings(pstmt);
			CGrDBFunc::PutValue(pstmt, "@ID"      , item.id      );
			CGrDBFunc::PutValue(pstmt, "@POSITION", item.position);
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_DONE){
				bRet = false;
				break;
			}
		}
		if(pstmt){
			sqlite3_finalize(pstmt);
		}
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

	return bRet;
}
