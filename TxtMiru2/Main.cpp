// main.cpp
//
#define STRICT // 型を宣言および使用時に、より厳密な型チェックが行われます。
#define MAIN_H
#pragma warning(disable:4786)

#define OEMRESOURCE
#include "main.h"
#include "VersionNo.h"

//#define __DBG__
#include "Debug.h"

////////////////////////////////////////////////////////
static void initKeyMap(CGrFunctionKeyMap::FunctionMap& fmap)
{
	struct KeyMap {
		UINT vk;            //  CAS
		BYTE ctl_alt_shift; // 0111, Ctl=0x04, Alt=0x02, Shift=0x01
		TxtMiru::FuncNameID fnid;
	} keyMap[] = {
		{'O'      , 0x04, TxtMiru::FnN_ReadFile            },{'O'      , 0x05, TxtMiru::FnN_ReadClipboard   },
		{VK_F5    , 0x00, TxtMiru::FnN_Reload              },{VK_DOWN  , 0x00, TxtMiru::FnN_NextPage        },
		{VK_LEFT  , 0x00, TxtMiru::FnN_NextPage            },{VK_NEXT  , 0x00, TxtMiru::FnN_NextPage        },
		{VK_SPACE , 0x00, TxtMiru::FnN_NextPage            },{VK_PRIOR , 0x00, TxtMiru::FnN_PrevPage        },
		{VK_RIGHT , 0x00, TxtMiru::FnN_PrevPage            },{VK_SPACE , 0x01, TxtMiru::FnN_PrevPage        },
		{VK_UP    , 0x00, TxtMiru::FnN_PrevPage            },{VK_HOME  , 0x00, TxtMiru::FnN_FirstPage       },
		{VK_END   , 0x00, TxtMiru::FnN_EndPage             },{'G'      , 0x04, TxtMiru::FnN_GotoPage        },
		{'H'      , 0x04, TxtMiru::FnN_GotoPage            },{VK_INSERT, 0x04, TxtMiru::FnN_AddBookmark     },
		{'1'      , 0x04, TxtMiru::FnN_GotoBookmark1       },{'2'      , 0x04, TxtMiru::FnN_GotoBookmark2   },
		{'3'      , 0x04, TxtMiru::FnN_GotoBookmark3       },{'4'      , 0x04, TxtMiru::FnN_GotoBookmark4   },
		{'5'      , 0x04, TxtMiru::FnN_GotoBookmark5       },{'6'      , 0x04, TxtMiru::FnN_GotoBookmark6   },
		{'7'      , 0x04, TxtMiru::FnN_GotoBookmark7       },{'8'      , 0x04, TxtMiru::FnN_GotoBookmark8   },
		{'9'      , 0x04, TxtMiru::FnN_GotoBookmark9       },{'0'      , 0x04, TxtMiru::FnN_GotoBookmark0   },
		{'B'      , 0x04, TxtMiru::FnN_ShowBookmark        },{VK_RETURN, 0x04, TxtMiru::FnN_FullScreen      },
		{'C'      , 0x04, TxtMiru::FnN_Copy                },{'F'      , 0x04, TxtMiru::FnN_Search          },
		{VK_F3    , 0x00, TxtMiru::FnN_SearchNext          },{VK_F3    , 0x01, TxtMiru::FnN_SearchPrev      },
		{VK_BACK  , 0x00, TxtMiru::FnN_BackPage            },{VK_BACK  , 0x01, TxtMiru::FnN_ForwardPage     },
		{'K'      , 0x04, TxtMiru::FnN_Config              },{VK_F1    , 0x00, TxtMiru::FnN_Help            },
		{VK_F1    , 0x01, TxtMiru::FnN_Version             },{VK_F4    , 0x04, TxtMiru::FnN_Exit            },
		{VK_ESCAPE, 0x00, TxtMiru::FnN_Exit                },{VK_RETURN, 0x02, TxtMiru::FnN_ShowProperty    },
		{'L'      , 0x04, TxtMiru::FnN_URLOpen             },{'S'      , 0x04, TxtMiru::FnN_Save            },
		{'D'      , 0x04, TxtMiru::FnN_ShowSubtitleBookmark},
	};
	for (const auto& km : keyMap) {
		fmap[CGrKeyboardState(km.vk, (km.ctl_alt_shift & 0x01) != 0, (km.ctl_alt_shift & 0x04) != 0, (km.ctl_alt_shift & 0x02) != 0)]
			= TxtMiru::l_TxtMiruFuncNameList[km.fnid];
	}
}

////////////

class CGrMainWindow;
class CGrAbCommand {
public:
	virtual bool doCommand(CGrMainWindow& data) = 0;
	virtual bool undoCommand(CGrMainWindow& data) = 0;
	virtual bool isSkipRedoCommand() { return false; }
	virtual bool isSkipUndoCommand() { return false; }
};
using CMD_STACK = std::stack<CGrAbCommand*>;

class CGrGotoPageCommand;

class CGrTxtFuncCanvas : public CGrTxtFuncICanvas
{
	CGrTxtCanvas canvas;
	CGrTxtDocument* m_pDoc;
	int m_iWidth = 0;
	int m_iHeight = 0;
public:
	CGrTxtFuncCanvas(CGrTxtDocument* pDoc) : m_pDoc(pDoc)
	{
		canvas.Attach(pDoc);
		canvas.Initialize();
		canvas.UpdateParam();
	}
	virtual ~CGrTxtFuncCanvas() {}
	virtual bool GetBitmap(int page, int width, int height, CGrBitmap** pBmp)
	{
		if (width != m_iWidth || height != m_iHeight) {
			m_iWidth = width;
			m_iHeight = height;
			canvas.Resize(m_iWidth, m_iHeight);
		}
		canvas.Update(page);
		if (pBmp) {
			*pBmp = &canvas.GetBitmap();
		}
		return true;
	}
};

//#include "Test.h"

static void replaceEscape(std::tstring& str, LPCTSTR repstr)
{
	std::replaceCRLF(str, repstr);
	TCHAR target[2] = {};
	for (int i = 0x01; i < 0x20; ++i) {
		target[0] = _T('\0') + i;
		std::replace(str, target, repstr);
	}
}
//////////////////////////////////////////////////
class CGrMainWindow : public IDropTarget, public CGrMouseEvent
{
private:
	bool GotoPageCommand(int to_page)
	{
		TxtMiru::TextPoint f;
		TxtMiru::TextPoint t;
		m_doc.StartPageToTextPoint(f, m_currentPage);
		m_doc.StartPageToTextPoint(t, to_page);
		return doCommand(new CGrGotoPageCommand(f, t));
	}
	void OpenDocumentCommandWithAddFileList(LPCTSTR lpFileName)
	{
		OpenDocumentCommand(lpFileName);
		if (m_doc.IsOpenFile()) {
			AddFileList();
		}
	}
	bool OpenDocumentCommand(LPCTSTR lpFilename)
	{
		std::tstring filename;
		// 短いパスを長いパスに変更
		std::unique_ptr<TCHAR[]> lpszLongPath(new TCHAR[MAX_PATH]);
		auto len = ::GetLongPathName(lpFilename, lpszLongPath.get(), MAX_PATH - 1);
		if (len > MAX_PATH) {
			lpszLongPath.reset(new TCHAR[len + 1]);
			len = ::GetLongPathName(lpFilename, lpszLongPath.get(), len);
		}
		if (len > 0) {
			CGrShell::ToPrettyFileName(lpszLongPath.get(), filename);
		}
		else {
			CGrShell::ToPrettyFileName(lpFilename, filename);
		}
		std::tstring old_filename;
		m_doc.GetFileName(old_filename);
		return doCommand(new CGrOpenFileCommand(old_filename.c_str(), filename.c_str()));
	}
	bool Undo()
	{
		if (m_undo_stack.empty()) {
			return false;
		}
		auto* cmd = m_undo_stack.top();
		if (!cmd) {
			return false;
		}
		m_redo_stack.push(cmd);
		m_undo_stack.pop();

		if (cmd->isSkipUndoCommand()) {
			return Undo();
		}
		else {
			return cmd->undoCommand(*this);
		}
	}
	bool Redo()
	{
		if (m_redo_stack.empty()) {
			return false;
		}
		auto* cmd = m_redo_stack.top();
		if (!cmd) {
			return false;
		}
		m_undo_stack.push(cmd);
		m_redo_stack.pop();

		if (cmd->isSkipRedoCommand()) {
			return Redo();
		}
		else {
			return cmd->doCommand(*this);
		}
	}
private:
	bool doCommand(CGrAbCommand* cmd)
	{
		while (!m_redo_stack.empty()) {
			m_redo_stack.pop();
		}
		m_undo_stack.push(cmd);
		return cmd->doCommand(*this);
	}
private:
	CMD_STACK m_redo_stack;
	CMD_STACK m_undo_stack;

	/**/
	struct OpenFileInfo {
		std::tstring title;
		std::tstring filename;
		bool operator ==(LPCTSTR str) const {
			return (_tcscmp(filename.c_str(), str) == 0);
		}
		bool operator <(LPCTSTR str) const {
			return (_tcscmp(filename.c_str(), str) > 0);
		}
	};
	POINT m_clickLPos = {};
	POINT m_gestureBeginPos = {};
	bool m_bSelectionMode = true;
	bool m_bSpSelectionMode = false;
	bool m_bSpSelectionModeCopied = true;
	int m_windowOffsetX = 0;
	int m_windowOffsetY = 0;
	int m_windowWidth = 0;
	int m_windowHeight = 0;
	bool m_bSinglePage = false;
	enum PAGE_MODE { PAGE_MODE_NORMAL, PAGE_MODE_AUTO, PAGE_MODE_SINGLE, PAGE_MODE_SINGLE_CENTER };
	PAGE_MODE m_pageMode = PAGE_MODE_SINGLE;
	CGrTxtCanvas m_txtCanvas;
	using pOnKeyCallbackFunc = void (CGrMainWindow::*)();
	//
	std::map<CGrKeyboardState, pOnKeyCallbackFunc> m_keyfunc_map;
	std::map<std::tstring, pOnKeyCallbackFunc> m_func_map;
	std::map<UINT, pOnKeyCallbackFunc> m_id_func_map; // リソースID(コマンド)と関数ポインタのマップ

	void OpenDocumentDelay(LPCTSTR lpFileName, UINT codeNotify = 0)
	{
		if (lpFileName && lpFileName[0]) {
			m_dropFileName = lpFileName; // OpenDocumentCommandWithAddFileList -> OpenDocumentCommand で ToPrettyFileName している
			FORWARD_WM_COMMAND(m_hWnd, TxtDocMessage::OPEN_DD_FILE/*id*/, NULL/*hwndCtl*/, codeNotify, PostMessage);
		}
	}
	void OpenDocumentDelayNoHistory(LPCTSTR lpFileName)
	{
		if (lpFileName && lpFileName[0]) {
			CGrShell::ToPrettyFileName(lpFileName, m_dropFileName);
			FORWARD_WM_COMMAND(m_hWnd, TxtDocMessage::OPEN_DD_FILE_NOHIST/*id*/, NULL/*hwndCtl*/, 0/*codeNotify*/, PostMessage);
		}
	}
private:
	/******************************************************************************
								   IDropTarget
	 ******************************************************************************/
	IDataObject* m_DataObject = nullptr;
	LONG m_RefCount = 1;
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv)
	{
		if (iid == IID_IDropTarget || iid == IID_IUnknown) {
			*ppv = static_cast<void*>(this);
			AddRef();
		}
		else {
			*ppv = 0;
			return S_FALSE;
		}
		return S_OK;
	}
	virtual ULONG __stdcall AddRef()
	{
		InterlockedIncrement(&m_RefCount);
		return static_cast<ULONG>(m_RefCount);
	}
	virtual ULONG __stdcall Release()
	{
		auto ret = static_cast<ULONG>(InterlockedDecrement(&m_RefCount));
		return static_cast<ULONG>(m_RefCount);
	}
	virtual HRESULT __stdcall DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
	{
		m_DataObject = pDataObject; // IDataObjectを記憶してDragOverでも使えるようにする
		return DragOver(grfKeyState, ptl, pdwEffect); //あとはDragOverと同じ
	}
	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
	{
		const POINT point = { ptl.x, ptl.y };
		*pdwEffect = DROPEFFECT_NONE;
		if (!IsWindowEnabled(m_hWnd) || ::WindowFromPoint(point) != m_hWnd) {
			// ファイルダイアログとかを表示している間 と 自分自身以外(のウインドウ)は、D&Dは受け付けない
			return S_OK;
		}
		//HDROPとTEXTを受け付ける
		static UINT CF_SHELLURL = -1;
		if (CF_SHELLURL == -1) {
			CF_SHELLURL = ::RegisterClipboardFormat(CFSTR_SHELLURL);
		}
		if (/**/QueryFormat(CF_SHELLURL)
			|| QueryFormat(CF_HDROP)
			|| QueryFormat(CF_UNICODETEXT/*CF_TEXT*/)
			|| QueryFormat(m_ClipFormat[CLIPFORMAT_UniformResourceLocatorFormat])
			|| QueryFormat(m_ClipFormat[CLIPFORMAT_UniformResourceLocatorFormatW])
			|| QueryFormat(m_ClipFormat[CLIPFORMAT_NetscapeBookmarkFormat])
			|| QueryFormat(m_ClipFormat[CLIPFORMAT_FiledescriptorFormat])
			|| QueryFormat(m_ClipFormat[CLIPFORMAT_FilecontentsFormat])
			) {
			*pdwEffect = DROPEFFECT_COPY;
		}
		return S_OK;
	}
	virtual HRESULT __stdcall DragLeave(void)
	{
		return S_OK;
	}
	std::tstring m_dropFileName;

	bool DropURL(IDataObject* pDataObject, CLIPFORMAT cf, bool bUnicode = false)
	{
		FORMATETC fmt = { 0 };
		STGMEDIUM medium;

		fmt.cfFormat = cf;
		fmt.ptd = nullptr;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;

		if (pDataObject->GetData(&fmt, &medium) == S_OK) {
			std::tstring url;
			if (bUnicode) {
				auto lpUrl = static_cast<LPCTSTR>(GlobalLock(medium.hGlobal));
				if (lpUrl) {
					url = lpUrl;
					OpenDocumentDelay(url.c_str());
				}
			}
			else {
				if (CGrText::MultiByteToTString(CP_THREAD_ACP, static_cast<LPCSTR>(GlobalLock(medium.hGlobal)), -1, url)) {
					OpenDocumentDelay(url.c_str());
				}
			}
			GlobalUnlock(medium.hGlobal);
			ReleaseStgMedium(&medium);
			return true;
		}
		return false;
	}
	virtual HRESULT __stdcall Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
	{
		FORMATETC fmt = { 0 };
		STGMEDIUM medium;

		fmt.cfFormat = CF_HDROP;
		fmt.ptd = NULL;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;

		// ドロップされたものがURL
		if (pDataObject->GetData(&fmt, &medium) == S_OK) {
			TCHAR fileName[_MAX_PATH];
			::DragQueryFile((HDROP)medium.hGlobal, 0, fileName, _countof(fileName));
			::ReleaseStgMedium(&medium);

			// InternetShortcut対応
			TCHAR buf[_MAX_PATH];
			if (GetPrivateProfileString(_T("InternetShortcut"), _T("URL"), _T(""), buf, sizeof(buf) / sizeof(TCHAR), fileName)) {
				OpenDocumentDelay(buf);
			}
			else {
				OpenDocumentDelay(fileName);
			}
			*pdwEffect = DROPEFFECT_COPY;
			return S_OK;
		}
		// ブラウザからLINK D&D
		if (DropURL(pDataObject, m_ClipFormat[CLIPFORMAT_UniformResourceLocatorFormat])) {
			*pdwEffect = DROPEFFECT_COPY;
			return S_OK;
		}
		if (DropURL(pDataObject, m_ClipFormat[CLIPFORMAT_UniformResourceLocatorFormatW], true)) {
			*pdwEffect = DROPEFFECT_COPY;
			return S_OK;
		}
		if (DropURL(pDataObject, m_ClipFormat[CLIPFORMAT_NetscapeBookmarkFormat])) {
			*pdwEffect = DROPEFFECT_COPY;
			return S_OK;
		}
		// ZipフォルダなどからDropされた仮想ファイル
		STGMEDIUM stgmedFile = { 0 };
		FORMATETC fmtetc_file_desc = { m_ClipFormat[CLIPFORMAT_FilecontentsFormat], 0, DVASPECT_CONTENT, 0, TYMED_HGLOBAL | TYMED_ISTREAM | TYMED_ISTORAGE };
		// n件あっても最初の１件のみ表示
		if (pDataObject->GetData(&fmtetc_file_desc, &stgmedFile) == S_OK) {
			BOOL bReadOK = FALSE;
			if (!bReadOK && (stgmedFile.tymed & TYMED_ISTREAM)) {
				//Now read data from a stream & process it
				//(If need be, it can be saved in a file)
				IStream* pstm = stgmedFile.pstm;
				//Size of data in a steam & archived file name
				STATSTG stg = {};
				if (pstm->Stat(&stg, STATFLAG_DEFAULT) == S_OK) {
					; // success
				}
				auto* pStorage = new char[(static_cast<long>(stg.cbSize.LowPart)) * sizeof(TCHAR)];
				//Then to read data from a stream
				//Call repeatedly until all or required data is read)
				DWORD ncbBytesRead = stg.cbSize.LowPart;
				DWORD ucbBytesRead = 0;
				if (pstm->Read(pStorage, ncbBytesRead, &ucbBytesRead) == S_OK) {
					; // success
				}
				std::tstring text;
				if (CGrText::MultiByteToTString(CP_THREAD_ACP, pStorage, -1, text)) {
					OpenDocumentText(text.c_str());
				}
				//If read and processed successfully
				delete[] pStorage;
				bReadOK = TRUE;
				//Release mem
				CoTaskMemFree(stg.pwcsName);
			}
			ReleaseStgMedium(&stgmedFile);
			if (bReadOK) {
				*pdwEffect = DROPEFFECT_COPY;
				return S_OK;
			}
		}
		// ドロップされたものがテキスト
		fmt.cfFormat = CF_UNICODETEXT/*CF_TEXT*/;
		if (pDataObject->GetData(&fmt, &medium) == S_OK) {
			auto* buf = static_cast<TCHAR*>(GlobalLock(medium.hGlobal));
			if (buf) {
				OpenDocumentText(buf);
			}
			GlobalUnlock(medium.hGlobal);
			ReleaseStgMedium(&medium);
			*pdwEffect = DROPEFFECT_COPY;
			return S_OK;
		}
		//
		InvalidateRect(m_hWnd, NULL, TRUE);
		*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}
	/*---------------------------------------------------------------------------------------*/
	bool QueryFormat(CLIPFORMAT cfFormat)
	{
		FORMATETC fmt = { 0 };

		fmt.cfFormat = cfFormat;
		fmt.ptd = nullptr;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;
		return (m_DataObject->QueryGetData(&fmt) == S_OK);
	}
