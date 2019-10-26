#pragma warning( disable : 4786 )

#include "TxtBookmark.h"
#include "TxtDocument.h"
#include "TxtMiru.h"
#include "TxtMiruDef.h"
#include "TxtParam.h"
#include "Shell.h"
#include "stlutil.h"
#include "CurrentDirectory.h"
#include "TxtFuncBookmark.h"

//#define __DBG__
#include "Debug.h"

// 書式  :
// siori : 001>#Style:スタイル名
// siroi : 002>#Ver:TxtMiru 2.0
// siori : 003>#FileName:ファイル名
// siori : 004>#Title:タイトル
// siori : 005>#Author:作者
// siori : 006>#Page:最後に表示していたページ/ページ数
// siori : 007>#LastWriteTime:最終更新日
// siori : 008>属性[=0x00,0x01],iLine,iIndex,iPos,名称
// siori : 009>属性[=0x02],iPage,0,0,名称
// 属性  : 0x00 = 無し
//         0x01 = 最後に表示していた位置(非表示)
//         0x02 = 最後に表示していたページ(非表示)
// iLine : 物理行
// iIndex: 文字タイプごとのインデックス
// iPos  : iIndex内の位置
// iPage : ページ

static void setName(const CGrTxtDocument &doc, CGrTxtBookmark::Bookmark &bookmark)
{
	doc.ToString(bookmark.str, bookmark.tp, TxtMiru::TextPoint{bookmark.tp.iLine+1, -1, 0});
}

static bool setPage(const CGrTxtDocument &doc, CGrTxtBookmark::Bookmark &bookmark)
{
	int page = doc.TextPointToPage(bookmark.tp);
	if(page >= 0){
		const auto &ll = doc.GetConstLineList();
		// 改丁の場合は、ページを足す
		if(bookmark.tp.iLine < static_cast<signed int>(ll.size())){
			const auto &text_list = ll[bookmark.tp.iLine].text_list;
			if(bookmark.tp.iIndex < static_cast<signed int>(text_list.size())){
				switch(text_list[bookmark.tp.iIndex].textType){
				case TxtMiru::TextType::NEXT_PAPER:
					// 奇数に
					if(page % 2 == 0){
						page += 1;
					} else {
						page += 2;
					}
					break;
				case TxtMiru::TextType::NEXT_PAPER_FIRST:
					if(page % 2 == 0){
						page += 2;
					} else {
						page += 1;
					}
					break;
				case TxtMiru::TextType::NEXT_PAGE:
					if(page == 0 && bookmark.tp.iIndex == 0 && bookmark.tp.iLine == 0){
						;
					} else {
						++page;
					}
					break;
				}
			}
		}
		bookmark.iPage = page;
		setName(doc, bookmark);
		return true;
	}
	bookmark.iPage = 0;
	return false;
}

static bool getBookmark(int page, const CGrTxtDocument &doc, CGrTxtBookmark::Bookmark &bookmark)
{
	if(doc.StartPageToTextPoint(bookmark.tp, page)){
		bookmark.iPage = page;
		setName(doc, bookmark);
		return true;
	}
	return false;
}

CGrTxtBookmark::CGrTxtBookmark()
{
}
CGrTxtBookmark::~CGrTxtBookmark()
{
}

void CGrTxtBookmark::AddPage(int page)
{
	if(!m_pDoc){ return; }
	Bookmark bookmark;
	if(getBookmark(page, *m_pDoc, bookmark)){
		// 栞は、重複して追加しない
		for(const auto &item : m_bookMarks){
			if(item.tp == bookmark.tp){
				return;
			}
		}
		m_bookMarks.push_back(std::move(bookmark));
		std::sort(std::begin(m_bookMarks), std::end(m_bookMarks), [](const Bookmark &c1, const Bookmark &c2){
			return c1.tp < c2.tp;
		});
	}
}

int CGrTxtBookmark::GetPage(int idx) const
{
	if(idx < 0 || static_cast<int>(m_bookMarks.size()) <= idx){
		return -1;
	}
	return m_bookMarks[idx].iPage;
}

int CGrTxtBookmark::Count() const
{
	return m_bookMarks.size();
}

const std::tstring &CGrTxtBookmark::GetName(int idx) const
{
	return m_bookMarks[idx].str;
}

void CGrTxtBookmark::Clear()
{
	m_txtFileName.clear();
	m_fileName.clear();
	m_bookMarks.clear();
	m_bookMarks.shrink_to_fit();
	SetLastPage(1);
	m_lastPage.iPage     = 0;
	m_lastPage.tp.iLine  = 0;
	m_lastPage.tp.iIndex = 0;
	m_lastPage.tp.iPos   = 0;
	m_lastPage.str.resize(0);
}

void CGrTxtBookmark::RebuildPageNo()
{
	if(!m_pDoc){ return; }
	for(auto &&item : m_bookMarks){
		setPage(*m_pDoc, item);
	}
	if(m_lastPage.tp.iLine < 0){
		getBookmark(m_lastPage.iPage, *m_pDoc, m_lastPage);
	} else {
		setPage(*m_pDoc, m_lastPage);
	}
}

