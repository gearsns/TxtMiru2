#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "BookDlg.h"
#include "TxtFunc.h"
#include "MessageBox.h"
#include "Text.h"
#include "Shell.h"
#include "BookmarkDB.h"
#include "BookListDlg.h"
#include "TxtMiruTheme.h"

class CImageDropTarget : public CDropTarget
{
private:
	CGrCover &m_cover;
	CLIPFORMAT CF_UNIFORMRESOURCELOCATOR = 0;
	CLIPFORMAT CF_NETSCAPEBOOKMARK       = 0;
	CLIPFORMAT CF_FILEDESCRIPTOR         = 0;
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
	CImageDropTarget(CGrCover &cover)
	: CDropTarget(cover), m_cover(cover)
	{
	}

	void AddSuportedFormats()
	{
		FORMATETC ftetc = {0};
		ftetc.dwAspect = DVASPECT_CONTENT;
		ftetc.lindex   = -1;
		ftetc.tymed    = TYMED_HGLOBAL;
		CF_UNIFORMRESOURCELOCATOR = RegisterClipboardFormat(_T("UniformResourceLocator")); ftetc.cfFormat = CF_UNIFORMRESOURCELOCATOR; AddSuportedFormat(ftetc);
		CF_NETSCAPEBOOKMARK       = RegisterClipboardFormat(_T("Netscape Bookmark"     )); ftetc.cfFormat = CF_NETSCAPEBOOKMARK      ; AddSuportedFormat(ftetc);
		/*                                                                              */ ftetc.cfFormat = CF_HDROP                 ; AddSuportedFormat(ftetc);
		CF_SHELLURL               = RegisterClipboardFormat(CFSTR_SHELLURL              ); ftetc.cfFormat = CF_SHELLURL              ; AddSuportedFormat(ftetc);
		CF_FILEDESCRIPTOR         = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR        ); ftetc.cfFormat = CF_FILEDESCRIPTOR        ; AddSuportedFormat(ftetc);
	}

	virtual HRESULT __stdcall DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
	{
		return CDropTarget::DragEnter(pDataObject, grfKeyState, ptl, pdwEffect);
	}
	virtual HRESULT __stdcall DragLeave()
	{
		return CDropTarget::DragLeave();
	}
	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
	{
		CDropTarget::DragOver(grfKeyState, pt, pdwEffect);
		if(*pdwEffect != DROPEFFECT_NONE){
			// サポートされている形式の時のみ
			*pdwEffect = DROPEFFECT_COPY;
		}
		return S_OK;
	}
	virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
	{
		if(medium.tymed == TYMED_HGLOBAL){
			std::tstring url;
			if(/**/pFmtEtc->cfFormat == CF_UNIFORMRESOURCELOCATOR
			   ||  pFmtEtc->cfFormat == CF_NETSCAPEBOOKMARK      ){
				// ブラウザからLINK D&D
				if(CGrText::MultiByteToTString(CP_THREAD_ACP, static_cast<LPCSTR>(GlobalLock(medium.hGlobal)), -1, url)){
					;
				}
				GlobalUnlock(medium.hGlobal);
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
						url = buf;
					} else {
						url = fileName;
					}
				}
			} else if(pFmtEtc->cfFormat ==CF_UNICODETEXT/*CF_TEXT*/){
				// NOP
				if(CGrText::MultiByteToTString(CP_THREAD_ACP, static_cast<LPCSTR>(GlobalLock(medium.hGlobal)), -1, url)){
					;
				}
				GlobalUnlock(medium.hGlobal);
			}
			if(url.size() > 0){
				m_cover.Load(url.c_str());
			}
		}
		return true; //let base free the medium
	}
};
class CURLDropTarget : public CDropTarget
{
private:
	CGrEditBox &m_editbox;
	CLIPFORMAT CF_UNIFORMRESOURCELOCATOR = 0;
	CLIPFORMAT CF_NETSCAPEBOOKMARK       = 0;
	CLIPFORMAT CF_FILEDESCRIPTOR         = 0;
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
	CURLDropTarget(CGrEditBox &editbox)
	: CDropTarget(editbox), m_editbox(editbox)
	{
	}