public:
	static inline CGrMainWindow& theApp()
	{
		static CGrMainWindow mainWindow;
		return mainWindow;
	}
	virtual ~CGrMainWindow()
	{
		int len = m_redo_stack.size();
		for (; len > 0; --len) {
			delete m_redo_stack.top();
			m_redo_stack.pop();
		}
		len = m_undo_stack.size();
		for (; len > 0; --len) {
			delete m_undo_stack.top();
			m_undo_stack.pop();
		}
	}
	bool PreTranslateMessage(LPMSG lpMsg)
	{
		if ((lpMsg->message == WM_KEYDOWN || lpMsg->message == WM_SYSKEYDOWN)
			&& m_dlgLink.IsLinkMember(lpMsg->hwnd)) {
			// 栞画面を閉じるショートカット
			const auto pFunc = getKeyFunc(lpMsg->wParam);
			if (pFunc == &CGrMainWindow::ToggleLinkPos) {
				(this->*pFunc)();
				return false;
			}
		}
		return true;
	}
	bool m_bWindowResize = true;
	LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg) {
			HANDLE_MSG(hWnd, WM_HSCROLL, OnHScroll);
			HANDLE_MSG(hWnd, WM_VSCROLL, OnVScroll);
			HANDLE_MSG(hWnd, WM_KEYDOWN, OnKey);
			HANDLE_MSG(hWnd, WM_KEYUP, OnKey);
			HANDLE_MSG(hWnd, WM_SYSKEYDOWN, OnSysKey);
			HANDLE_MSG(hWnd, WM_SYSKEYUP, OnSysKey);
			HANDLE_MSG(hWnd, WM_LBUTTONDOWN, OnLButtonDown);
			HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
			HANDLE_MSG(hWnd, WM_MOUSEMOVE, OnMouseMove);
			HANDLE_MSG(hWnd, WM_NCMOUSEMOVE, OnNCMouseMove);
			HANDLE_MSG(hWnd, WM_RBUTTONUP, OnRButtonUp);
			HANDLE_MSG(hWnd, WM_LBUTTONUP, OnLButtonUp);
			HANDLE_MSG(hWnd, WM_MBUTTONUP, OnMButtonUp);
			HANDLE_MSG(hWnd, WM_XBUTTONUP, OnXButtonUp);
			HANDLE_MSG(hWnd, WM_RBUTTONDOWN, OnRButtonDown);
			HANDLE_MSG(hWnd, WM_MBUTTONDOWN, OnMButtonDown);
			HANDLE_MSG(hWnd, WM_XBUTTONDOWN, OnXButtonDown);
			HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
			HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
			HANDLE_MSG(hWnd, WM_CONTEXTMENU, OnContextMenu);
			HANDLE_MSG(hWnd, WM_SIZE, OnSize);
			HANDLE_MSG(hWnd, WM_KILLFOCUS, OnKillFocus);
		case WM_SYSCOMMAND:
		{
			switch (wParam & 0xFFF0) {
			case SC_MINIMIZE:
			case SC_MAXIMIZE:
			case SC_RESTORE:
				m_bWindowResize = true;
				break;
			}
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		case WM_ERASEBKGND:
		{
			RECT rect;
			::GetClientRect(hWnd, &rect);
			auto hdc = reinterpret_cast<HDC>(wParam);
			const auto& param = CGrTxtMiru::theApp().Param();
			COLORREF page_color[3] = {}; // paper, shadow, back
			param.GetPoints(CGrTxtParam::PageColor, reinterpret_cast<int*>(page_color), sizeof(page_color) / sizeof(COLORREF));
			RECT crect = {};
			if (!m_bWindowResize && !m_bNoPaint) {
				crect = m_txtCanvas.GetWindowRect();
				if (m_bSinglePage) {
					crect.right -= m_windowOffsetX + m_txtCanvas.GetPatternOffset(m_windowWidth);
					m_txtCanvas.Draw(hdc, rect, rect, m_windowOffsetX + m_txtCanvas.GetPatternOffset(m_windowWidth), m_windowOffsetY);
				}
				else {
					crect.right -= m_windowOffsetX;
					m_txtCanvas.Draw(hdc, rect, rect, m_windowOffsetX, m_windowOffsetY);
				}
			}
			m_bWindowResize = false;
			RECT bkrect = rect;
			COLORREF oldclr = ::SetBkColor(hdc, page_color[2]);
			bkrect.top = crect.bottom;
			bkrect.left = 0;
			::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &bkrect, NULL, 0, NULL);
			bkrect.top = 0;
			bkrect.left = crect.right;
			::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &bkrect, NULL, 0, NULL);
			::SetBkColor(hdc, oldclr);
		}
		return TRUE;
		case WM_GESTURE:
			if (CGrGesture::Gesture().IsSupported()) {
				// WM_LBUTTONDOWN をキャンセル
				m_clickLPos.x = INT_MIN;
				m_clickLPos.y = INT_MIN;
				//
				auto hgi = reinterpret_cast<HGESTUREINFO>(lParam);
				GESTUREINFO gi = { sizeof(GESTUREINFO) };
				auto ok = CGrGesture::Gesture().GetGestureInfo(hgi, &gi);
				if (ok) {
					switch (gi.dwID) {
					case GID_BEGIN:
						break;
					case GID_ZOOM:
						break;
					case GID_PAN:
						if (gi.dwFlags & GF_BEGIN) {
							m_gestureBeginPos.x = gi.ptsLocation.x;
							m_gestureBeginPos.y = gi.ptsLocation.y;
						}
						else {
							if (m_gestureBeginPos.x > INT_MIN && m_gestureBeginPos.y > INT_MIN) { // 2.0.42.0
								ULARGE_INTEGER ulArguments;
								ulArguments.QuadPart = gi.ullArguments;
								DWORD dwDist = ulArguments.LowPart; // 2 Finger-Tap : Distance Between Fingers
								POINT pos;
								pos.x = gi.ptsLocation.x;
								pos.y = gi.ptsLocation.y;
								int w = 1;
								int delta = m_gestureBeginPos.x - pos.x;
								if (w <= delta) {
									m_gestureBeginPos.x = INT_MIN;
									m_gestureBeginPos.y = INT_MIN;
									if (dwDist > 0) {
										callFunc(IDPREVFILE);
									}
									else {
										callFunc(IDPREVPAGE);
									}
								}
								else if (delta <= -w) {
									m_gestureBeginPos.x = INT_MIN;
									m_gestureBeginPos.y = INT_MIN;
									if (dwDist > 0) {
										callFunc(IDNEXTFILE);
									}
									else {
										callFunc(IDNEXTPAGE);
									}
								}
							}
						}
						if (gi.dwFlags & GF_END) {
							m_gestureBeginPos.x = INT_MIN;
							m_gestureBeginPos.y = INT_MIN;
						}
						break;
					case GID_ROTATE:
						break;
					case GID_PRESSANDTAP:
						ShowLinkPos();
						break;
					default:
						return DefWindowProc(hWnd, WM_GESTURE, wParam, lParam);
					}
				}
				CGrGesture::Gesture().CloseGestureInfoHandle(hgi);
				return 0;
			}
			break;
		case WM_MOUSEWHEEL:
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wParam); // マルチモニタ環境では、負の値を持つことがあるので一旦 shortに変換する必要がある
			if ((WHEEL_DELTA / 6) <= delta) {
				if (execKeyFunc(VK_WHEEL_SCROLLUP)) {
					return TRUE;
				}
				else {
					callFunc(IDPREVPAGE);
				}
			}
			else if (delta <= -(WHEEL_DELTA / 6)) {
				if (execKeyFunc(VK_WHEEL_SCROLLDW)) {
					return TRUE;
				}
				else {
					callFunc(IDNEXTPAGE);
				}
			}
		}
		break;
		case WM_MOUSEHWHEEL:
		{
			int fwKeys = GET_KEYSTATE_WPARAM(wParam);
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if ((WHEEL_DELTA / 6) <= zDelta) {
				if (execKeyFunc(VK_TILT_RBUTTON)) {
					return TRUE;
				}
				else {
					callFunc(IDPREVPAGE);
				}
			}
			else if (zDelta <= -(WHEEL_DELTA / 6)) {
				if (execKeyFunc(VK_TILT_LBUTTON)) {
					return TRUE;
				}
				else {
					callFunc(IDNEXTPAGE);
				}
			}
		}
		break;
		case WM_CLOSE:
		{
			auto&& param = CGrTxtMiru::theApp().Param();
			{ // 最近使ったファイル の 保存
				int len = m_openFileList.size();
				if (len > 0) {
					auto* it = &m_openFileList[len - 1];
					len = min(9, len);
					for (int idx = 0; idx < len; ++idx, --it) {
						std::tstring str;
						str = it->filename;
						replaceEscape(str, _T(""));
						param.SetText(static_cast<CGrTxtParam::TextType>(CGrTxtParam::HistFile1 + idx), str.c_str());
						str = it->title;
						replaceEscape(str, _T(""));
						param.SetText(static_cast<CGrTxtParam::TextType>(CGrTxtParam::HistFileTitle1 + idx), str.c_str());
					}
				}
			}
			param.SetBoolean(CGrTxtParam::ShowHScroll, m_bVisibleHScrollbar);
			param.SetBoolean(CGrTxtParam::ShowVScroll, m_bVisibleVScrollbar);
			int auto_save_mode[2] = { m_bBookMarkAutoSave ? 1 : 0, m_autoSaveInterval };
			param.SetPoints(CGrTxtParam::BookMarkAutoSave, auto_save_mode, sizeof(auto_save_mode) / sizeof(int));
			param.SetBoolean(CGrTxtParam::SelectionMode, m_bSelectionMode);
			param.SetBoolean(CGrTxtParam::AutoHideMenu, m_bAutoHideMenu);
			// ウインドウサイズの保存
			WINDOWPLACEMENT wndp = { sizeof(WINDOWPLACEMENT) };
			if (param.GetBoolean(CGrTxtParam::SaveWindowSize)) {
				if (m_bFullScreen) {
					wndp = m_wndp;
				}
				else {
					GetWindowPlacement(m_hWnd, &wndp);
				}
				int link_pos[6] = {};
				param.GetPoints(CGrTxtParam::LinkPos, link_pos, sizeof(link_pos) / sizeof(int));
				link_pos[0] = isLinkPosVisible() ? 1 : 0;
				param.SetPoints(CGrTxtParam::LinkPos, link_pos, sizeof(link_pos) / sizeof(int));
				//
				param.GetPoints(CGrTxtParam::FavoritePos, link_pos, sizeof(link_pos) / sizeof(int));
				link_pos[0] = isBookPosVisible() ? 1 : 0;
				param.SetPoints(CGrTxtParam::FavoritePos, link_pos, sizeof(link_pos) / sizeof(int));
				//
				int window_pos[5] = {};
				// ルーペ
				param.GetPoints(CGrTxtParam::LupePos, window_pos, sizeof(window_pos) / sizeof(int));
				window_pos[0] = isLupePosVisible() ? 1 : 0;
				if (CGrLupeWnd::ST_WindowPos == m_LupeWnd.GetSelectType()) { // ST_MousePos:カーソル位置を拡大する or ST_WindowPos:ウインドウ位置を拡大する
					window_pos[0] |= 0x02;
				}
				switch (m_LupeWnd.GetZoom()) {
				case 150: window_pos[0] |= 0x04; break;
				case 200: window_pos[0] |= 0x08; break;
				case 400: window_pos[0] |= 0x0C; break;
				}
				param.SetPoints(CGrTxtParam::LupePos, window_pos, sizeof(window_pos) / sizeof(int));
				//
				param.SetBoolean(CGrTxtParam::FullScreen, m_bFullScreen);
			}
			param.SetPoints(CGrTxtParam::WindowSize, reinterpret_cast<int*>(&wndp), sizeof(wndp) / sizeof(int));
			auto i_page_mode = static_cast<int>(m_pageMode);
			param.SetPoints(CGrTxtParam::PageMode, &i_page_mode, 1);
		}
		TM_FreeLibrary();
		::DestroyWindow(hWnd);
		break;
		case WM_QUERYENDSESSION: return TRUE;
		case WM_ENDSESSION: ::DestroyWindow(hWnd); break;
		case WM_DESTROY: OnDestroy(); break;
		case WM_CHANGECBCHAIN:
			// クリップボードビューアチェーンの変更
			if (reinterpret_cast<HWND>(wParam) == m_hNextChainWnd) {
				m_hNextChainWnd = reinterpret_cast<HWND>(lParam);
			}
			else if (m_hNextChainWnd != NULL) {
				::SendMessage(m_hNextChainWnd, uMsg, wParam, lParam);
			}
			break;
		case WM_DRAWCLIPBOARD:
			// クリップボードの内容変更
			if (m_hNextChainWnd != NULL) {
				::SendMessage(m_hNextChainWnd, uMsg, wParam, lParam);
			}
			{
				UINT clipFormat = CF_TEXT;
				Menu_setEnabled(IDOPENCLIPBOARD, ::GetPriorityClipboardFormat(&clipFormat, 1) > 0);
			}
			break;
		case WM_TIMER:
			if (ID_TIMER_LUPE == wParam) {
				KillTimer(hWnd, ID_TIMER_LUPE);
				UpdateLupe();
			}
			else if (ID_TIMER_PAGEFLIP == wParam) {
				KillTimer(hWnd, ID_TIMER_PAGEFLIP);
				UpdatePageFlip();
			}
			else if (ID_TIMER_AUTOSAVE == wParam) {
				KillTimer(m_hWnd, ID_TIMER_AUTOSAVE);
				m_doc.SetLastPage(m_displayPage);
				m_doc.Save();
			}
			else {
				CGrMouseEvent::MouseTimer(static_cast<UINT>(wParam));
			}
			break;
		case WM_COPYDATA:
			if (!CGrTxtMiru::theApp().IsWaitting()) {
				auto pcd = reinterpret_cast<PCOPYDATASTRUCT>(lParam);
				if (pcd && pcd->lpData) {
					OpenDocumentDelay(static_cast<LPCTSTR>(pcd->lpData), 1);
					::SetFocus(m_hWnd);
				}
			}
			break;
		case WM_MEASUREITEM:
			TxtMiruTheme_MenuMeasureItem(hWnd, lParam);

			break;
		case WM_DRAWITEM:
			TxtMiruTheme_MenuDrawItem(hWnd, lParam);
			break;
		case WM_MENUSELECT:
			if ((m_bFullScreen || m_bAutoHideMenu) && ::GetMenu(m_hWnd)) {
				HMENU hmenuPopup = (HIWORD(wParam) & MF_POPUP) ? GetSubMenu(reinterpret_cast<HMENU>(lParam), LOWORD(wParam)) : 0L;
				auto flags = static_cast<UINT>((static_cast<short>(HIWORD(wParam)) == -1) ? 0xFFFFFFFF : HIWORD(wParam));
				if (hmenuPopup == 0 && flags == 0xFFFFFFFF) {
					hideMenu(); // フルスクリーンのときメニューが表示されていて メニューが閉じられた時は、メニューを隠す
				}
			}
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		case WM_DISPLAYCHANGE:
		{
			DEVMODE dm = { sizeof(DEVMODE) };
			LPCTSTR lpszDeviceName = nullptr;
			WINDOWPLACEMENT wndp = { sizeof(WINDOWPLACEMENT) };
			GetWindowPlacement(m_hWnd, &wndp);
			POINT point = { wndp.rcNormalPosition.left, wndp.rcNormalPosition.top };
			auto hMonitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY); // プライマリモニタのハンドル取得
			MONITORINFOEX mi = {};
			mi.cbSize = sizeof(MONITORINFOEX);
			if (hMonitor && ::GetMonitorInfo(hMonitor, &mi)) {
				lpszDeviceName = mi.szDevice;
			}
			if (0 == EnumDisplaySettings(lpszDeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
				// determine new orientaion
				switch (dm.dmDisplayOrientation) {
				case DMDO_DEFAULT:
					//dm.dmDisplayOrientation = DMDO_270;
					break;
				case DMDO_270:
					//dm.dmDisplayOrientation = DMDO_180;
					break;
				case DMDO_180:
					//dm.dmDisplayOrientation = DMDO_90;
					break;
				case DMDO_90:
					//dm.dmDisplayOrientation = DMDO_DEFAULT;
					break;
				default:
					// unknown orientation value
					// add exception handling here
					break;
				}
			}
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		case WM_SETTINGCHANGE:
			if (TxtMiruTheme_IsImmersiveColorSet(lParam)) {
				TxtMiruTheme_UpdateDarkMode(m_hWnd);
				InvalidateRect(m_hWnd, nullptr, TRUE);
			}
			break;
		default:
			if (uMsg == m_wm_funcCall) {
				switch (wParam) {
				case CGrTxtFuncIParam::FncCT_GetCurrentPage:
					return m_currentPage;
				case CGrTxtFuncIParam::FncCT_GetCurrentDisplayPage:
					return m_displayPage;
				case CGrTxtFuncIParam::FncCT_GetSinglePage:
					return m_bSinglePage ? 1 : 0;
				case CGrTxtFuncIParam::FunCT_AddBookmark:
					AddBookmark();
					break;
				case CGrTxtFuncIParam::FunCT_DeleteBookmark:
					m_doc.DeleteBookmark(lParam);
					break;
				}
				break;
			}
			else {
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}
		}
		return 0L;
	}
