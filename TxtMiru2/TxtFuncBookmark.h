#ifndef __TXTFUNCBOOKMARK_H__
#define __TXTFUNCBOOKMARK_H__

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "stltchar.h"
#include "TxtMiruDef.h"
#include "LoadDllFunc.h"
#include "TxtFuncIBookmark.h"

#include <vector>

class CGrTxtDocument;
class CGrTxtFuncBookmark : public CGrLoadDllFunc
{
public:
	enum class ModelWindowID {
		LINKDLG,
		BOOKDLG,
	};
private:
	bool (cdecl *m_fOpen)(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam) = nullptr;
	bool (cdecl *m_fSave)(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam) = nullptr;
	bool (cdecl *m_fShow)(HWND hWnd, LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam) = nullptr;
	bool (cdecl *m_fLink)(HWND hWnd, int mode, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam) = nullptr;
	bool (cdecl *m_fRefreshBk)() = nullptr;
	bool (cdecl *m_fIsShowBk)() = nullptr;
	bool (cdecl *m_fIsMemBk)(HWND hWnd) = nullptr;
	bool (cdecl *m_fRefreshLn)() = nullptr;
	bool (cdecl *m_fIsShowLn)() = nullptr;
	bool (cdecl *m_fIsMemLn)(HWND hWnd) = nullptr;
	bool (cdecl *m_fUpdPage)() = nullptr;
	HWND (cdecl *m_fGetWnd)(ModelWindowID id) = nullptr;
	void (cdecl *m_fUnInst)() = nullptr;
	bool (cdecl *m_fSearchFil)(HWND hWnd, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam) = nullptr;
	bool (cdecl *m_fRuby)(HWND hWnd, void *pDoc, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam) = nullptr;
	bool (cdecl *m_fIsShowRb)() = nullptr;
	bool (cdecl *m_fRefreshRb)() = nullptr;
	bool (cdecl *m_fSaveText)(HWND hWnd, void *pDoc, LPCTSTR lpFileName, LPCTSTR lpRealFileName, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam) = nullptr;
	bool (cdecl* m_fSearch)(HWND hWnd, void* pDoc, const TxtMiru::TextPoint& point, TxtMiru::TextPoint& out_point, CGrTxtFuncIBookmark* pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam* pParam) = nullptr;
	bool (cdecl* m_fAddFavorite)(HWND hWnd, LPCTSTR lpURL) = nullptr;
protected:
	CGrLoadDllFunc *m_pfunc = nullptr;
	CGrTxtFuncIBookmark &m_bookmark;
	CGrTxtFuncISubTitle &m_subTitle;
	CGrTxtDocument &m_doc;
private:
	bool showLink(HWND hWnd, int mode);
public:
	CGrTxtFuncBookmark(CGrTxtDocument &doc);
	virtual ~CGrTxtFuncBookmark();
	virtual void Unload();
	bool ShowLink(HWND hWnd);
	bool HideLink();
	bool ShowBookmark(HWND hWnd);
	bool ShowSubtitle(HWND hWnd);
	bool RefreshBook();
	bool IsShowBook();
	bool IsBookMember(HWND hWnd);
	bool RefreshLink();
	bool IsShowLink();
	bool IsLinkMember(HWND hWnd);
	bool SearchFiles(HWND hWnd);
	bool ShowRubyList(HWND hWnd, CGrTxtDocument *pDoc, CGrTxtFuncIBookmark *pBookkmark = nullptr);
	bool HideRubyList();
	bool IsShowRubyList();
	bool RefreshRubyList();
	bool SaveText(HWND hWnd, CGrTxtDocument *pDoc, CGrTxtFuncIBookmark *pBookkmark = nullptr);
	bool Search(HWND hWnd, TxtMiru::TextPoint &point, CGrTxtDocument *pDoc, CGrTxtFuncIBookmark *pBookkmark = nullptr);
	//
	bool Open(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookkmark = nullptr, CGrTxtFuncISubTitle *pSubtitle = nullptr);
	bool SaveAs(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookkmark = nullptr, CGrTxtFuncISubTitle *pSubtitle = nullptr);
	bool ShowBook(HWND hWnd, LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookkmark = nullptr, CGrTxtFuncISubTitle *pSubtitle = nullptr);
	bool HideBook();
	bool AddFavorite(HWND hWnd, LPCTSTR lpURL);
	//
	bool UpdatePage();
	//
	HWND GetWnd(ModelWindowID id);
	void UnInstall();
};

#endif // __TXTFUNCBOOKMARK_H__
