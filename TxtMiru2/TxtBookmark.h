#ifndef __TXTBOOKMARK_H__
#define __TXTBOOKMARK_H__

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "stltchar.h"
#include "TxtMiruDef.h"
#include "LoadDllFunc.h"

#include <vector>

class CGrTxtDocument;
#include "TxtFuncIBookmark.h"
class CGrTxtBookmark : public CGrTxtFuncIBookmark 
{
public:
	struct Bookmark : public CGrTxtFuncIBookmarkLeaf {
		int iPage = 0;
		TxtMiru::TextPoint tp;
		std::tstring str;
		Bookmark(){}
		Bookmark(int l, int i, int p) : tp{l, i, p}{}
		virtual int GetPage() const { return iPage; };
		virtual int GetLine() const { return tp.iLine; };
		virtual int GetIndex() const { return tp.iIndex; };
		virtual int GetPos() const { return tp.iPos; };
		virtual LPCTSTR GetName() const { return str.c_str(); };
	};
protected:
	Bookmark m_lastPage;
	std::vector<Bookmark> m_bookMarks;
	CGrTxtDocument *m_pDoc = nullptr;
	std::tstring m_txtFileName;
	std::tstring m_fileName;
	std::tstring m_lastWriteTime;
	CGrLoadDllFunc *m_pfunc = nullptr;
	int m_iUpdateCheck = 0;

	bool saveAs(LPCTSTR lpFileName);
public:
	CGrTxtBookmark();
	virtual ~CGrTxtBookmark();

	void Attach(CGrTxtDocument *pDoc);
	//
	virtual void AddPage(int page);
	virtual void SetLastPage(int page);
	//
	void RebuildPageNo();
	void Clear();
	void Delete(int idx);
	//
	int GetPage(int idx) const;
	int GetPageById(LPCTSTR lpID) const;
	//
	virtual int Count() const;
	virtual CGrTxtFuncIBookmarkLeaf* GetLeaf(int idx);
	virtual CGrTxtFuncIBookmarkLeaf* GetLatest() ;
	virtual void ClearLeaf();
	virtual bool AddLeaf(int iPage, int iLine, int iIndex, int iPos, LPCTSTR name);
	virtual bool SetLatest(int iPage, int iLine, int iIndex, int iPos, LPCTSTR name);
	int GetLastPage() const;
	const std::tstring &GetName(int idx) const;

	virtual void SetParamString(LPCTSTR lpName, LPCTSTR lpValue);
	virtual void SetParamInteger(LPCTSTR lpName, int iValue);
	virtual LPCTSTR GetParamString(LPCTSTR lpName);
	virtual int GetParamInteger(LPCTSTR lpName);

	void SetTextFileName(const std::tstring &str);
	void GetTextFileName(std::tstring &str) const;
	void SetFileName(const std::tstring &str);
	void GetFileName(std::tstring &str) const;
	void SetLastWriteTime(const std::tstring &str);
	void GetLastWriteTime(std::tstring &str) const;

	bool Open(LPCTSTR lpFileName);
	bool Save();
	bool SaveAs(LPCTSTR lpFileName);

	bool ShowBookmarkList(HWND hWnd, bool bShow = true);

	static LPCTSTR GetExtName();
	void UpdateUI();
	bool isLinkWindowVisible();
	virtual void Unload();
};

#endif // __TXTBOOKMARK_H__
