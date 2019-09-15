// main.cpp
//
#define STRICT // 型を宣言および使用時に、より厳密な型チェックが行われます。
#define MAIN_H
#pragma warning(disable:4786)

#define OEMRESOURCE
#include <windows.h>
#include <windowsx.h>
#include <ShlObj.h>
#include "resource.h"
#include "TxtMiru.h"
#include "WindowCtrl.h"
#include "stltchar.h"
#include "Shell.h"
#include "stlutil.h"
#include "Shlwapi.h"
#include <process.h>
#include "BookMarkDB.h"
#include "ListView.h"
#include "SplitWnd.h"
#include "ndc.h"
#include "Aozora.h"
#include "DragDropImpl.h"
#include "csvtext.h"
#include "KeyState.h"
#include "TxtMiruTheme.h"

#define TXTMIRU_PROP_NAME _T("TxtMiru2.0-Aozora")

class CGrMainWindow;
// メインウィンドウ
static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct BookCompareParam {
	int column;
};
static int CALLBACK bookCompare(LPARAM lp1, LPARAM lp2, LPARAM param)
{
	auto *pParam = reinterpret_cast<BookCompareParam*>(param);
	auto *pBook1 = reinterpret_cast<AozoraBooks*>(lp1);
	auto *pBook2 = reinterpret_cast<AozoraBooks*>(lp2);
	int ret = 0;
	switch(pParam->column){
	case 0: // IDS_BMH_TITLE
		if(pBook1->bBookmark != pBook2->bBookmark){
			if(pBook1->bBookmark){
				ret = 1;
			} else {
				ret = -1;
			}
		}
		if(ret == 0){
			ret = pBook1->title_s.compare(pBook2->title_s);
		}
		if(ret == 0){
			ret = pBook1->subtitle_r.compare(pBook2->subtitle_r);
		}
		if(ret == 0){
			ret = pBook1->subtitle_r.compare(pBook2->subtitle_r);
		}
		if(ret == 0){
			ret = pBook1->family_name_s.compare(pBook2->family_name_s);
		}
		if(ret == 0){
			ret = pBook1->given_name_s.compare(pBook2->given_name_s);
		}
		if(ret == 0){
			ret = pBook1->html_url.compare(pBook2->html_url);
		}
		break;
	case 1: // IDS_BMH_TITLE
		ret = pBook1->title_s.compare(pBook2->title_s);
		if(ret == 0){
			ret = pBook1->subtitle_r.compare(pBook2->subtitle_r);
		}
		if(ret == 0){
			ret = pBook1->family_name_s.compare(pBook2->family_name_s);
		}
		if(ret == 0){
			ret = pBook1->given_name_s.compare(pBook2->given_name_s);
		}
		if(ret == 0){
			ret = pBook1->html_url.compare(pBook2->html_url);
		}
		break;
	case 2: // IDS_BMH_AUTHOR
		ret = pBook1->family_name_s.compare(pBook2->family_name_s);
		if(ret == 0){
			ret = pBook1->given_name_s.compare(pBook2->given_name_s);
		}
		if(ret == 0){
			ret = pBook1->title_s.compare(pBook2->title_s);
		}
		if(ret == 0){
			ret = pBook1->subtitle_r.compare(pBook2->subtitle_r);
		}
		if(ret == 0){
			ret = pBook1->html_url.compare(pBook2->html_url);
		}
		break;
	case 3: // IDS_BMH_URL
		ret = pBook1->html_url.compare(pBook2->html_url);
		if(ret == 0){
			ret = pBook1->title_s.compare(pBook2->title_s);
		}
		if(ret == 0){
			ret = pBook1->subtitle_r.compare(pBook2->subtitle_r);
		}
		if(ret == 0){
			ret = pBook1->given_name_s.compare(pBook2->given_name_s);
		}
		if(ret == 0){
			ret = pBook1->family_name_s.compare(pBook2->family_name_s);
		}
		break;
	case 4: // IDS_BMH_UPDATE_DATE
		ret = pBook2->release_date.compare(pBook1->release_date);
		if(ret == 0){
			ret = pBook1->title_s.compare(pBook2->title_s);
		}
		if(ret == 0){
			ret = pBook1->subtitle_r.compare(pBook2->subtitle_r);
		}
		if(ret == 0){
			ret = pBook1->given_name_s.compare(pBook2->given_name_s);
		}
		if(ret == 0){
			ret = pBook1->family_name_s.compare(pBook2->family_name_s);
		}
		if(ret == 0){
			ret = pBook1->html_url.compare(pBook2->html_url);
		}
		break;
	}
	return ret;
}

static LPTSTR copyText(LPTSTR lpDst, std::tstring &str, TCHAR ch)
{
	for(auto lpSrc = str.c_str(); *lpSrc; ++lpSrc, ++lpDst){
		*lpDst = *lpSrc;
	}
	*lpDst = ch;
	return lpDst + 1;
}

