#ifndef __TXTLAYOUT_H__
#define __TXTLAYOUT_H__

#include "TxtMiruDef.h"
#include "CSVText.h"

class CGrTxtDocument;
class CGrTxtLayout
{
public:
	CGrTxtLayout();
	virtual ~CGrTxtLayout();
	virtual bool Open(LPCTSTR lpFileName);
	virtual bool Save(LPCTSTR lpFileName = NULL);
	virtual bool Reload();
	virtual void GetFileName(std::tstring &filename) const { filename = m_fileName; }
	virtual void SetInitialize() = 0;
	//
	virtual int GetMaxLine() const { return m_szPageCount.cx; }
	virtual int GetMaxCharacters() const { return m_szPageCount.cy; }
	virtual SIZE GetPaperSize() const { return m_szPaper; }
	virtual void Clear();
	//
	virtual bool IsSupported(LPCTSTR lpFileName) const;
	virtual bool IsSupported(CGrCSVText &csv) const;
	virtual void operator=(const CGrTxtLayout &layout);
	//
	virtual void SetMaxLine(int l){ m_szPageCount.cx = l; }
	virtual void SetMaxCharacters(int l){ m_szPageCount.cy = l; }
	virtual void SetPaper(const SIZE &size){ m_szPaper = size; }
	//
	virtual void print(FILE *fp) const;
	//
	virtual LPCTSTR LayoutType() const = NULL;
	void GetLayoutName(std::tstring &name) const { name = m_layoutName; }
	void SetLayoutName(LPCTSTR name){ m_layoutName = name; }
	//
	virtual int ConfigurationDlg(HWND hWnd, CGrTxtDocument &doc) = 0;
	//
	enum FormatType {
		FT_Nombre1 = 0,
		FT_Nombre2    ,
		FT_MaxNum     ,
	};
	const std::tstring &GetFormat(enum FormatType ft) const { return m_format[ft]; }
	enum LineSizeType {
		LST_Text = 0    ,
		LST_Ruby        ,
		LST_Nombre      ,
		LST_Note        ,
		LST_RunningHeads,
		LST_MaxNum      ,
	};
	virtual void SetLineSize(LineSizeType lst, const TxtMiru::LineSize &ls){ m_ls[lst] = ls;}
	virtual TxtMiru::LineSize GetLineSize(LineSizeType ls) const { return m_ls[ls]; }
	enum LayoutListType {
		LLT_Text = 0    ,
		LLT_Note        ,
		LLT_Nombre      ,
		LLT_RunningHeads,
		LLT_MaxNum      ,
	};
	virtual TxtMiru::TxtLayoutList &GetLayoutList(LayoutListType llt){ return m_ll[llt]; }
	virtual const TxtMiru::TxtLayoutList &GetConstLayoutList(LayoutListType llt) const { return m_ll[llt]; }
	virtual void AddLayout(LayoutListType llt, const TxtMiru::TxtLayout &layout){ m_ll[llt].push_back(layout); }
	//
	enum NombreFormatType {
		NFT_outside, NFT_center, NFT_inside
	};
	void SetNombreFormatType(NombreFormatType lft);
	NombreFormatType GetNombreFormatType() const { return m_nombreFormatType; }
	LPCTSTR OpenLayoutType() const { return m_openLayoutType.c_str(); }
protected:
	virtual void getItem(const CSV_COLMN &csv_col);
	virtual void setItem(CGrCSVText &csv);
	TxtMiru::TxtLayoutList m_ll    [LLT_MaxNum] = {};
	TxtMiru::LineSize      m_ls    [LST_MaxNum] = {};
	std::tstring           m_format[FT_MaxNum ];

	NombreFormatType m_nombreFormatType;
	SIZE m_szPaper = {};
	SIZE m_szPageCount = {};
	std::tstring m_fileName      ;
	std::tstring m_layoutName    ;
	std::tstring m_openLayoutType;
};

class CGrCustomTxtLayout : public CGrTxtLayout
{
public:
	CGrCustomTxtLayout();
	virtual ~CGrCustomTxtLayout();
	virtual LPCTSTR LayoutType() const;
	virtual int ConfigurationDlg(HWND hWnd, CGrTxtDocument &doc);
	virtual void SetInitialize();
	virtual bool IsSupported(LPCTSTR lpFileName) const;
	virtual bool IsSupported(CGrCSVText &csv) const;
};

#endif // __TXTLAYOUT_H__