	void AddSuportedFormats()
	{
		FORMATETC ftetc = {0};
		ftetc.dwAspect = DVASPECT_CONTENT;
		ftetc.lindex   = -1;
		ftetc.tymed    = TYMED_HGLOBAL;
		CF_UNIFORMRESOURCELOCATOR = RegisterClipboardFormat(_T("UniformResourceLocator")); ftetc.cfFormat = CF_UNIFORMRESOURCELOCATOR; AddSuportedFormat(ftetc);
		CF_NETSCAPEBOOKMARK       = RegisterClipboardFormat(_T("Netscape Bookmark"     )); ftetc.cfFormat = CF_NETSCAPEBOOKMARK      ; AddSuportedFormat(ftetc);
		/*                                                                              */ ftetc.cfFormat = CF_HDROP                 ; AddSuportedFormat(ftetc);
		CF_SHELLURL               = RegisterClipboardFormat(CFSTR_SHELLURL              ); ftetc.cfFormat = CF_SHELLURL              ; AddSuportedFormat(ftetc);
		CF_FILEDESCRIPTOR         = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR        ); ftetc.cfFormat = CF_FILEDESCRIPTOR        ; AddSuportedFormat(ftetc);
	}

	virtual HRESULT __stdcall DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
	{
		return CDropTarget::DragEnter(pDataObject, grfKeyState, ptl, pdwEffect);
	}
	virtual HRESULT __stdcall DragLeave()
	{
		return CDropTarget::DragLeave();
	}
	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
	{
		CDropTarget::DragOver(grfKeyState, pt, pdwEffect);
		if(*pdwEffect != DROPEFFECT_NONE){
			// サポートされている形式の時のみ
			*pdwEffect = DROPEFFECT_COPY;
		}
		return S_OK;
	}
	virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
	{
		if(medium.tymed == TYMED_HGLOBAL){
			std::tstring url;
			if(/**/pFmtEtc->cfFormat == CF_UNIFORMRESOURCELOCATOR
			   ||  pFmtEtc->cfFormat == CF_NETSCAPEBOOKMARK      ){
				// ブラウザからLINK D&D
				if(CGrText::MultiByteToTString(CP_THREAD_ACP, static_cast<LPCSTR>(GlobalLock(medium.hGlobal)), -1, url)){
				}
				GlobalUnlock(medium.hGlobal);
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
						url = buf;
					} else {
						url = fileName;
					}
				}
			} else if(pFmtEtc->cfFormat ==CF_UNICODETEXT/*CF_TEXT*/){
				if(CGrText::MultiByteToTString(CP_THREAD_ACP, static_cast<LPCSTR>(GlobalLock(medium.hGlobal)), -1, url)){
				}
				GlobalUnlock(medium.hGlobal);
			}
			if(url.size() > 0){
				m_editbox.SetText(url.c_str());
			}
		}
		return true; //let base free the medium
	}
};

CGrBookDlg::CGrBookDlg(TxtFuncBookmark::Book &book, int parent, int pre_id)
: m_book(book), m_parent(parent), m_pre_id(pre_id)
{
}

CGrBookDlg::~CGrBookDlg()
{
	if(m_pDropImageTarget){
		delete m_pDropImageTarget;
	}
	if(m_pDropURLTarget){
		delete m_pDropURLTarget;
	}
}

int CGrBookDlg::DoModal(HWND hWnd)
{
	return DialogBoxParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_BOOK), hWnd, (DLGPROC)CGrWinCtrl::WindowMapProc, (LPARAM)this);
}

LRESULT CGrBookDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG , OnInitDialog);
		HANDLE_MSG(hWnd, WM_COMMAND    , OnCommand   );
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

