#ifndef __TXTFUNCIBOOKMARK_H__
#define __TXTFUNCIBOOKMARK_H__

class cdecl CGrTxtFuncISubTitleLeaf
{
public:
	CGrTxtFuncISubTitleLeaf(){};
	virtual ~CGrTxtFuncISubTitleLeaf(){};

	virtual int GetPage() const = 0;
	virtual int GetLine() const = 0;
	virtual int GetIndex() const = 0;
	virtual int GetPos() const = 0;
	virtual int GetOrder() const = 0;
	virtual int GetLevel() const = 0;
	virtual LPCTSTR GetName() const = 0;
	virtual LPCTSTR GetFileName() const = 0;
};
class cdecl CGrTxtFuncISubTitle
{
public:
	CGrTxtFuncISubTitle(){};
	virtual ~CGrTxtFuncISubTitle(){};
	//
	virtual int Count() const = 0;
	virtual CGrTxtFuncISubTitleLeaf* GetLeaf(int idx) = 0;
};
class cdecl CGrTxtFuncIBookmarkLeaf
{
public:
	CGrTxtFuncIBookmarkLeaf(){};
	virtual ~CGrTxtFuncIBookmarkLeaf(){};

	virtual int GetPage() const = 0;
	virtual int GetLine() const = 0;
	virtual int GetIndex() const = 0;
	virtual int GetPos() const = 0;
	virtual LPCTSTR GetName() const = 0;
};
class cdecl CGrTxtFuncIBookmark
{
public:
	CGrTxtFuncIBookmark(){};
	virtual ~CGrTxtFuncIBookmark(){};

	virtual void AddPage(int page) = 0;
	//
	virtual void SetLastPage(int page) = 0;
	virtual int GetLastPage() const = 0;
	//
	virtual int Count() const = 0;
	virtual CGrTxtFuncIBookmarkLeaf* GetLeaf(int idx) = 0;
	virtual CGrTxtFuncIBookmarkLeaf* GetLatest() = 0;
	virtual void ClearLeaf() = 0;
	virtual bool AddLeaf(int iPage, int iLine, int iIndex, int iPos, LPCTSTR name) = 0;
	virtual bool SetLatest(int iPage, int iLine, int iIndex, int iPos, LPCTSTR name) = 0;
	//
	virtual void SetParamString(LPCTSTR lpName, LPCTSTR lpValue) = 0;
	virtual void SetParamInteger(LPCTSTR lpName, int iValue) = 0;
	virtual LPCTSTR GetParamString(LPCTSTR lpName) = 0;
	virtual int GetParamInteger(LPCTSTR lpName) = 0;
};
#include "TxtMiruDef.h"
class cdecl CGrTxtFuncIEachAllTool
{
public:
	CGrTxtFuncIEachAllTool(){}
	virtual ~CGrTxtFuncIEachAllTool(){}

	virtual const TxtMiru::TextPoint *CurTextPoint() = 0;
	virtual LPCTSTR String() = 0;
	virtual size_t StringLength() = 0;
	virtual WORD CharType() = 0;
	virtual WORD TextType() = 0;
	virtual TxtMiru::TextListPos BeginTextListPos() = 0;
	virtual TxtMiru::TextListPos EndTextListPos() = 0;
	virtual int Page() = 0;
	virtual LPCTSTR RangeString() = 0;
};

#endif // __TXTFUNCIBOOKMARK_H__
