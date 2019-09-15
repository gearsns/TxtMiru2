#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "TxtFuncBookmark.h"
#include "TxtDocument.h"
#include "Text.h"
#include "TxtMiru.h"
#include "CurrentDirectory.h"
#include "Win32Wrap.h"

CGrTxtFuncBookmark::CGrTxtFuncBookmark(CGrTxtDocument &doc) : m_doc(doc), m_bookmark(doc.GetBookmark()), m_subTitle(doc.GetSubtitle()), CGrLoadDllFunc(_T("TxtFuncBookmark.dll"))
{
}
CGrTxtFuncBookmark::~CGrTxtFuncBookmark()
{
}

bool CGrTxtFuncBookmark::showLink(HWND hWnd, int mode)
{
	if(!m_fLink){
		SetProcPtr(m_fLink, GetProcAddress("TxtFuncShowLinkList", NULL));
		if(!m_fLink){
			return false;
		}
	}
	auto &&param = CGrTxtMiru::theApp().Param();
	return m_fLink(hWnd, mode, &m_bookmark, &m_subTitle, CGrTxtMiru::GetDataPath(), &param);
}

bool CGrTxtFuncBookmark::HideLink()
{
	return showLink(NULL, 3);
}

bool CGrTxtFuncBookmark::ShowLink(HWND hWnd)
{
	return showLink(hWnd, 0);
}

bool CGrTxtFuncBookmark::ShowBookmark(HWND hWnd)
{
	return showLink(hWnd, 1);
}

bool CGrTxtFuncBookmark::ShowSubtitle(HWND hWnd)
{
	return showLink(hWnd, 2);
}

bool CGrTxtFuncBookmark::RefreshBook()
{
	if(!m_fRefreshBk){
		SetProcPtr(m_fRefreshBk, GetProcAddress("TxtFuncRefreshBookList", NULL));
		if(!m_fRefreshBk){
			return false;
		}
	}
	return m_fRefreshBk();
}

bool CGrTxtFuncBookmark::IsShowBook()
{
	if(!m_fIsShowBk){
		SetProcPtr(m_fIsShowBk, GetProcAddress("TxtFuncIsShowBookList", NULL));
		if(!m_fIsShowBk){
			return false;
		}
	}
	return m_fIsShowBk();
}

bool CGrTxtFuncBookmark::IsBookMember(HWND hWnd)
{
	if(!m_fIsMemBk){
		SetProcPtr(m_fIsMemBk, GetProcAddress("TxtFuncIsBookListMember", NULL));
		if(!m_fIsMemBk){
			return false;
		}
	}
	return m_fIsMemBk(hWnd);
}

bool CGrTxtFuncBookmark::RefreshLink()
{
	if(!m_fRefreshLn){
		SetProcPtr(m_fRefreshLn, GetProcAddress("TxtFuncRefreshLinkList", NULL));
		if(!m_fRefreshLn){
			return false;
		}
	}
	return m_fRefreshLn();
}

bool CGrTxtFuncBookmark::IsShowLink()
{
	if(!m_fIsShowLn){
		SetProcPtr(m_fIsShowLn, GetProcAddress("TxtFuncIsShowLinkList", NULL));
		if(!m_fIsShowLn){
			return false;
		}
	}
	return m_fIsShowLn();
}

bool CGrTxtFuncBookmark::IsLinkMember(HWND hWnd)
{
	if(!m_fIsMemLn){
		SetProcPtr(m_fIsMemLn, GetProcAddress("TxtFuncIsLinkListMember", NULL));
		if(!m_fIsMemLn){
			return false;
		}
	}
	return m_fIsMemLn(hWnd);
}