static void TagMap2TagList(std::vector<TagInfo>& tag_list, const std::map<int, std::vector<TagInfo>> &tag_list_map, int id, int level = 0)
{
	const auto it = tag_list_map.find(id);
	if (it == tag_list_map.end()) {
		return;
	}
	for (const auto& item : it->second) {
		TagInfo tmp(item);
		tmp.title = _T("");
		for (int i = 0; i < level; ++i) {
			tmp.title += _T(" ");
		}
		tmp.title += item.title;
		tag_list.push_back(tmp);
		TagMap2TagList(tag_list, tag_list_map, item.id, level + 1);
	}
}

// 栞フォルダ
static bool GetTagList(std::vector<TagInfo> &tag_list)
{
	bool bRet = true;

	std::map<int, std::vector<TagInfo>> tag_list_map;
	do {
		CGrBookmarkDB db;
		if (!db.Open()) {
			bRet = false;
			break;
		}

		auto* pSqlite3 = db.GetSqlite3();
		if (!pSqlite3) {
			bRet = false;
			break;
		}
		bool bret = true;
		sqlite3_stmt* pstmt = nullptr;
		do {
			int ret = sqlite3_prepare16(pSqlite3
										, _T("SELECT B.ID,B.TITLE,B.POSITION,B.PARENT FROM TXTMIRU_BOOKS B WHERE B.PLACE_ID IS NULL ORDER BY B.POSITION")
										, -1, &pstmt, nullptr);
			if (ret != SQLITE_OK || !pstmt) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt);
			// 実行
			bret = false;
			while (true) {
				ret = sqlite3_step(pstmt);
				if (ret != SQLITE_ROW) {
					break;
				}
				TagInfo tag;
				int iChildExists = 0;
				CGrDBFunc::GetValue(pstmt, 0, tag.id);
				CGrDBFunc::GetValue(pstmt, 1, tag.title);
				CGrDBFunc::GetValue(pstmt, 2, tag.position);
				CGrDBFunc::GetValue(pstmt, 3, tag.parent);
				tag_list_map[tag.parent].push_back(tag);

				bret = true;
			};
		} while (0);

		if (pstmt) {
			sqlite3_finalize(pstmt);
		}
	} while (0);

	TagMap2TagList(tag_list, tag_list_map, NULL_INT_VALUE);

	return bRet;
}

BOOL CGrBookDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowPosCenter();

	m_EditTitle .SetWnd(GetDlgItem(hwnd, IDC_EDIT_TITLE ));
	m_EditURL   .SetWnd(GetDlgItem(hwnd, IDC_EDIT_URL   ));
	m_EditAuthor.SetWnd(GetDlgItem(hwnd, IDC_EDIT_AUTHOR));
	m_EditNum   .SetWnd(GetDlgItem(hwnd, IDC_EDIT_NUM   ));

	m_EditTitle .SetText(m_book.title .c_str());
	m_EditURL   .SetText(m_book.url   .c_str());
	m_EditAuthor.SetText(m_book.author.c_str());
	auto hWndCombo = GetDlgItem(hwnd, IDC_COMBO_FOLDER);
	if (hWndCombo) {
		int select_index = 0;
		GetTagList(m_tag_list);
		for (const auto& item : m_tag_list) {
			auto index = ComboBox_AddString(hWndCombo, item.title.c_str());
			ComboBox_SetItemData(hWndCombo, index, item.id);
			if (item.id == m_parent) {
				select_index = index;
			}
		}
		ComboBox_SetCurSel(hWndCombo, select_index);
	}
	//
	m_cover.Attach(GetDlgItem(hwnd, IDC_STATIC_IMG));
	m_cover.Load(m_book.id);
	m_pDropImageTarget = new CImageDropTarget(m_cover);
	if(FAILED(RegisterDragDrop(m_cover, m_pDropImageTarget))){
		delete m_pDropImageTarget;
		m_pDropImageTarget = nullptr;
	} else {
		static_cast<CImageDropTarget*>(m_pDropImageTarget)->AddSuportedFormats();
	}
	m_pDropURLTarget = new CURLDropTarget(m_EditURL);
	if(FAILED(RegisterDragDrop(m_EditURL, m_pDropURLTarget))){
		delete m_pDropURLTarget;
		m_pDropURLTarget = nullptr;
	} else {
		static_cast<CURLDropTarget*>(m_pDropURLTarget)->AddSuportedFormats();
	}
	auto lpImgFilename = m_cover.GetFilename();
	if(lpImgFilename){
		m_imgFilename = lpImgFilename;
	}
	closePFunc();
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);

	return TRUE;
}