class CGrAozoraListView : public CGrListView
{
private:
	CGrKeyboardState m_copyKeyState;
public:
	CGrAozoraListView()
	{
		m_copyKeyState = CGrKeyboardState('C', false, true, false);
	}
	virtual ~CGrAozoraListView(){}
protected:
	virtual void OnKeyDown(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
	{
		if(m_copyKeyState == CGrKeyboardState(vk)){
			//
			std::list<AozoraBooks*> book_list;
			int cnt = GetSelectedCount();
			UINT flags = (cnt <= 0) ? (LVNI_ALL | LVNI_FOCUSED) : (LVNI_ALL | LVNI_SELECTED);
			int iItem = -1;
			int text_size = 0;
			while((iItem=GetNextItem(iItem, flags))!=-1){
				LVITEM item = { LVIF_PARAM };
				item.iItem = iItem;
				if(FALSE == GetItem(&item)){
					break;
				}
				auto* pBook = reinterpret_cast<AozoraBooks*>(item.lParam);
				if(pBook){
					book_list.push_back(pBook);
					text_size += pBook->html_url.size();
					text_size += pBook->title.size();
					text_size += pBook->subtitle.size();
					text_size += pBook->family_name.size();
					text_size += pBook->given_name.size();
					text_size += 6;
				}
			}
			if(book_list.size() <= 0){
				return;
			}
			auto hMem = ::GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, (text_size+1) * sizeof(TCHAR));
			if (hMem) {
				auto lpText = static_cast<LPTSTR>(GlobalLock(hMem));
				if (!lpText) { GlobalFree(hMem); return; } // メモリロックに失敗 : メモリの解放
				//
				auto lpBegin = lpText;
				for (const auto& pItem : book_list) {
					lpBegin = copyText(lpBegin, pItem->title, _T(' '));
					lpBegin = copyText(lpBegin, pItem->subtitle, _T('\t'));
					lpBegin = copyText(lpBegin, pItem->family_name, _T(' '));
					lpBegin = copyText(lpBegin, pItem->given_name, _T('\t'));
					lpBegin = copyText(lpBegin, pItem->html_url, _T('\r'));
					*lpBegin = _T('\n');
					++lpBegin;
				}
				book_list.clear();
				*lpBegin = _T('\0');
				//
				GlobalUnlock(hMem);
			}
			// クリップボードを開きます。
			if(!::OpenClipboard(NULL)){
				::GlobalFree(hMem);
				return;
			}
			// クリップボードを空にします。
			if(!::EmptyClipboard()){
				::GlobalFree(hMem);
				// クリップボードを閉じます。
				::CloseClipboard();
				return;
			}
			// クリップボードにデータを設定します。
#ifdef UNICODE
			::SetClipboardData(CF_UNICODETEXT, hMem);
#else
			::SetClipboardData(CF_TEXT, hMem);
#endif
			// クリップボードを閉じます。
			::CloseClipboard();
			//
		} else {
			CGrListView::OnKeyDown(hWnd, vk, fDown, cRepeat, flags);
		}
	}
};