// 2.0.17.0
bool CGrTxtFuncBookmark::ShowRubyList(HWND hWnd, CGrTxtDocument *pDoc, CGrTxtFuncIBookmark *pBookkmark)
{
	if(!m_fRuby){
		SetProcPtr(m_fRuby, GetProcAddress("TxtFuncShowRubyList", NULL));
		if(!m_fRuby){
			return false;
		}
	}
	auto &&param = CGrTxtMiru::theApp().Param();
	return m_fRuby(hWnd, pDoc, &m_bookmark, CGrTxtMiru::GetDataPath(), &param);
}
// 2.0.17.0
bool CGrTxtFuncBookmark::HideRubyList()
{
	if(!m_fRuby){
		SetProcPtr(m_fRuby, GetProcAddress("TxtFuncShowRubyList", NULL));
		if(!m_fRuby){
			return false;
		}
	}
	return m_fRuby(NULL, nullptr, nullptr, nullptr, nullptr);
}

// 2.0.17.0
bool CGrTxtFuncBookmark::IsShowRubyList()
{
	if(!m_fIsShowRb){
		SetProcPtr(m_fIsShowRb, GetProcAddress("TxtFuncIsShowRubyList", NULL));
		if(!m_fIsShowRb){
			return false;
		}
	}
	return m_fIsShowRb();
}
// 2.0.17.0
bool CGrTxtFuncBookmark::RefreshRubyList()
{
	if(!m_fRefreshRb){
		SetProcPtr(m_fRefreshRb, GetProcAddress("TxtFuncRefreshRubyList", NULL));
		if(!m_fRefreshRb){
			return false;
		}
	}
	return m_fRefreshRb();
}
// 2.0.18.0
bool CGrTxtFuncBookmark::SaveText(HWND hWnd, CGrTxtDocument *pDoc, CGrTxtFuncIBookmark *pBookkmark)
{
	if(!m_fSaveText){
		SetProcPtr(m_fSaveText, GetProcAddress("TxtFuncSaveText", NULL));
		if(!m_fSaveText){
			return false;
		}
	}
	std::tstring fileName;
	std::tstring realFileName;
	pDoc->GetFileName(fileName);
	pDoc->GetRealFileName(realFileName);
	auto &&param = CGrTxtMiru::theApp().Param();
	return m_fSaveText(hWnd, pDoc, fileName.c_str(), realFileName.c_str(), &m_bookmark, CGrTxtMiru::GetDataPath(), &param);
}

// 2.0.19.0
bool CGrTxtFuncBookmark::Search(HWND hWnd, TxtMiru::TextPoint &point, CGrTxtDocument *pDoc, CGrTxtFuncIBookmark *pBookkmark/* = nullptr*/)
{
	if(!m_fSearch){
		SetProcPtr(m_fSearch, GetProcAddress("TxtFuncSearch", NULL));
		if(!m_fSearch){
			return false;
		}
	}
	auto &&param = CGrTxtMiru::theApp().Param();
	return m_fSearch(hWnd, pDoc, point, point, &m_bookmark, CGrTxtMiru::GetDataPath(), &param);
}

bool CGrTxtFuncBookmark::Open(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookkmark, CGrTxtFuncISubTitle *pSubtitle)
{
	if(!m_fOpen){
		SetProcPtr(m_fOpen, GetProcAddress("TxtFuncBookmarkOpen", NULL));
		if(!m_fOpen){
			return false;
		}
	}
	if(!pBookkmark){
		pBookkmark = &m_bookmark;
	}
	if(!pSubtitle){
		pSubtitle = &m_subTitle;
	}
	auto &&param = CGrTxtMiru::theApp().Param();
	return m_fOpen(lpFileName, pBookkmark, pSubtitle, CGrTxtMiru::GetDataPath(), &param);
}

bool CGrTxtFuncBookmark::SaveAs(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookkmark, CGrTxtFuncISubTitle *pSubtitle)
{
	if(!m_fSave){
		SetProcPtr(m_fSave, GetProcAddress("TxtFuncBookmarkSaveAs", NULL));
		if(!m_fSave){
			return false;
		}
	}
	if(!pBookkmark){
		pBookkmark = &m_bookmark;
	}
	if(!pSubtitle){
		pSubtitle = &m_subTitle;
	}
	auto &&param = CGrTxtMiru::theApp().Param();
	CGrCurrentDirectory cur;
	return m_fSave(lpFileName, pBookkmark, pSubtitle, CGrTxtMiru::GetDataPath(), &param);
}