#define ID_CHK_NEXT (10)
void CGrBookDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDOK:
		{
			m_EditTitle .GetText(m_book.title );
			m_EditURL   .GetText(m_book.url   );
			m_EditAuthor.GetText(m_book.author);
			auto hWndCombo = GetDlgItem(hwnd, IDC_COMBO_FOLDER);
			if (hWndCombo) {
				int select_index = 0;
				GetTagList(m_tag_list);
				for (const auto& item : m_tag_list) {
					auto index = ComboBox_AddString(hWndCombo, item.title.c_str());
					ComboBox_SetItemData(hWndCombo, index, item.id);
					if (item.id == m_parent) {
						select_index = index;
					}
				}
				auto index = ComboBox_GetCurSel(hWndCombo);
				if (index != CB_ERR) {
					auto id = ComboBox_GetItemData(hWndCombo, index);
					if (id >= 0) {
						m_parent = id;
					}
				}
			}
			if (m_parent < 0) {
				std::tstring message;
				CGrText::LoadString(IDS_ERROR_FOLDER, message);
				CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, message.c_str(), CGrTxtFunc::AppName());
				break;
			}
			//
			if(m_parent >= 0){
				if(m_book.title.empty() && m_book.url.size() > 1){
					std::tstring url;
					CGrShell::ToPrettyFileName(m_book.url.c_str(), url);
					if(url[url.size()-1] == _T('/')){
						url[url.size()-1] = _T('\0');
					}
					m_book.title = CGrShell::GetFileName(m_book.url.c_str());
				}
				if(m_book.id < 0){
					m_book.id = addItem();
					if(m_book.id < 0){

					}
				} else {
					if(updateItem()){

					}
				}
				auto lpImgFilename = m_cover.GetFilename();
				if(lpImgFilename && m_imgFilename != lpImgFilename){
					std::tstring fromImgFilename(lpImgFilename);
					auto &param = CGrTxtFunc::Param();
					std::tstring str;
					CGrTxtFunc::GetBookmarkFolder(&param, str);
					TCHAR todir[MAX_PATH];
					_stprintf_s(todir, _T("%s/cover"), str.c_str());
					CGrShell::CreateFolder(todir);
					TCHAR path[MAX_PATH];
					_stprintf_s(path, _T("%s/%d.%s"), todir, m_book.id, CGrShell::GetFileExtConst(fromImgFilename.c_str()));
					std::tstring toImgFilename = path;
					if(!m_imgFilename.empty()){ // backup
						CGrTxtFunc::BackupImgFile(m_book.id);
					}
					::CopyFile(fromImgFilename.c_str(), toImgFilename.c_str(), FALSE);
				}
			}
			AbortUpdate();
			::EndDialog(m_hWnd, IDOK);
		}
		break;
	case IDCANCEL:
		AbortUpdate();
		::EndDialog(m_hWnd, IDCANCEL);
		break;
	case IDC_BUTTON_IMAGE  : getImage        (); break; // 画像を選択
	case IDC_BUTTON_GETINFO: getInfo         (); break; // URLからタイトル/作者を取得
	case IDC_BUTTON_URL    : getUrl          (); break; // ファイル選択
	case ID_CHK_NEXT       : getInfoUpdate   (); break; // URLからタイトル/作者を取得
	case IDC_BUTTON_LATEST: // 最近表示したページ
		{
			std::tstring text(_T(":Latest"));
			std::tstring num;
			m_EditNum.GetText(num);
			if(_tstol(num.c_str()) > 0){
				text += num;
			}
			m_EditURL.SetText(text.c_str());
			CGrText::LoadString(IDS_TITLE_LATEST, text);
			m_EditTitle.SetText(text.c_str());
			m_EditAuthor.SetText(_T(""));
		}
		break;
	case IDC_BUTTON_READ  : // よく読むページ
		{
			std::tstring text(_T(":Visit"));
			std::tstring num;
			m_EditNum.GetText(num);
			if(_tstol(num.c_str()) > 0){
				text += num;
			}
			m_EditURL.SetText(text.c_str());
			CGrText::LoadString(IDS_TITLE_VISIT, text);
			m_EditTitle.SetText(text.c_str());
			m_EditAuthor.SetText(_T(""));
		}
		break;
	case IDC_BUTTON_SIORI : // 栞一覧
		{
			m_EditURL.SetText(_T(":Siori"));
			std::tstring text;
			CGrText::LoadString(IDS_TITLE_SIORI, text);
			m_EditTitle.SetText(text.c_str());
			m_EditAuthor.SetText(_T(""));
		}
		break;
	case IDC_BUTTON_SP:  // その他
		{
			if(m_bOpenSPFunc){
				closePFunc();
			} else {
				openSPFunc();
			}
		}
		break;
	}
}

