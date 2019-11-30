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
	virtual LPCTSTR LayoutType() const = 0;
	void GetLayoutName(std::tstring &name) const { name = m_layoutName; }
	void SetLayoutName(LPCTSTR name){ m_layoutName = name; }
	enum class OrientationType {
		Vertical = 0,
		Horizontal  ,
		MaxNum      ,
	};
	virtual OrientationType GetOrientation() const { return m_orientationType; }
	//
	virtual int ConfigurationDlg(HWND hWnd, CGrTxtDocument &doc) = 0;
	//
	enum class FormatType {
		Nombre1 = 0,
		Nombre2    ,
		MaxNum     ,
	};
	const std::tstring &GetFormat(FormatType ft) const { return m_format[static_cast<unsigned int>(ft)]; }
	enum class LineSizeType {
		Text = 0    ,
		Ruby        ,
		Nombre      ,
		Note        ,
		RunningHeads,
		MaxNum      ,
	};
	virtual void SetLineSize(LineSizeType lst, const TxtMiru::LineSize &ls){ m_ls[static_cast<unsigned int>(lst)] = ls;}
	virtual TxtMiru::LineSize GetLineSize(LineSizeType ls) const { return m_ls[static_cast<unsigned int>(ls)]; }
	enum class LayoutListType {
		Text = 0    ,
		Note        ,
		Nombre      ,
		RunningHeads,
		MaxNum      ,
	};
	virtual TxtMiru::TxtLayoutList &GetLayoutList(LayoutListType llt){ return m_ll[static_cast<unsigned int>(llt)]; }
	virtual const TxtMiru::TxtLayoutList &GetConstLayoutList(LayoutListType llt) const { return m_ll[static_cast<unsigned int>(llt)]; }
	virtual void AddLayout(LayoutListType llt, const TxtMiru::TxtLayout &layout){ m_ll[static_cast<unsigned int>(llt)].push_back(layout); }
	//
	enum class NombreFormatType {
		outside, center, inside
	};
	void SetNombreFormatType(NombreFormatType lft);
	NombreFormatType GetNombreFormatType() const { return m_nombreFormatType; }
	LPCTSTR OpenLayoutType() const { return m_openLayoutType.c_str(); }
protected:
	virtual void getItem(const CSV_COLMN &csv_col);
	virtual void setItem(CGrCSVText &csv);
	TxtMiru::TxtLayoutList m_ll    [static_cast<unsigned int>(LayoutListType::MaxNum)] = {};
	TxtMiru::LineSize      m_ls    [static_cast<unsigned int>(LineSizeType::MaxNum)] = {};
	std::tstring           m_format[static_cast<unsigned int>(FormatType::MaxNum)];

	NombreFormatType m_nombreFormatType;
	SIZE m_szPaper = {};
	SIZE m_szPageCount = {};
	std::tstring m_fileName      ;
	std::tstring m_layoutName    ;
	std::tstring m_openLayoutType;
	OrientationType m_orientationType = OrientationType::Vertical;
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