static const TCHAR *ExtSiori = _T(".siori");
LPCTSTR CGrTxtBookmark::GetExtName()
{
	return ExtSiori;
}

enum class PARAMTYPE {
	AUTHOR       ,
	BMFILENAME   ,
	FILENAME     ,
	LASTWRITETIME,
	PAGE         ,
	TITLE        ,
	VER          ,
	IS_UPDATECHEK,
	IS_WAITTING  ,
	MaxNum
};
static struct NameTypeMap {
	LPCTSTR name;
	PARAMTYPE type;
} l_param_map[] = {
	// 予め name で ソートしておくこと
	{_T("Author"       ), PARAMTYPE::AUTHOR       },
	{_T("BMFileName"   ), PARAMTYPE::BMFILENAME   },
	{_T("FileName"     ), PARAMTYPE::FILENAME     },
	{_T("IsUpdateCheck"), PARAMTYPE::IS_UPDATECHEK},
	{_T("IsWaitting"   ), PARAMTYPE::IS_WAITTING  },
	{_T("LastWriteTime"), PARAMTYPE::LASTWRITETIME},
	{_T("Page"         ), PARAMTYPE::PAGE         },
	{_T("Title"        ), PARAMTYPE::TITLE        },
	{_T("Ver"          ), PARAMTYPE::VER          },
};
static int NameTypeMapCompare(const void *key, const void *pdata)
{
	auto name  = static_cast<LPCTSTR>(key);
	auto *pntm = static_cast<const NameTypeMap*>(pdata);
	return _tcscmp(name, pntm->name);
}

static PARAMTYPE GetParamType(LPCTSTR str)
{
	auto *pntm = static_cast<NameTypeMap*>(bsearch(str, l_param_map, sizeof(l_param_map)/sizeof(NameTypeMap), sizeof(NameTypeMap), NameTypeMapCompare));
	if(pntm){
		return pntm->type;
	}
	return PARAMTYPE::MaxNum;
}

void CGrTxtBookmark::SetParamString(LPCTSTR lpName, LPCTSTR lpValue)
{
	switch(GetParamType(lpName)){
	case PARAMTYPE::BMFILENAME   :
		if(lpValue){
			SetFileName(std::tstring(lpValue));
		} else {
			m_fileName.clear();
		}
		break;
	case PARAMTYPE::FILENAME     :
		if(lpValue){
			SetTextFileName(std::tstring(lpValue));
		} else {
			m_txtFileName.clear();
		}
		break;
	case PARAMTYPE::LASTWRITETIME:
		if(lpValue){
			SetLastWriteTime(std::tstring(lpValue));
		} else {
			m_lastWriteTime.clear();
		}
		break;
	}
}
void CGrTxtBookmark::SetParamInteger(LPCTSTR lpName, int iValue)
{
	switch(GetParamType(lpName)){
	case PARAMTYPE::IS_UPDATECHEK: m_iUpdateCheck = iValue; break;
	}
}

static LPCTSTR STR_BOOKMARK_VER   = _T("TxtMiru 2.0.4.21");
static LPCTSTR STR_BOOKMARK_EMPTY = _T(""                );

LPCTSTR CGrTxtBookmark::GetParamString(LPCTSTR lpName)
{
	switch(GetParamType(lpName)){
	case PARAMTYPE::AUTHOR       : return m_pDoc->GetAuthor().c_str(); break;
	case PARAMTYPE::BMFILENAME   : return m_fileName         .c_str(); break;
	case PARAMTYPE::FILENAME     : return m_txtFileName      .c_str(); break;
	case PARAMTYPE::LASTWRITETIME: return m_lastWriteTime    .c_str(); break;
	case PARAMTYPE::TITLE        : return m_pDoc->GetTitle() .c_str(); break;
	case PARAMTYPE::VER          : return STR_BOOKMARK_VER           ; break;
	}
	return STR_BOOKMARK_EMPTY;
}

int CGrTxtBookmark::GetParamInteger(LPCTSTR lpName)
{
	switch(GetParamType(lpName)){
	case PARAMTYPE::PAGE         : return m_pDoc->GetTotalPage(); break;
	case PARAMTYPE::IS_UPDATECHEK: return m_iUpdateCheck; break;
	case PARAMTYPE::IS_WAITTING  : return CGrTxtMiru::theApp().IsWaitting() ? TRUE : FALSE; break;
	}
	return -1;
}

void CGrTxtBookmark::SetTextFileName(const std::tstring &str)
{
	CGrShell::ToPrettyFileName(str.c_str(), m_txtFileName);
}

void CGrTxtBookmark::GetTextFileName(std::tstring &str) const
{
	str = m_txtFileName;
}

void CGrTxtBookmark::SetFileName(const std::tstring &str)
{
	CGrShell::ToPrettyFileName(str.c_str(), m_fileName);
}

void CGrTxtBookmark::GetFileName(std::tstring &str) const
{
	str = m_fileName;
}

void CGrTxtBookmark::SetLastWriteTime(const std::tstring &str)
{
	m_lastWriteTime = str;
}