void CGrBookDlg::openSPFunc()
{
	m_bOpenSPFunc = true;
	auto hwnd_spbar = GetDlgItem(m_hWnd, IDC_STATIC_SPBAR);
	if(hwnd_spbar){
		RECT winrect;
		GetWindowRect(m_hWnd, &winrect);
		RECT clirect;
		GetClientRect(m_hWnd, &clirect);
		RECT rect;
		GetWindowRect(hwnd_spbar, &rect);
		ScreenToClient(m_hWnd, reinterpret_cast<LPPOINT>(&rect));
		ScreenToClient(m_hWnd, reinterpret_cast<LPPOINT>(&rect)+1);
		int width = winrect.right - winrect.left;
		int height = (winrect.bottom - winrect.top) - (clirect.bottom - clirect.top) + rect.bottom;
		SetWindowPos(m_hWnd, NULL, 0, 0, width, height, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING|SWP_NOZORDER);
		for (int id : {IDC_BUTTON_SIORI, IDC_BUTTON_LATEST, IDC_BUTTON_READ, IDC_EDIT_NUM}) {
			ShowWindow(GetDlgItem(m_hWnd, id), SW_SHOW);
		}
	}
}

void CGrBookDlg::closePFunc()
{
	m_bOpenSPFunc = false;
	auto hwnd_spbar = GetDlgItem(m_hWnd, IDC_STATIC_SPBAR);
	if(hwnd_spbar){
		RECT winrect;
		GetWindowRect(m_hWnd, &winrect);
		RECT clirect;
		GetClientRect(m_hWnd, &clirect);
		RECT rect;
		GetWindowRect(hwnd_spbar, &rect);
		ScreenToClient(m_hWnd, reinterpret_cast<LPPOINT>(&rect));
		int width = winrect.right - winrect.left;
		int height = (winrect.bottom - winrect.top) - (clirect.bottom - clirect.top) + rect.top;
		SetWindowPos(m_hWnd, NULL, 0, 0, width, height, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING|SWP_NOZORDER);
		for (int id : {IDC_BUTTON_SIORI, IDC_BUTTON_LATEST, IDC_BUTTON_READ, IDC_EDIT_NUM}) {
			ShowWindow(GetDlgItem(m_hWnd, id), SW_HIDE);
		}
	}
}