namespace TxtFuncBookmark
{
	enum ImageIcon {
		ii_none           ,
		ii_stat_tag_favorite           ,
		ii_max
	};
};
//////////////////////////////////////////////////
class CGrMainWindow
{
private:
	int m_win_size_x_min = 0;
	int m_win_size_y_min = 0;
	HINSTANCE m_hInstance = NULL;
	const int listwnd_id = 3000;
	HWND m_hWnd = NULL;
	CGrAozoraListView m_listView;
	CGrSplitWnd m_splitWnd;
	struct TreeItemInfo {
		std::tstring cond;
		LPARAM lParam = 0;
		bool bExpand = false;
		int level = 0;
		int type = 0;
	};
	using TreeItemList = std::list<TreeItemInfo *>;
	TreeItemList m_tree_list;
	std::vector<AozoraBooks> m_book_list;
	CGrWinCtrl m_editBox;
	HIMAGELIST m_hImg = NULL;
public:
	static inline CGrMainWindow &theApp()
	{
		static CGrMainWindow mainWindow;
		return mainWindow;
	}
	BOOL PreTranslateMessage(MSG &msg)
	{
		if(msg.message == WM_KEYDOWN && msg.hwnd == m_editBox){
			if(msg.wParam == VK_RETURN){
				::PostMessage(m_hWnd, WM_COMMAND, IDSEARCH, 0);
				return FALSE;
			}
		}
		return TRUE;
	}
	virtual ~CGrMainWindow()
	{
		DeleteAllItem();
	}
	HWND Create(HINSTANCE hInstance)
	{
		m_hInstance = hInstance;
		WNDCLASSEX wc = { sizeof(WNDCLASSEX) };

		wc.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wc.hIcon = static_cast<HICON>(LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP)));
		wc.hIconSm = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
		wc.lpszClassName = _T("TxtMiruDBTool");
		wc.cbWndExtra = DLGWINDOWEXTRA;
		wc.hInstance = hInstance;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = ::MainWndProc;

		if (!RegisterClassEx(&wc)){
			return NULL;
		}

		return CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, reinterpret_cast<DLGPROC>(::MainWndProc));
	}
	int getLeafLevel(HWND hWnd, HTREEITEM htItem)
	{
		int level = -1;
		for(; htItem; ++level){
			htItem = TreeView_GetParent(hWnd, htItem);
		}
		return level;
	}
	LRESULT MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg){
			HANDLE_MSG(hWnd, WM_COMMAND          , OnCommand    );
			HANDLE_MSG(hWnd, WM_INITDIALOG       , OnInitDialog );
			HANDLE_MSG(hWnd, WM_SIZE             , OnSize       );
		case WM_CLOSE:
			::DestroyWindow(hWnd);
			break;
		case WM_GETMINMAXINFO:
			{
				auto lpmm = reinterpret_cast<LPMINMAXINFO>(lParam);
				lpmm->ptMinTrackSize.x = m_win_size_x_min;
				lpmm->ptMinTrackSize.y = m_win_size_y_min;
			}
			break;
		case WM_NOTIFY:
			{
				auto lpnmh = reinterpret_cast<LPNMHDR>(lParam);
				if (wParam == listwnd_id) {
					switch (lpnmh->code) {
					case NM_DBLCLK:
						{
							auto lpnmitem = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
							openFile(lpnmitem->iItem);
						}
						break;
					case LVN_COLUMNCLICK:
						{
							auto lpnmlist = reinterpret_cast<LPNMLISTVIEW>(lParam);
							BookCompareParam param;
							param.column = lpnmlist->iSubItem;
							m_listView.SortItems(bookCompare, reinterpret_cast<LPARAM>(&param));
						}
						break;
					case LVN_BEGINDRAG:
						{
							auto pnmv = reinterpret_cast<LPNMLISTVIEW>(lParam);
							std::list<AozoraBooks*> book_list;
							int cnt = m_listView.GetSelectedCount();
							UINT flags = (cnt <= 0) ? (LVNI_ALL | LVNI_FOCUSED) : (LVNI_ALL | LVNI_SELECTED);
							int iItem = -1;
							int text_size = 0;
							while ((iItem = m_listView.GetNextItem(iItem, flags)) != -1) {
								LVITEM item = { LVIF_PARAM };
								item.iItem = iItem;
								if (FALSE == m_listView.GetItem(&item)) {
									break;
								}
								auto* pBook = reinterpret_cast<AozoraBooks*>(item.lParam);
								if (pBook) {
									book_list.push_back(pBook);
									text_size += pBook->html_url.size();
									text_size += pBook->title.size();
									text_size += pBook->subtitle.size();
									text_size += pBook->family_name.size();
									text_size += pBook->given_name.size();
									text_size += 5;
								}
							}
							if (book_list.size() <= 0) {
								break;
							}
							auto *pDropSource = new CDropSource;

							///////////////
							auto *pDataObject = new CDataObject(pDropSource);

							FORMATETC fmtetc = {};
							fmtetc.cfFormat = RegisterClipboardFormat(_T("TxtMiruAozoraBooksInfo"));
							fmtetc.dwAspect = DVASPECT_CONTENT;
							fmtetc.lindex = -1;
							fmtetc.tymed = TYMED_HGLOBAL;
							STGMEDIUM medium = {};
							medium.tymed = TYMED_HGLOBAL;
							// URL,TITLE,AUTHOR
							medium.hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (text_size + 1) * sizeof(TCHAR));
							if (medium.hGlobal) {
								auto lpText = static_cast<LPTSTR>(GlobalLock(medium.hGlobal));
								//
								if (lpText) {
									auto lpBegin = lpText;
									for (const auto& pItem : book_list) {
										lpBegin = copyText(lpBegin, pItem->html_url, _T('\0'));
										lpBegin = copyText(lpBegin, pItem->title, _T(' '));
										lpBegin = copyText(lpBegin, pItem->subtitle, _T('\0'));
										lpBegin = copyText(lpBegin, pItem->family_name, _T(' '));
										lpBegin = copyText(lpBegin, pItem->given_name, _T('\0'));
									}
									book_list.clear();
									*lpBegin = _T('\0');
								}
								//
								GlobalUnlock(medium.hGlobal);
							}
							pDataObject->SetData(&fmtetc, &medium, TRUE);
							///////////////
							CDragSourceHelper dragSrcHelper;
							dragSrcHelper.InitializeFromWindow(m_listView, pnmv->ptAction, pDataObject);

							DWORD dwEffect;
							auto hr = ::DoDragDrop(pDataObject, pDropSource, DROPEFFECT_MOVE, &dwEffect);
							if (SUCCEEDED(hr)) {
								;
							}
							else {
								TCHAR buf[2048];
								_stprintf_s(buf, _T("%x"), hr);
								MessageBox(NULL, buf, _T("OK"), MB_OK);
							}
							pDropSource->Release();
							pDataObject->Release();
						}
						break;
					}
				}
				else if (wParam == IDC_TREE_BM) {
					if (lpnmh->code == NM_CUSTOMDRAW) {
						auto lpNMCustomDraw = reinterpret_cast<LPNMTVCUSTOMDRAW>(lParam);
						if (lpNMCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
							return CDRF_NOTIFYITEMDRAW;
						else if (lpNMCustomDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
							auto inFocus = (GetFocus() == lpNMCustomDraw->nmcd.hdr.hwndFrom);
							if (!inFocus) {
								lpNMCustomDraw->clrText = TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT);
								return CDRF_NEWFONT;
							}
						}
					}
					else if (lpnmh->code == TVN_SELCHANGED) {
						auto lpnmtreeview = reinterpret_cast<LPNMTREEVIEW>(lParam);
						auto *pInfo = reinterpret_cast<TreeItemInfo*>(lpnmtreeview->itemNew.lParam);
						if (pInfo) {
							if (!pInfo->cond.empty()) {
								selectItem(pInfo->cond, pInfo->type);
							}
						}
					}
					else if (lpnmh->code == TVN_ITEMEXPANDING) {
						auto lpnmtreeview = reinterpret_cast<LPNMTREEVIEW>(lParam);
						auto *pInfo = reinterpret_cast<TreeItemInfo*>(lpnmtreeview->itemNew.lParam);
						if (pInfo && !pInfo->bExpand) {
							expandItem(lpnmh->hwndFrom, lpnmtreeview->itemNew.hItem, pInfo);
						}
					}
				}
			}
			break;
		case WM_SETTINGCHANGE:
			if(TxtMiruTheme_IsImmersiveColorSet(lParam)){
				auto hBookmarkTree = GetDlgItem(m_hWnd, IDC_TREE_BM);
				if (hBookmarkTree) {
					TreeView_SetBkColor(hBookmarkTree, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
					TreeView_SetTextColor(hBookmarkTree, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
				}
				if (m_listView) {
					ListView_SetBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
					ListView_SetTextColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
					ListView_SetTextBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
				}
				TxtMiruTheme_UpdateDarkMode(m_hWnd);
				InvalidateRect(m_hWnd, nullptr, TRUE);
			}
			break;
		case WM_DESTROY: OnDestory(); break;
		default:
			return DefWindowProc(hWnd,uMsg,wParam,lParam);
		}
		return 0L;
	}