void CGrTxtBookmark::GetLastWriteTime(std::tstring &str) const
{
	str = m_lastWriteTime;
}

int CGrTxtBookmark::GetLastPage() const
{
	return m_lastPage.iPage;
}

void CGrTxtBookmark::SetLastPage(int page)
{
	if(!m_pDoc){ return; }
	if(!getBookmark(page, *m_pDoc, m_lastPage)){
		if (page > m_pDoc->GetTotalPage()) {
			m_lastPage.iPage = m_pDoc->GetTotalPage();
		}
		else {
			m_lastPage.iPage = page;
		}
		m_lastPage.tp.iLine  = 0;
		m_lastPage.tp.iIndex = 0;
		m_lastPage.tp.iPos   = 0;
		m_lastPage.str.resize(0);
	}
}

void CGrTxtBookmark::Attach(CGrTxtDocument *pDoc)
{
	m_pDoc = pDoc;
}

void CGrTxtBookmark::Delete(int idx)
{
	if(static_cast<int>(m_bookMarks.size()) > idx){
		auto it=std::begin(m_bookMarks);
		m_bookMarks.erase(it+idx);
	}
}

bool CGrTxtBookmark::Open(LPCTSTR lpFileName)
{
	Clear();
	auto *pFunc = static_cast<CGrTxtFuncBookmark*>(CGrTxtMiru::theApp().GetDllFunc(CGrTxtMiru::DLLFncType::TxtFuncBookmark));
	return pFunc ? pFunc->Open(lpFileName, this, &(m_pDoc->GetSubtitle())) : false;

}

bool CGrTxtBookmark::saveAs(LPCTSTR lpFileName)
{
	auto *pFunc = static_cast<CGrTxtFuncBookmark*>(CGrTxtMiru::theApp().GetDllFunc(CGrTxtMiru::DLLFncType::TxtFuncBookmark));
	return pFunc ? pFunc->SaveAs(lpFileName, this, &(m_pDoc->GetSubtitle())) : false;
}

bool CGrTxtBookmark::Save()
{
	return saveAs(m_fileName.c_str());
}

bool CGrTxtBookmark::SaveAs(LPCTSTR lpFileName)
{
	std::tstring filename;
	auto lpExt = CGrShell::GetFileExtConst(lpFileName);
	if(!lpExt || !*lpExt){
		CGrText::FormatMessage(filename, _T("%1!s!%2!s!"), lpFileName, ExtSiori);
		lpFileName = filename.c_str();
	}
	return saveAs(lpFileName);
}

CGrTxtFuncIBookmarkLeaf* CGrTxtBookmark::GetLeaf(int idx)
{
	if(idx < static_cast<signed int>(m_bookMarks.size())){
		return &(m_bookMarks[idx]);
	}
	return nullptr;
}
CGrTxtFuncIBookmarkLeaf* CGrTxtBookmark::GetLatest()
{
	return &m_lastPage;
}
void CGrTxtBookmark::ClearLeaf()
{
	m_bookMarks.clear();
	m_bookMarks.shrink_to_fit();
}
bool CGrTxtBookmark::AddLeaf(int iPage, int iLine, int iIndex, int iPos, LPCTSTR name)
{
	Bookmark bookmark;
	bookmark.iPage     = iPage ;
	bookmark.tp.iLine  = iLine ;
	bookmark.tp.iIndex = iIndex;
	bookmark.tp.iPos   = iPos  ;
	if(name){
		bookmark.str = name;
	}
	m_bookMarks.push_back(std::move(bookmark));
	return true;
}
bool CGrTxtBookmark::SetLatest(int iPage, int iLine, int iIndex, int iPos, LPCTSTR name)
{
	m_lastPage.iPage     = iPage ;
	m_lastPage.tp.iLine  = iLine ;
	m_lastPage.tp.iIndex = iIndex;
	m_lastPage.tp.iPos   = iPos  ;
	if(name){
		m_lastPage.str = name;
	}
	return true;
}

int CGrTxtBookmark::GetPageById(LPCTSTR lpID) const
{
	for(const auto &item : m_bookMarks){
		if(_tcsicmp(item.str.c_str(), lpID) == 0){
			return item.iPage;
		}
	}
	return -1;
}

bool CGrTxtBookmark::ShowBookmarkList(HWND hWnd, bool bShow)
{
	auto *pFunc = static_cast<CGrTxtFuncBookmark*>(CGrTxtMiru::theApp().GetDllFunc(CGrTxtMiru::DLLFncType::TxtFuncBookmark));
	if (pFunc) {
		if (bShow) {
			return pFunc->ShowBook(hWnd, m_txtFileName.c_str(), this);
		}
		else {
			return pFunc->HideBook();
		}
	}
	else {
		return false;
	}
}

void CGrTxtBookmark::UpdateUI()
{
	if(m_pfunc->isLoaded()){
		;
	}
}

bool CGrTxtBookmark::isLinkWindowVisible()
{
	if(m_pfunc->isLoaded()){
		;
	}
	return false;
}

void CGrTxtBookmark::Unload()
{
}