void CGrBookDlg::getImage()
{
	std::tstring str;
	TCHAR filter[2048] = {};
	CGrText::LoadString(IDS_COVERFILE, filter, sizeof(filter)/sizeof(TCHAR));

	TCHAR filename[_MAX_PATH] = {};
	OPENFILENAME of = {sizeof(OPENFILENAME)};
	of.hwndOwner       = m_hWnd;
	of.hInstance       = CGrTxtFunc::GetDllModuleHandle();
	of.lpstrFilter     = filter;
	of.nMaxCustFilter  = 40;
	of.lpstrFile       = filename;
	of.nMaxFile        = sizeof(filename)/sizeof(TCHAR) - 1;
	of.Flags           = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	if(::GetOpenFileName(&of)){
		if(!m_cover.Load(filename)){
			std::tstring message;
			CGrText::FormatMessage(CGrTxtFunc::GetDllModuleHandle(), message, IDS_IMAGE_NOSUPPORT, filename);
			CGrMessageBox::Show(CGrTxtFunc::GetDllModuleHandle(), m_hWnd, message.c_str(), CGrTxtFunc::AppName());
		}
	}
}

void CGrBookDlg::getUrl()
{
	std::tstring str;
	TCHAR filter[2048] = {};
	CGrText::LoadString(IDS_URLFILE, filter, sizeof(filter)/sizeof(TCHAR));

	TCHAR filename[_MAX_PATH] = {};
	OPENFILENAME of = {sizeof(OPENFILENAME)};
	of.hwndOwner       = m_hWnd;
	of.hInstance       = CGrTxtFunc::GetDllModuleHandle();
	of.lpstrFilter     = filter;
	of.nMaxCustFilter  = 40;
	of.lpstrFile       = filename;
	of.nMaxFile        = sizeof(filename)/sizeof(TCHAR) - 1;
	of.Flags           = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	if(::GetOpenFileName(&of)){
		m_EditURL.SetText(filename);
	}
}


using TxtMiruFunc_CGrDocOpenProcCallbak = void (_stdcall *)(int ret, LPARAM lParam);
using TxtMiruFunc_UpdateCheck = HANDLE (cdecl *)(HWND hWnd, LPCTSTR lpFilename, TxtMiruFunc_CGrDocOpenProcCallbak callback, LPARAM lParam, unsigned int *threadid);
void __stdcall CGrBookDlg::CGrDocOpenProcCallbak(int ret, LPARAM lParam)
{
	auto *pDlg = reinterpret_cast<CGrBookDlg*>(lParam);
	if(pDlg){
		if(pDlg->m_hBackgroundPrc){
			CloseHandle(pDlg->m_hBackgroundPrc);
			pDlg->m_hBackgroundPrc = NULL;
			pDlg->m_BackgroundThreadID = 0;
		}
		FORWARD_WM_COMMAND(pDlg->GetWnd(), ID_CHK_NEXT, 0, 0, PostMessage);
	}
}

bool CGrBookDlg::AbortUpdate()
{
	if(m_hBackgroundPrc){
		PostThreadMessage(m_BackgroundThreadID, WM_DESTROY, 0, 0);
	}
	return true;
}