private:
	void expandItem(HWND hBookmarkTree, HTREEITEM hItem, TreeItemInfo *pInfo)
	{
		{
			auto hItemC = TreeView_GetChild(hBookmarkTree, hItem);
			TreeView_DeleteItem(hBookmarkTree, hItemC);
		}
		pInfo->bExpand = true;
		CGrBookmarkDB db;
		db.Open();
		if(pInfo->type == IDS_TREE_NDC || pInfo->type == IDS_TREE_NDC_C){
			int indc = 0;
			LPCTSTR *pndc = nullptr;
			if(pInfo->level == 0){
				pndc = NDC1_TABLE;
			} else if(pInfo->level == 1){
				indc = pInfo->lParam * 10;
				pndc = NDC2_TABLE + indc;
			} else if(pInfo->level == 2){
				indc = pInfo->lParam * 10;
				pndc = NDC3_TABLE + indc;
			}
			if(pndc){
				for(int i=0; i<10; ++i, ++pndc, ++indc){
					addTreeItemNDC(hBookmarkTree, db, hItem, *pndc, indc, pInfo->type);
				}
			}
		} else if(pInfo->type == IDS_TREE_AUTHOR || pInfo->type == IDS_TREE_TITLE){
			std::vector<AozoraFirstCharBooks> book_list;
			if(pInfo->type == IDS_TREE_AUTHOR){
				db.GetFamilyName1List(book_list);
			} else {
				db.GetTitleS1List(book_list);
			}
			struct LineInfo {
				LPCTSTR title;
				LPCTSTR str_line;
				int count;
				HTREEITEM hItem;
			} line_info[] = {
				{_T("あ行"), _T("あいうえお"), 0, NULL}, // あ行
				{_T("か行"), _T("かきくけこ"), 0, NULL}, // か行
				{_T("さ行"), _T("さしすせそ"), 0, NULL}, // さ行
				{_T("た行"), _T("たちつてと"), 0, NULL}, // た行
				{_T("な行"), _T("なにぬねの"), 0, NULL}, // な行
				{_T("は行"), _T("はひふへほ"), 0, NULL}, // は行
				{_T("ま行"), _T("まみむめも"), 0, NULL}, // ま行
				{_T("や行"), _T("やゆよ"    ), 0, NULL}, // や行
				{_T("ら行"), _T("らりるれろ"), 0, NULL}, // ら行
				{_T("わ行"), _T("わゐゑを"  ), 0, NULL}, // わ行
			};
			LineInfo other_info = {_T("他"), _T(""), 0, NULL};
			for(const auto &item : book_list){
				bool bNotFind = true;
				for(auto &&info : line_info){
					if(_tcsstr(info.str_line, item.first_char.c_str())){
						info.count += item.count;
						bNotFind = false;
						break;
					}
				}
				if(bNotFind){
					other_info.count += item.count;
				}
			}
			for(auto &&info : line_info){
				if(info.count > 0){
					info.hItem = addTreeItem(hBookmarkTree, hItem, info.title, info.count, pInfo->type);
				}
			}
			if(other_info.count > 0){
				other_info.hItem = addTreeItem(hBookmarkTree, hItem, other_info.title, other_info.count, pInfo->type);
			}
			for(const auto &item : book_list){
				bool bNotFind = true;
				for(auto &&info : line_info){
					if(info.hItem && _tcsstr(info.str_line, item.first_char.c_str())){
						addTreeItem(hBookmarkTree, info.hItem, item.first_char.c_str(), item.count, pInfo->type, item.first_char.c_str());
						bNotFind = false;
						break;
					}
				}
				if(other_info.hItem && bNotFind){
					addTreeItem(hBookmarkTree, other_info.hItem, item.first_char.c_str(), item.count, pInfo->type, item.first_char.c_str());
				}
			}
		}
		db.Close();
	}
	void selectItem(std::tstring &cond, int type)
	{
		CGrBookmarkDB db;
		db.Open();
		m_book_list.clear();
		switch(type){
		case IDS_TREE_NEW   : db.GetLatestList     (              m_book_list); break;
		case IDS_TREE_NDC   : db.GetClassList      (cond.c_str(), m_book_list); break;
		case IDS_TREE_NDC_C : db.GetClassList      (cond.c_str(), m_book_list); break;
		case IDS_TREE_AUTHOR: db.GetFamilyNameSList(cond.c_str(), m_book_list); break;
		case IDS_TREE_TITLE : db.GetTitleSList     (cond.c_str(), m_book_list); break;
		}
		setBookList();
		db.Close();
	}
	void setBookList()
	{
		m_listView.SetRedraw(false);
		m_listView.DeleteAllItems();
		for(const auto &book : m_book_list){
			LVITEM item = {0};
			item.mask     = LVIF_TEXT | LVIF_PARAM;
			item.iItem    = m_listView.GetItemCount();
			item.lParam   = reinterpret_cast<LPARAM>(&book);
			item.iSubItem = 0;
			item.pszText  = const_cast<TCHAR*>((book.bBookmark) ? _T("☆") : _T(""));
			item.iItem    = m_listView.InsertItem(&item);
			item.mask     = LVIF_TEXT;
			item.iSubItem = 1;
			//
			std::tstring str;
			str = book.title;
			str += _T(" ");
			str += book.subtitle;

			item.iSubItem = 1;
			item.pszText  = const_cast<LPTSTR>(str.c_str());
			m_listView.SetItem(&item);

			str = book.family_name;
			str += _T(" ");
			str += book.given_name;
			item.mask     = LVIF_TEXT;
			item.pszText  = const_cast<LPTSTR>(str.c_str());
			item.iSubItem = 2;
			m_listView.SetItem(&item);

			item.pszText  = const_cast<LPTSTR>(book.html_url.c_str());
			item.iSubItem = 3;
			m_listView.SetItem(&item);

			item.pszText  = const_cast<LPTSTR>(book.release_date.c_str());
			item.iSubItem = 4;
			m_listView.SetItem(&item);
		}
		m_listView.SetRedraw(true);
	}
	void updateDB(LPCTSTR lpURL)
	{
		auto ret = AozoraUpdate(lpURL);

		m_listView.DeleteAllItems();
		DeleteAllItem();
		setTreeTop(true);
		{
			std::tstring ini_filename;
			CGrTxtFunc::GetBookmarkFolder(ini_filename);
			ini_filename += _T("/aozora.ini");
			std::tstring strdate;
			GetAozoraUpdateDate(strdate);
			::WritePrivateProfileString(_T("Aozora"), _T("UpdateDate"), strdate.c_str(), ini_filename.c_str());
			auto hStUpdateD = GetDlgItem(m_hWnd, IDC_STATIC_UPDATE_D);
			SetWindowText(hStUpdateD, strdate.c_str());
		}

		UINT id = IDS_UPDATE_ERROR;
		/**/ if(ret == ADBERR_SUCCESS        ){ id = IDS_DOWNLOAD_END   ; }
		else if(ret == ADBERR_CSVFILE_OPEN   ){ id = IDS_CSVFILE_OPEN   ; }
		else if(ret == ADBERR_CSVFILE_NODATA ){ id = IDS_CSVFILE_NODATA ; }
		else if(ret == ADBERR_ZIPFILE_OPEN   ){ id = IDS_ZIPFILE_OPEN   ; }
		else if(ret == ADBERR_ZIPFILE_GETINFO){ id = IDS_ZIPFILE_GETINFO; }
		else if(ret == ADBERR_ZIPFILE_CUROPEN){ id = IDS_ZIPFILE_CUROPEN; }
		else if(ret == ADBERR_ZIPFILE_NODATA ){ id = IDS_ZIPFILE_NODATA ; }
		else if(ret == ADBERR_WEBFILE_OPEN   ){ id = IDS_WEBFILE_OPEN   ; }

		std::tstring mes;
		CGrText::LoadString(id, mes);
		MessageBox(m_hWnd, mes.c_str(), TXTMIRU, MB_OK);
	}

	void DeleteAllItem()
	{
		for(auto &pitem : m_tree_list){
			if(pitem){
				delete pitem;
			}
		}
		m_tree_list.clear();
	}
	void openFile(int iItem)
	{
		TCHAR url[2048] = {};
		ListView_GetItemText(m_listView, iItem, 3, url, sizeof(url)/sizeof(TCHAR));
		TCHAR curPath[MAX_PATH];
		CGrShell::GetExePath(curPath);
		TCHAR exeFileName[MAX_PATH];
		_stprintf_s(exeFileName, _T("%s\\TxtMiruCli.exe"), curPath);
		ShellExecute(m_hWnd, NULL, exeFileName, url, NULL, SW_SHOWDEFAULT);
	}
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		switch(id){
		case IDOPEN:
			{
				UINT flags = LVNI_ALL | LVNI_FOCUSED;
				int iItem = -1;
				while((iItem=m_listView.GetNextItem(iItem, flags))!=-1){
					break;
				}
				if(iItem >= 0){
					openFile(iItem);
				}
			}
			break;
		case IDUPDATE:
			{
				std::tstring mes;
				CGrText::LoadString(IDS_DOWNLOAD_CONF, mes);
				if(MessageBox(hwnd, mes.c_str(), TXTMIRU, MB_YESNO) == IDYES){
					updateDB(_T("https://www.aozora.gr.jp/index_pages/list_person_all_extended_utf8.zip"));
				}
			}
			break;
		case IDUPDATE_FILE:
			{
				TCHAR filter[2048] = {};
				CGrText::LoadString(IDS_CSVFILE, filter, sizeof(filter)/sizeof(TCHAR));
				std::tstring str;
				CGrText::LoadString(IDS_OPENFILE, str);

				OPENFILENAME of = {sizeof(OPENFILENAME)};
				TCHAR fileName[MAX_PATH] = {};

				of.hwndOwner       = m_hWnd;
				of.lpstrFilter     = filter;
				of.lpstrTitle      = str.c_str();
				of.nMaxCustFilter  = 40;
				of.lpstrFile       = fileName;
				of.nMaxFile        = MAX_PATH - 1;
				of.Flags           = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
				if(::GetOpenFileName(&of)){
					updateDB(fileName);
				}
			}
			break;
		case IDSEARCH:
			{
				auto hSearchEdit = GetDlgItem(m_hWnd, IDC_EDIT_SEARCH);
				int len = Edit_GetTextLength(hSearchEdit);
				if(len <= 0){
					break;
				}
				++len;
				auto *buf = new TCHAR[len+1];
				Edit_GetText(hSearchEdit, buf, len);

				CGrBookmarkDB db;
				db.Open();
				m_book_list.clear();
				db.GetSearchWord(buf, m_book_list);
				setBookList();
				db.Close();
				delete [] buf;

				auto hBookmarkTree = GetDlgItem(m_hWnd, IDC_TREE_BM);
				TreeView_SelectItem(hBookmarkTree, NULL);
			}
			break;
		case IDCANCEL:
		case ID_EXIT:
			::PostMessage(m_hWnd, WM_CLOSE, 0, 0L);
			break;
		}
	}
	HTREEITEM addTreeItemNDC(HWND hBookmarkTree, CGrBookmarkDB &db, HTREEITEM hParent, LPCTSTR title, LPARAM lParam, int type)
	{
		if(_tcslen(title) <= 0){
			return NULL;
		}
		int level = getLeafLevel(hBookmarkTree, hParent) + 1;
		TCHAR buf[512] = {};
		if(type == IDS_TREE_NDC){
			if(level == 1){
				_stprintf_s(buf, _T("%% %01d%%"), lParam % 10);
			} else if(level == 2){
				_stprintf_s(buf, _T("%% %02d%%"), lParam % 100);
			} else if(level == 3){
				_stprintf_s(buf, _T("%% %03d%%"), lParam % 1000);
			}
		} else {
			if(level == 1){
				_stprintf_s(buf, _T("%% K%01d%%"), lParam % 10);
			} else if(level == 2){
				_stprintf_s(buf, _T("%% K%02d%%"), lParam % 100);
			} else if(level == 3){
				_stprintf_s(buf, _T("%% K%03d%%"), lParam % 1000);
			}
		}
		int count = 0;
		db.GetClassCount(buf, count);
		if(count <= 0){
			return NULL;
		}
		auto *pInfo = new TreeItemInfo;
		pInfo->lParam = lParam;
		pInfo->type   = type;
		pInfo->level  = level;
		if(level == 3){
			pInfo->cond = buf;
		}
		m_tree_list.push_back(pInfo);

		TCHAR title_buf[2048] = {};
		_stprintf_s(title_buf, _T("%s (%d)"), title, count);

		auto hItem = InsertTreeItem(hBookmarkTree, hParent, title_buf, pInfo);
		if(level < 3){
			InsertTreeDummyItem(hBookmarkTree, hItem);
		}

		return hItem;
	}
	HTREEITEM InsertTreeItem(HWND hBookmarkTree, HTREEITEM hParent, LPTSTR title, TreeItemInfo *pInfo)
	{
		TVINSERTSTRUCT is = {};
		is.hParent           = hParent;
		is.hInsertAfter      = TVI_LAST;
		is.itemex.mask       = TVIF_PARAM | TVIF_TEXT;
		is.itemex.pszText    = title;
		is.itemex.cchTextMax = _tcslen(title);
		is.itemex.lParam     = reinterpret_cast<LPARAM>(pInfo);
		return TreeView_InsertItem(hBookmarkTree, &is);
	}
	HTREEITEM InsertTreeDummyItem(HWND hBookmarkTree, HTREEITEM hParent)
	{
		TVINSERTSTRUCT is = {};
		is.hParent      = hParent;
		return TreeView_InsertItem(hBookmarkTree, &is);
	}
	HTREEITEM addTreeItem(HWND hBookmarkTree, HTREEITEM hParent, LPCTSTR title, int count, int type, LPCTSTR lpCond = nullptr)
	{
		if(_tcslen(title) <= 0){
			return NULL;
		}
		int level = getLeafLevel(hBookmarkTree, hParent) + 1;
		auto *pInfo = new TreeItemInfo;
		if(lpCond){
			pInfo->cond = lpCond;
			pInfo->cond += _T("%");
		}
		pInfo->type    = type;
		pInfo->bExpand = true;
		pInfo->level   = level;
		m_tree_list.push_back(pInfo);

		TCHAR title_buf[2048] = {};
		_stprintf_s(title_buf, _T("%s (%d)"), title, count);

		return InsertTreeItem(hBookmarkTree, hParent, title_buf, pInfo);
	}
	HTREEITEM addTreeItem(HWND hBookmarkTree, int id)
	{
		std::tstring str_name;
		CGrText::LoadString(id, str_name);
		auto *pInfo = new TreeItemInfo;
		pInfo->lParam = id;
		pInfo->type   = id;
		m_tree_list.push_back(pInfo);
		//
		auto hItem = InsertTreeItem(hBookmarkTree, NULL, const_cast<LPTSTR>(str_name.c_str()), pInfo);
		InsertTreeDummyItem(hBookmarkTree, hItem);
		return hItem;
	}
	void setTreeTop(bool bNoShowUpdateMes = false)
	{
		bool bUpdate = false;
		auto hBookmarkTree = GetDlgItem(m_hWnd, IDC_TREE_BM);
		TreeView_DeleteAllItems(hBookmarkTree);
		DeleteAllItem();
		{
			int count = 0;
			CGrBookmarkDB db;
			db.Open();
			db.GetLatestCount(count);
			{
				int total_count = 0;
				db.GetCount(total_count);
				bUpdate = (total_count == 0);
			}
			db.Close();
			if(count > 0){
				std::tstring str_name;
				CGrText::LoadString(IDS_TREE_NEW, str_name);
				auto hItem = addTreeItem(hBookmarkTree, NULL, str_name.c_str(), count, IDS_TREE_NEW, _T("*"));
				TreeView_SelectItem(hBookmarkTree, hItem);
			}
		}
		addTreeItem(hBookmarkTree, IDS_TREE_NDC   );
		addTreeItem(hBookmarkTree, IDS_TREE_NDC_C );
		addTreeItem(hBookmarkTree, IDS_TREE_AUTHOR);
		addTreeItem(hBookmarkTree, IDS_TREE_TITLE );

		if(!bNoShowUpdateMes && bUpdate){
			FORWARD_WM_COMMAND(m_hWnd, IDUPDATE, 0, 0, PostMessage);
		}
	}

	BOOL OnInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lParam)
	{
		m_hWnd = hWnd;
		SetProp(m_hWnd, TXTMIRU_PROP_NAME, reinterpret_cast<HANDLE>(-1));
		//
		{
			RECT rect;
			GetWindowRect(hWnd, &rect);
			m_win_size_x_min = rect.right - rect.left;
			m_win_size_y_min = rect.bottom - rect.top;
		}
		SHFILEINFO fileInfo = {};
		auto hBookmarkTree = GetDlgItem(m_hWnd, IDC_TREE_BM);
		TxtMiruTheme_SetWindowTheme(hBookmarkTree, L"Explorer", NULL);
		TreeView_SetBkColor(hBookmarkTree, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
		TreeView_SetTextColor(hBookmarkTree, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
		TreeView_SetExtendedStyle(hBookmarkTree, TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
		SendMessage(hBookmarkTree, WM_CHANGEUISTATE, MAKELONG(UIS_SET, UISF_HIDEFOCUS), 0);

		//
		auto hListWnd = CreateWindow(_T("syslistview32"),
									 NULL,
									 WS_BORDER | WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ALIGNLEFT | WS_TABSTOP,
									 0,0,10,10,m_hWnd, reinterpret_cast<HMENU>(listwnd_id), m_hInstance,NULL);
		m_listView.Attach(hListWnd);
		m_listView.SetExtendedListViewStyle(m_listView.GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);
		TxtMiruTheme_SetWindowTheme(m_listView, L"Explorer", NULL);
		ListView_SetBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
		ListView_SetTextColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
		ListView_SetTextBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
		TxtMiruTheme_UpdateDarkMode(m_hWnd);
		//
		SendMessage(m_listView, WM_CHANGEUISTATE, MAKELONG(UIS_SET, UISF_HIDEFOCUS), 0);
		std::tstring str_name;
		SetWindowFont(m_listView, GetWindowFont(hBookmarkTree), TRUE);
		//
		TxtMiruTheme_SetWindowSubclass(m_hWnd);
		//
		LVCOLUMN lvc = {};
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;

		lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
		lvc.iSubItem = 0;
		lvc.cx = 30;
		m_listView.InsertColumn(lvc.iSubItem, &lvc);
		//
		CGrText::LoadString(IDS_BMH_TITLE, str_name);
		lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
		lvc.iSubItem = 1;
		lvc.cx = 300;
		m_listView.InsertColumn(lvc.iSubItem, &lvc);

		CGrText::LoadString(IDS_BMH_AUTHOR, str_name);
		lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
		lvc.iSubItem = 2;
		lvc.cx = 200;
		m_listView.InsertColumn(lvc.iSubItem, &lvc);

		CGrText::LoadString(IDS_BMH_URL, str_name);
		lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
		lvc.iSubItem = 3;
		lvc.cx = 300;
		m_listView.InsertColumn(lvc.iSubItem, &lvc);

		CGrText::LoadString(IDS_BMH_UPDATE_DATE, str_name);
		lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
		lvc.iSubItem = 4;
		lvc.cx = 100;
		m_listView.InsertColumn(lvc.iSubItem, &lvc);
		//
		//
		m_splitWnd.Create(hWnd, hBookmarkTree, m_listView);
		m_splitWnd.SetPosition(200);
		//
		{
			std::tstring ini_filename;
			CGrTxtFunc::GetBookmarkFolder(ini_filename);
			ini_filename += _T("/aozora.ini");
			TCHAR val[2048];
			::GetPrivateProfileString(_T("Aozora"), _T("WindowPos"), _T(""), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
			WINDOWPLACEMENT wndp = {sizeof(WINDOWPLACEMENT)};
			GetWindowPlacement(m_hWnd, &wndp);
			CGrCSVText csv;
			CSV_COLMN *pcsv_colmn;
			csv.AddTail(val);
			int row = csv.GetRowSize() - 1;
			if(row >= 0 && csv.GetColmnSize(row) >= 4){
				RECT rect;
				rect.left   = csv.GetInteger(row, 0, 0);
				rect.top    = csv.GetInteger(row, 1, 0);
				rect.right  = csv.GetInteger(row, 2, 0);
				rect.bottom = csv.GetInteger(row, 3, 0);
				wndp.rcNormalPosition = rect;
				wndp.showCmd = SW_SHOWNORMAL;
				POINT point = {wndp.rcNormalPosition.left, wndp.rcNormalPosition.top};
				auto hMonitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST); // 指定した点に最も近い位置にあるディスプレイモニタのハンドルが返ります。
				MONITORINFO mi = {sizeof(MONITORINFO)};
				RECT desktop_rc = {};
				if(hMonitor && ::GetMonitorInfo(hMonitor, &mi)){
					if(rect.right - rect.left <= 0 || rect.bottom - rect.top <= 0){
						desktop_rc = mi.rcMonitor;
					}
				} else {
					::SystemParametersInfo(SPI_GETWORKAREA, 0, &desktop_rc, 0);
				}
				// center;
				if(desktop_rc.right - desktop_rc.left > 0 && desktop_rc.bottom - desktop_rc.top > 0){
					int w0 = desktop_rc.right - desktop_rc.left;
					int h0 = desktop_rc.bottom - desktop_rc.top;
					int w = w0 * 80 / 100;
					int h = h0 * 80 / 100;
					int x = w0 / 2 - w / 2;
					int y = h0 / 2 - h / 2;
					wndp.rcNormalPosition.left   = x    ;
					wndp.rcNormalPosition.right  = x + w;
					wndp.rcNormalPosition.top    = y    ;
					wndp.rcNormalPosition.bottom = y + h;
				}
				SetWindowPlacement(m_hWnd, &wndp);
			}
			int ival;
			::GetPrivateProfileString(_T("Aozora"), _T("TreeSplitPos"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
			ival = _ttol(val);
			if(ival >= 0){
				m_splitWnd.SetPosition(ival);
			}
			::GetPrivateProfileString(_T("Aozora"), _T("ListColumnWidth"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
			csv.Clear();
			csv.AddTail(val);
			pcsv_colmn = csv.GetRow(0);
			unsigned int count = Header_GetItemCount(m_listView.GetHeader());
			if(pcsv_colmn){
				int ini_colmun_count = pcsv_colmn->size() - 1;
				if(ini_colmun_count >= static_cast<signed int>(count)){
					unsigned int column = 0;
					for(const auto &item : *pcsv_colmn){
						m_listView.SetColumnWidth(column, _ttol(item.c_str()));
						++column;
						if(column >= count){
							break;
						}
					}
				} else if(ini_colmun_count >= (static_cast<signed int>(count)-1)){
					unsigned int column = 1;
					for(const auto &item : *pcsv_colmn){
						m_listView.SetColumnWidth(column, _ttol(item.c_str()));
						++column;
						if(column >= count){
							break;
						}
					}
				}
			}
			::GetPrivateProfileString(_T("Aozora"), _T("UpdateDate"), _T(""), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
			if(val[0] == _T('\0')){
				std::tstring str;
				GetAozoraUpdateDate(str);
				_tcsncpy_s(val, str.c_str(), sizeof(val)/sizeof(TCHAR));
			}
			auto hStUpdateD = GetDlgItem(m_hWnd, IDC_STATIC_UPDATE_D);
			SetWindowText(hStUpdateD, val);
		}
		//
		//
		setTreeTop();
		//
		m_editBox.Attach(GetDlgItem(m_hWnd, IDC_EDIT_SEARCH));

		RECT rect;
		GetClientRect(hWnd, &rect);
		setWindowSize(rect.right-rect.left, rect.bottom-rect.top);

		return TRUE;
	}
#define PADDING 10
	void setWindowSize(int cx, int cy)
	{
		if(cx <= 0 || cy <= 0){
			return;
		}
		RECT client_rect = {0,0,cx,cy};
		auto hUpdateBtn     = GetDlgItem(m_hWnd, IDUPDATE           );
		auto hUpdateFileBtn = GetDlgItem(m_hWnd, IDUPDATE_FILE      );
		auto hOpenBtn       = GetDlgItem(m_hWnd, IDOPEN             );
		auto hCancelBtn     = GetDlgItem(m_hWnd, IDCANCEL           );
		auto hBookmarkTree  = GetDlgItem(m_hWnd, IDC_TREE_BM        );
		auto hStUpdateH     = GetDlgItem(m_hWnd, IDC_STATIC_UPDATE_H);
		auto hStUpdateD     = GetDlgItem(m_hWnd, IDC_STATIC_UPDATE_D);
		if(hUpdateBtn && hUpdateFileBtn && hOpenBtn && hCancelBtn && hBookmarkTree && hStUpdateH && hStUpdateD){
			auto rect = client_rect;
			rect.bottom -= PADDING;
			RECT item_rect;
			{
				GetWindowRect(hUpdateBtn, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hUpdateBtn, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
			}
			{
				GetWindowRect(hUpdateFileBtn, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hUpdateFileBtn, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
			}
			{
				GetWindowRect(hOpenBtn, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hOpenBtn, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
			}
			{
				GetWindowRect(hStUpdateH, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hStUpdateH, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
			}
			{
				GetWindowRect(hStUpdateD, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hStUpdateD, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
			}
			{
				GetWindowRect(hCancelBtn, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				MoveWindow(hCancelBtn, rect.right - nWidth - PADDING, rect.bottom - nHeight, nWidth, nHeight, TRUE);

				rect.bottom -= nHeight;
			}
			int right = rect.right;
			{
				auto hSearchBtn = GetDlgItem(m_hWnd, IDSEARCH);
				GetWindowRect(hSearchBtn, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				right = rect.right - nWidth - PADDING;
				MoveWindow(hSearchBtn, right, pos.y, nWidth, nHeight, TRUE);
			}
			{
				auto hSearchEdit = GetDlgItem(m_hWnd, IDC_EDIT_SEARCH);
				GetWindowRect(hSearchEdit, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hSearchEdit, right - nWidth - PADDING, pos.y, nWidth, nHeight, TRUE);
			}
			{
				GetWindowRect(hBookmarkTree, &item_rect);
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				int nWidth  = rect.right - rect.left - PADDING * 2;
				int nHeight = rect.bottom - pos.y - PADDING;
				RECT client_rect = {rect.left + PADDING, pos.y, rect.right - PADDING, rect.bottom - PADDING};
				m_splitWnd.SetWindowSize(&client_rect);
			}
		}
	}
	void OnSize(HWND hwnd, UINT state, int cx, int cy)
	{
		if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
			return;
		}
		setWindowSize(cx, cy);
	}
	void OnDestory()
	{
		RemoveProp(m_hWnd, TXTMIRU_PROP_NAME);
		///
		std::tstring ini_filename;
		CGrTxtFunc::GetBookmarkFolder(ini_filename);
		ini_filename += _T("/aozora.ini");
		TCHAR val[2048];
		WINDOWPLACEMENT wndp = {sizeof(WINDOWPLACEMENT)};
		GetWindowPlacement(m_hWnd, &wndp);
		_stprintf_s(val, _T("%d,%d,%d,%d"), wndp.rcNormalPosition.left, wndp.rcNormalPosition.top, wndp.rcNormalPosition.right, wndp.rcNormalPosition.bottom);
		::WritePrivateProfileString(_T("Aozora"), _T("WindowPos"), val, ini_filename.c_str());
		_stprintf_s(val, _T("%d"), m_splitWnd.GetPosition());
		::WritePrivateProfileString(_T("Aozora"), _T("TreeSplitPos"), val, ini_filename.c_str());
		{
			std::tstring str;
			int count = Header_GetItemCount(m_listView.GetHeader());
			for(int i=0; i<count; ++i){
				_stprintf_s(val, _T("%d,"), m_listView.GetColumnWidth(i));
				str += val;
			}
			::WritePrivateProfileString(_T("Aozora"), _T("ListColumnWidth"), str.c_str(), ini_filename.c_str());
		}
		///
		::PostQuitMessage(0);
	}
private:
	CGrMainWindow()
	{
	}
public:
	LPCTSTR m_lpsCmdLine = nullptr;
};

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CGrMainWindow::theApp().MainWndProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI _tWinMain(_In_ HINSTANCE hCurInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPTSTR lpsCmdLine, _In_ int nCmdShow)
{
	MSG    msg = {};
	CGrTxtMiru::theApp().SetUseDialogMessage(true);

	TCHAR szLanguageName[100];
	auto idLocal = GetSystemDefaultLCID();
	::GetLocaleInfo(idLocal, LOCALE_SENGLANGUAGE, szLanguageName, _countof(szLanguageName));
	_tsetlocale(LC_ALL,szLanguageName);

	UNREFERENCED_PARAMETER(nCmdShow  );
	UNREFERENCED_PARAMETER(lpsCmdLine);
	UNREFERENCED_PARAMETER(hPrevInst );

	CGrMainWindow::theApp().m_lpsCmdLine = lpsCmdLine;
	std::tstring str;
	CGrTxtFunc::GetBookmarkFolder(str);

	if (::OleInitialize(nullptr) == S_OK) {
		;
	}
	if (::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED) == S_OK) {
		;
	}

	TxtMiruTheme_Hook();
	auto hwnd = CGrMainWindow::theApp().Create(hCurInst);
	if(!hwnd){
		return FALSE;
	}
	// アクセラレータテーブル読み込み
	while(::GetMessage(&msg, NULL, 0, 0)){
		if(!CGrMainWindow::theApp().PreTranslateMessage(msg)){
			continue;
		}
		if(CGrTxtMiru::theApp().IsDialogMessage(hwnd, msg)){
			continue;
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	TxtMiruTheme_Unhook();
	::CoUninitialize();
	::OleUninitialize();

	return msg.wParam;
}
