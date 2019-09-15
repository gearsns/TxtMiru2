#ifndef __TXTSUBTITLE_H__
#define __TXTSUBTITLE_H__

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "stltchar.h"
#include "TxtMiruDef.h"
#include "LoadDllFunc.h"

#include <vector>

class CGrTxtDocument;
class CGrTxtBuffer;
#include "TxtFuncIBookmark.h"

class CGrTxtSubtitle : public CGrTxtFuncISubTitle
{
public:
	struct Subtitle : public CGrTxtFuncISubTitleLeaf {
	public:
		int iPage = -1;
		TxtMiru::TextPoint tpb;
		TxtMiru::TextPoint tpe;
		std::tstring str;
		std::tstring filename;
		int level = -1;
		int order = 0;
	public:
		Subtitle(){};
		virtual ~Subtitle(){};

		virtual int GetPage() const{ return iPage; };
		virtual int GetLine() const{ return tpb.iLine; };
		virtual int GetIndex() const{ return tpb.iIndex; };
		virtual int GetPos() const{ return tpb.iPos; };
		virtual int GetOrder() const{ return order; };
		virtual int GetLevel() const{ return level; };
		virtual LPCTSTR GetName() const{ return str.c_str(); };
		virtual LPCTSTR GetFileName() const{ return filename.c_str(); };
	};
protected:
	std::vector<Subtitle> m_subtitles;
	const CGrTxtDocument *m_pDoc = nullptr;
public:
	CGrTxtSubtitle();
	virtual ~CGrTxtSubtitle();

	void Attach(const CGrTxtDocument *pDoc);
	void RebuildPageNo();
	int GetPageById(LPCTSTR lpID) const;
	LPCTSTR GetNameByPage(int page) const;
	//
	virtual void ClearLeaf();
	virtual bool AddLeaf(int order, LPCTSTR lpName, LPCTSTR lpFilename);
	bool AddLeaf(CGrTxtBuffer &txtBuffer);
	//
	virtual int Count() const;
	virtual CGrTxtFuncISubTitleLeaf* GetLeaf(int idx);
	const CGrTxtFuncISubTitleLeaf* GetConstLeaf(int idx) const;
};

#endif // __TXTSUBTITLE_H__