bool CGrBookDlg::getInfoUpdate()
{
	m_EditURL.GetText(m_book.url);
	std::tstring url;
	CGrShell::ToPrettyFileName(m_book.url.c_str(), url);
	CGrBookmarkDB db;
	if(!db.Open()){
		return false;
	}
	Place place;
	if(!db.GetPlace(place, url.c_str())){
		return false;
	}
	m_EditTitle .SetText(place.title .c_str());
	m_EditAuthor.SetText(place.author.c_str());
	return true;
}
bool CGrBookDlg::getInfo()
{
	AbortUpdate();
	m_EditURL.GetText(m_book.url);
	std::tstring url;
	CGrShell::ToPrettyFileName(m_book.url.c_str(), url);
	bool bRet = true;
	do {
		CGrBookmarkDB db;
		if(!db.Open()){
			bRet = false;
			break;
		}
		Place place;
		if(!db.GetPlace(place, url.c_str())){
			bRet = false;
			break;
		}
		m_EditTitle .SetText(place.title .c_str());
		m_EditAuthor.SetText(place.author.c_str());
	} while(0);
	if(!bRet){
		if(m_hBackgroundPrc){
			return false;
		}
		auto func = reinterpret_cast<TxtMiruFunc_UpdateCheck>(GetProcAddress(GetModuleHandle(NULL), "TxtMiruFunc_UpdateCheck"));
		if(!func){
			return false;
		}
		auto h = func(m_hWnd, url.c_str(), CGrDocOpenProcCallbak, reinterpret_cast<LPARAM>(this), &m_BackgroundThreadID);
		if(!m_hBackgroundPrc){
			return false;
		}
		m_hBackgroundPrc = h;
	}

	return true;
}

int CGrBookDlg::addItem()
{
	CGrBookmarkDB db;
	if(!db.Open()){
		return NULL_INT_VALUE;
	}
	int id = NULL_INT_VALUE;
	do {
		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			break;
		}
		bool bret = true;
		if(SQLITE_OK != db.BeginSession()){
			bret = false;
			break;
		}
		std::tstring url;
		CGrShell::ToPrettyFileName(m_book.url.c_str(), url);

		Place place;
		bret = db.GetPlace(place, url.c_str());
		if(!bret){
			place.id              = NULL_INT_VALUE;
			place.url             = url           ;
			place.title           = m_book.title  ;
			place.author          = m_book.author ;
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
		std::tstring sysdate;
		CGrDBFunc::GetSysDate(sysdate);
		CGrDBBooks books;
		int position = 0;
		books.MaxPosition(pSqlite3, m_parent, position);

		::Book book;
		book.id          = NULL_INT_VALUE;
		book.place_id    = place.id      ;
		book.parent      = m_parent      ;
		book.type        = 0             ;
		book.position    = position+1    ;
		book.base_url    = place.url     ;
		book.title       = m_book.title  ;
		book.author      = m_book.author ;
		book.insert_date = sysdate       ;
		bret = books.Put(pSqlite3, book);
		if(!bret){
			break;
		}
		id = static_cast<int>(sqlite3_last_insert_rowid(pSqlite3));
		if(m_pre_id >= 0){
			do {
				bret = books.SortPosition(pSqlite3, m_parent, m_pre_id, id, 0);
				if(!bret){
					break;
				}
			} while(0);
		}
		if(bret){
			if(SQLITE_OK != db.Commit()){
				bret = false;
				break;
			}
		} else {
			db.Rollback();
			break;
		}
	} while(0);
	return id;
}

bool CGrBookDlg::updateItem()
{
	CGrBookmarkDB db;
	if(!db.Open()){
		return false;
	}
	bool bret = false;
	do {
		auto *pSqlite3 = db.GetSqlite3();
		if(!pSqlite3){
			break;
		}
		std::tstring url;
		CGrShell::ToPrettyFileName(m_book.url.c_str(), url);

		Place place;
		bret = db.GetPlace(place, url.c_str());
		if(!bret){
			place.id              = NULL_INT_VALUE;
			place.url             = url           ;
			place.title           = m_book.title  ;
			place.author          = m_book.author ;
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
		std::tstring sysdate;
		CGrDBFunc::GetSysDate(sysdate);
		CGrDBBooks books;

		::Book book;
		book.id          = m_book.id       ;
		book.place_id    = place.id        ;
		book.parent      = m_parent        ;
		book.type        = 0               ;
		book.position    = m_book.position ;
		book.base_url    = place.url       ;
		book.title       = m_book.title    ;
		book.author      = m_book.author   ;
		book.insert_date = sysdate         ;
		bret = books.Put(pSqlite3, book);
		if(!bret){
			break;
		}
	} while(0);
	return bret;
}
