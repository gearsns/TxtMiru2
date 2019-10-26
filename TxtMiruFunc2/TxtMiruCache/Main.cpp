// main.cpp
//
#define STRICT // 型を宣言および使用時に、より厳密な型チェックが行われます。
#define MAIN_H
#pragma warning(disable:4786)

#define OEMRESOURCE
#include <windows.h>
#include <windowsx.h>
#include <mutex>
#include <deque>
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
#include "DragDropImpl.h"
#include "csvtext.h"
#include "KeyState.h"
#include "TxtMiruTheme.h"
#include "JScript.h"
#include "AddURLDlg.h"
#include "MessageBox.h"
#include "Font.h"
#include "PictRendererMgr.h"
#include "ImportDlg.h"
#include "EditBox.h"
#include < stdarg.h>

#define TXTMIRU_PROP_NAME _T("TxtMiru2.0-Cache")
#define TXTMIRU_APP_NAME _T("TxtMiru2.0")

static LPCTSTR l_icon_name_list[2/*TxtFuncBookmark::ii_max*/] = {
	_T("stat_tag_favorite"),
	_T("stat_book_normal")
	};

class CGrMainWindow;
// メインウィンドウ
static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct BookCompareParam {
	int column;
	const std::vector<CacheItem> &item_list;
};
static int CALLBACK bookCompare(LPARAM lp1, LPARAM lp2, LPARAM param)
{
	auto *pParam = reinterpret_cast<BookCompareParam*>(param);
	const auto &item1 = pParam->item_list[lp1];
	const auto &item2 = pParam->item_list[lp2];
	int ret = 0;
	switch (pParam->column) {
	case 0: // IDS_BMH_TITLE
		if (ret == 0) {
			ret = item1.title.compare(item2.title);
		}
		if (ret == 0) {
			ret = item1.url.compare(item2.url);
		}
		break;
	case 1: // IDS_BMH_AUTHOR
		if (ret == 0) {
			ret = item1.author.compare(item2.author);
		}
		if (ret == 0) {
			ret = item1.url.compare(item2.url);
		}
		break;
	case 2: // IDS_BMH_URL
		if (ret == 0) {
			ret = item1.url.compare(item2.url);
		}
		break;
	case 3: // IDS_BMH_UPDATE_DATE
		if (ret == 0) {
			ret = item1.update_date.compare(item2.update_date);
		}
		if (ret == 0) {
			ret = item1.url.compare(item2.url);
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

class CGrCacheListView : public CGrListView
{
public:
	CGrCacheListView()
	{
	}
	virtual ~CGrCacheListView(){}
};

namespace TxtFuncBookmark
{
	enum class ImageIcon {
		ii_none           ,
		ii_stat_tag_favorite           ,
		ii_max
	};
};

class CCacheListropTarget : public CDropTarget
{
private:
	CGrMainWindow &m_mainWnd;
	CGrCacheListView &m_listView;
	CLIPFORMAT CF_TXTMIRUAOZORABOOKSINFO = 0;
	CLIPFORMAT CF_UNIFORMRESOURCELOCATOR = 0;
	CLIPFORMAT CF_NETSCAPEBOOKMARK = 0;
	CLIPFORMAT CF_FILEDESCRIPTOR = 0;
	CLIPFORMAT CF_FILECONTENTS = 0;
	CLIPFORMAT CF_SHELLURL = 0;
private:
	LPCTSTR getNextData(LPCTSTR lpSrc, LPCTSTR lpEnd)
	{
		for (; *lpSrc; ++lpSrc) {
			if (lpSrc >= lpEnd) {
				return nullptr;
			}
		}
		return lpSrc + 1;
	}
public:
	CCacheListropTarget(HWND hTargetWnd, CGrMainWindow &mainWnd);

	void AddSuportedFormats();
	virtual HRESULT __stdcall DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
	{
		m_listView.DragMode(true);
		return CDropTarget::DragEnter(pDataObject, grfKeyState, ptl, pdwEffect);
	}
	virtual HRESULT __stdcall DragLeave()
	{
		m_listView.DragMode(false);
		return CDropTarget::DragLeave();
	}
	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
	{
		CDropTarget::DragOver(grfKeyState, pt, pdwEffect);
		if (*pdwEffect != DROPEFFECT_NONE) {
			// サポートされている形式の時のみ
			LVHITTESTINFO ht;
			ht.pt.x = pt.x;
			ht.pt.y = pt.y;
			ScreenToClient(m_hTargetWnd, &ht.pt);
			*pdwEffect = DROPEFFECT_NONE;
			int iItem = ListView_HitTest(m_hTargetWnd, &ht);
			if (iItem < 0) {
				*pdwEffect = DROPEFFECT_MOVE;
				iItem = ListView_GetItemCount(m_hTargetWnd);
			}
			else {
				*pdwEffect = DROPEFFECT_MOVE;
				RECT rect;
				ListView_GetItemRect(m_hTargetWnd, iItem, &rect, LVIR_BOUNDS);
				if (ht.pt.y > rect.top + (rect.bottom - rect.top) / 2) {
					++iItem;
				}
			}
			m_listView.SetDrawInsertPos(iItem);
		}
		return S_OK;
	}
	virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect);
};

//////////////////////////////////////////////////
static std::mutex l_mtx;
class CGrMainWindow
{
	friend CCacheListropTarget;
#define WM_APPEND_LOG_MESSAGE (WM_APP + 1)
public:
	static void PostWokerMessage(LPCTSTR message)
	{
		size_t size = 0;
		auto &&main_window = CGrMainWindow::theApp();
		do {
			std::lock_guard<std::mutex> lock(l_mtx);
			size = main_window.m_wokerLogMessage.size();
			main_window.m_wokerLogMessage.push_back(message);
		} while (0);
		if (size == 0) {
			// すでに要求済みの場合は、要求しなおさない。
			PostMessage(main_window.m_hWnd, WM_APPEND_LOG_MESSAGE, 0, 0);
		}
	}
private:

	CDropTarget *m_pDropTarget = nullptr;
	int m_win_size_x_min = 0;
	int m_win_size_y_min = 0;
	HINSTANCE m_hInstance = NULL;
	const int listwnd_id = 3000;
	HWND m_hWnd = NULL;
	CGrCacheListView m_listView;
	std::vector<CacheItem> m_item_list;
	CGrWinCtrl m_editBox;
	HIMAGELIST m_hImg = NULL;

	enum class WorkerType {
		wt_update,
		wt_import
	};
	bool m_bWorkerRunnging = false;
	HANDLE m_workerThread = INVALID_HANDLE_VALUE;
	unsigned int m_workerThreadId = 0;
	struct WorkerInfo {
		WorkerType wt = WorkerType::wt_update;
		std::tstring url;
	};
	std::deque<WorkerInfo> m_workerQueue;
	std::deque<std::tstring> m_wokerLogMessage;
	static void PostWokerFormatMessage(LPCTSTR lpFmt, ...)
	{
		do {
			va_list args;
			va_start(args, lpFmt);
			wchar_t buf[255];
			if (_vstprintf_p(buf, _countof(buf), lpFmt, args) >= 0) {
				PostWokerMessage(buf);
			}
			else {
				auto n = _vsctprintf(lpFmt, args) + 1;
				std::tstring s(n + 1, 0);
				_vsntprintf_s(&(s[0]), s.size(), s.size(), lpFmt, args);
				PostWokerMessage(s.c_str());
			}
			va_end(args);
		} while (0);
	}
	static unsigned int __stdcall WorkerThread(void *arg)
	{
		if (::OleInitialize(NULL) == S_OK) {
			;
		}
		if (::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) == S_OK) {
			;
		}
		auto pMain = reinterpret_cast<CGrMainWindow*>(arg);
		bool bFirst = true;
		while (pMain->m_bWorkerRunnging) {
			bool bEmpty = false;
			WorkerInfo info;
			{
				std::lock_guard<std::mutex> lock(l_mtx);
				auto &queue = pMain->m_workerQueue;
				if (queue.empty()) {
					bEmpty = true;
				}
				else {
					info = queue.front();
					queue.pop_front();
				}
			}
			if (bEmpty) {
				if (pMain->m_bWorkerRunnging) {
					bFirst = true;
					SuspendThread(GetCurrentThread());
				}
			}
			else {
				if (!bFirst) {
					Sleep(1000);
				}
				bFirst = false;
				PostWokerFormatMessage(_T("%s\r\n"), info.url.c_str());
				std::tstring errstr;
				if (!MakeCache(errstr, info.url.c_str())) {
					if (errstr.size() == 0) {
						PostWokerFormatMessage(_T("Error:%s\r\n"), info.url.c_str());
					}
					else {
						PostWokerFormatMessage(_T("Error:%s\r\n [%s]\r\n"), info.url.c_str(), errstr.c_str());
					}
				}
				else {
					PostWokerFormatMessage(_T("Done:%s\r\n"), info.url.c_str());
				}
			}
		}
		::CoUninitialize();
		::OleUninitialize();
		_endthreadex(0);
		return 0;
	}
	bool TerminateWokerThread()
	{
		{
			std::lock_guard<std::mutex> lock(l_mtx);
			m_bWorkerRunnging = false;
			m_workerQueue.clear();
		}
		if (INVALID_HANDLE_VALUE != m_workerThread) {
			DWORD dwThread;
			GetExitCodeThread(m_workerThread, &dwThread);
			if (dwThread == STILL_ACTIVE) {
				ResumeThread(m_workerThread);
				WaitForSingleObject(m_workerThread, INFINITE);
			}
			CloseHandle(m_workerThread);
			m_workerThread = INVALID_HANDLE_VALUE;
			m_workerThreadId = 0;
		}
		return true;
	}
public:
	bool PostWorkerQueue(LPCTSTR lpUrl, WorkerType wt = WorkerType::wt_update)
	{
		{
			std::lock_guard<std::mutex> lock(l_mtx);
			// WokerThreadで処理中なら追加しない
			for (const auto &item : m_workerQueue) {
				if (item.wt == wt && item.url == lpUrl) {
					return false;
				}
			}
			m_workerQueue.push_back({ wt, lpUrl });
		}
		if (INVALID_HANDLE_VALUE == m_workerThread) {
			m_bWorkerRunnging = true;
			m_workerThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, WorkerThread, this, 0, &m_workerThreadId));
			if (INVALID_HANDLE_VALUE == m_workerThread) {
				return false;
			}
		}
		else {
			ResumeThread(m_workerThread);
		}
		// createevent
		// mutex
		// std::deque
		return true;
	}
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
		if (m_pDropTarget) {
			delete m_pDropTarget;
		}
		DeleteAllItem();
	}
	HWND Create(HINSTANCE hInstance)
	{
		m_hInstance = hInstance;
		WNDCLASSEX wc = { sizeof(WNDCLASSEX) };

		wc.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wc.hIcon = static_cast<HICON>(LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP)));
		wc.hIconSm = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
		wc.lpszClassName = _T("TxtMiruCache");
		wc.cbWndExtra = DLGWINDOWEXTRA;
		wc.hInstance = hInstance;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = ::MainWndProc;

		if (!RegisterClassEx(&wc)){
			return NULL;
		}

		return CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, reinterpret_cast<DLGPROC>(::MainWndProc));
	}
	LRESULT MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg){
		case WM_APPEND_LOG_MESSAGE:
			{
				auto hEdit = GetDlgItem(hWnd, IDC_EDIT_LOG);
				if (hEdit) {
					CGrEditBox editbox;
					editbox.SetWnd(hEdit);
					std::lock_guard<std::mutex> lock(l_mtx);
					std::tstring buffer;
					for (const auto &str : m_wokerLogMessage) {
						buffer += str;
					}
					auto l = editbox.GetTextLength();
					editbox.SetSel(l, l);
					editbox.ReplaceSel(buffer.c_str());
					m_wokerLogMessage.clear();
					auto hStopBtn = GetDlgItem(m_hWnd, IDSTOP);
					if (hStopBtn) {
						auto &queue = m_workerQueue;
						if (queue.empty()) {
							EnableWindow(hStopBtn, FALSE);
						}
						else {
							EnableWindow(hStopBtn, TRUE);
						}
					}
				}
			}
			break;
			HANDLE_MSG(hWnd, WM_COMMAND          , OnCommand    );
			HANDLE_MSG(hWnd, WM_INITDIALOG, OnInitDialog);
			HANDLE_MSG(hWnd, WM_SIZE             , OnSize       );
		case WM_CLOSE:
			TerminateWokerThread();
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
					case LVN_COLUMNCLICK:
						{
							auto lpnmlist = reinterpret_cast<LPNMLISTVIEW>(lParam);
							BookCompareParam param = { lpnmlist->iSubItem, m_item_list };
							m_listView.SortItems(bookCompare, reinterpret_cast<LPARAM>(&param));
						}
						break;
					}
				}
			}
			break;
		case WM_SETTINGCHANGE:
			if (TxtMiruTheme_IsImmersiveColorSet(lParam)) {
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
	void setLatestList()
	{
		m_listView.SetRedraw(false);
		m_listView.DeleteAllItems();
		m_item_list.clear();
		CGrDBCache db;
		if (!db.Open()) {
			return;
		}
		db.GetLatest(m_item_list);
		int i = 0;
		for (const auto &cache : m_item_list) {
			LVITEM item = { };
			item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
			item.iItem = m_listView.GetItemCount();
			item.lParam = i;
			item.iSubItem = 0;
			item.iImage = 1;
			item.pszText = const_cast<LPTSTR>(cache.title.c_str());
			item.iItem = m_listView.InsertItem(&item);
			//
			m_listView.SetItemText(item.iItem, 1, cache.author.c_str());
			m_listView.SetItemText(item.iItem, 2, cache.url.c_str());
			m_listView.SetItemText(item.iItem, 3, cache.update_date.c_str());

			++i;
		}
		m_listView.SetRedraw(true);
	}
	void setSearchList(LPCTSTR lpWord)
	{
		m_listView.SetRedraw(false);
		m_listView.DeleteAllItems();
		m_item_list.clear();
		CGrDBCache db;
		if (!db.Open()) {
			return;
		}
		db.GetSearchWord(m_item_list, lpWord);
		int i = 0;
		for (const auto &cache : m_item_list) {
			LVITEM item = { };
			item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
			item.iItem = m_listView.GetItemCount();
			item.lParam = i;
			item.iSubItem = 0;
			item.iImage = 1;
			item.pszText = const_cast<LPTSTR>(cache.title.c_str());
			item.iItem = m_listView.InsertItem(&item);
			//
			m_listView.SetItemText(item.iItem, 1, cache.author.c_str());
			m_listView.SetItemText(item.iItem, 2, cache.url.c_str());
			m_listView.SetItemText(item.iItem, 3, cache.update_date.c_str());

			++i;
		}
		m_listView.SetRedraw(true);
	}
	void Refresh()
	{
		setLatestList();
	}

	void DeleteAllItem()
	{
	}
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		switch(id){
		case IDSTOP:
			{
				auto hStopBtn = GetDlgItem(m_hWnd, IDSTOP);
				EnableWindow(hStopBtn, FALSE);
				TerminateWokerThread();
			}
			break;
		case IDCLEAR_LOG:
			{
				auto hEdit = GetDlgItem(hwnd, IDC_EDIT_LOG);
				if (hEdit) {
					CGrEditBox editbox;
					editbox.SetWnd(hEdit);
					editbox.SetText(L"");
				}
			}
			break;
		case IDCANCEL:
			break;
		case IDADD:
			{
				CGrAddURLDlg dlg;
				if (IDOK == dlg.DoModal(m_hInstance, hwnd)) {
					std::tstring url;
					dlg.Get(url);
					if (url.length() <= 0) {
						CGrMessageBox::Show(m_hWnd, IDS_ERROR_MAKECACHE, TXTMIRU_APP_NAME);
						break;
					}
					PostWorkerQueue(url.c_str());
				}
			}
			break;
		case IDUPDATE:
			{
				auto cnt = m_listView.GetSelectedCount();
				if (cnt > 0) {
					int iItem = -1;
					while ((iItem = m_listView.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED)) != -1) {
						LVITEM item = { LVIF_IMAGE };
						item.iItem = iItem;
						if (FALSE == m_listView.GetItem(&item)) {
							continue;
						}
						//
						auto index = m_listView.GetParam(iItem);
						if (index < 0 || index >= static_cast<signed int>(m_item_list.size())) {
							continue;
						}
						auto &cache_item = m_item_list[index];
						if (cache_item.url.size() == 0) {
							continue;
						}
						PostWorkerQueue(cache_item.url.c_str());
					}
				}
				else {
					CGrMessageBox::Show(m_hWnd, IDS_MESSAGE_UPDATE_SELECT, TXTMIRU_APP_NAME);
				}
			}
			break;
		case IDDELETE:
			{
				auto cnt = m_listView.GetSelectedCount();
				if (cnt > 0) {
					if (IDYES != CGrMessageBox::Show(m_hWnd, IDS_CONF_DELETE, TXTMIRU_APP_NAME, MB_YESNO)) {
						break;
					}
					auto bRet = true;
					int iItem = -1;
					m_listView.SetRedraw(FALSE);
					while ((iItem = m_listView.GetNextItem(iItem, LVNI_ALL | LVNI_SELECTED)) != -1) {
						LVITEM item = { LVIF_IMAGE };
						item.iItem = iItem;
						if (FALSE == m_listView.GetItem(&item)) {
							continue;
						}
						//
						auto index = m_listView.GetParam(iItem);
						if (index < 0 || index >= static_cast<signed int>(m_item_list.size())) {
							continue;
						}
						auto &cache_item = m_item_list[index];
						if (cache_item.url.size() == 0) {
							continue;
						}
						{
							auto err = DeleteCache(cache_item.url.c_str());
							if (err != 0) {
								bRet = false;
								PostWokerFormatMessage(_T("Delete Error:%s\r\n"), cache_item.url.c_str());
								break;
							}
							PostWokerFormatMessage(_T("Delete:%s\r\n"), cache_item.url.c_str());
						}
						//
					}
					m_listView.SetRedraw(TRUE);
					if (bRet) {
						CGrMessageBox::Show(m_hWnd, IDS_MESSAGE_DELETE_OK, TXTMIRU_APP_NAME);
					}
					else {
						CGrMessageBox::Show(m_hWnd, IDS_MESSAGE_DELETE_NG, TXTMIRU_APP_NAME);
					}
					Refresh();
				}
				else {
					CGrMessageBox::Show(m_hWnd, IDS_MESSAGE_DELETE_SELECT, TXTMIRU_APP_NAME);
				}

			}
			break;
		case IDIMPORT:
			{
				CGrImportDlg dlg;
				if (IDOK == dlg.DoModal(m_hInstance, hwnd)) {
				}
			}
			break;
		case IDSEARCH:
			{
				auto hSearchEdit = GetDlgItem(m_hWnd, IDC_EDIT_SEARCH);
				int len = Edit_GetTextLength(hSearchEdit);
				if(len <= 0){
					setLatestList();
					break;
				}
				++len;
				auto *buf = new TCHAR[len+1];
				Edit_GetText(hSearchEdit, buf, len);

				setSearchList(buf);

				delete [] buf;
			}
			break;
		case ID_EXIT:
			::PostMessage(m_hWnd, WM_CLOSE, 0, 0L);
			break;
		}
	}
	BOOL OnInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lParam)
	{
		m_hWnd = hWnd;
		SetProp(m_hWnd, TXTMIRU_PROP_NAME, reinterpret_cast<HANDLE>(-1));
		{
			RECT rect;
			GetWindowRect(hWnd, &rect);
			m_win_size_x_min = rect.right - rect.left;
			m_win_size_y_min = rect.bottom - rect.top;
		}
		SendMessage(hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_APP))));
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR)));
		//
		auto hListWnd = CreateWindow(_T("syslistview32"),
									 NULL,
									 WS_BORDER | WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ALIGNLEFT | WS_TABSTOP,
									 0,0,10,10,m_hWnd, reinterpret_cast<HMENU>(listwnd_id), m_hInstance,NULL);
		//
		{
			LOGFONT logfont = { 0 };
			GetObject(GetWindowFont(hListWnd), sizeof(logfont), &logfont);
			logfont.lfHeight *= 2;
			CGrFont font;
			font.CreateFontIndirect(logfont);
			int height = font.Height();
			/*HIMAGELIST*/m_hImg = ImageList_Create(height, height, ILC_COLOR32, 1, 1);
			//
			CGrBitmap bmp_list;
			CGrBitmap bmp_tree;
			bmp_list.Create(height*2/*TxtFuncBookmark::ii_max*/, height);
			{
				{
					auto color = GetSysColor(COLOR_WINDOW);
					int r = GetRValue(color);
					int g = GetGValue(color);
					int b = GetBValue(color);
					{
						int l = height * 2/*TxtFuncBookmark::ii_max*/ * height;
						auto lpRGB = bmp_list.GetBits();
						for (; l > 0; --l, ++lpRGB) {
							lpRGB->rgbRed = r;
							lpRGB->rgbGreen = g;
							lpRGB->rgbBlue = b;
							lpRGB->rgbReserved = 0;
						}
					}
				}
				CGrPictRendererMgr pictMgr;
				pictMgr.Initialize();
				auto lpDir = CGrTxtMiru::GetDataPath();
				TCHAR curPath[MAX_PATH];
				CGrShell::GetExePath(curPath);
				for (int i = 0; i < 2/*TxtFuncBookmark::ii_max*/; ++i) {
					TCHAR path[_MAX_PATH] = {};
					_stprintf_s(path, _T("%s\\toolbar_icon\\list_icon_%s"), lpDir, l_icon_name_list[i]);
					std::tstring image_filename;
					if (pictMgr.GetSupportedFile(image_filename, path)) {
						pictMgr.Draw(bmp_list, i*height, 0, height, height, image_filename.c_str());
					}
					else {
						_stprintf_s(path, _T("%s\\toolbar_icon\\list_icon_%s"), curPath, l_icon_name_list[i]);
						if (pictMgr.GetSupportedFile(image_filename, path)) {
							pictMgr.Draw(bmp_list, i*height, 0, height, height, image_filename.c_str());
						}
					}
				}
			}
			ImageList_Add(m_hImg, bmp_list, NULL);
		}
		//
		m_listView.Attach(hListWnd);
		m_listView.SetImageList(m_hImg, LVSIL_SMALL);
		m_listView.SetExtendedListViewStyle(m_listView.GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);
		TxtMiruTheme_SetWindowTheme(m_listView, L"Explorer", NULL);
		ListView_SetBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
		ListView_SetTextColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
		ListView_SetTextBkColor(m_listView, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
		//
		SendMessage(m_listView, WM_CHANGEUISTATE, MAKELONG(UIS_SET, UISF_HIDEFOCUS), 0);
		std::tstring str_name;
		//
		TxtMiruTheme_SetWindowSubclass(m_hWnd);
		TxtMiruTheme_UpdateDarkMode(m_hWnd);
		//

		LVCOLUMN lvc = {};
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;

		CGrText::LoadString(IDS_BMH_TITLE, str_name);
		lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
		lvc.iSubItem = 0;
		lvc.cx = 400;
		m_listView.InsertColumn(lvc.iSubItem, &lvc);
		//
		CGrText::LoadString(IDS_BMH_AUTHOR, str_name);
		lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
		lvc.iSubItem = 1;
		lvc.cx = 200;
		m_listView.InsertColumn(lvc.iSubItem, &lvc);
		//
		CGrText::LoadString(IDS_BMH_URL, str_name);
		lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
		lvc.iSubItem = 2;
		lvc.cx = 400;
		m_listView.InsertColumn(lvc.iSubItem, &lvc);
		//
		CGrText::LoadString(IDS_BMH_UPDATE_DATE, str_name);
		lvc.pszText = const_cast<LPTSTR>(str_name.c_str());
		lvc.iSubItem = 3;
		lvc.cx = 200;
		m_listView.InsertColumn(lvc.iSubItem, &lvc);
		//
		{
			std::tstring ini_filename(CGrTxtMiru::GetDataPath());
			ini_filename += _T("/cache.ini");
			TCHAR val[2048] = {};
			::GetPrivateProfileString(_T("Cache"), _T("WindowPos"), _T(""), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
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
			::GetPrivateProfileString(_T("Cache"), _T("ListColumnWidth"), _T("-1"), val, sizeof(val)/sizeof(TCHAR), ini_filename.c_str());
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
		}
		//
		//
		m_editBox.Attach(GetDlgItem(m_hWnd, IDC_EDIT_SEARCH));
		{
			auto hEdit = GetDlgItem(hWnd, IDC_EDIT_LOG);
			if (hEdit) {
				CGrEditBox editbox;
				editbox.SetWnd(hEdit);
				editbox.LimitText(0);
			}
		}
		RECT rect;
		GetClientRect(hWnd, &rect);
		setWindowSize(rect.right-rect.left, rect.bottom-rect.top);
		//
		setLatestList();
		//
		m_pDropTarget = new CCacheListropTarget(m_hWnd, *this);
		if (FAILED(RegisterDragDrop(m_listView, m_pDropTarget))) {
			delete m_pDropTarget;
			m_pDropTarget = nullptr;
		}
		else {
			static_cast<CCacheListropTarget*>(m_pDropTarget)->AddSuportedFormats();
		}

		return TRUE;
	}
#define PADDING 10
	void setWindowSize(int cx, int cy)
	{
		if(cx <= 0 || cy <= 0){
			return;
		}
		RECT client_rect = {0,0,cx,cy};
		auto hAddBtn = GetDlgItem(m_hWnd, IDADD);
		auto hUpdateBtn = GetDlgItem(m_hWnd, IDUPDATE);
		auto hDeleteBtn = GetDlgItem(m_hWnd, IDDELETE);
		auto hCancelBtn = GetDlgItem(m_hWnd, ID_EXIT);
		auto hImportBtn = GetDlgItem(m_hWnd, IDIMPORT);
		if(hAddBtn && hUpdateBtn && hCancelBtn && hImportBtn){
			auto rect = client_rect;
			rect.bottom -= PADDING;
			RECT item_rect;
			{
				GetWindowRect(hAddBtn, &item_rect);
				int nWidth = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = { item_rect.left,item_rect.top };
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hAddBtn, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
			}
			{
				GetWindowRect(hUpdateBtn, &item_rect);
				int nWidth = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = { item_rect.left,item_rect.top };
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hUpdateBtn, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
			}
			{
				GetWindowRect(hDeleteBtn, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hDeleteBtn, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
			}
			{ // 2.0.40.0
				GetWindowRect(hImportBtn, &item_rect);
				int nWidth = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = { item_rect.left,item_rect.top };
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hImportBtn, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
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
				auto hListFrame = GetDlgItem(m_hWnd, IDC_STATIC_LIST);
				GetWindowRect(hListFrame, &item_rect);
				POINT pos = { item_rect.left,item_rect.top };
				ScreenToClient(m_hWnd, &pos);
				int nWidth = client_rect.right - pos.x * 2;
				int nHeight = rect.bottom - pos.y - pos.x;
				MoveWindow(m_listView, pos.x, pos.y, nWidth, nHeight, TRUE);
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
		std::tstring ini_filename(CGrTxtMiru::GetDataPath());
		ini_filename += _T("/cache.ini");
		TCHAR val[2048];
		WINDOWPLACEMENT wndp = {sizeof(WINDOWPLACEMENT)};
		GetWindowPlacement(m_hWnd, &wndp);
		_stprintf_s(val, _T("%d,%d,%d,%d"), wndp.rcNormalPosition.left, wndp.rcNormalPosition.top, wndp.rcNormalPosition.right, wndp.rcNormalPosition.bottom);
		::WritePrivateProfileString(_T("Cache"), _T("WindowPos"), val, ini_filename.c_str());
		{
			std::tstring str;
			int count = Header_GetItemCount(m_listView.GetHeader());
			for(int i=0; i<count; ++i){
				_stprintf_s(val, _T("%d,"), m_listView.GetColumnWidth(i));
				str += val;
			}
			::WritePrivateProfileString(_T("Cache"), _T("ListColumnWidth"), str.c_str(), ini_filename.c_str());
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
	MSG msg = {};
	CGrTxtMiru::theApp().SetUseDialogMessage(true);

	TCHAR szLanguageName[100];
	auto idLocal = GetSystemDefaultLCID();
	::GetLocaleInfo(idLocal, LOCALE_SENGLANGUAGE, szLanguageName, _countof(szLanguageName));
	_tsetlocale(LC_ALL,szLanguageName);

	UNREFERENCED_PARAMETER(nCmdShow  );
	UNREFERENCED_PARAMETER(lpsCmdLine);
	UNREFERENCED_PARAMETER(hPrevInst );

	CGrMainWindow::theApp().m_lpsCmdLine = lpsCmdLine;

	if (::OleInitialize(NULL) == S_OK) {
		;
	}
	if (::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) == S_OK) {
		;
	}

	TxtMiruTheme_Hook();
	LPVOID lpMsgBuf;

	SetLastError(NO_ERROR);		//エラー情報をクリアする
	auto hwnd = CGrMainWindow::theApp().Create(hCurInst);
	if(!hwnd){

		//ここにチェックしたい処理を書く

		FormatMessage(				//エラー表示文字列作成
									FORMAT_MESSAGE_ALLOCATE_BUFFER |
									FORMAT_MESSAGE_FROM_SYSTEM |
									FORMAT_MESSAGE_IGNORE_INSERTS,
									NULL, GetLastError(),
									MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
									(LPTSTR)&lpMsgBuf, 0, NULL);

		MessageBox(NULL, (LPCTSTR)lpMsgBuf, NULL, MB_OK);	//メッセージ表示

		LocalFree(lpMsgBuf);
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

//////////////
CCacheListropTarget::CCacheListropTarget(HWND hTargetWnd, CGrMainWindow &mainWnd) : CDropTarget(hTargetWnd), m_mainWnd(mainWnd), m_listView(mainWnd.m_listView) {}
void CCacheListropTarget::AddSuportedFormats()
{
	FORMATETC ftetc = { 0 };
	ftetc.dwAspect = DVASPECT_CONTENT;
	ftetc.lindex = -1;
	ftetc.tymed = TYMED_HGLOBAL;
	CF_TXTMIRUAOZORABOOKSINFO = RegisterClipboardFormat(_T("TxtMiruAozoraBooksInfo")); ftetc.cfFormat = CF_TXTMIRUAOZORABOOKSINFO; AddSuportedFormat(ftetc);
	CF_UNIFORMRESOURCELOCATOR = RegisterClipboardFormat(_T("UniformResourceLocator")); ftetc.cfFormat = CF_UNIFORMRESOURCELOCATOR; AddSuportedFormat(ftetc);
	CF_NETSCAPEBOOKMARK = RegisterClipboardFormat(_T("Netscape Bookmark")); ftetc.cfFormat = CF_NETSCAPEBOOKMARK; AddSuportedFormat(ftetc);
	/*                                                                              */ ftetc.cfFormat = CF_HDROP; AddSuportedFormat(ftetc);
	CF_SHELLURL = RegisterClipboardFormat(CFSTR_SHELLURL); ftetc.cfFormat = CF_SHELLURL; AddSuportedFormat(ftetc);
	CF_FILEDESCRIPTOR = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR); ftetc.cfFormat = CF_FILEDESCRIPTOR; AddSuportedFormat(ftetc);
}
bool CCacheListropTarget::OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
{
	if (medium.tymed == TYMED_HGLOBAL) {
		if (pFmtEtc->cfFormat == CF_TXTMIRUAOZORABOOKSINFO) {
			do {
				int text_size = GlobalSize(medium.hGlobal) / sizeof(TCHAR);
				auto lpText = static_cast<LPCTSTR>(GlobalLock(medium.hGlobal));
				if (lpText) {
					*pdwEffect = DROPEFFECT_NONE;
					auto lpEnd = &lpText[text_size];
					while (true) {
						auto lpUrl = lpText; lpText = getNextData(lpText, lpEnd); if (!lpText) { break; }
						auto lpTitle = lpText; lpText = getNextData(lpText, lpEnd); if (!lpText) { break; }
						auto lpAuthor = lpText; lpText = getNextData(lpText, lpEnd); if (!lpText) { break; }
						if (lpText >= lpEnd) {
							break;
						}
						m_mainWnd.PostWorkerQueue(lpUrl);
					}
				}
				GlobalUnlock(medium.hGlobal);
			} while (0);
		}
		else if (/**/pFmtEtc->cfFormat == CF_UNIFORMRESOURCELOCATOR
				 || pFmtEtc->cfFormat == CF_NETSCAPEBOOKMARK) {
			// ブラウザからLINK D&D
			std::tstring url;
			if (CGrText::MultiByteToTString(CP_THREAD_ACP, static_cast<LPCSTR>(GlobalLock(medium.hGlobal)), -1, url)) {
				m_mainWnd.PostWorkerQueue(url.c_str());
			}
			GlobalUnlock(medium.hGlobal);
		}
		else if (pFmtEtc->cfFormat == CF_FILECONTENTS) {
			// ZipフォルダなどからDropされた仮想ファイル
			// NOP
		}
		else if (pFmtEtc->cfFormat == CF_FILEDESCRIPTOR
				 || pFmtEtc->cfFormat == CF_SHELLURL
				 || pFmtEtc->cfFormat == CF_HDROP) {
			// ドロップされたものがURL
			TCHAR fileName[_MAX_PATH];
			auto len = ::DragQueryFile(static_cast<HDROP>(medium.hGlobal), 0, fileName, _countof(fileName));
			if (len == 0) {
			}
			else {
				// InternetShortcut対応
				TCHAR buf[_MAX_PATH];
				if (GetPrivateProfileString(_T("InternetShortcut"), _T("URL"), _T(""), buf, sizeof(buf) / sizeof(TCHAR), fileName)) {
					m_mainWnd.PostWorkerQueue(buf);
				}
				else {
					m_mainWnd.PostWorkerQueue(fileName);
				}
			}
		}
		else if (pFmtEtc->cfFormat == CF_UNICODETEXT/*CF_TEXT*/) {
			// NOP
		}
	}
	m_listView.DragMode(false);
	return true; //let base free the medium
}

void PostWokerMessage(LPCTSTR message) {
	CGrMainWindow::PostWokerMessage(message);
}
bool PostWorkerQueue(LPCTSTR lpUrl)
{
	return CGrMainWindow::theApp().PostWorkerQueue(lpUrl);
}