bool CGrTxtFuncBookmark::ShowBook(HWND hWnd, LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookkmark, CGrTxtFuncISubTitle *pSubtitle)
{
	if(!m_fShow){
		SetProcPtr(m_fShow, GetProcAddress("TxtFuncShowBookList", NULL));
		if(!m_fShow){
			return false;
		}
	}
	if(!pBookkmark){
		pBookkmark = &m_bookmark;
	}
	if(!pSubtitle){
		pSubtitle = &m_subTitle;
	}
	auto &&param = CGrTxtMiru::theApp().Param();
	return m_fShow(hWnd, lpFileName, pBookkmark, CGrTxtMiru::GetDataPath(), &param);
}

// 2.0.13.0
bool CGrTxtFuncBookmark::SearchFiles(HWND hWnd)
{
	if(!m_fSearchFil){
		SetProcPtr(m_fSearchFil, GetProcAddress("TxtFuncSearchFiles", NULL));
		if(!m_fSearchFil){
			return false;
		}
	}
	auto &&param = CGrTxtMiru::theApp().Param();
	return m_fSearchFil(hWnd, CGrTxtMiru::GetDataPath(), &param);
}

bool CGrTxtFuncBookmark::HideBook()
{
	if(!m_fShow){
		SetProcPtr(m_fShow, GetProcAddress("TxtFuncShowBookList", NULL));
		if(!m_fShow){
			return false;
		}
	}
	return m_fShow(NULL, nullptr, nullptr, nullptr, nullptr);
}

bool CGrTxtFuncBookmark::UpdatePage()
{
	if(!m_fUpdPage){
		SetProcPtr(m_fUpdPage, GetProcAddress("TxtFuncUpdatePage", NULL));
		if(!m_fUpdPage){
			return false;
		}
	}
	return m_fUpdPage();
}

HWND CGrTxtFuncBookmark::GetWnd(CGrTxtFuncBookmark::ModelWindowID id)
{
	if(!m_fGetWnd){
		SetProcPtr(m_fGetWnd, GetProcAddress("TxtFuncGetWnd", NULL));
		if(!m_fGetWnd){
			return false;
		}
	}
	return m_fGetWnd(id);
}

void CGrTxtFuncBookmark::UnInstall()
{
	if(!m_fUnInst){
		SetProcPtr(m_fUnInst, GetProcAddress("TxtFuncUnInstall", NULL));
		if(!m_fUnInst){
			return;
		}
	}
	m_fUnInst();
}

void CGrTxtFuncBookmark::Unload()
{
	if(m_hDLL){
		UnInstall();
		FreeLibrary(m_hDLL);
	}
	m_hDLL         = NULL;
	m_fOpen        = nullptr;
	m_fSave        = nullptr;
	m_fShow        = nullptr;
	m_fLink        = nullptr;
	m_fRefreshBk   = nullptr;
	m_fIsShowBk    = nullptr;
	m_fIsMemBk     = nullptr;
	m_fRefreshLn   = nullptr;
	m_fIsShowLn    = nullptr;
	m_fIsMemLn     = nullptr;
	m_fUpdPage     = nullptr;
	m_fGetWnd      = nullptr;
	m_fUnInst      = nullptr;
	m_fSearchFil   = nullptr;
	m_fRuby        = nullptr;
	m_fIsShowRb    = nullptr;
	m_fRefreshRb   = nullptr;
	m_fSaveText    = nullptr;
	m_fSearch      = nullptr;
	m_fAddFavorite = nullptr;
}

bool CGrTxtFuncBookmark::AddFavorite(HWND hWnd, LPCTSTR lpURL)
{
	if (!m_fAddFavorite) {
		SetProcPtr(m_fAddFavorite, GetProcAddress("TxtFuncAddFavorite", NULL));
		if (!m_fAddFavorite) {
			return false;
		}
	}
	return m_fAddFavorite(hWnd, lpURL) == IDOK;
}
