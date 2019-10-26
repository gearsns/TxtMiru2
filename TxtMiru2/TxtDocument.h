#ifndef __TXTDOCUMENT_H__
#define __TXTDOCUMENT_H__

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "stltchar.h"

#include <vector>
#include <list>
#include <stack>
#include <map>

#include "TxtParser.h"
#include "TxtParam.h"
#include "TxtBookmark.h"
#include "TxtSubtitle.h"
#include "TxtSearch.h"
#include "TxtBuffer.h"
#include "TxtLayout.h"

using CGrDocOpenProcCallbak = void (__stdcall *)(int ret, LPARAM lParam);
class CGrTxtDocument
{
public:
	enum error_code {err_out_of_range};
	enum class TXTMIRU_FT {
		ARCHIVE,
		SIORI  ,
		TEXT   ,
		LINK   ,
		NONE   ,
	};
	static TXTMIRU_FT GetFileType(LPCTSTR lpFileName);
public:
	CGrTxtDocument();
	virtual ~CGrTxtDocument();
public:
	bool Open(LPCTSTR lpFileName = NULL);
	bool OpenClipboard();
	bool OpenText(LPCTSTR lpText);
	void OpenError();
	bool IsOpenFile(){ return m_bOpenFile; }
	TxtMiru::LineInfo &GetLineInfo(int row);
	void GetUsedTopN(std::tstring &out_str, int num = 10);

	// TxtBufferへのアクセス
	const CGrTxtBuffer &GetTxtBuffer() const { return m_txtBuffer; }
	const TxtMiru::LineList &GetConstLineList() const { return m_txtBuffer.GetConstLineList(); }
	TxtMiru::LineList &GetLineList() { return m_txtBuffer.GetLineList(); }
	TxtMiru::TextStyleMap &GetTextStyleMap() { return m_txtBuffer.GetTextStyleMap(); }
	const TxtMiru::TextOffsetMap &GetConstTextOffsetMap() const;
	const TxtMiru::PictPosMap    &GetConstPictPosMap   () const;
	TxtMiru::TextOffsetMap &GetTextOffsetMap();
	TxtMiru::PictPosMap    &GetPictPosMap   ();

	void UpdateLayout();
	void CalcTotalPage();
	int GetTotalPage() const;
	int LineToPage(int line/*論理行*/) const;
	int TextPointToPage(const TxtMiru::TextPoint &pos) const;

	bool Save(LPCTSTR lpFileName = nullptr);

	const bool IsBold(const TxtMiru::TextPoint &tp) const;
	void ToString(std::tstring &out_str, const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, int max_length=0) const;
	void ToStringRuby(std::tstring &out_str, const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, int max_length=0) const;
	bool StartPageToTextPoint(TxtMiru::TextPoint &tp, int start_page) const;
	bool TextPointNextN(TxtMiru::TextPoint &out_pos, const TxtMiru::TextPoint &pos, int n) const;
	// 栞追加
	void AddBookmark(int page);
	void SetLastPage(int page){ m_bookMark.SetLastPage(page); }
	int GetBookmarkCount(){ return m_bookMark.Count(); } 
	int GetLastPage(){ return m_bookMark.GetLastPage(); }
	int GetBookmarkPage(int idx);
	const std::tstring &GetBookmarkName(int idx) const { return m_bookMark.GetName(idx); }
	void DeleteBookmark(int idx) { m_bookMark.Delete(idx); }
	bool SaveBookmark(LPCTSTR lpFileName);
	//
	bool ShowBookmarkList(HWND hWnd, bool bShow = true) { return m_bookMark.ShowBookmarkList(hWnd, bShow); }
	//
	bool Search(LPCTSTR lpSrc, TxtMiru::TextPoint &pos, bool bLoop, bool bUseRegExp, bool bDown);
	bool SearchNext(TxtMiru::TextPoint &pos);
	bool SearchPrev(TxtMiru::TextPoint &pos);
	int SearchStringLength() const { return m_search.StringLength(); }
	//
	const std::tstring &GetTitle() const;
	const std::tstring &GetAuthor() const;
	const std::tstring &GetDocInfo() const;
	const void GetFileName(std::tstring &str) const;
	const void GetRealFileName(std::tstring &str) const;
	const void GetCurDir(std::tstring &dir) const;
	//
	CGrTxtSubtitle &GetSubtitle(){ return m_subTitle; }
	CGrTxtBookmark &GetBookmark(){ return m_bookMark; }
	int GetPageByLinkNo(int iLinkNo) const;
	LPCTSTR GetLinkFileNameByOrder(int order) const;
	//
	int ForwardPage();
	int BackPage();
	//
	const CGrCustomTxtLayout &GetConstLayout() const { return m_layout; }
	void SetLayout(CGrTxtLayout *playout);
	int ConfigurationDlg(HWND hWnd);
	void ShowPropertyDialog(HWND hWnd) const;
	//
	void SetMainWnd(HWND hWnd);
	HWND GetMainWnd();
	HANDLE OpenThread(HWND hWnd, LPCTSTR lpFilename, CGrDocOpenProcCallbak callback = nullptr);
public:
	static unsigned __stdcall OpenProc(void *lpParam);
private:
	void clear();
	bool openFile(LPCTSTR lpFileName);
private:
	std::stack<TxtMiru::TextPoint> m_pageForwardHistory;
	std::stack<TxtMiru::TextPoint> m_pageBackHistory;
	std::tstring m_fileName;
	std::tstring m_arcFileName;
	SIZE m_pageSize = {};
	int m_maxPage = 0;
	CGrTxtBuffer m_txtBuffer;
	CGrTxtBookmark m_bookMark;
	CGrTxtSearch   m_search;
	CGrTxtSubtitle m_subTitle;
	bool m_bOpenBookmark = false;
	bool m_bOpenFile = false;
	HWND m_hMainWnd = NULL;

	CGrCustomTxtLayout m_layout;
};

#endif // __TXTDOCUMENT_H__