private:
	void hideMenu()
	{
		m_txtCanvas.SetOffset(0, 0);
		::SetMenu(m_hWnd, NULL);
		::UpdateWindow(m_hWnd);
	}
	void resize(int cx, int cy)
	{
		if (m_bNoPaint) {
			return;
		}
		m_bSinglePage = false;
		m_windowOffsetX = 0;
		m_windowOffsetY = 0;
		cx += m_iVScrollWidth;
		cy += m_iHScrollHeight;
		m_windowWidth = cx;
		m_windowHeight = cy;
		if (m_pageMode != PAGE_MODE_NORMAL) { // １ページ表示対応
			const auto& paperSize = m_doc.GetConstLayout().GetPaperSize();
			int h = cx * paperSize.cy;
			int w = cy * paperSize.cx / 2;
			if (h < w) {
				// １ページ表示対応
				cx *= 2;
				m_bSinglePage = true;
			}
			else {
				if (m_pageMode == PAGE_MODE_SINGLE || m_pageMode == PAGE_MODE_SINGLE_CENTER) {
					// 常に１ページ表示
					if (m_bFullScreen || m_bAutoHideMenu) {
						if (::GetMenu(m_hWnd)) {
							w = (cy + GetSystemMetrics(SM_CYMENU)) * paperSize.cx / 2;
						}
					}
					int new_w = w * 2 / paperSize.cy;
					if (cx < new_w) {
						if (m_pageMode == PAGE_MODE_SINGLE) {
							// １ページを端にあわせる
							m_windowWidth = new_w - cx;
							cx = new_w;
						}
						else {
							// １ページをセンター寄
							if (cx > new_w / 2) {
								m_windowWidth = new_w / 2;
								cx = new_w + cx - m_windowWidth;
							}
						}
						m_bSinglePage = true;
					}
					/*}*else {
					  // 自動設定(２ページ表示)
					 */
				}
			}
			if (m_bSinglePage) {
				if (m_bFullScreen || m_bAutoHideMenu) {
					if (::GetMenu(m_hWnd)) {
						m_windowOffsetY = GetSystemMetrics(SM_CYMENU) / 2;
						cy += GetSystemMetrics(SM_CYMENU);
					}
				}
				if (m_displayPage % 2 == 0) {
					m_windowOffsetX = m_windowWidth;
				}
				else {
					m_windowOffsetX = 0;
				}
			}
		}
		m_txtCanvas.Resize(cx, cy);
		showScrollBar();
		//
		m_txtCanvas.SetCurrentHoverLink(-1);
		// メニューの表示・非表示で選択が解除されるので再設定するように
		TxtMiru::TextPoint tpb, tpe;
		bool bselect = m_txtCanvas.GetSelectTextPoint(tpb, tpe);
		//
		m_txtCanvas.Update();
		if (bselect) {
			m_txtCanvas.SetBeginSelect(tpb);
			m_txtCanvas.SetEndSelect(tpe);
		}
		//
		InvalidateRect(m_hWnd, NULL, FALSE);
		//
		m_LupeWnd.SetFromCanvasRect(m_txtCanvas.GetCanvasRect());
		m_LupeWnd.SetCanvasOffset(m_windowOffsetX, m_windowOffsetY);
		// Tips
		if (m_toolTips) {
			setToolTips();
		}
		if (m_MessWnd) {
			m_MessWnd.Hide();
		}
		SetWindowTitle();
	}
	void setToolTips()
	{
		m_toolTips.SetTipsList(m_hWnd, m_txtCanvas.GetTipsInfoList());
	}
	void setWhiteTrans()
	{
		const auto& param = CGrTxtMiru::theApp().Param();
		int i_white_rate;
		param.GetPoints(CGrTxtParam::WhiteTransRate, &i_white_rate, 1);
		CGrImage::SetWhiteTrans(param.GetBoolean(CGrTxtParam::WhiteTrans), static_cast<double>(i_white_rate) / 100);
	}
	void updateSetting()
	{
		setWhiteTrans();
		setAntialias();
		m_doc.UpdateLayout(); // ★サイズが大きいと時間がかかる
		m_txtCanvas.SetCurrentHoverLink(-1);
		m_txtCanvas.UpdateParam();
		m_LupeWnd.UpdateParam();
	}
	void refreshToolWindow()
	{
		m_dlgLink.RefreshLink();
		m_dlgLink.RefreshBook();
		m_dlgLink.RefreshRubyList();
	}
	void refresh(bool bUpdateSetting = true/* リサイズ時のレスポンス改善 */)
	{
		if (bUpdateSetting) { // リサイズ時のレスポンス改善
			updateSetting();
		}
		m_totalPage = m_doc.GetTotalPage();
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		resize(rect.right, rect.bottom);
		// レイアウトが変わって、ページ数が合わないと描画が上手くいないので
		// 最大行に合わせて、ページを移動する必要がある。
		// 例：元表示行：4 -> 先最終ページ：2 のとき
		//     4ページの目の表示はできないので
		GotoPage(m_displayPage);
		refreshToolWindow();
	}
	HMENU getMenu()
	{
		// FullScreen 時、hMenu をブランクにしているので m_hMenu を使用する
		auto hMenu = GetMenu(m_hWnd);
		return hMenu ? hMenu : m_hMenu;
	}
	void Menu_setStatus(int id, UINT fState)
	{
		CGrMenuFunc::SetMenuItemState(getMenu(), id, fState);
	}
	// メニューのチェックを設定します。
	//   id      : メニューID
	//   bCheck  : チェック : true
	void Menu_setChecked(int id, bool bCheck) { Menu_setStatus(id, bCheck ? MFS_CHECKED : MFS_UNCHECKED); }
	bool Menu_checked(UINT id) {
		auto hMenu = getMenu();
		if (!hMenu) {
			return false;
		}
		MENUITEMINFO mi = { sizeof(MENUITEMINFO) };
		mi.fMask = MIIM_STATE;
		::GetMenuItemInfo(hMenu, id, FALSE, &mi);
		return (mi.fState & MFS_CHECKED) == MFS_CHECKED;
	}
	// メニューの選択可不可を設定します。
	//   id      : メニューID
	//   enabled : 選択可なら TRUE
	void Menu_setEnabled(int id, BOOL enabled) { Menu_setStatus(id, enabled ? MFS_ENABLED : MFS_DISABLED); }
	// 2.0.11.0
	pOnKeyCallbackFunc getKeyFunc(UINT vk, bool delay = false)
	{
		auto it = m_keyfunc_map.find(CGrKeyboardState(vk, delay));
		if (it != m_keyfunc_map.end() && it->second) {
			return it->second;
		}
		return nullptr;
	}
	bool execKeyFunc(UINT vk)
	{
		CGrMouseEvent::Clear();
		if (CGrTxtMiru::theApp().IsWaitting()) {
			return false;
		}
		const auto pFunc = getKeyFunc(vk);
		if (pFunc) {
			// リンク用の機能は、フォーカスがリンクある場合のみ使用可能に。
			if (pFunc == &CGrMainWindow::LinkGoto || pFunc == &CGrMainWindow::LinkOpen) {
				int id = m_txtCanvas.GetCurrentHoverLink();
				if (id < 0) {
					return false;
				}
			}
			(this->*pFunc)();
			return true;
		}
		return false;
	}
	void OnPaint(HWND hwnd)
	{
		PAINTSTRUCT ps;
		auto hdc = ::BeginPaint(hwnd, &ps);
		RECT rect;
		::GetClientRect(hwnd, &rect);
		if (m_bSinglePage) {
			m_txtCanvas.Draw(hdc, rect, ps.rcPaint, m_windowOffsetX + m_txtCanvas.GetPatternOffset(m_windowWidth), m_windowOffsetY);
		}
		else {
			m_txtCanvas.Draw(hdc, rect, ps.rcPaint, m_windowOffsetX, m_windowOffsetY);
		}
		::EndPaint(hwnd, &ps);
	}
	void UpdateLupe()
	{
		m_LupeWnd.Update(m_currentPage);
	}
	void UpdatePageFlip()
	{
		if (m_pageFlipInterval > 0) {
			if (m_txtCanvas.PageFilpNext()) {
				::InvalidateRect(m_hWnd, NULL, FALSE);
				::UpdateWindow(m_hWnd);
				::SetTimer(m_hWnd, ID_TIMER_PAGEFLIP, m_pageFlipInterval, NULL);
			}
			else if (m_LupeWnd) {
				::SetTimer(m_hWnd, ID_TIMER_LUPE, 500, NULL);
			}
		}
	}
	void showScrollBar(bool bVisible, SCROLLINFO& si, int sb, int nPos)
	{
		if (m_bFullScreen || m_bAutoHideMenu) {
			if ((sb == SB_HORZ && m_iHScrollHeight == 0) || (sb == SB_VERT && m_iVScrollWidth == 0)) {
				bVisible = false;
			}
		}
		if (bVisible) {
			if (m_totalPage >= 2) {
				si.nMax = m_totalPage;
				si.nPos = nPos;
			}
			else {
				si.nMax = 1;
			}
			::SetScrollInfo(m_hWnd, sb, &si, TRUE);
			::ShowScrollBar(m_hWnd, sb, TRUE);
		}
		else {
			::ShowScrollBar(m_hWnd, sb, FALSE);
		}
	}
	void showScrollBar()
	{
		SCROLLINFO si = { sizeof(SCROLLINFO) }; /*si.nMin = 0; si.nPos = 0; */
		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		si.nMax = si.nPage = 2;
		if (m_bSinglePage) {
			si.nPage = 1;
			showScrollBar(m_bVisibleHScrollbar, si, SB_HORZ, m_totalPage - m_displayPage);
			showScrollBar(m_bVisibleVScrollbar, si, SB_VERT, m_displayPage);
		}
		else {
			showScrollBar(m_bVisibleHScrollbar, si, SB_HORZ, m_totalPage - m_currentPage - 1);
			showScrollBar(m_bVisibleVScrollbar, si, SB_VERT, m_currentPage);
		}
	}
	void addInt(std::tstring& str, int i)
	{
		TCHAR buf[512];
		_stprintf_s(buf, _T("%d"), i);
		str += buf;
	}
	void SetWindowTitle()
	{
		std::tstring title;
		if (m_totalPage > 0) {
			std::tstring format;
			const auto& param = CGrTxtMiru::theApp().Param();
			param.GetText(CGrTxtParam::TitleFormat, format);
			for (auto p = format.c_str(); *p; ++p) {
				if (*p == _T('%')) {
					++p;
					if (!*p) {
						title += _T("%");
						break;
					}
					switch (*p) {
					case _T('p'): title += CGrTxtMiru::AppName(); break; // プログラム名
					case _T('v'): title += FILEVER_STR; break; // バージョン
					case _T('T'): title += m_doc.GetTitle(); break; // タイトル
					case _T('A'): title += m_doc.GetAuthor(); break; // 著者
					case _T('P'):                                        // カレントページ
						if (m_bSinglePage) {
							addInt(title, m_displayPage + 1);
						}
						else {
							addInt(title, m_currentPage + 1);
						}
						break;
					case _T('N'):                                        // ページ数
						if (m_bSinglePage) {
							addInt(title, (m_totalPage + 1) / 2 * 2);
						}
						else {
							addInt(title, m_totalPage);
						}
						break;
					case _T('F'):                                        // ファイル名
					{
						std::tstring filename;
						m_doc.GetFileName(filename);
						title += filename;
					}
					break;
					case _T('f'):                                        // ファイル名
					{
						std::tstring filename;
						m_doc.GetFileName(filename);
						title += CGrShell::GetFileName(filename.c_str());
					}
					break;
					case _T('%'): title += _T("%"); break; // %
					default:
						title += _T("%");
						title += *p;
						break;
					}
				}
				else {
					title += *p;
				}
			}
		}
		else {
			CGrText::FormatMessage(title, _T("%1!s!"), CGrTxtMiru::AppName());
		}
		std::replaceCRLF(title, _T(" "));
		SetWindowText(m_hWnd, title.c_str());
	}
	void UpdateCmdUI()
	{
		Menu_setChecked(IDSUBTITLEBOOKMARK, isLinkPosVisible());
		Menu_setChecked(IDFAVORITE, isBookPosVisible());
		Menu_setChecked(IDRUBYLIST, isRubyListVisible());
		Menu_setChecked(IDFULLSCREEN, m_bFullScreen);
		Menu_setChecked(IDAUTOHIDEMENU, m_bAutoHideMenu);
		Menu_setChecked(IDCOPYRUBY, isCopyRuby());
		Menu_setChecked(IDLUPE, isLupePosVisible());
		Menu_setChecked(IDLUPEWINDOWPOS, isLupeMousePos());
		Menu_setChecked(IDLUPE100, FALSE);
		Menu_setChecked(IDLUPE150, FALSE);
		Menu_setChecked(IDLUPE200, FALSE);
		Menu_setChecked(IDLUPE400, FALSE);
		switch (m_LupeWnd.GetZoom()) {
		case 100: Menu_setChecked(IDLUPE100, TRUE); break;
		case 150: Menu_setChecked(IDLUPE150, TRUE); break;
		case 200: Menu_setChecked(IDLUPE200, TRUE); break;
		case 400: Menu_setChecked(IDLUPE400, TRUE); break;
		}
		SetWindowTitle();
	}
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		if (callFunc(id)) {
			return;
		}
		switch (id) {
		case TxtDocMessage::OPEN_DD_FILE:
		{
			int iLinkNo = -1;
			int iPage = -1;
			auto lpFileName = m_dropFileName.c_str();
			if (codeNotify == 1) {
				int nArgs;
				auto* lplpszArgsBegin = CommandLineToArgvW(lpFileName, &nArgs);
				auto* lplpszArgs = lplpszArgsBegin;
				for (int i = 0; i < nArgs; ++i, ++lplpszArgs) {
					auto lpArgs = *lplpszArgs;
					if (lpArgs[0] == _T('-')) {
						if (CGrText::niCmp(lpArgs, _T("-s="), 3) == 0) {
							iLinkNo = CGrText::toInt(&(lpArgs[3])) - 1; // 表紙の分を引く
						}
						else if (CGrText::niCmp(lpArgs, _T("-p="), 3) == 0) {
							iPage = CGrText::toInt(&(lpArgs[3])); // ページ番号
						}
					}
					else {
						m_dropFileName = *lplpszArgs;
						lpFileName = m_dropFileName.c_str();
					}
				}
				if (lplpszArgsBegin) {
					LocalFree(lplpszArgsBegin);
				}
			}
			OpenDocumentCommandWithAddFileList(lpFileName);
			if (iLinkNo >= 0) {
				iPage = m_doc.GetPageByLinkNo(iLinkNo);
			}
			if (iPage >= 0) {
				GotoPage(iPage);
			}
		}
		break;
		case TxtDocMessage::OPEN_DD_FILE_NOHIST:
			OpenDocument(m_dropFileName.c_str());
			break;
		case TxtDocMessage::OPEN_FILE:
			m_dropFileName = reinterpret_cast<LPCTSTR>(hwndCtl);
			OpenDocumentCommandWithAddFileList(m_dropFileName.c_str());
			break;
		case IDBOOKMARKAUTOSAVE:
			m_bBookMarkAutoSave = !m_bBookMarkAutoSave;
			Menu_setChecked(IDBOOKMARKAUTOSAVE, m_bBookMarkAutoSave);
			break;
		case IDSELECTIONMODE:
			ToggleSelectionMode();
			break;
		case IDRPAGEMODENORMAL:
			SetPageMode(PAGE_MODE_NORMAL);
			break;
		case IDPPAGEMODEAUTO:
			SetPageMode(PAGE_MODE_AUTO);
			break;
		case IDPPAGEMODESINGLE:
			SetPageMode(PAGE_MODE_SINGLE);
			break;
		case IDPPAGEMODESINGLECENTER:
			SetPageMode(PAGE_MODE_SINGLE_CENTER);
			break;
		case IDBMAOZORA:
			ShowAozoraList();
			break;
		case IDBMDBTOOL:
			ExecuteTool(_T("TxtMiruDBTool"));
			break;
		case IDBMDBIMPORT:
			ExecuteTool(_T("TxtMiruDBImportBookmark"));
			break;
		case IDBMCACHE:
			ExecuteTool(_T("TxtMiruCache"));
			break;
		case IDAUTOHIDEMENU:
			setAutoHide(!m_bAutoHideMenu);
			break;
		case TxtDocMessage::UPDATE_CMD_UI:
			UpdateCmdUI();
			break;
		case TxtDocMessage::UPDATE_KEYBORD: loadKeyMap(); break;
		case TxtDocMessage::UPDATE_CONFIG: /* through */// refresh(); break;
		case TxtDocMessage::UPDATE_LAYOUT: refresh(); break;
		case TxtDocMessage::UPDATE_STYLE_LIST: SetStyleMenu(); break;
		case TxtDocMessage::GOTO_NPAGE:
			GotoPageCommand(static_cast<int>(codeNotify));
			::SetFocus(m_hWnd);
			break;
		default:
			if (IDRECENTFILE + 1 + 1 <= id && id <= IDRECENTFILE + 1 + 10) {
				// 最近使ったファイルを メニューから開く
				int num = id - (IDRECENTFILE + 1 + 1);
				int index = static_cast<int>(m_openFileList.size()) - num - 1;
				if (index >= 0 && index < static_cast<int>(m_openFileList.size())) {
					const auto& ofi = m_openFileList[index];
					OpenDocumentCommandWithAddFileList(ofi.filename.c_str());
				}
			}
			else if (IDPREPARSER_DEF == id) {
				if (Menu_checked(id)) {
					break;
				}
				std::tstring folderName;
				if (CGrShell::MakeFullPath(folderName, nullptr, _T("Script"))) {
					auto&& param = CGrTxtMiru::theApp().Param();
					param.SetText(CGrTxtParam::PreParserFolder, folderName.c_str());
				}
				setCheckPreParserMenu(id);
				Reload();
			}
			else if (IDRPREPARSERMENU <= id && id <= IDRPREPARSERMENU + 100) {
				if (Menu_checked(id)) {
					break;
				}
				int idx = id - IDRPREPARSERMENU;
				if (idx >= 0 && idx < static_cast<signed int>(m_preParserFolderList.size())) {
					const auto& ofi = m_preParserFolderList[idx];
					auto&& param = CGrTxtMiru::theApp().Param();
					param.SetText(CGrTxtParam::PreParserFolder, ofi.filename.c_str());
					setCheckPreParserMenu(id);
					Reload();
				}
			}
			else if (IDINITSTYLEMENU <= id && id <= IDINITSTYLEMENU + 100) {
				int idx = id - IDINITSTYLEMENU;
				if (idx >= 0 && idx < static_cast<signed int>(m_styleList.size())) {
					const auto& ofi = m_styleList[idx];
					loadStyle(ofi.filename.c_str());
				}
			}
			break;
		}
	}
	// 水平スクロール
	void OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
	{
		SCROLLINFO si = { sizeof(SCROLLINFO) };
		// スクロールバーの情報取得
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
		::GetScrollInfo(hwnd, SB_HORZ, &si);

		if (!m_bSinglePage) {
			si.nMax -= 1;
		}
		switch (code) {
		case SB_LINEUP: NextPage(); break;
		case SB_LINEDOWN: PrevPage(); break;
		case SB_THUMBPOSITION: GotoPage(si.nMax - pos); break;
		case SB_THUMBTRACK: GotoPage(si.nMax - si.nTrackPos); break;
		case SB_PAGEUP: NextPage(); break;
		case SB_PAGEDOWN: PrevPage(); break;
		case SB_TOP: GotoPage(m_totalPage); break;
		case SB_BOTTOM: GotoPage(0); break;
		case SB_ENDSCROLL: break;
		default: break;
		}
	}
	void OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
	{
		SCROLLINFO si = { sizeof(SCROLLINFO) };
		// スクロールバーの情報取得
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
		::GetScrollInfo(hwnd, SB_VERT, &si);

		switch (code) {
		case SB_LINEUP: PrevPage(); break;
		case SB_LINEDOWN: NextPage(); break;
		case SB_THUMBPOSITION: GotoPage(pos); break;
		case SB_THUMBTRACK: GotoPage(si.nTrackPos); break;
		case SB_PAGEUP: PrevPage(); break;
		case SB_PAGEDOWN: NextPage(); break;
		case SB_TOP: GotoPage(0); break;
		case SB_BOTTOM: GotoPage(m_totalPage); break;
		case SB_ENDSCROLL: break;
		default: break;
		}
	}
	void startMouseSpProc()
	{
		m_txtCanvas.ValidateSelect(false);
		m_bSpSelectionModeCopied = false;
		SetSpSelectionMode(true);
		auto hcursor = static_cast<HCURSOR>(::LoadImage(NULL, MAKEINTRESOURCE(OCR_IBEAM), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
		::SetCursor(hcursor);
		m_txtCanvas.SetBeginSelect(m_clickLPos);
	}
	void ExecuteSpProc(bool bCancel)
	{
		if (!m_bSpSelectionModeCopied) {
			const auto& param = CGrTxtMiru::theApp().Param();
			if (param.GetBoolean(CGrTxtParam::RunExecCopyText)) {
				ExecCopyText();
			}
			else {
				CopyText();
			}
		}
		if (bCancel) {
			SetSpSelectionMode(false);
		}
		m_bSpSelectionModeCopied = true;
	}
	void SetSpSelectionMode(bool bMode)
	{
		if (m_bSpSelectionMode == bMode) {
			return;
		}
		m_bSpSelectionMode = bMode;
		if (m_bSpSelectionMode) {
			;
		}
		else {
			if (!m_bSelectionMode) {
				m_txtCanvas.SetCurrentHoverLink(-1);
				m_txtCanvas.ValidateSelect(false);
			}
			else if (m_txtCanvas.GetCurrentHoverLink() >= 0) {
				m_txtCanvas.SetCurrentHoverLink(-1);
			}
			else {
				return;
			}
		}
		m_txtCanvas.Update(); // 選択モードの解除も兼ねる
		::InvalidateRect(m_hWnd, NULL, FALSE);
		::UpdateWindow(m_hWnd);
	}
	void OnKillFocus(HWND hwnd, HWND hwndNewFocus)
	{
		if ((m_bFullScreen || m_bAutoHideMenu) && ::GetMenu(m_hWnd)) {
			hideMenu(); // フルスクリーンのときメニューが表示されていて メニューが閉じられた時は、メニューを隠す
		}
		SetSpSelectionMode(false);
	}
#define IsKeyDown(vk) (::GetAsyncKeyState(vk) & 0x8000)
	bool isMouseLButtonDown()
	{
		if (::GetSystemMetrics(SM_SWAPBUTTON)) { // 左右逆の設定
			if (IsKeyDown(VK_RBUTTON)) {
				return true;
			}
		}
		else {
			if (IsKeyDown(VK_LBUTTON)) {
				return true;
			}
		}
		return false;
	}
	bool KeyEvent(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
	{
		if (fDown) {
			if (!m_bSelectionMode) {
				if (IsKeyDown(VK_SHIFT)) {
					const auto& param = CGrTxtMiru::theApp().Param();
					if (param.GetBoolean(CGrTxtParam::SpSelectionMode)) {
						SetSpSelectionMode(true);
						auto hcursor = static_cast<HCURSOR>(::LoadImage(NULL, MAKEINTRESOURCE(OCR_IBEAM), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
						::SetCursor(hcursor);
					}
				}
				else if (vk == VK_ESCAPE) {
					SetSpSelectionMode(false);
				}
			}
			if (flags & KF_REPEAT) {
				if (m_bKeyRepeat && CGrMouseEvent::IsNoBind() && getKeyFunc(vk) && !getKeyFunc(vk, true)) {
					return execKeyFunc(vk);
				}
			}
			else if (CGrMouseEvent::IsNoBind() && getKeyFunc(vk)) {
				return execKeyFunc(vk);
			}
		}
		else {
			// KF_REPEAT : HIWORD(lParam) = flags
			const auto& param = CGrTxtMiru::theApp().Param();
			if (param.GetBoolean(CGrTxtParam::AutoCopyMode)) {
				if (!IsKeyDown(VK_SHIFT) && !isMouseLButtonDown()) {
					ExecuteSpProc(true);
				}
			}
			else if (m_bSpSelectionMode && !IsKeyDown(VK_SHIFT) && !isMouseLButtonDown()) {
				m_bSpSelectionMode = false;
			}
		}
		return false;
	}
	void OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
	{
		KeyEvent(hwnd, vk, fDown, cRepeat, flags);
	}
	void OnSysKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
	{
		if (!KeyEvent(hwnd, vk, fDown, cRepeat, flags)) {
			if (fDown) {
				if (m_bFullScreen || m_bAutoHideMenu) {
					// フルスクリーンのときは、メニューを表示させる
					m_txtCanvas.SetOffset(0, 0);
					if (!::GetMenu(m_hWnd)) {
						m_txtCanvas.SetOffset(0, -GetSystemMetrics(SM_CYMENU));
						SetFocus(m_hWnd);
						::SetMenu(m_hWnd, m_hMenu);
						::InvalidateRect(m_hWnd, NULL, FALSE);
						::UpdateWindow(m_hWnd);
					}
				}
				FORWARD_WM_SYSKEYDOWN(hwnd, vk, cRepeat, flags, DefWindowProc);
			}
			else {
				FORWARD_WM_SYSKEYUP(hwnd, vk, cRepeat, flags, DefWindowProc);
			}
		}
		else {
			// KeyEvent後は、ALTキーを離したことにする
			FORWARD_WM_SYSKEYUP(hwnd, VK_MENU, 0, 0, DefWindowProc);
		}
	}
	void OnMouseButtonDown(UINT vk, UINT vk_dbl, pOnKeyCallbackFunc pFunc)
	{
		if (CGrMouseEvent::IsNoBind()) {
			auto singleClick = getKeyFunc(vk);
			if (!singleClick) {
				singleClick = pFunc;
			}
			CGrMouseEvent::SetFunction(
				singleClick,
				getKeyFunc(vk, true),
				getKeyFunc(vk_dbl, false),
				getKeyFunc(vk_dbl, true)
			);
		}
		CGrMouseEvent::OnButtonDown(vk);
	}
	// Mouse Left Button
	void OnLButtonSingleClick()
	{
		if (m_txtCanvas.GetCurrentHoverLink() >= 0) {
			return;
		}
		POINT pos = {};
		if (GetCursorPos(&pos)) {
			::ScreenToClient(m_hWnd, &pos);
		}
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		int center = rect.left + (rect.right - rect.left) / 2;
		if (pos.x < center) {
			NextPage();
		}
		else {
			PrevPage();
		}
	}
	void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		m_clickLPos.x = INT_MIN;
		m_clickLPos.y = INT_MIN;
		POINT pos = { x,y };
		::SetFocus(::WindowFromPoint(pos));
		if (m_bSelectionMode || m_bSpSelectionMode) {
			bool bpress_shift_key = IsKeyDown(VK_SHIFT) != 0;
			if (bpress_shift_key) {
				m_txtCanvas.ValidateSelect(false);
			}
			m_bSpSelectionModeCopied = false;
			pos.x += m_windowOffsetX;
			pos.y += m_windowOffsetY;
			if (bpress_shift_key && m_txtCanvas.SetEndSelect(pos)) {
				m_txtCanvas.ValidateSelect(true);
			}
			else if (!m_txtCanvas.SetBeginSelect(pos)) {
				return;
			}
		}
		else {
			m_clickLPos = pos;
			m_txtCanvas.ValidateSelect(false);
			OnMouseButtonDown(VK_LBUTTON, VK_LBUTTON_DBL, &CGrMainWindow::OnLButtonSingleClick);
		}
		::InvalidateRect(m_hWnd, NULL, FALSE);
		::UpdateWindow(m_hWnd);
	}
	void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
	{
		if (m_bSpSelectionMode) {
			const auto& param = CGrTxtMiru::theApp().Param();
			if (param.GetBoolean(CGrTxtParam::AutoCopyMode)) {
				m_txtCanvas.ValidateSelect(true);
				if (!IsKeyDown(VK_SHIFT)) {
					ExecuteSpProc(true);
				}
			}
		}
		else if (m_bSelectionMode) {
			m_txtCanvas.ValidateSelect(true);
			const auto& param = CGrTxtMiru::theApp().Param();
			if (param.GetBoolean(CGrTxtParam::AutoCopyMode)) {
				if (!IsKeyDown(VK_SHIFT)) {
					ExecuteSpProc(true);
				}
			}
		}
		else if (!linkOpenGoto() && m_clickLPos.x > INT_MIN && m_clickLPos.y > INT_MIN) {
			CGrMouseEvent::OnButtonUp(VK_LBUTTON);
		}
		m_clickLPos.x = INT_MIN;
		m_clickLPos.y = INT_MIN;
	}
	// Right Mouse Button
	void OnRButtonSingleClick()
	{
		SetSpSelectionMode(false);
		UINT flags = 0;
		POINT pos = {};
		if (GetCursorPos(&pos)) {
			::ScreenToClient(m_hWnd, &pos);
		}
		::DefWindowProc(m_hWnd, WM_RBUTTONUP, flags, MAKELPARAM((pos.x), (pos.y)));
	}
	void OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		OnMouseButtonDown(VK_RBUTTON, VK_RBUTTON_DBL, &CGrMainWindow::OnRButtonSingleClick);
	}
	void OnRButtonUp(HWND hwnd, int x, int y, UINT flags)
	{
		CGrMouseEvent::OnButtonUp(VK_RBUTTON);
	}
	// Mouse Middle Button
	void OnMButtonSingleClick()
	{
		execKeyFunc(VK_MBUTTON);
	}
	void OnMButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		OnMouseButtonDown(VK_MBUTTON, VK_MBUTTON, &CGrMainWindow::OnMButtonSingleClick);
	}
	void OnMButtonUp(HWND hwnd, int x, int y, UINT flags)
	{
		CGrMouseEvent::OnButtonUp(VK_MBUTTON);
	}
	// Mouse X Button
	void OnXButton1SingleClick()
	{
		execKeyFunc(VK_XBUTTON1);
	}
	void OnXButton2SingleClick()
	{
		execKeyFunc(VK_XBUTTON2);
	}
	void OnXButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		switch (GET_XBUTTON_WPARAM(keyFlags)) {
		case XBUTTON1: OnMouseButtonDown(VK_XBUTTON1, VK_XBUTTON1_DBL, &CGrMainWindow::OnXButton1SingleClick); break;
		case XBUTTON2: OnMouseButtonDown(VK_XBUTTON2, VK_XBUTTON2_DBL, &CGrMainWindow::OnXButton2SingleClick); break;
		}
	}
	void OnXButtonUp(HWND hwnd, int x, int y, UINT flags)
	{
		switch (GET_XBUTTON_WPARAM(flags)) {
		case XBUTTON1: CGrMouseEvent::OnButtonUp(VK_XBUTTON1); break;
		case XBUTTON2: CGrMouseEvent::OnButtonUp(VK_XBUTTON2); break;
		}
	}
	//////////
#define MOUSEEVENTF_FROMTOUCH 0xFF515700
	void OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
	{
		POINT pos = { static_cast<LONG>(xPos),static_cast<LONG>(yPos) };
		if ((xPos == 65535 && yPos == 65535) || (xPos == 4294967295 && yPos == 4294967295)) {
			// MENUキー
			pos.x = 0;
			pos.y = 0;
			if (!GetCursorPos(&pos)) {
				::ClientToScreen(m_hWnd, &pos);
			}
		}
		else if (hwndContext == m_hWnd) {
			if (HTCLIENT != FORWARD_WM_NCHITTEST(hwnd, xPos, yPos, SendMessage)) {
				FORWARD_WM_CONTEXTMENU(hwnd, hwndContext, xPos, yPos, DefWindowProc);
				return;
			}
		}
		CGrTxtMiruMenu m;
		m.Open();
		auto fnid = m.Show(m_hWnd, pos.x, pos.y, m_bSelectionMode, m_txtCanvas.GetCurrentHoverLink() >= 0);
		if (fnid < TxtMiru::FnN_MaxNum && fnid != TxtMiru::FnN_Nop) {
			const auto it = m_func_map.find(TxtMiru::l_TxtMiruFuncNameList[fnid]);
			if (it != m_func_map.end()) {
				auto pFunc = (it->second);
				(this->*pFunc)();
			}
		}
	}
	void OnNCMouseMove(HWND hwnd, int x, int y, UINT codeHitTest)
	{
		if (m_bFullScreen || m_bAutoHideMenu) {
			bool bUpdate = false;
			bool bUpdateScrollBar = false;
			POINT pos = { x,y };
			::ScreenToClient(m_hWnd, &pos);
			if (codeHitTest != HTMENU && ::GetMenu(m_hWnd)) {
				if (IsKeyDown(VK_MENU)) {
					//
				}
				else {
					m_txtCanvas.SetOffset(0, 0);
					::SetMenu(m_hWnd, NULL);
					bUpdate = true;
				}
			}
			if (codeHitTest != HTHSCROLL && m_bVisibleHScrollbar && m_iHScrollHeight != 0) {
				m_iHScrollHeight = 0;
				bUpdateScrollBar = true;
			}
			if (codeHitTest != HTVSCROLL && m_bVisibleVScrollbar && m_iVScrollWidth != 0) {
				m_iVScrollWidth = 0;
				bUpdateScrollBar = true;
			}
			if (bUpdateScrollBar) {
				showScrollBar();
			}
			if (bUpdate || bUpdateScrollBar) {
				::UpdateWindow(m_hWnd);
			}
		}
		//FORWARD_WM_NCMOUSEMOVE(hwnd, x, y, codeHitTest, DefWindowProc);
	}
	void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
	{
		POINT pos = { x,y };
		m_LupeWnd.UpdatePos(pos);
		pos.x += m_windowOffsetX;
		pos.x += m_windowOffsetY;
		if (!m_bInvalid) {
			if (m_bSelectionMode || m_bSpSelectionMode) {
				if (m_txtCanvas.IsTextArea(pos.x, pos.y)) {
					auto hcursor = static_cast<HCURSOR>(::LoadImage(NULL, MAKEINTRESOURCE(OCR_IBEAM), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
					::SetCursor(hcursor);
				}
				// 選択モード
				if (m_txtCanvas.SetEndSelect(pos)) {
					m_bSpSelectionModeCopied = false;
					::InvalidateRect(m_hWnd, NULL, FALSE);
					::UpdateWindow(m_hWnd);
					return;
				}
			}
			else {
				int id = m_txtCanvas.GetLinkIndex(pos.x, pos.y);
				if (m_txtCanvas.GetCurrentHoverLink() != id) {
					m_txtCanvas.SetCurrentHoverLink(id);
					m_txtCanvas.Update();
					InvalidateRect(m_hWnd, NULL, FALSE);
					std::tstring url(m_txtCanvas.GetLinkURL(id));
					m_MessWnd.Hide();
					m_MessWnd.Show(m_hWnd, url.c_str());
				}
				if (id >= 0) {
					auto hcursor = static_cast<HCURSOR>(::LoadImage(NULL, MAKEINTRESOURCE(OCR_HAND), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
					::SetCursor(hcursor);
				}
			}
		}
		if (m_bFullScreen || m_bAutoHideMenu) {
			if (y < GetSystemMetrics(SM_CYMENU)) {
				// 上にきたらメニュー表示
				m_txtCanvas.SetOffset(0, -GetSystemMetrics(SM_CYMENU));
				if (!::GetMenu(m_hWnd)) {
					::SetMenu(m_hWnd, m_hMenu);
					::UpdateWindow(m_hWnd);
				}
			}
			else if (::GetMenu(m_hWnd)) {
				if (!IsKeyDown(VK_MENU)) {
					hideMenu();
				}
			}
			else {
				bool bUpdate = false;
				RECT rect;
				GetClientRect(m_hWnd, &rect);
				if (m_bVisibleHScrollbar) {
					// 水平スクロールバー付近にきたらスクロールバー表示
					if (m_iHScrollHeight == 0) {
						if (y > rect.bottom - GetSystemMetrics(SM_CYHSCROLL)) {
							m_iHScrollHeight = GetSystemMetrics(SM_CYHSCROLL);
							bUpdate = true;
						}
					}
					else if (y < rect.bottom) {
						m_iHScrollHeight = 0;
						bUpdate = true;
					}
				}
				if (m_bVisibleVScrollbar) {
					// 垂直スクロールバー付近にきたらスクロールバー表示
					if (m_iVScrollWidth == 0) {
						if (x > rect.right - GetSystemMetrics(SM_CXVSCROLL)) {
							m_iVScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
							bUpdate = true;
						}
					}
					else if (x < rect.right) {
						m_iVScrollWidth = 0;
						bUpdate = true;
					}
				}
				if (bUpdate) {
					showScrollBar();
					::UpdateWindow(m_hWnd);
				}
			}
		}
	}
	void OnSize(HWND hwnd, UINT nType, int cx, int cy)
	{
		resize(cx, cy);
	}
	//
	void addPreParserFolderList(LPCTSTR path)
	{
		// Pre-Parserメニュー
		std::vector<std::tstring> flist;
		CGrShell::GetFolderList(flist, path);
		int len = flist.size();
		if (len > 0) {
			int first_no = m_preParserFolderList.size();
			if (first_no != 0) {
				m_preParserFolderList.push_back(OpenFileInfo());
			}
			for (const auto& item : flist) {
				OpenFileInfo ofi;
				ofi.filename = item;
				ofi.title = CGrShell::GetFileName(item.c_str());
				m_preParserFolderList.push_back(ofi);
			}
			std::sort(std::begin(m_preParserFolderList) + first_no, std::end(m_preParserFolderList), SortCompare());
		}
	}
	//
	void setCheckPreParserMenu(UINT iSelectID)
	{
		auto hMenu = getMenu();
		if (!hMenu) {
			return;
		}
		CheckMenuRadioItem(hMenu, IDPREPARSER_DEF, IDRPREPARSERMENU + 100, iSelectID, MF_BYCOMMAND);
	}
	//
	struct SortCompare {
		SortCompare() {}
		bool operator()(const OpenFileInfo& c1, const OpenFileInfo& c2) const {
			return c1.filename < c2.filename;
		}
	};
	void addStyleList(LPCTSTR path)
	{
		std::vector<WIN32_FIND_DATA> flist;
		CGrShell::GetFileList(flist, path);
		int len = flist.size();
		if (len > 0) {
			int first_no = m_styleList.size();
			bool bFirst = true;
			for (const auto& wfd : flist) {
				if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					continue;
				}
				auto ext = CGrShell::GetFileExtConst(wfd.cFileName);
				if (!ext || CGrText::niCmp(_T("ini"), ext, 3) != 0) {
					continue;
				}
				OpenFileInfo ofi;
				std::tstring str;
				CGrText::FormatMessage(str, _T("%1!s!\\%2!s!"), path, wfd.cFileName);
				CGrShell::ToPrettyFileName(str.c_str(), ofi.filename);
				TCHAR buf[512];
				::GetPrivateProfileString(_T("TxtMiru"), _T("StyleName"), _T(""), buf, sizeof(buf) / sizeof(TCHAR), ofi.filename.c_str());
				if (_tcslen(buf) > 0) {
					ofi.title = buf;
					if (bFirst) {
						m_styleList.push_back(OpenFileInfo());
						bFirst = false;
					}
					m_styleList.push_back(ofi);
				}
			}
			std::sort(m_styleList.begin() + first_no, m_styleList.end(), SortCompare());
		}
	}
	void SetStyleMenu()
	{
		m_styleList.clear();
		m_styleList.shrink_to_fit();
		auto hMenu = getMenu();
		if (!hMenu) {
			return;
		}
		CGrMenuFunc::DeleteMenuRange(hMenu, IDINITSTYLEMENU, 100);
		do {
			// 本体のパスから
			std::tstring folderName;
			if (CGrShell::MakeFullPath(folderName, nullptr, _T("Style"))) {
				addStyleList(folderName.c_str());
			}
			// データのパスから
			auto lpDataPath = CGrTxtMiru::GetDataPath();
			if (lpDataPath) {
				std::tstring datafolder;
				CGrShell::MakeFullPath(datafolder, lpDataPath, _T("Style"));
				if (_tcsicmp(folderName.c_str(), datafolder.c_str()) != 0) {
					// 本体のパスと別のパスが指定されていた場合のみ
					addStyleList(datafolder.c_str());
				}
			}
			//
			int len = m_styleList.size();
			if (len > 0) {
				auto hMenuDef = CGrMenuFunc::GetParentMenu(hMenu, IDINITSTYLE);
				if (!hMenuDef) {
					break;
				}
				MENUITEMINFO mi = { sizeof(MENUITEMINFO) };
				mi.wID = IDINITSTYLEMENU;
				std::tstring str;
				for (const auto& item : m_styleList) {
					if (item.title.empty()) {
						mi.fMask = MIIM_FTYPE | MIIM_ID;
						mi.fType = MFT_SEPARATOR;
					}
					else {
						mi.fMask = MIIM_STRING | MIIM_ID;
						CGrText::FormatMessage(str, _T("%1!s!"), item.title.c_str());
						mi.dwTypeData = const_cast<LPTSTR>(str.c_str());
					}
					::InsertMenuItem(hMenuDef, -1, TRUE, &mi);
					++mi.wID;
				}
			}
		} while (0);
	}
	void SetPreParserMenu()
	{
		m_preParserFolderList.clear();
		m_preParserFolderList.shrink_to_fit();
		auto hMenu = getMenu();
		if (!hMenu) {
			return;
		}
		CGrMenuFunc::DeleteMenuRange(hMenu, IDRPREPARSERMENU, 100);
		//
		const auto& param = CGrTxtMiru::theApp().Param();
		std::tstring filename;
		param.GetText(CGrTxtParam::PreParserFolder, filename);
		do {
			// 本体のパスから
			std::tstring folderName;
			if (CGrShell::MakeFullPath(folderName, nullptr, _T("Script"))) {
				addPreParserFolderList(folderName.c_str());
			}
			//
			// データのパスから
			auto lpDataPath = CGrTxtMiru::GetDataPath();
			if (lpDataPath) {
				std::tstring datafolder;
				CGrShell::MakeFullPath(datafolder, lpDataPath, _T("Script"));
				if (_tcsicmp(folderName.c_str(), datafolder.c_str()) != 0) {
					// 本体のパスと別のパスが指定されていた場合のみ
					addPreParserFolderList(datafolder.c_str());
				}
			}
			//
			int len = m_preParserFolderList.size();
			if (len > 0) {
				auto hMenuDef = CGrMenuFunc::GetParentMenu(hMenu, IDPREPARSER_DEF);
				if (!hMenuDef) {
					break;
				}
				UINT iSelectID = IDPREPARSER_DEF;
				MENUITEMINFO mi = { sizeof(MENUITEMINFO) };
				mi.wID = IDRPREPARSERMENU;
				std::tstring str;
				for (const auto& item : m_preParserFolderList) {
					if (item.title.empty()) {
						mi.fMask = MIIM_FTYPE | MIIM_ID;
						mi.fType = MFT_SEPARATOR;
					}
					else {
						mi.fMask = MIIM_STRING | MIIM_ID;
						CGrText::FormatMessage(str, _T("%1!s!"), item.title.c_str());
						mi.dwTypeData = const_cast<LPTSTR>(str.c_str());
						if (_tcsicmp(filename.c_str(), item.filename.c_str()) == 0) {
							iSelectID = mi.wID;
						}
					}
					::InsertMenuItem(hMenuDef, -1, TRUE, &mi);
					++mi.wID;
				}
				setCheckPreParserMenu(iSelectID);
			}
		} while (0);
	}
	void SetPageMode(PAGE_MODE pm)
	{
		Menu_setChecked(IDRPAGEMODENORMAL, FALSE);
		Menu_setChecked(IDPPAGEMODEAUTO, FALSE);
		Menu_setChecked(IDPPAGEMODESINGLE, FALSE);
		Menu_setChecked(IDPPAGEMODESINGLECENTER, FALSE);
		switch (pm) {
		case PAGE_MODE_NORMAL: Menu_setChecked(IDRPAGEMODENORMAL, TRUE); m_pageMode = PAGE_MODE_NORMAL; break;
		case PAGE_MODE_AUTO: Menu_setChecked(IDPPAGEMODEAUTO, TRUE); m_pageMode = PAGE_MODE_AUTO; break;
		case PAGE_MODE_SINGLE: Menu_setChecked(IDPPAGEMODESINGLE, TRUE); m_pageMode = PAGE_MODE_SINGLE; break;
		case PAGE_MODE_SINGLE_CENTER: Menu_setChecked(IDPPAGEMODESINGLECENTER, TRUE); m_pageMode = PAGE_MODE_SINGLE_CENTER; break;
		default:                      Menu_setChecked(IDRPAGEMODENORMAL, TRUE); m_pageMode = PAGE_MODE_NORMAL; break;
		}
		refresh();
	}
	void loadStyle(LPCTSTR lpFileName)
	{
		auto&& param = CGrTxtMiru::theApp().Param();
		if (param.Load(lpFileName)) {
			refresh();
		}
	}
	//
	HMENU m_hRecentMenu = NULL;
	HMENU getRecentMenu()
	{
		if (m_hRecentMenu) {
			return m_hRecentMenu;
		}
		// FullScreen 時、hMenu をブランクにしているので
		auto hMenu = getMenu();
		if (!hMenu) {
			return NULL;
		}
		m_hRecentMenu = ::CreatePopupMenu();
		MENUITEMINFO mi = { sizeof(MENUITEMINFO) };
		mi.fMask = MIIM_SUBMENU;
		mi.hSubMenu = m_hRecentMenu;
		::SetMenuItemInfo(hMenu, IDRECENTFILE, FALSE, &mi);
		return m_hRecentMenu;
	}

	// 最近使ったファイルメニュー
	void SetRecentMenu()
	{
		auto hMenu = getRecentMenu();
		if (!hMenu) {
			return;
		}
		CGrMenuFunc::DeleteMenuRange(hMenu, IDRECENTFILE + 1, (12 - 1), false);
		MENUITEMINFO mi = { sizeof(MENUITEMINFO) };
		mi.fMask = MIIM_TYPE | MIIM_ID;
		mi.hSubMenu = hMenu;
		std::tstring str;
		int len = m_openFileList.size();
		if (len == 0) {
			CGrText::LoadString(IDS_NOFILE, str);
			mi.dwTypeData = const_cast<LPTSTR>(str.c_str());
			mi.wID = IDRECENTFILE + 1;
			::InsertMenuItem(hMenu, 1, TRUE, &mi);
		}
		else {
			const auto* it = &m_openFileList[len - 1];
			len = min(10, len);
			for (int idx = 1; idx <= len; ++idx, --it) {
				auto lpTitle = it->title.c_str();
				if (it->title.empty()) {
					lpTitle = it->filename.c_str();
				}
				CGrText::FormatMessage(str, _T("&%1!d!: %2!s!"), idx % 10, lpTitle);
				if (str.size() > 200) {
					str.resize(200);
					str += _T("...");
				}
				mi.dwTypeData = const_cast<LPTSTR>(str.c_str());
				mi.wID = IDRECENTFILE + 1 + idx;
				::InsertMenuItem(hMenu, idx, TRUE, &mi);
			}
		}
	}
	void OpenDocumentText(LPCTSTR lpText)
	{
		m_bInvalid = true;
		CGrMouseEvent::Clear();
		CGrWaitCursor wait;
		CloseDocument();
		m_currentPage = 1;
		m_displayPage = 1;
		m_doc.OpenText(lpText);
		m_txtCanvas.Initialize();
		m_txtCanvas.UpdateParam();
		m_LupeWnd.UpdateParam();
		m_totalPage = m_doc.GetTotalPage();
		int pageFlipInterval = m_pageFlipInterval;
		m_pageFlipInterval = 0;
		GotoPage(1);
		m_pageFlipInterval = pageFlipInterval;
		SetWindowTitle();
		Menu_setEnabled(IDBOOKMARKSAVEAS, false);
		refreshToolWindow();
		m_bInvalid = false;
	}
	void OpenDocument(LPCTSTR lpFileName)
	{
		if (!lpFileName || lpFileName[0] == '\0') {
			return;
		}
		m_bInvalid = true;
		CGrMouseEvent::Clear();
		CGrWaitCursor wait;
		CloseDocument();

		m_currentPage = 1;
		m_displayPage = 1;
		if (m_doc.OpenThread(m_hWnd, lpFileName)) {
			//
		}
		else {
			// エラーの時、履歴から削除
			bool bEraseData = true;
			while (bEraseData) {
				bEraseData = false;
				auto it = std::find(m_openFileList.begin(), m_openFileList.end(), lpFileName);
				if (it != m_openFileList.end()) {
					m_openFileList.erase(it);
					bEraseData = true;
				}
			}
			m_currentPage = 1;
			m_displayPage = 1;
			SetRecentMenu();
			m_doc.OpenError();
		}
		m_txtCanvas.Initialize();
		m_txtCanvas.UpdateParam();
		m_LupeWnd.UpdateParam();
		m_totalPage = m_doc.GetTotalPage();
		int pageFlipInterval = m_pageFlipInterval;
		m_pageFlipInterval = 0;
		GotoPage(m_doc.GetLastPage());
		m_pageFlipInterval = pageFlipInterval;
		Menu_setEnabled(IDBOOKMARKSAVEAS, true);

		refreshToolWindow();
		SetWindowTitle();
		const auto& param = CGrTxtMiru::theApp().Param();
		if (param.GetBoolean(CGrTxtParam::FileAutoReload)) {
			std::tstring filename;
			m_doc.GetFileName(filename);
			m_FileUpdateWatcher.start(filename.c_str(), m_hWnd);
		}
		m_bInvalid = false;
	}
	// 最近使ったファイルの一覧に追加
	void AddFileList()
	{
		std::tstring filename;
		m_doc.GetFileName(filename); // 栞ファイルから開いているかもしれないので、本体ファイル名を取得する
		auto&& param = CGrTxtMiru::theApp().Param();
		param.SetText(CGrTxtParam::LastFolder, filename.c_str());
		param.SetText(CGrTxtParam::LastFile, filename.c_str());
		auto it = std::find(m_openFileList.begin(), m_openFileList.end(), filename.c_str());
		if (it != m_openFileList.end()) {
			m_openFileList.erase(it);
		}
		OpenFileInfo ofi;
		ofi.filename = filename.c_str();
		if (!m_doc.GetTitle().empty() || !m_doc.GetAuthor().empty()) {
			CGrText::FormatMessage(ofi.title, _T("%1!s!:%2!s!"), m_doc.GetTitle().c_str(), m_doc.GetAuthor().c_str());
			std::replaceCRLF(ofi.title, _T(""));
		}
		m_openFileList.push_back(ofi);
		Menu_setEnabled(IDBOOKMARKSAVEAS, true);
		SetRecentMenu();
	}
	void OpenDocumentClipboard()
	{
		m_bInvalid = true;
		CGrMouseEvent::Clear();
		std::tstring filename;
		std::tstring old_filename;
		m_doc.GetFileName(old_filename);
		doCommand(new CGrOpenFileCommand(old_filename.c_str(), filename.c_str()));
		//
		CGrWaitCursor wait;
		CloseDocument();
		m_currentPage = 1;
		m_displayPage = 1;
		m_doc.OpenClipboard();
		m_txtCanvas.Initialize();
		m_txtCanvas.UpdateParam();
		m_LupeWnd.UpdateParam();
		m_totalPage = m_doc.GetTotalPage();
		GotoPage(1);
		SetWindowTitle();
		Menu_setEnabled(IDBOOKMARKSAVEAS, false);
		refreshToolWindow();
		m_bInvalid = false;
	}
	void CloseDocument()
	{
		KillTimer(m_hWnd, ID_TIMER_AUTOSAVE);
		SetSpSelectionMode(false);
		m_FileUpdateWatcher.stop();
		m_MessWnd.Hide();
		if (m_bBookMarkAutoSave) {
			m_doc.SetLastPage(m_displayPage);
			m_doc.Save();
		}
		m_txtCanvas.SetCurrentHoverLink(-1);
	}
	void loadKeyMap()
	{
		CGrFunctionKeyMap::FunctionMap fmap;
		if (!CGrFunctionKeyMap::Load(m_keyBindFileName, fmap)) {
			initKeyMap(fmap);
			CGrFunctionKeyMap::Save(m_keyBindFileName, fmap);
		}
		m_keyfunc_map.clear();
		for (const auto& item : fmap) {
			const auto& ks = item.first;
			const auto& fn = item.second;
			m_keyfunc_map[ks] = m_func_map[fn];
		}
		// 非選択モードの時、左クリック長押しで一時的に選択モードに切り替える
		const auto& param = CGrTxtMiru::theApp().Param();
		int iSpSelectionMode[2] = {};
		param.GetPoints(CGrTxtParam::SpSelectionMode, iSpSelectionMode, sizeof(iSpSelectionMode) / sizeof(int));
		if (iSpSelectionMode[1] == 1) {
			m_keyfunc_map[CGrKeyboardState(VK_LBUTTON, false, false, false, true)] = &CGrMainWindow::startMouseSpProc;
		}
		//
	}
	void restoreWindow()
	{
		WINDOWPLACEMENT wndp = m_wndp;
		::SetWindowPlacement(m_hWnd, &wndp);
		if (m_wndp.showCmd & ~SW_SHOWMAXIMIZED) { // タスクバーから消えることがあるので、明示的に ShowWindow を行う
			::ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);
		}
		else {
			::ShowWindow(m_hWnd, SW_SHOW);
		}
	}
	void setScreenMode(bool bFullScreen, bool bAutoHideMenu)
	{
		enum SCREEN_MODE { SCRMOD_FULLSCREEN, SCRMOD_MENUHIDE, SCRMOD_NORMAL };
		auto from_mode = SCRMOD_NORMAL;
		auto to_mode = SCRMOD_NORMAL;
		if (m_bFullScreen) {
			from_mode = SCRMOD_FULLSCREEN;
		}
		else if (m_bAutoHideMenu) {
			from_mode = SCRMOD_MENUHIDE;
		}
		if (bFullScreen) {
			to_mode = SCRMOD_FULLSCREEN;
		}
		else if (bAutoHideMenu) {
			to_mode = SCRMOD_MENUHIDE;
		}
		m_bAutoHideMenu = bAutoHideMenu;
		m_bFullScreen = bFullScreen;
		Menu_setChecked(IDAUTOHIDEMENU, m_bAutoHideMenu);
		Menu_setChecked(IDFULLSCREEN, m_bFullScreen);
		if (from_mode == to_mode) {
			return;
		}
		m_bNoPaint = true;
		m_txtCanvas.SetOffset(0, 0);
		m_iHScrollHeight = 0;
		m_iVScrollWidth = 0;
		m_hMenu = getMenu();
		switch (from_mode) {
		case SCRMOD_NORMAL:
			m_style = ::GetWindowLong(m_hWnd, GWL_STYLE);
			break;
		case SCRMOD_MENUHIDE:
			m_style = ::GetWindowLong(m_hWnd, GWL_STYLE);
			break;
		case SCRMOD_FULLSCREEN:
			::SetWindowLong(m_hWnd, GWL_STYLE, m_style);
			{
				const auto& param = CGrTxtMiru::theApp().Param();
				if (param.GetBoolean(CGrTxtParam::TopMost)) {
					::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				}
				else {
					::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				}
			}

			break;
		}
		switch (to_mode) {
		case SCRMOD_FULLSCREEN:
		{
			WINDOWPLACEMENT wndp = { sizeof(WINDOWPLACEMENT) };
			::GetWindowPlacement(m_hWnd, &wndp);
			m_wndp = wndp;
			RECT rect = { 0 };
			auto hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFO mi = { sizeof(MONITORINFO) };
			if (hMonitor && ::GetMonitorInfo(hMonitor, &mi)) {
				rect.left = mi.rcMonitor.left;
				rect.top = mi.rcMonitor.top;
				rect.right = mi.rcMonitor.right - mi.rcMonitor.left;
				rect.bottom = mi.rcMonitor.bottom - mi.rcMonitor.top;
			}
			else {
				::GetWindowRect(m_hWnd, &rect);
			}
			::SetWindowLong(m_hWnd, GWL_STYLE, m_style & ~(WS_CAPTION | WS_SYSMENU | WS_SIZEBOX) | (0));
			::SetMenu(m_hWnd, NULL);
			{
				const auto& param = CGrTxtMiru::theApp().Param();
				if (param.GetBoolean(CGrTxtParam::TopMost)) {
					::SetWindowPos(m_hWnd, HWND_TOPMOST, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
				}
				else {
					::SetWindowPos(m_hWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
				}
			}
		}
		break;
		case SCRMOD_MENUHIDE:
			::SetMenu(m_hWnd, NULL);
			if (from_mode == SCRMOD_FULLSCREEN) {
				restoreWindow();
			}
			break;
		default:
			if (m_hMenu) {
				::SetMenu(m_hWnd, m_hMenu);
			}
			if (from_mode == SCRMOD_FULLSCREEN) {
				restoreWindow();
			}
			break;
		}
		showScrollBar();
		m_bNoPaint = false;
		refresh(false); // リサイズ時のレスポンス改善
		::UpdateWindow(m_hWnd);
		::SetFocus(m_hWnd);
	}

	void setAutoHide(bool bAutoHideMenu)
	{
		setScreenMode(m_bFullScreen, bAutoHideMenu);
	}
	void ToggleFullScreen()
	{
		fullScreen(!m_bFullScreen);
	}
	void fullScreen(bool bFullScreen)
	{
		setScreenMode(bFullScreen, m_bAutoHideMenu);
	}
	void ToggleSelectionMode()
	{
		m_bSelectionMode = !m_bSelectionMode;
		Menu_setChecked(IDSELECTIONMODE, m_bSelectionMode);
		if (!m_bSelectionMode) {
			m_txtCanvas.SetCurrentHoverLink(-1);
			m_txtCanvas.ValidateSelect(false);
		}
		else if (m_txtCanvas.GetCurrentHoverLink() >= 0) {
			m_txtCanvas.SetCurrentHoverLink(-1);
		}
		else {
			return;
		}
		m_txtCanvas.Update();
		::InvalidateRect(m_hWnd, NULL, FALSE);
		::UpdateWindow(m_hWnd);
	}

	BOOL OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
	{
		m_hWnd = hWnd;
		//
		SetProp(m_hWnd, TXTMIRU_PROP_NAME, (HANDLE)-1);
		// WorkPathが取得できるかどうかをチェック
		if (!CGrTxtMiru::GetWorkPath()) {
			CGrMessageBox::Show(hWnd, IDS_CANNOT_FOUND_WORKPATH, CGrTxtMiru::AppName());
			return FALSE;
		}
		//
		::RegisterDragDrop(m_hWnd, static_cast<IDropTarget*>(this));
		m_ClipFormat[CLIPFORMAT_UniformResourceLocatorFormat] = static_cast<CLIPFORMAT>(::RegisterClipboardFormat(_T("UniformResourceLocator")));
		m_ClipFormat[CLIPFORMAT_UniformResourceLocatorFormatW] = static_cast<CLIPFORMAT>(::RegisterClipboardFormat(_T("UniformResourceLocatorW")));
		m_ClipFormat[CLIPFORMAT_NetscapeBookmarkFormat] = static_cast<CLIPFORMAT>(::RegisterClipboardFormat(_T("Netscape Bookmark")));
		m_ClipFormat[CLIPFORMAT_FiledescriptorFormat] = static_cast<CLIPFORMAT>(::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR));
		m_ClipFormat[CLIPFORMAT_FilecontentsFormat] = static_cast<CLIPFORMAT>(::RegisterClipboardFormat(CFSTR_FILECONTENTS));
		//
		m_wm_funcCall = RegisterWindowMessage(WM_TXTMIRU_FUNC_CALL);
		//
		if (CGrGesture::Gesture().Initialize()) {
			GESTURECONFIG gc = { 0, GC_ALLGESTURES, 0 };
			CGrGesture::Gesture().SetGestureConfig(m_hWnd, 0, 1, &gc, sizeof(GESTURECONFIG));
		}
		CGrTxtMiru::theApp().InstallDLLFunc(CGrTxtMiru::DLLFNC_TxtFuncBookmark, &m_dlgLink);
		const auto& param = CGrTxtMiru::theApp().Param();
		// 最近使ったファイルの一覧読み込み
		for (int idx = 0; idx < 9; ++idx) {
			OpenFileInfo ofi;
			param.GetText(static_cast<CGrTxtParam::TextType>(CGrTxtParam::HistFile9 - idx), ofi.filename);
			if (!ofi.filename.empty()) {
				param.GetText(static_cast<CGrTxtParam::TextType>(CGrTxtParam::HistFileTitle9 - idx), ofi.title);
				m_openFileList.push_back(ofi);
			}
		}
		m_toolTips.Create(hWnd);
		//
		m_txtCanvas.Attach(&m_doc);
		m_LupeWnd.Attach(&m_doc);
		// クリップボードチェーンに追加
		m_hNextChainWnd = ::SetClipboardViewer(hWnd);
		//
		updateSetting();
		// Key bind
		_stprintf_s(m_keyBindFileName, _T("%s/%s"), CGrTxtMiru::GetDataPath(), KEYBIND_FILE);
		loadKeyMap();
		//
		m_bVisibleHScrollbar = param.GetBoolean(CGrTxtParam::ShowHScroll);
		m_bVisibleVScrollbar = param.GetBoolean(CGrTxtParam::ShowVScroll);
		Menu_setChecked(IDVISIBLEHSCROLLBAR, m_bVisibleHScrollbar);
		Menu_setChecked(IDVISIBLEVSCROLLBAR, m_bVisibleVScrollbar);
		//
		int auto_save_mode[2] = {};
		param.GetPoints(CGrTxtParam::BookMarkAutoSave, auto_save_mode, sizeof(auto_save_mode) / sizeof(int));
		m_bBookMarkAutoSave = (auto_save_mode[0] == 1);
		m_autoSaveInterval = auto_save_mode[1];
		Menu_setChecked(IDBOOKMARKAUTOSAVE, m_bBookMarkAutoSave);
		Menu_setEnabled(IDBOOKMARKSAVEAS, false);
		m_bKeyRepeat = param.GetBoolean(CGrTxtParam::KeyRepeat);
		//
		m_bSelectionMode = param.GetBoolean(CGrTxtParam::SelectionMode);
		Menu_setChecked(IDSELECTIONMODE, m_bSelectionMode);
		//
		int link_pos[6] = {};
		param.GetPoints(CGrTxtParam::LinkPos, link_pos, sizeof(link_pos) / sizeof(int));
		if (link_pos[0] == 1) {
			ShowLinkPos();
		}
		param.GetPoints(CGrTxtParam::FavoritePos, link_pos, sizeof(link_pos) / sizeof(int));
		if (link_pos[0] == 1) {
			ShowBookPos();
		}
		//
		int window_pos[5] = {};
		// ルーペ       8421
		//   window_pos ||||
		//              |||Visible  :___0=Hide       ,___1=Show
		//              ||CursorMode:__0_=ST_MousePos,__1_=ST_WindowPos
		//              Zoom        :00__=100%       ,01__=150%
		//                           10__=200%       ,11__=400%
		param.GetPoints(CGrTxtParam::LupePos, window_pos, sizeof(window_pos) / sizeof(int));
		if ((window_pos[0] & 0x01) == 1) {
			ShowLupe();
		}
		if (window_pos[0] & 0x02) {
			SetLupeWindowPos(); // ST_WindowPos:ウインドウ位置を拡大する
		}
		else {
			SetLupeMousePos();  // ST_MousePos:カーソル位置を拡大する
		}
		switch (window_pos[0] & 0x0C) {
		case 0x04: m_LupeWnd.Zoom(150); break;
		case 0x08: m_LupeWnd.Zoom(200); break;
		case 0x0C: m_LupeWnd.Zoom(400); break;
		default:   m_LupeWnd.Zoom(100); break;
		}
		//
		setAutoHide(param.GetBoolean(CGrTxtParam::AutoHideMenu));
		//
		setWhiteTrans();
		m_txtCanvas.Initialize();
		m_txtCanvas.UpdateParam();
		m_LupeWnd.UpdateParam();
		UpdateCmdUI();
		//
		SetRecentMenu();
		SetPreParserMenu();
		SetStyleMenu();
		CGrTxtMiruMenu::Load();
		CGrTxtMiruMenu::ConvertMenuString(getMenu(), TxtMiru::l_TxtMiruFuncMenuIDList);
		int i_page_mode;
		param.GetPoints(CGrTxtParam::PageMode, &i_page_mode, 1);
		SetPageMode((PAGE_MODE)i_page_mode);
		int ikeyPressInterval = 0;
		param.GetPoints(CGrTxtParam::KeyInterval, &ikeyPressInterval, 1);
		CGrMouseEvent::SetLongPressTime(ikeyPressInterval);
		//
		TxtMiruTheme_UpdateDarkMode(m_hWnd);
		//
		setPageFlipInterval();

		// 設定が一通り終わってから、ファイル読み込み
		if (m_lpsCmdLine && *m_lpsCmdLine) {
			OpenDocumentDelay(m_lpsCmdLine, 1);
		}
		else {
			std::tstring filename;
			param.GetText(CGrTxtParam::LastFile, filename);
			OpenDocumentDelayNoHistory(filename.c_str());
		}
		return TRUE;
	}
	void OnDestroy()
	{
		RemoveProp(m_hWnd, TXTMIRU_PROP_NAME);
		if (m_hRecentMenu) {
			::DestroyMenu(m_hRecentMenu);
		}
		// クリップボードリストから削除
		if (m_hNextChainWnd != NULL) {
			::ChangeClipboardChain(m_hWnd, m_hNextChainWnd);
			m_hNextChainWnd = NULL;
		}
		//
		std::tstring filename;
		m_doc.GetFileName(filename);
		if (!filename.empty()) {
			auto&& param = CGrTxtMiru::theApp().Param();
			param.SetText(CGrTxtParam::LastFile, filename.c_str());
		}
		//
		CloseDocument();
		{ // アーカイブの一時フォルダを削除
			CGrTxtMiru::MoveDataDir(); // Application Dataにカレントを移す
			auto lpWorkPath = CGrTxtMiru::GetWorkPath();
			if (lpWorkPath) {
				TCHAR curTmpDir[_MAX_PATH + 1] = { 0 };
				auto lpchr = curTmpDir;
				for (; *lpWorkPath; ++lpWorkPath, ++lpchr) {
					if (*lpWorkPath == _T('/')) {
						*lpchr = _T('\\');
					}
					else {
						*lpchr = *lpWorkPath;
					}
				}
				if (CGrShell::IsBackSlash(*(lpchr - 1))) {
					*(lpchr - 1) = '\0';
				}
				SHFILEOPSTRUCT sfo = { 0 };
				sfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
				sfo.wFunc = FO_DELETE;
				sfo.pFrom = curTmpDir;
				SHFileOperation(&sfo);
			}
		}
		::RevokeDragDrop(m_hWnd);
		CloseAozoraList(m_hWnd);
		//
		::PostQuitMessage(0);
	}
private:
	bool callFunc(UINT id)
	{
		if (CGrTxtMiru::theApp().IsWaitting()) {
			return true;
		}
		const auto it = m_id_func_map.find(id);
		if (it != m_id_func_map.end() && it->second) {
			const auto pFunc = (it->second);
			(this->*pFunc)();
			return true;
		}
		return false;
	}
	void SetFuncMap(TxtMiru::FuncNameID fnid, pOnKeyCallbackFunc func)
	{
		m_func_map[TxtMiru::l_TxtMiruFuncNameList[fnid]] = func;
		if (TxtMiru::l_TxtMiruFuncMenuIDList[fnid] > 0) {
			m_id_func_map[TxtMiru::l_TxtMiruFuncMenuIDList[fnid]] = func;
		}
	}
	CGrMainWindow() : m_dlgLink(m_doc), CGrMouseEvent(m_hWnd, this)
	{
		m_clickLPos.x = INT_MIN;
		m_clickLPos.y = INT_MIN;
		m_gestureBeginPos.x = INT_MIN;
		m_gestureBeginPos.y = INT_MIN;
		m_doc.UpdateLayout();
		//
		SetFuncMap(TxtMiru::FnN_ReadFile, &CGrMainWindow::ReadFile); SetFuncMap(TxtMiru::FnN_ReadClipboard, &CGrMainWindow::OpenDocumentClipboard);
		SetFuncMap(TxtMiru::FnN_Reload, &CGrMainWindow::Reload); SetFuncMap(TxtMiru::FnN_URLOpen, &CGrMainWindow::URLOpen);
		SetFuncMap(TxtMiru::FnN_OpenBrowser, &CGrMainWindow::OpenBrowser);
		SetFuncMap(TxtMiru::FnN_NextPage, &CGrMainWindow::NextPage); SetFuncMap(TxtMiru::FnN_PrevPage, &CGrMainWindow::PrevPage);
		SetFuncMap(TxtMiru::FnN_FirstPage, &CGrMainWindow::FirstPage); SetFuncMap(TxtMiru::FnN_EndPage, &CGrMainWindow::EndPage);
		SetFuncMap(TxtMiru::FnN_GotoPage, &CGrMainWindow::GotoPage);
		SetFuncMap(TxtMiru::FnN_Copy, &CGrMainWindow::CopyText); SetFuncMap(TxtMiru::FnN_Save, &CGrMainWindow::Save);
		SetFuncMap(TxtMiru::FnN_SaveAsBookmark, &CGrMainWindow::SaveAsBookmark); SetFuncMap(TxtMiru::FnN_AddBookmark, &CGrMainWindow::AddBookmark);
		SetFuncMap(TxtMiru::FnN_GotoBookmark1, &CGrMainWindow::GotoBookmark1); SetFuncMap(TxtMiru::FnN_GotoBookmark2, &CGrMainWindow::GotoBookmark2);
		SetFuncMap(TxtMiru::FnN_GotoBookmark3, &CGrMainWindow::GotoBookmark3); SetFuncMap(TxtMiru::FnN_GotoBookmark4, &CGrMainWindow::GotoBookmark4);
		SetFuncMap(TxtMiru::FnN_GotoBookmark5, &CGrMainWindow::GotoBookmark5); SetFuncMap(TxtMiru::FnN_GotoBookmark6, &CGrMainWindow::GotoBookmark6);
		SetFuncMap(TxtMiru::FnN_GotoBookmark7, &CGrMainWindow::GotoBookmark7); SetFuncMap(TxtMiru::FnN_GotoBookmark8, &CGrMainWindow::GotoBookmark8);
		SetFuncMap(TxtMiru::FnN_GotoBookmark9, &CGrMainWindow::GotoBookmark9); SetFuncMap(TxtMiru::FnN_GotoBookmark0, &CGrMainWindow::GotoBookmark0);
		SetFuncMap(TxtMiru::FnN_ShowBookmark, &CGrMainWindow::ShowBookmark);
		SetFuncMap(TxtMiru::FnN_Search, &CGrMainWindow::Search); SetFuncMap(TxtMiru::FnN_SearchNext, &CGrMainWindow::SearchNext);
		SetFuncMap(TxtMiru::FnN_SearchPrev, &CGrMainWindow::SearchPrev); SetFuncMap(TxtMiru::FnN_Config, &CGrMainWindow::Config);
		SetFuncMap(TxtMiru::FnN_Exit, &CGrMainWindow::Exit); SetFuncMap(TxtMiru::FnN_Version, &CGrMainWindow::ShowVersion);
		SetFuncMap(TxtMiru::FnN_ShowHScrollBar, &CGrMainWindow::ShowHScrollBar); SetFuncMap(TxtMiru::FnN_ShowVScrollBar, &CGrMainWindow::ShowVScrollBar);
		SetFuncMap(TxtMiru::FnN_HideHScrollBar, &CGrMainWindow::HideHScrollBar); SetFuncMap(TxtMiru::FnN_HideVScrollBar, &CGrMainWindow::HideVScrollBar);
		SetFuncMap(TxtMiru::FnN_ToggleHScrollBar, &CGrMainWindow::ToggleHScrollBar); SetFuncMap(TxtMiru::FnN_ToggleVScrollBar, &CGrMainWindow::ToggleVScrollBar);
		SetFuncMap(TxtMiru::FnN_Help, &CGrMainWindow::ShowHelp); SetFuncMap(TxtMiru::FnN_ShowSubtitle, &CGrMainWindow::ShowSubtitle);
		SetFuncMap(TxtMiru::FnN_ToggleLupe, &CGrMainWindow::ToggleLupe); SetFuncMap(TxtMiru::FnN_ToggleLupePos, &CGrMainWindow::ToggleLupePos);
		SetFuncMap(TxtMiru::FnN_ShowLupe, &CGrMainWindow::ShowLupe); SetFuncMap(TxtMiru::FnN_HideLupe, &CGrMainWindow::HideLupe);
		SetFuncMap(TxtMiru::FnN_SetLupeZoom100, &CGrMainWindow::SetLupeZoom100); SetFuncMap(TxtMiru::FnN_SetLupeZoom150, &CGrMainWindow::SetLupeZoom150);
		SetFuncMap(TxtMiru::FnN_SetLupeZoom200, &CGrMainWindow::SetLupeZoom200); SetFuncMap(TxtMiru::FnN_SetLupeZoom400, &CGrMainWindow::SetLupeZoom400);
		SetFuncMap(TxtMiru::FnN_FullScreen, &CGrMainWindow::ToggleFullScreen);
		SetFuncMap(TxtMiru::FnN_ForwardPage, &CGrMainWindow::ForwardPage); SetFuncMap(TxtMiru::FnN_BackPage, &CGrMainWindow::BackPage);
		SetFuncMap(TxtMiru::FnN_LayoutSet, &CGrMainWindow::LayoutSet); SetFuncMap(TxtMiru::FnN_ShowProperty, &CGrMainWindow::ShowProperty);
		SetFuncMap(TxtMiru::FnN_ShowDocInfo, &CGrMainWindow::ShowDocInfo); SetFuncMap(TxtMiru::FnN_Nop, &CGrMainWindow::Nop);
		SetFuncMap(TxtMiru::FnN_RefreshPreParserList, &CGrMainWindow::SetPreParserMenu);
		SetFuncMap(TxtMiru::FnN_NextFile, &CGrMainWindow::NextFile); SetFuncMap(TxtMiru::FnN_PrevFile, &CGrMainWindow::PrevFile);
		SetFuncMap(TxtMiru::FnN_ShowSubtitleBookmark, &CGrMainWindow::ToggleLinkPos);
		SetFuncMap(TxtMiru::FnN_ShowBookList, &CGrMainWindow::ToggleBookPos);
		SetFuncMap(TxtMiru::FnN_ToggleCopyRuby, &CGrMainWindow::ToggleCopyRuby);
		SetFuncMap(TxtMiru::FnN_ExecOpenFiile, &CGrMainWindow::ExecOpenFiile1);
		SetFuncMap(TxtMiru::FnN_ExecOpenFiile1, &CGrMainWindow::ExecOpenFiile2);
		SetFuncMap(TxtMiru::FnN_ExecOpenFiile2, &CGrMainWindow::ExecOpenFiile3);
		SetFuncMap(TxtMiru::FnN_ShowAozoraList, &CGrMainWindow::ShowAozoraList);
		SetFuncMap(TxtMiru::FnN_TM_FreeLibrary, &CGrMainWindow::TM_FreeLibrary);
		SetFuncMap(TxtMiru::FnN_LinkGoto, &CGrMainWindow::LinkGoto); SetFuncMap(TxtMiru::FnN_LinkOpen, &CGrMainWindow::LinkOpen);
		SetFuncMap(TxtMiru::FnN_ToggleSelectionMode, &CGrMainWindow::ToggleSelectionMode);
		SetFuncMap(TxtMiru::FnN_ShowContextMenu, &CGrMainWindow::ShowContextMenu);
		SetFuncMap(TxtMiru::FnN_SearchFiles, &CGrMainWindow::SearchFiles);
		SetFuncMap(TxtMiru::FnN_ShowRubyList, &CGrMainWindow::ToggleRubyPos);
		SetFuncMap(TxtMiru::FnN_OpenFolder, &CGrMainWindow::OpenFolder);
		SetFuncMap(TxtMiru::FnN_AddFavorite, &CGrMainWindow::AddFavorite);
	}
	void GotoPage(int page)
	{
		SetSpSelectionMode(false);
		m_currentPage = max(min(page, m_doc.GetTotalPage()), 0);
		m_displayPage = m_currentPage;
		if (m_bSinglePage) {
			if (m_displayPage % 2 == 0) {
				m_windowOffsetX = m_windowWidth;
			}
			else {
				m_windowOffsetX = 0;
			}
			m_LupeWnd.SetCanvasOffset(m_windowOffsetX, m_windowOffsetY);
		}
		showScrollBar();                              // showScrollBarによりリサイズ処理が実行され m_txtCanvas.Update が複数回処理されるので事前に設定する
		if (m_pageFlipInterval > 0) {
			page = m_txtCanvas.SetPageFlip(m_currentPage, m_bSinglePage);
		}
		else {
			m_txtCanvas.SetCurrentHoverLink(-1);
			page = m_txtCanvas.Update(m_currentPage);
		}
		m_currentPage = page; // 偶数ページになるように
		setToolTips();
		m_txtCanvas.ValidateSelect(false);

		if (page == m_currentPage) { showScrollBar(); } // 念のため
		m_MessWnd.Hide();
		::InvalidateRect(m_hWnd, NULL, FALSE);
		::UpdateWindow(m_hWnd);
		if (m_pageFlipInterval > 0) {
			::SetTimer(m_hWnd, ID_TIMER_PAGEFLIP, m_pageFlipInterval, nullptr);
		}
		else if (m_LupeWnd) {
			::SetTimer(m_hWnd, ID_TIMER_LUPE, 500, nullptr);
		}
		SetWindowTitle();
		m_dlgLink.UpdatePage();
		if (m_autoSaveInterval > 0) {
			::SetTimer(m_hWnd, ID_TIMER_AUTOSAVE, m_autoSaveInterval * 1000, nullptr);
		}
	}
	bool isCopyRuby() { return CGrTxtMiru::theApp().Param().GetBoolean(CGrTxtParam::CopyRuby); }
	bool isBookPosVisible() { return m_dlgLink.IsShowBook(); }
	bool isLinkPosVisible() { return m_dlgLink.IsShowLink(); }
	bool isLupePosVisible() { return (m_LupeWnd && (IsWindowVisible(m_LupeWnd) == TRUE)); }
	bool isLupeMousePos() { return m_LupeWnd.GetSelectType() == CGrLupeWnd::ST_MousePos; }
	bool isRubyListVisible() { return m_dlgLink.IsShowRubyList(); }
private:
	// Function
	//
	void Save() {
		CGrLoadDllFunc func(_T("TxtFuncPict.dll"));
		bool (cdecl * SavePicture)(HWND hWnd, LPCTSTR ini_filename, LPCTSTR filename, int total_page, SIZE paper_size, CGrTxtFuncICanvas * pCanvas);
		SetProcPtr(SavePicture, func.GetProcAddress("TxtFuncSavePicture", m_hWnd));
		if (SavePicture) {
			CGrTxtFuncCanvas canvas(&m_doc);
			const auto& layout = m_doc.GetConstLayout();
			std::tstring buffilename;
			m_doc.GetFileName(buffilename);
			TCHAR iniFileName[MAX_PATH];
			_stprintf_s(iniFileName, _T("%s/%s.ini"), CGrTxtMiru::GetDataPath(), CGrTxtMiru::AppName());
			SavePicture(m_hWnd, iniFileName, buffilename.c_str(), m_doc.GetTotalPage(), layout.GetPaperSize(), &canvas);
		}
	}
	void URLOpen() {
		std::tstring filename;
		m_doc.GetFileName(filename);
		CGrTxtMiru::theApp().SetWatting(true);
		CGrOpenURLDlg dlg;
		if (dlg.DoModal(m_hWnd, filename.c_str()) == IDOK) {
			CGrTxtMiru::theApp().SetWatting(false);
			std::tstring url;
			dlg.GetURL(url);
			if (dlg.IsRefresh()) {
				DeleteUrlCacheEntry(url.c_str());
			}
			OpenDocumentCommandWithAddFileList(url.c_str());
		}
		else {
			CGrTxtMiru::theApp().SetWatting(false);
		}
	}
	void OpenBrowser() {
		CGrLoadDllFunc func(_T("TxtFuncConfig.dll"));
		bool (cdecl * fGetBrowserURL)(LPTSTR * ppURL, CGrTxtFuncIParam * pParam);
		SetProcPtr(fGetBrowserURL, func.GetProcAddress("TxtFuncGetBrowserURL", m_hWnd));
		if (fGetBrowserURL) {
			m_FileUpdateWatcher.stop();
			auto&& param = CGrTxtMiru::theApp().Param();
			LPTSTR pURL = nullptr;
			if (fGetBrowserURL(&pURL, &param)) {
				if (pURL) {
					OpenDocumentCommandWithAddFileList(pURL);
				}
			}
			if (pURL) {
				::GlobalFree(pURL);
			}
		}
	}
	void ShowBookList() {
		KillTimer(m_hWnd, ID_TIMER_AUTOSAVE);
		m_FileUpdateWatcher.stop();
		m_doc.SetLastPage(m_displayPage);
		m_doc.Save();
		m_doc.ShowBookmarkList(m_hWnd);
		const auto& param = CGrTxtMiru::theApp().Param();
		if (param.GetBoolean(CGrTxtParam::FileAutoReload)) {
			std::tstring filename;
			m_doc.GetFileName(filename);
			m_FileUpdateWatcher.start(filename.c_str(), m_hWnd);
		}
	}
	void ShowDocInfo() {
		const auto& buffer = m_doc.GetTxtBuffer();
		std::tstring filename;
		m_doc.GetFileName(filename);
		CGrTxtDocInfoDlg::DoModal(m_hWnd,
			m_doc.GetTitle().c_str(),
			m_doc.GetAuthor().c_str(),
			filename.c_str(),
			m_doc.GetDocInfo().c_str(),
			buffer.GetLastWriteTime().c_str(),
			m_displayPage,
			m_totalPage);
	}
	void LayoutSet() {
		m_doc.ConfigurationDlg(m_hWnd);
	}
	//
	void nextfile(int order)
	{
		auto lpLinkFileName = m_doc.GetLinkFileNameByOrder(order);
		if (lpLinkFileName) {
			auto&& param = CGrTxtMiru::theApp().Param();
			param.SetText(CGrTxtParam::LastFile, lpLinkFileName);
			OpenDocumentCommand(lpLinkFileName);
			return;
		}
		std::tstring filename;
		m_doc.GetRealFileName(filename);
		std::tstring outfilename;
		const auto& param = CGrTxtMiru::theApp().Param();
		const auto& ftMap = param.GetFileTypeMap();
		int len = ftMap.size();
		if (len > 0) {
			std::unique_ptr<LPCTSTR[]> extList(new LPCTSTR[len]);
			const auto* p = ftMap.data();
			for (int i = 0; i < len; ++i, ++p) {
				extList[i] = p->ext.c_str();
			}
			if (order > 0) {
				CGrShell::GetNextFile(outfilename, filename.c_str(), extList.get(), len);
			}
			else {
				CGrShell::GetPrevFile(outfilename, filename.c_str(), extList.get(), len);
			}
		}
		if (!outfilename.empty()) {
			OpenDocumentCommandWithAddFileList(outfilename.c_str());
		}
	}

	void NextFile() {
		nextfile(1);
	}
	void PrevFile() {
		nextfile(-1);
	}
	//
	void NextPage() {
		if (m_bSinglePage) {
			if ((m_displayPage + 1) <= m_totalPage) {
				GotoPage(m_displayPage + 1);
			}
		}
		else {
			if ((m_currentPage + 2) <= m_totalPage) {
				GotoPage(m_currentPage + 2);
			}
		}
	}
	void PrevPage() {
		if (m_bSinglePage) {
			if (m_displayPage >= 1) {
				GotoPage(m_displayPage - 1);
			}
		}
		else {
			if (m_currentPage >= 2) {
				GotoPage(m_currentPage - 2);
			}
		}
	}
	void FirstPage() { GotoPage(0); }
	void EndPage() { GotoPage(m_totalPage - 1); }
	void ReadFile()
	{
		TCHAR filter[2048] = {};
		CGrText::LoadString(IDS_TEXTFILE, filter, sizeof(filter) / sizeof(TCHAR));
		std::tstring str;
		CGrText::LoadString(IDS_OPENFILE, str);

		OPENFILENAME of = { sizeof(OPENFILENAME) };
		TCHAR fileName[MAX_PATH] = {};

		of.hwndOwner = m_hWnd;
		of.lpstrFilter = filter;
		of.lpstrTitle = str.c_str();
		of.nMaxCustFilter = 40;
		of.lpstrFile = fileName;
		of.nMaxFile = MAX_PATH - 1;
		of.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
		// カレントディレクトリをデフォルトパスに
		TCHAR cur_dir[MAX_PATH];
		std::tstring tmp_cur_dir;
		m_doc.GetFileName(tmp_cur_dir);
		if (tmp_cur_dir.empty()) {
			::GetCurrentDirectory(sizeof(cur_dir) / sizeof(TCHAR), cur_dir);
		}
		else {
			CGrShell::GetParentDir(const_cast<TCHAR*>(tmp_cur_dir.c_str()), cur_dir);
		}
		auto lpchr = cur_dir;
		for (; *lpchr; ++lpchr) {
			if (*lpchr == _T('/')) {
				*lpchr = _T('\\');
			}
		}
		if (CGrShell::IsBackSlash(*(lpchr - 1))) {
			*(lpchr - 1) = '\0';
		}
		*(lpchr + 1) = '\0'; // 末尾を \0\0にする
		of.lpstrInitialDir = cur_dir;
		if (::GetOpenFileName(&of)) {
			OpenDocumentCommandWithAddFileList(fileName);
		}
	}
	void TM_FreeLibrary()
	{
		m_dlgLink.Unload();
	}
	void ShowAozoraList()
	{
		showAozoraList(m_hWnd);
	}
	void ExecOpenFiile1()
	{
		ExecOpenFiile(CGrTxtParam::OpenFileExe, CGrTxtParam::OpenFilePrm);
	}
	void ExecOpenFiile2()
	{
		ExecOpenFiile(CGrTxtParam::OpenFileExe1, CGrTxtParam::OpenFilePrm1);
	}
	void ExecOpenFiile3()
	{
		ExecOpenFiile(CGrTxtParam::OpenFileExe2, CGrTxtParam::OpenFilePrm2);
	}
	void ExecOpenFiile(CGrTxtFuncIParam::TextType file, CGrTxtFuncIParam::TextType prm)
	{
		std::tstring filename;
		m_doc.GetFileName(filename);
		if (filename.empty()) {
			return;
		}
		openFiile(filename.c_str(), file, prm);
	}
	void openFiile(LPCTSTR lpFileName, CGrTxtFuncIParam::TextType file, CGrTxtFuncIParam::TextType prm)
	{
		std::tstring exec_filen;
		std::tstring exec_param;
		const auto& param = CGrTxtMiru::theApp().Param();
		param.GetText(file, exec_filen);
		param.GetText(prm, exec_param);
		if (exec_filen.empty()) {
			return;
		}
		std::wregex re(L"%(.)");
		auto f = [&](const std::wsmatch& m) {
			switch (m.str(1)[0]) {
			case L'%':
				return std::tstring(L"%");
			case L'1':
			case L'f':
			case L'F':
				return std::tstring(lpFileName);
			case L'n':
			case L'N':
				return std::tstring(CGrShell::GetFileName(lpFileName));
			case L'd':
			case L'D':
			{
				TCHAR cur_dir[MAX_PATH];
				CGrShell::GetParentDir((TCHAR*)lpFileName, cur_dir);
				return std::tstring(cur_dir);
			}
			case 'l':
			case 'L':
			{
				TxtMiru::TextPoint tp;
				if (m_doc.StartPageToTextPoint(tp, m_displayPage)) {
					try {
						const auto& ll = m_doc.GetLineInfo(tp.iLine);
						TCHAR buf[100];
						_stprintf_s(buf, _T("%d"), ll.iFileLineNo);
						return std::tstring(buf);
					}
					catch (...) {
						// NOP
					}
				}
				return std::tstring(L"1");
			}
			case L's':
			case L'S':
			{
				TxtMiru::TextPoint tpb, tpe;
				if (!m_txtCanvas.GetSelectTextPoint(tpb, tpe)) {
					return std::tstring();
				}
				std::tstring str;
				if (isCopyRuby()) {
					m_doc.ToStringRuby(str, tpb, tpe);
				}
				else {
					m_doc.ToString(str, tpb, tpe);
				}
				return str;
			}
			default:
				return m.str(0);
			}
		};
		exec_filen = std::regex_replace(exec_filen, re, f);
		exec_param = std::regex_replace(exec_param, re, f);
		ShellExecute(m_hWnd, NULL, exec_filen.c_str(), exec_param.c_str(), NULL, SW_SHOWDEFAULT);
	}
	void ExecCopyText()
	{
		std::tstring exec_filen;
		std::tstring exec_param;
		const auto& param = CGrTxtMiru::theApp().Param();
		param.GetText(CGrTxtParam::CopyTextExe, exec_filen);
		param.GetText(CGrTxtParam::CopyTextPrm, exec_param);
		if (exec_filen.empty()) {
			CopyText();
			return;
		}
		TxtMiru::TextPoint tpb, tpe;
		if (!m_txtCanvas.GetSelectTextPoint(tpb, tpe)) {
			return;
		}
		std::tstring str;
		if (isCopyRuby()) {
			m_doc.ToStringRuby(str, tpb, tpe);
		}
		else {
			m_doc.ToString(str, tpb, tpe);
		}
		std::replace(str, _T("\\"), _T("\\\\"));
		std::replace(str, _T("\""), _T("\\\""));
		std::replace(exec_filen, _T("%1"), str.c_str());
		std::replace(exec_param, _T("%1"), str.c_str());
		ShellExecute(m_hWnd, NULL, exec_filen.c_str(), exec_param.c_str(), NULL, SW_SHOWDEFAULT);
	}
	void CopyText() {
		TxtMiru::TextPoint tpb, tpe;
		if (!m_txtCanvas.GetSelectTextPoint(tpb, tpe)) {
			return;
		}
		std::tstring str;
		if (isCopyRuby()) {
			m_doc.ToStringRuby(str, tpb, tpe);
		}
		else {
			m_doc.ToString(str, tpb, tpe);
		}

		// 移動可能な共有メモリを確保する。
		auto hMem = ::GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(TCHAR) * (str.size() + 1));
		if (!hMem) { return; } // メモリ確保に失敗
		// 確保したメモリをロックをして、アクセス可能にする。
		auto* buf = static_cast<TCHAR*>(GlobalLock(hMem));
		if (!buf) { GlobalFree(hMem); return; } // メモリロックに失敗 : メモリの解放
		// メモリに文字列を複写する。
		::CopyMemory(buf, str.c_str(), sizeof(TCHAR) * str.size());
		// メモリのロックを解除する。
		::GlobalUnlock(hMem);
		// クリップボードを開きます。
		if (!::OpenClipboard(NULL)) {
			::GlobalFree(hMem);
			return;
		}
		// クリップボードを空にします。
		if (!::EmptyClipboard()) {
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
	}
	void Config() {
		CGrLoadDllFunc func(_T("TxtFuncConfig.dll"));
		bool (cdecl * fConfig)(HWND hWnd, LPCTSTR lpkeyBindFileName, LPCTSTR lpDataDir, CGrTxtFuncIParam * pParam);
		SetProcPtr(fConfig, func.GetProcAddress("TxtFuncConfig", m_hWnd));
		if (fConfig) {
			m_FileUpdateWatcher.stop();
			loadKeyMap();
			//
			auto&& param = CGrTxtMiru::theApp().Param();
			fConfig(m_hWnd, m_keyBindFileName, CGrTxtMiru::GetDataPath(), &param);
			//
			{
				TCHAR iniFileName[MAX_PATH];
				_stprintf_s(iniFileName, _T("%s/%s.ini"), CGrTxtMiru::GetDataPath(), CGrTxtMiru::AppName());
				param.Save(iniFileName);
			}
			int auto_save_mode[2] = {};
			param.GetPoints(CGrTxtParam::BookMarkAutoSave, auto_save_mode, sizeof(auto_save_mode) / sizeof(int));
			m_bBookMarkAutoSave = (auto_save_mode[0] == 1);
			m_autoSaveInterval = auto_save_mode[1];
			Menu_setChecked(IDBOOKMARKAUTOSAVE, m_bBookMarkAutoSave);
			m_bKeyRepeat = param.GetBoolean(CGrTxtParam::KeyRepeat);
			CGrTxtMiruMenu::Load();
			CGrTxtMiruMenu::ConvertMenuString(getMenu(), TxtMiru::l_TxtMiruFuncMenuIDList);
			int ikeyPressInterval = 0;
			param.GetPoints(CGrTxtParam::KeyInterval, &ikeyPressInterval, 1);
			CGrMouseEvent::SetLongPressTime(ikeyPressInterval);
			setPageFlipInterval();
			if (param.GetBoolean(CGrTxtParam::FileAutoReload)) {
				std::tstring filename;
				m_doc.GetFileName(filename);
				m_FileUpdateWatcher.start(filename.c_str(), m_hWnd);
			}
		}
	}
	void GotoPage() {
		int page = 0;
		if (CGrTxtGotoPageDlg::DoModal(m_hWnd, m_totalPage, m_displayPage, &page) == IDOK) {
			if (page < 0) {
				page = 1;
			}
			else if (page > m_totalPage) {
				page = m_totalPage - 1;
			}
			GotoPageCommand(page);
		}
	}
	void Reload() {
		m_bInvalid = true;
		CGrMouseEvent::Clear();
		CGrWaitCursor wait;
		m_FileUpdateWatcher.stop();
		{
			std::tstring filename;
			m_doc.GetFileName(filename);
			if (!filename.empty()) {
				DeleteUrlCacheEntry(filename.c_str());
			}
		}
		int page = m_displayPage;
		if (!m_doc.Open()) {
			m_doc.OpenError();
		}
		m_txtCanvas.Initialize();
		m_txtCanvas.UpdateParam();
		m_LupeWnd.UpdateParam();
		m_totalPage = m_doc.GetTotalPage();
		if (page > m_totalPage) {
			page = m_totalPage;
		}
		GotoPage(page);
		SetWindowTitle();
		refreshToolWindow();
		const auto& param = CGrTxtMiru::theApp().Param();
		if (param.GetBoolean(CGrTxtParam::FileAutoReload)) {
			std::tstring filename;
			m_doc.GetFileName(filename);
			m_FileUpdateWatcher.start(filename.c_str(), m_hWnd);
		}
		m_bInvalid = false;
	}
	void AddBookmark() {
		m_doc.AddBookmark(m_displayPage);
		refreshToolWindow();
	}
	void GotoBookmark(int idx) {
		int page = m_doc.GetBookmarkPage(idx);
		if (page >= 0) {
			GotoPage(page);
		}
	}
	void SaveAsBookmark() {
		OPENFILENAME of = { sizeof(OPENFILENAME) };
		std::tstring str;
		TCHAR filter[2048] = { 0 };
		CGrText::LoadString(IDS_BOOKMARKFILE, filter, sizeof(filter) / sizeof(TCHAR));
		CGrText::LoadString(IDS_SAVEASBOOKMARKFILE, str);

		TCHAR fileName[MAX_PATH] = { 0 };

		of.hwndOwner = m_hWnd;
		of.lpstrFilter = filter;
		of.lpstrTitle = str.c_str();
		of.nMaxCustFilter = 40;
		of.lpstrFile = fileName;
		of.nMaxFile = MAX_PATH - 1;
		of.lpstrInitialDir = _T(".\\");
		of.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
		if (::GetSaveFileName(&of)) {
			m_doc.SaveBookmark(fileName);
		}
	}
	void ToggleCopyRuby()
	{
		bool b_cr = !isCopyRuby();
		auto&& param = CGrTxtMiru::theApp().Param();
		param.SetBoolean(CGrTxtParam::CopyRuby, b_cr);
		Menu_setChecked(IDCOPYRUBY, b_cr);
	}
	void GotoBookmark1() { GotoBookmark(0); }
	void GotoBookmark2() { GotoBookmark(1); }
	void GotoBookmark3() { GotoBookmark(2); }
	void GotoBookmark4() { GotoBookmark(3); }
	void GotoBookmark5() { GotoBookmark(4); }
	void GotoBookmark6() { GotoBookmark(5); }
	void GotoBookmark7() { GotoBookmark(6); }
	void GotoBookmark8() { GotoBookmark(7); }
	void GotoBookmark9() { GotoBookmark(8); }
	void GotoBookmark0() { GotoBookmark(9); }
	void ShowBookmark() {
		m_dlgLink.ShowBookmark(m_hWnd);
		Menu_setChecked(IDSUBTITLEBOOKMARK, true);
	}
	void SearchFiles() {
		m_dlgLink.SearchFiles(m_hWnd);
	}
	//
	void ShowLinkPos() {
		m_dlgLink.ShowLink(m_hWnd);
		Menu_setChecked(IDSUBTITLEBOOKMARK, true);
	}
	void HideLinkPos() {
		m_dlgLink.HideLink();
		Menu_setChecked(IDSUBTITLEBOOKMARK, false);
	}
	void ShowBookPos() {
		m_doc.ShowBookmarkList(m_hWnd);
		Menu_setChecked(IDFAVORITE, true);
	}
	void HideBookPos() {
		m_doc.ShowBookmarkList(m_hWnd, false);
		Menu_setChecked(IDFAVORITE, false);
	}
	void ToggleBookPos() {
		if (isBookPosVisible()) {
			HideBookPos();
		}
		else {
			ShowBookPos();
		}
	}
	void ToggleLinkPos() {
		if (isLinkPosVisible()) {
			HideLinkPos();
		}
		else {
			ShowLinkPos();
		}
	}
	void ShowRubyListPos() {
		m_dlgLink.ShowRubyList(m_hWnd, &m_doc);
		Menu_setChecked(IDRUBYLIST, true);
	}
	void HideRubyListPos() {
		m_dlgLink.HideRubyList();
		Menu_setChecked(IDRUBYLIST, false);
	}
	void ToggleRubyPos() {
		if (isRubyListVisible()) {
			HideRubyListPos();
		}
		else {
			ShowRubyListPos();
		}
	}
	void OpenFolder() {
		std::tstring str;
		CGrText::LoadString(IDS_SELECTFOLDER, str);
		TCHAR dst_file[MAX_PATH] = {};
		::GetCurrentDirectory(sizeof(dst_file) / sizeof(TCHAR), dst_file);
		BROWSEINFO bi = {};
		bi.hwndOwner = m_hWnd;
		bi.lpszTitle = str.c_str();
		bi.pszDisplayName = dst_file;
		bi.lpfn = [](HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) -> int {
			if (uMsg == BFFM_INITIALIZED) {
				SendMessage(hwnd, BFFM_SETSELECTION, static_cast<WPARAM>(TRUE), lpData);
			}
			return 0;
		};
		bi.lParam = reinterpret_cast<LPARAM>(dst_file);
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON;
		auto idlist = SHBrowseForFolder(&bi);
		if (idlist) {
			TCHAR dir[MAX_PATH] = {};
			if (SHGetPathFromIDList(idlist, dir)) {
				OpenDocumentDelay(dir);
			}
			CoTaskMemFree(idlist);
		}
	}
	// ルーペ
	void ShowLupe() {
		m_LupeWnd.Show(m_hWnd, m_currentPage);
		Menu_setChecked(IDLUPE, true);
	}
	void HideLupe() {
		if (m_LupeWnd) {
			m_LupeWnd.Hide();
		}
		Menu_setChecked(IDLUPE, false);
	}
	void ToggleLupe() {
		if (isLupePosVisible()) {
			HideLupe();
		}
		else {
			ShowLupe();
		}
	}
	void SetLupeMousePos() {
		m_LupeWnd.SetSelectType(CGrLupeWnd::ST_MousePos);
		Menu_setChecked(IDLUPEWINDOWPOS, true);
	}
	void SetLupeWindowPos() {
		m_LupeWnd.SetSelectType(CGrLupeWnd::ST_WindowPos);
		Menu_setChecked(IDLUPEWINDOWPOS, false);
	}
	void ToggleLupePos() {
		if (isLupeMousePos()) {
			SetLupeWindowPos();
		}
		else {
			SetLupeMousePos();
		}
	}
	void SetLupeZoom100() { m_LupeWnd.Zoom(100); UpdateCmdUI(); }
	void SetLupeZoom150() { m_LupeWnd.Zoom(150); UpdateCmdUI(); }
	void SetLupeZoom200() { m_LupeWnd.Zoom(200); UpdateCmdUI(); }
	void SetLupeZoom400() { m_LupeWnd.Zoom(400); UpdateCmdUI(); }
	//
	void Exit() { ::PostMessage(m_hWnd, WM_CLOSE, 0, 0L); }
	void search_goto(const TxtMiru::TextPoint& pos)
	{
		GotoPage(m_doc.TextPointToPage(pos));
		m_txtCanvas.SetBeginSelect(pos);
		TxtMiru::TextPoint end_pos;
		m_doc.TextPointNextN(end_pos, pos, m_doc.SearchStringLength() - 1);
		m_txtCanvas.SetEndSelect(end_pos);
		::InvalidateRect(m_hWnd, NULL, TRUE);
	}
	void Search()
	{
		TxtMiru::TextPoint pos;
		m_doc.StartPageToTextPoint(pos, m_currentPage);
		// 上へ検索、下へ検索の時、続きから検索できるように
		TxtMiru::TextPoint tpb, tpe;
		bool bselect = m_txtCanvas.GetSelectTextPoint(tpb, tpe);
		if (bselect) {
			if (m_displayPage == m_doc.TextPointToPage(tpb)) {
				pos = tpb;
			}
		}
		if (m_dlgLink.Search(m_hWnd, pos, &m_doc)) {
			search_goto(pos);
		}
	}
	void SearchNext()
	{
		TxtMiru::TextPoint pos;
		if (m_doc.SearchNext(pos)) {
			search_goto(pos);
		}
		else {
			CGrMessageBox::Show(m_hWnd, IDS_SEARCH_NOT_FOUND, CGrTxtMiru::AppName());
		}
	}
	void SearchPrev()
	{
		TxtMiru::TextPoint pos;
		if (m_doc.SearchPrev(pos)) {
			search_goto(pos);
		}
		else {
			CGrMessageBox::Show(m_hWnd, IDS_SEARCH_NOT_FOUND, CGrTxtMiru::AppName());
		}
	}
	void ShowVersion()
	{
		CGrVersionDlg::DoModal(m_hWnd);
	}
	void switchShowScrollBar()
	{
		showScrollBar();
		Menu_setChecked(IDVISIBLEHSCROLLBAR, m_bVisibleHScrollbar);
		Menu_setChecked(IDVISIBLEVSCROLLBAR, m_bVisibleVScrollbar);
	}
	void ShowHScrollBar() { m_bVisibleHScrollbar = true; switchShowScrollBar(); }
	void ShowVScrollBar() { m_bVisibleVScrollbar = true; switchShowScrollBar(); }
	void HideHScrollBar() { m_bVisibleHScrollbar = false; switchShowScrollBar(); }
	void HideVScrollBar() { m_bVisibleVScrollbar = false; switchShowScrollBar(); }
	void ToggleHScrollBar() { m_bVisibleHScrollbar = !m_bVisibleHScrollbar; switchShowScrollBar(); }
	void ToggleVScrollBar() { m_bVisibleVScrollbar = !m_bVisibleVScrollbar; switchShowScrollBar(); }
	void ExecuteTool(LPCTSTR lpFileName)
	{
		TCHAR curPath[MAX_PATH];
		CGrShell::GetExePath(curPath);
		TCHAR exeFile[MAX_PATH];
		_stprintf_s(exeFile, _T("%s\\%s.exe"), curPath, lpFileName);
		CGrShell::Execute(m_hWnd, exeFile);
	}
	void SaveText()
	{
		m_dlgLink.SaveText(m_hWnd, &m_doc);
	}
	void ShowHelp()
	{
		TCHAR curPath[MAX_PATH];
		CGrShell::GetExePath(curPath);
		TCHAR helpFile[MAX_PATH];
		_stprintf_s(helpFile, _T("%s\\help\\index.html"), curPath);
		CGrShell::Execute(m_hWnd, helpFile);
	}
	void ShowSubtitle()
	{
		m_dlgLink.ShowSubtitle(m_hWnd);
		Menu_setChecked(IDSUBTITLEBOOKMARK, true);
	}
	void ForwardPage()
	{
		Redo();
	}
	void BackPage()
	{
		Undo();
	}
	void ShowProperty()
	{
		m_doc.ShowPropertyDialog(m_hWnd);
	}
	void Nop() {}
	void setPageFlipInterval()
	{
		const auto& param = CGrTxtMiru::theApp().Param();
		if (param.GetBoolean(CGrTxtParam::PageFlip)) {
			int iInterval = 0;
			param.GetPoints(CGrTxtParam::PageFlipInterval, &iInterval, 1);
			m_pageFlipInterval = iInterval;
		}
		else {
			m_pageFlipInterval = 0;
		}
	}
	void setAntialias()
	{
		const auto& param = CGrTxtMiru::theApp().Param();
		int iAntiAlias = 0;
		param.GetPoints(CGrTxtParam::AntiAlias, &iAntiAlias, 1);
		m_txtCanvas.SetAntialias(iAntiAlias);
	}
	bool linkOpenGoto(TxtMiru::FuncNameID fnid = TxtMiru::FnN_MaxNum)
	{
		int id = m_txtCanvas.GetCurrentHoverLink();
		if (id < 0) {
			return false;
		}
		// Anchorからのページ内ジャンプ
		int page = -1;
		std::tstring url(m_txtCanvas.GetLinkURL(id));
		std::tstring filename;
		m_doc.GetFileName(filename);
		if (url.size() >= 2 && url[0] == _T('#')) {
			// Link先が、IDでsubtitle一覧であればそのページに移動
			page = m_doc.GetSubtitle().GetPageById(&(url.c_str()[1]));
		}
		if (page == -1 && (CGrShell::IsURI(url.c_str()) || !CGrShell::GetSearchDir(url.c_str()))) {
			// 開いているファイルと同じURLで途中に '#'があれば、以降をIDとみなし そのIDでsubtitle一覧であればそのページに移動
			if (CGrText::isMatchChar(url.c_str(), filename.c_str()) && url[filename.size()] == _T('#')) {
				page = m_doc.GetSubtitle().GetPageById(&(url.c_str()[filename.size() + 1]));
			}
		}
		if (fnid == TxtMiru::FnN_MaxNum) {
			fnid = TxtMiru::FnN_LinkGoto;
			const auto pFunc = getKeyFunc(VK_LBUTTON);
			if (pFunc) {
				if (pFunc == &CGrMainWindow::LinkGoto) {
					fnid = TxtMiru::FnN_LinkGoto;
				}
				else if (pFunc == &CGrMainWindow::LinkOpen) {
					fnid = TxtMiru::FnN_LinkOpen;
				}
				else if (pFunc != &CGrMainWindow::Nop) {
					(this->*pFunc)();
					return false;
				}
			}
		}
		if (fnid == TxtMiru::FnN_LinkGoto) {
			if (page >= 0) {
				GotoPage(page);
			}
			else {
				auto&& param = CGrTxtMiru::theApp().Param();
				param.SetText(CGrTxtParam::LastFile, url.c_str());
				OpenDocumentCommand(url.c_str());
			}
		}
		else if (fnid == TxtMiru::FnN_LinkOpen) {
			openFiile(url.c_str(), CGrTxtParam::OpenLinkExe, CGrTxtParam::OpenLinkPrm);
		}
		else {
			return false;
		}
		return true;
	}
	void LinkGoto()
	{
		linkOpenGoto(TxtMiru::FnN_LinkGoto);
	}
	void LinkOpen()
	{
		linkOpenGoto(TxtMiru::FnN_LinkOpen);
	}
	void ShowContextMenu()
	{
		OnContextMenu(m_hWnd, m_hWnd, 65535, 65535);
	}
	void AddFavorite()
	{
		std::tstring filename;
		m_doc.GetFileName(filename);
		m_dlgLink.AddFavorite(m_hWnd, filename.c_str());
	}
private:
	CGrToolTips         m_toolTips;
	CGrTxtFuncBookmark  m_dlgLink;
	TCHAR m_keyBindFileName[MAX_PATH];
	HWND m_hWnd = NULL;
	HWND m_hNextChainWnd = NULL;
	CGrTxtDocument m_doc;

	bool m_bInvalid = false;
	int m_totalPage = 0;
	int m_currentPage = 0;
	int m_displayPage = 0;
	int m_pageFlipInterval = 0;
	int m_autoSaveInterval = 0;
	int m_bKeyRepeat = true;
	//
	std::vector<OpenFileInfo> m_openFileList;
	std::vector<OpenFileInfo> m_preParserFolderList;
	std::vector<OpenFileInfo> m_styleList;

	bool m_bVisibleHScrollbar = true;
	bool m_bVisibleVScrollbar = false;
	int m_iHScrollHeight = 0;
	int m_iVScrollWidth = 0;
	//
	bool m_bBookMarkAutoSave = true;
	bool m_bFullScreen = false;
	bool m_bAutoHideMenu = false;
	WINDOWPLACEMENT m_wndp;
	HMENU m_hMenu = NULL;
	DWORD m_style = 0;
	bool m_bNoPaint = false;
	CGrFileUpdateWatcher m_FileUpdateWatcher;
	//
	CGrMessageWnd m_MessWnd;
	CGrLupeWnd m_LupeWnd;
	//
	enum {
		CLIPFORMAT_UniformResourceLocatorFormat,
		CLIPFORMAT_UniformResourceLocatorFormatW,
		CLIPFORMAT_NetscapeBookmarkFormat,
		CLIPFORMAT_FiledescriptorFormat,
		CLIPFORMAT_FilecontentsFormat,
		CLIPFORMAT_Num
	};
	CLIPFORMAT m_ClipFormat[CLIPFORMAT_Num];
	UINT m_wm_funcCall = 0;
public:
	LPCTSTR m_lpsCmdLine = nullptr;
	//
	struct CGrGotoPageCommand : public CGrAbCommand
	{
	private:
		TxtMiru::TextPoint from_tp;
		TxtMiru::TextPoint to_tp;
	public:
		CGrGotoPageCommand(const TxtMiru::TextPoint& f, const TxtMiru::TextPoint& t) : from_tp(f), to_tp(t)
		{
		}
		virtual bool doCommand(CGrMainWindow& data)
		{
			data.GotoPage(data.m_doc.TextPointToPage(to_tp));
			return true;
		}
		virtual bool undoCommand(CGrMainWindow& data)
		{
			data.GotoPage(data.m_doc.TextPointToPage(from_tp));
			return true;
		}
	};
	struct CGrOpenFileCommand : public CGrAbCommand
	{
	private:
		std::tstring from_filename;
		std::tstring to_filename;
		int old_page;
		void command(CGrMainWindow& data, std::tstring& filename)
		{
			int page = old_page;
			old_page = data.m_currentPage;
			if (!filename.empty()) {
				data.OpenDocument(filename.c_str());
				if (page >= 0) {
					data.GotoPage(page);
				}
			}
		}
	public:
		CGrOpenFileCommand(LPCTSTR lpFromFilename, LPCTSTR lpToFilename) : from_filename(lpFromFilename), to_filename(lpToFilename), old_page(-1)
		{
		}
		virtual bool doCommand(CGrMainWindow& data)
		{
			command(data, to_filename);
			return true;
		}
		virtual bool undoCommand(CGrMainWindow& data)
		{
			command(data, from_filename);
			return true;
		}
		virtual bool isSkipRedoCommand()
		{
			return to_filename.empty();
		}
		virtual bool isSkipUndoCommand()
		{
			return from_filename.empty();
		}
	};
};

//
// メインウィンドウ
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CGrMainWindow::theApp().MainWndProc(hWnd, uMsg, wParam, lParam);
}

#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <strsafe.h>
using MINIDUMPWRITEDUMP = BOOL(WINAPI*)(
	HANDLE hProcee, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType,
	PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
	auto hDbgHelp = LoadLibrary(_T("Dbghelp.dll"));
	if (hDbgHelp)
	{
		auto pMiniDumpWriteDump = (MINIDUMPWRITEDUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
		if (pMiniDumpWriteDump)
		{
			TCHAR tempPath[MAX_PATH] = {};
			if (::GetTempPath(_countof(tempPath), tempPath) == 0) {
				return EXCEPTION_EXECUTE_HANDLER;
			}
			TCHAR dumpPath[MAX_PATH];
			StringCchPrintf(dumpPath, MAX_PATH, _T("%s\\TxtMiru.dmp"), tempPath);
			HANDLE dumpFile = CreateFile(dumpPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, 0);
			if (INVALID_HANDLE_VALUE == dumpFile) {
				return EXCEPTION_EXECUTE_HANDLER;
			}
			if (dumpFile != INVALID_HANDLE_VALUE)
			{
				MINIDUMP_EXCEPTION_INFORMATION mei;
				mei.ThreadId = GetCurrentThreadId();
				mei.ExceptionPointers = pExceptionInfo;
				mei.ClientPointers = TRUE;

				pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile, MiniDumpNormal, &mei, nullptr, nullptr);
			}
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

int WINAPI _tWinMain(_In_ HINSTANCE hCurInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPTSTR lpsCmdLine, _In_ int nCmdShow)
{
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
	WINDOWPLACEMENT wndp = {};
	MSG    msg = {};
	HWND   hwnd = NULL;
	CGrStdWin stdwin(hCurInst);

	TCHAR szLanguageName[100];
	auto idLocal = GetSystemDefaultLCID();
	::GetLocaleInfo(idLocal, LOCALE_SENGLANGUAGE, szLanguageName, _countof(szLanguageName));
	_tsetlocale(LC_ALL, szLanguageName);
	UNREFERENCED_PARAMETER(nCmdShow);
	UNREFERENCED_PARAMETER(lpsCmdLine);
	UNREFERENCED_PARAMETER(hPrevInst);

	CGrMainWindow::theApp().m_lpsCmdLine = lpsCmdLine;
	if (!stdwin.InitApp(MainWndProc, CGrTxtMiru::ClassName(), IDI_APP, IDR_MENU)) {
		return FALSE;
	}

	// 設定ファイルの読み込み
	TCHAR iniFileName[MAX_PATH];
	_stprintf_s(iniFileName, _T("%s/%s.ini"), CGrTxtMiru::GetDataPath(), CGrTxtMiru::AppName());

	auto&& param = CGrTxtMiru::theApp().Param();
	param.Load(iniFileName);
	// フォントのロード 2.0.39.0
	std::vector<std::tstring> font_list;
	if (param.GetBoolean(CGrTxtParam::UseFont)) {
		// 本体のパスから
		std::tstring folderName;
		if (CGrShell::MakeFullPath(folderName, nullptr, _T("Font"))) {
			CGrShell::GetFullPathList(font_list, folderName.c_str(), _T("*.*"), false);
		}
		// データのパスから
		auto lpDataPath = CGrTxtMiru::GetDataPath();
		if (lpDataPath) {
			std::tstring datafolder;
			CGrShell::MakeFullPath(datafolder, lpDataPath, _T("Font"));
			if (_tcsicmp(folderName.c_str(), datafolder.c_str()) != 0) {
				// 本体のパスと別のパスが指定されていた場合のみ
				CGrShell::GetFullPathList(font_list, datafolder.c_str(), _T("*.*"), false);
			}
		}
		for (const auto& item : font_list) {
			auto filename = item.c_str();
			auto ext = CGrShell::GetFileExtConst(filename);
			if (/**/0 != _tcsicmp(ext, _T("emf"))
				&& 0 != _tcsicmp(ext, _T("svg"))) {
				AddFontResourceEx(filename, FR_PRIVATE, nullptr);
			}
		}
	}
	int wndp_num = param.GetPoints(CGrTxtParam::WindowSize, reinterpret_cast<int*>(&wndp), sizeof(wndp) / sizeof(int));
	wndp.length = sizeof(WINDOWPLACEMENT);
	if (wndp.rcNormalPosition.bottom - wndp.rcNormalPosition.top <= 0 || wndp_num < sizeof(wndp) / sizeof(int)) {
		// 範囲外の場合は、画面中央に移動
		// デュアルディスプレイ対応
		POINT point = { wndp.rcNormalPosition.left, wndp.rcNormalPosition.top };
		auto hMonitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY); // プライマリモニタのハンドル取得
		MONITORINFO mi = { sizeof(MONITORINFO) };
		if (hMonitor && ::GetMonitorInfo(hMonitor, &mi)) {
			// center;
			RECT desktop_rc;
			::SystemParametersInfo(SPI_GETWORKAREA, 0, &desktop_rc, 0);
			int w0 = desktop_rc.right - desktop_rc.left;
			int h0 = desktop_rc.bottom - desktop_rc.top;
			int w = w0 * 80 / 100;
			int h = h0 * 80 / 100;
			int x = w0 / 2 - w / 2;
			int y = h0 / 2 - h / 2;
			wndp.rcNormalPosition.left = x;
			wndp.rcNormalPosition.right = x + w;
			wndp.rcNormalPosition.top = y;
			wndp.rcNormalPosition.bottom = y + h;
			wndp.showCmd = SW_SHOWNORMAL;
		}
	}
	if (::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) == S_OK) {
		;
	}
	bool bFullScreen = param.GetBoolean(CGrTxtParam::FullScreen);
	if (bFullScreen) {
		// フルスクリーン対応 : 初回の画面表示を抑制して後でフルスクリーンにしてから画面に表示
		wndp.showCmd = SW_HIDE;
		nCmdShow = SW_HIDE;
	}
	hwnd = stdwin.InitInstance(nCmdShow, CGrTxtMiru::ClassName(), CGrTxtMiru::AppName(), &wndp);
	if (!hwnd) {
		return FALSE;
	}
	if (bFullScreen) {
		// フルスクリーン
		::SendMessage(hwnd, WM_COMMAND, IDFULLSCREEN, 0);
		::ShowWindow(hwnd, SW_SHOW);
		::UpdateWindow(hwnd);
	}
	// アクセラレータテーブル読み込み
	while (::GetMessage(&msg, NULL, 0, 0)) {
		if (!CGrMainWindow::theApp().PreTranslateMessage(&msg)) {
			continue;
		}
		if (CGrTxtMiru::theApp().IsDialogMessage(hwnd, msg)) {
			continue;
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	::CoUninitialize();
	::OleUninitialize();
	// iniファイル保存
	param.Save(iniFileName);
	// フォントのアンロード
	{
		for (const auto& item : font_list) {
			RemoveFontResourceEx(item.c_str(), FR_PRIVATE, nullptr);
		}
	}

	return msg.wParam;
}
