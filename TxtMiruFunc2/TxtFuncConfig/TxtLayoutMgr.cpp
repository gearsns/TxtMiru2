#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "TxtLayoutMgr.h"
#include "Shell.h"
#include "CurrentDirectory.h"
#include "TxtFunc.h"

CGrTxtLayoutMgr::CGrTxtLayoutMgr() : m_pTxtLayout(nullptr)
{
	m_pDefTxtLayout = new CGrBunkoTxtLayout();
}

CGrTxtLayoutMgr::~CGrTxtLayoutMgr()
{
	if(m_pTxtLayout){
		delete m_pTxtLayout;
	}
	if(m_pDefTxtLayout){
		delete m_pDefTxtLayout;
	}
}

bool CGrTxtLayoutMgr::Open(LPCTSTR lpFileName)
{
	if(m_pDefTxtLayout->IsSupported(lpFileName)){
		if(m_pTxtLayout){
			delete m_pTxtLayout;
		}
		m_pTxtLayout = nullptr;
		return GeTxtLayout().Open(lpFileName);
	}
	auto *pLayout = new CGrCustomTxtLayout();
	if(pLayout->Open(lpFileName)){
		if(m_pTxtLayout){
			delete m_pTxtLayout;
		}
		m_pTxtLayout = pLayout;
	} else {
		delete pLayout;
		return false;
	}
	return true;
}

bool CGrTxtLayoutMgr::Save(LPCTSTR lpFileName)
{
	return GeTxtLayout().Save(lpFileName);
}

const CGrTxtLayout &CGrTxtLayoutMgr::GetConstTxtLayout() const
{
	if(m_pTxtLayout){
		return *m_pTxtLayout;
	}
	return *m_pDefTxtLayout;
}

CGrTxtLayout &CGrTxtLayoutMgr::GeTxtLayout()
{
	if(m_pTxtLayout){
		return *m_pTxtLayout;
	}
	return *m_pDefTxtLayout;
}

void CGrTxtLayoutMgr::SetLayout(CGrTxtLayout *playout)
{
	if(m_pTxtLayout){
		delete m_pTxtLayout;
	}
	m_pTxtLayout = playout;
}

int CGrTxtLayoutMgr::ConfigurationDlg(HWND hWnd, LPCTSTR type, LPCTSTR name, LPCTSTR lpFileName)
{
	CGrCustomTxtLayout l;
	l.Open(lpFileName);
	return l.ConfigurationDlg(hWnd, type, name, lpFileName);
}

CGrTxtLayout *CGrTxtLayoutMgr::GetTxtLayout(const CGrTxtLayoutMgr::LaytoutInfo &li)
{
	CGrTxtLayout *pLayout = nullptr;
	if(!li.filename.empty()){
		pLayout = new CGrCustomTxtLayout();
		if(!pLayout->Open(li.filename.c_str())){
			delete pLayout;
			pLayout = nullptr;
		}
	}
	if(!pLayout){
		/*   */if(CGrText::isMatchChar(li.type.c_str(), CGrBunkoTxtLayout       ::Name())){ pLayout = new CGrBunkoTxtLayout       ();
		} else if(CGrText::isMatchChar(li.type.c_str(), CGrShinShoTate2TxtLayout::Name())){ pLayout = new CGrShinShoTate2TxtLayout();
		} else { pLayout = new CGrBunkoTxtLayout();
		}
	}
	return pLayout;
}

static void getLayoutList(std::vector<CGrTxtLayoutMgr::LaytoutInfo> &lil, LPCTSTR lpFolder)
{
	WIN32_FIND_DATA wfd = {};
	std::tstring target;
	CGrShell::ToPrettyFileName(lpFolder, target);
	if(!CGrShell::EndBackSlash(const_cast<TCHAR*>(target.c_str()))){
		target += _T("/");
	}
	target += _T("Layout/");
	std::tstring findfilename = target;
	findfilename += _T("*.lay");
	auto hFile = ::FindFirstFile(findfilename.c_str(), &wfd);
	CGrCustomTxtLayout layout;
	do {
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			continue;
		}
		std::tstring filename = target;
		filename += wfd.cFileName;
		if(layout.Open(filename.c_str())){
			CGrTxtLayoutMgr::LaytoutInfo li;
			li.type     = layout.OpenLayoutType();
			li.filename = filename;
			layout.GetLayoutName(li.name);
			if(li.name.empty()){
				/*   */if(CGrText::isMatchChar(CGrBunkoTxtLayout       ::Name(), li.type.c_str())){ li.name = _T("文庫本");
				} else if(CGrText::isMatchChar(CGrShinShoTate2TxtLayout::Name(), li.type.c_str())){ li.name = _T("新書縦2段");
				} else if(li.type.empty()){ li.name = filename;
				} else                    { li.name = li.type;
				}
			}
			auto lil_it = std::lower_bound(lil.begin(),lil.end(),li,CGrTxtLayoutMgr::LaytoutInfo::LessType);
			if(lil_it == lil.end() || lil_it->type != li.type || lil_it->name != li.name){
				lil.push_back(li);
				std::sort(lil.begin(),lil.end(),CGrTxtLayoutMgr::LaytoutInfo::LessType);
			}
		}
	} while(::FindNextFile(hFile, &wfd));
	::FindClose(hFile);
}

void CGrTxtLayoutMgr::GetLayoutList(std::vector<CGrTxtLayoutMgr::LaytoutInfo> &lil)
{
	getLayoutList(lil, CGrTxtFunc::GetDataPath());
	{
		auto lil_it = std::lower_bound(lil.begin(),lil.end(),LaytoutInfo{CGrBunkoTxtLayout       ::Name()},LaytoutInfo::LessType);
		if(lil_it == lil.end() || lil_it->type != CGrBunkoTxtLayout       ::Name()){
			LaytoutInfo li;
			li.type = CGrBunkoTxtLayout::Name();
			li.name = _T("文庫本");
			li.filename = _T("Layout/Bunko.lay");
			lil.push_back(li);
		}
	}
	{
		auto lil_it = std::lower_bound(lil.begin(),lil.end(),LaytoutInfo{CGrShinShoTate2TxtLayout::Name()},LaytoutInfo::LessType);
		if(lil_it == lil.end() || lil_it->type != CGrShinShoTate2TxtLayout::Name()){
			LaytoutInfo li;
			li.type = CGrShinShoTate2TxtLayout::Name();
			li.name = _T("新書縦2段");
			li.filename = _T("Layout/ShinshoTate2.lay");
			lil.push_back(li);
		}
	}
}
