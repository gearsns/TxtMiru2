#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <numeric>
#include "resource.h"
#include "TxtLayout.h"
#include "CSVText.h"
#include "Text.h"
#include "Shell.h"
#include "CurrentDirectory.h"
#include "TxtFunc.h"

CGrTxtLayout::CGrTxtLayout()
{
}

CGrTxtLayout::~CGrTxtLayout()
{
}

static void addLayout(const CSV_COLMN &csv_col, TxtMiru::TxtLayoutList &layout)
{
	if(csv_col.size() < 7){
		return;
	}
	TxtMiru::TxtLayout tl;
	tl.left       =     CGrText::toInt(csv_col[1])    ;
	tl.top        =     CGrText::toInt(csv_col[2])    ;
	tl.right      =     CGrText::toInt(csv_col[3])    ;
	tl.bottom     =     CGrText::toInt(csv_col[4])    ;
	tl.lines      = max(CGrText::toInt(csv_col[5]), 2); /* パーサー上 1以下の場合、無限ループになるので */
	tl.characters =     CGrText::toInt(csv_col[6])    ;
	layout.push_back(tl);
}

static void addLineSize(const CSV_COLMN &csv_col, TxtMiru::LineSize &ls)
{
	if(csv_col.size() < 3){
		return;
	}
	ls.width = CGrText::toInt(csv_col[1]);
	ls.space = CGrText::toInt(csv_col[2]);
}

static void addSize(const CSV_COLMN &csv_col, SIZE &size)
{
	if(csv_col.size() < 3){
		return;
	}
	size.cx = CGrText::toInt(csv_col[1]);
	size.cy = CGrText::toInt(csv_col[2]);
}

bool CGrTxtLayout::IsSupported(LPCTSTR lpFileName) const
{
	CGrCSVText csv;

	if(!csv.Open(lpFileName)){
		return false;
	}
	return IsSupported(csv);
}

bool CGrTxtLayout::IsSupported(CGrCSVText &csv) const
{
	const auto &csv_row = csv.GetCSVROW();
	int len = csv_row.size();
	if(len > 0){
		const auto *it = &csv_row[0];
		for(; len>0; --len, ++it){
			const auto &csv_col = (*it);
			int num = csv_col.size();
			if(num != 2){
				continue;
			}
			auto lpSrc = csv_col[0].c_str();
			if(CGrText::isMatchChar(csv_col[0].c_str(), _T("LayoutType"))){
				return CGrText::isMatchChar(csv_col[1].c_str(), LayoutType());
			}
		}
	}
	return false;
}

void CGrTxtLayout::getItem(const CSV_COLMN &csv_col)
{
	auto lpSrc = csv_col[0].c_str();
	/*   */if(CGrText::isMatchChar(lpSrc, _T("PaperSize"         ))){ addSize(csv_col, m_szPaper    );
	} else if(CGrText::isMatchChar(lpSrc, _T("PageCharCount"     ))){ addSize(csv_col, m_szPageCount);
	} else if(CGrText::isMatchChar(lpSrc, _T("TextSize"          ))){ addLineSize(csv_col, m_ls[static_cast<int>(LineSizeType::Text        )]);
	} else if(CGrText::isMatchChar(lpSrc, _T("RubySize"          ))){ addLineSize(csv_col, m_ls[static_cast<int>(LineSizeType::Ruby        )]);
	} else if(CGrText::isMatchChar(lpSrc, _T("NombreSize"        ))){ addLineSize(csv_col, m_ls[static_cast<int>(LineSizeType::Nombre      )]);
	} else if(CGrText::isMatchChar(lpSrc, _T("NoteSize"          ))){ addLineSize(csv_col, m_ls[static_cast<int>(LineSizeType::Note        )]);
	} else if(CGrText::isMatchChar(lpSrc, _T("RunningHeadsSize"  ))){ addLineSize(csv_col, m_ls[static_cast<int>(LineSizeType::RunningHeads)]);
	} else if(CGrText::isMatchChar(lpSrc, _T("TextLayout"        ))){ addLayout(csv_col, m_ll[static_cast<int>(LayoutListType::Text        )]);
	} else if(CGrText::isMatchChar(lpSrc, _T("NombreLayout"      ))){ addLayout(csv_col, m_ll[static_cast<int>(LayoutListType::Nombre      )]);
	} else if(CGrText::isMatchChar(lpSrc, _T("RunningHeadsLayout"))){ addLayout(csv_col, m_ll[static_cast<int>(LayoutListType::RunningHeads)]);
	} else if(CGrText::isMatchChar(lpSrc, _T("NoteLayout"        ))){ addLayout(csv_col, m_ll[static_cast<int>(LayoutListType::Note        )]);
	} else if(csv_col.size() >= 2){
		/*   */if(CGrText::isMatchChar(lpSrc, _T("LayoutType"        ))){ m_openLayoutType = csv_col[1];
		} else if(CGrText::isMatchChar(lpSrc, _T("LayoutName"        ))){ m_layoutName     = csv_col[1];
		} else if(CGrText::isMatchChar(lpSrc, _T("Nombre1Format"     ))){ m_format[static_cast<unsigned int>(FormatType::Nombre1)] = csv_col[1];
		} else if(CGrText::isMatchChar(lpSrc, _T("Nombre2Format"     ))){ m_format[static_cast<unsigned int>(FormatType::Nombre2)] = csv_col[1];
		} else if(CGrText::isMatchChar(lpSrc, _T("NumbreFormatType"  ))){ m_nombreFormatType = static_cast<NombreFormatType>(CGrText::toInt(csv_col[1]));
		}
	}
}

bool CGrTxtLayout::Open(LPCTSTR lpFileName)
{
	CGrCSVText csv;

	CGrCurrentDirectory cd;
	CGrTxtFunc::MoveDataDir(); // Application Dataにカレントを移す
	std::tstring filename;
	if(lpFileName){
		filename = lpFileName;
	} else{
		filename = _T("Layout/Layout.lay");
	}
	if(!csv.Open(filename.c_str())){
		return false;
	}
	// Versionチェック
	if(!IsSupported(csv)){
		return false;
	}
	//
	m_fileName = filename;
	Clear();
	m_openLayoutType = LayoutType();
	m_format[static_cast<int>(FormatType::Nombre1)] = _T("%1!d!");
	m_format[static_cast<int>(FormatType::Nombre2)] = _T("%1!d!");

	const auto &csv_row = csv.GetCSVROW();
	int len = csv_row.size();
	if(len > 0){
		const auto *it = &csv_row[0];
		for(; len>0; --len, ++it){
			const auto &csv_col = (*it);
			int num = csv_col.size();
			if(num <= 1){
				continue;
			}
			getItem(csv_col);
		}
	}
	struct counter { int operator()(int lines, const TxtMiru::TxtLayout &l){ return lines+l.lines; } };
	m_szPageCount.cx = std::accumulate(m_ll[static_cast<int>(LayoutListType::Text)].begin(), m_ll[static_cast<int>(LayoutListType::Text)].end(), 0, counter());

	if(m_szPageCount.cx <= 0){
		SetInitialize();
		return false;
	}

	return true;
}

bool CGrTxtLayout::Reload()
{
	if(m_fileName.empty()){
		SetInitialize();
		return true;
	} else {
		auto filename = m_fileName;
		return Open(filename.c_str());
	}
}

void CGrTxtLayout::setItem(CGrCSVText &csv)
{
	csv.AddFormatTail(_T("ss" ), _T("LayoutType"      ), LayoutType()                );
	csv.AddFormatTail(_T("ss" ), _T("LayoutName"      ), m_layoutName.c_str()        );
	csv.AddFormatTail(_T("sdd"), _T("PaperSize"       ), m_szPaper                   );
	csv.AddFormatTail(_T("sdd"), _T("PageCharCount"   ), m_szPageCount               );
	csv.AddFormatTail(_T("sdd"), _T("TextSize"        ), m_ls[static_cast<int>(LineSizeType::Text        )]      );
	csv.AddFormatTail(_T("sdd"), _T("RubySize"        ), m_ls[static_cast<int>(LineSizeType::Ruby        )]      );
	csv.AddFormatTail(_T("sdd"), _T("NombreSize"      ), m_ls[static_cast<int>(LineSizeType::Nombre      )]      );
	csv.AddFormatTail(_T("sdd"), _T("NoteSize"        ), m_ls[static_cast<int>(LineSizeType::Note        )]      );
	csv.AddFormatTail(_T("sdd"), _T("RunningHeadsSize"), m_ls[static_cast<int>(LineSizeType::RunningHeads)]      );
	csv.AddFormatTail(_T("ss") , _T("Nombre1Format"   ), m_format[static_cast<int>(FormatType::Nombre1)].c_str());
	csv.AddFormatTail(_T("ss") , _T("Nombre2Format"   ), m_format[static_cast<int>(FormatType::Nombre2)].c_str());
	csv.AddFormatTail(_T("sd") , _T("NumbreFormatType"), m_nombreFormatType          );
	struct LayoutListConvert
	{
		CGrCSVText &csv;
		LayoutListConvert(CGrCSVText &c) : csv(c){}
		void AddTail(LPCTSTR name, const TxtMiru::TxtLayoutList &tll){
			int len = tll.size();
			if(len > 0){
				const auto *it=&tll[0];
				for(; len>0; --len, ++it){
					const auto &tl = (*it);
					csv.AddFormatTail(_T("sdddddd"), name, tl);
				}
			}
		}
	} llc(csv);
	llc.AddTail(_T("TextLayout"        ), m_ll[static_cast<int>(LayoutListType::Text        )]);
	llc.AddTail(_T("NombreLayout"      ), m_ll[static_cast<int>(LayoutListType::Nombre      )]);
	llc.AddTail(_T("RunningHeadsLayout"), m_ll[static_cast<int>(LayoutListType::RunningHeads)]);
	llc.AddTail(_T("NoteLayout"        ), m_ll[static_cast<int>(LayoutListType::Note        )]);
}

bool CGrTxtLayout::Save(LPCTSTR lpFileName)
{
	CGrCurrentDirectory cd;
	CGrTxtFunc::MoveDataDir(); // Application Dataにカレントを移す
	if(lpFileName == nullptr){
		if(m_fileName.size() < 0){
			return false;
		}
		lpFileName = m_fileName.c_str();
	}
	CGrCSVText csv;
	setItem(csv);

	return csv.Save(lpFileName) == TRUE;
}

void CGrTxtLayout::Clear()
{
	int idx;
	for(idx=0; idx<sizeof(m_ll)/sizeof(TxtMiru::TxtLayoutList); ++idx){
		m_ll[idx].clear();
	}
}

void CGrTxtLayout::SetNombreFormatType(NombreFormatType lft)
{
	switch(lft){
	case NombreFormatType::center :
	case NombreFormatType::inside :
	case NombreFormatType::outside:
		m_nombreFormatType = lft;
		break;
	default:
		m_nombreFormatType = NombreFormatType::outside;
		break;
	}
}

void CGrTxtLayout::operator=(const CGrTxtLayout &layout)
{
	int idx;
	for(idx=0; idx<sizeof(m_ll)/sizeof(TxtMiru::TxtLayoutList); ++idx){
		m_ll[idx] = layout.m_ll[idx];
	}
	CopyMemory(&m_ls, &layout.m_ls, sizeof(m_ls));
	m_nombreFormatType = layout.m_nombreFormatType;

	m_szPaper     = layout.m_szPaper    ;
	m_szPageCount = layout.m_szPageCount;
}

void _ftprintLayout(FILE *fp, LPCTSTR titie, TxtMiru::TxtLayoutList::const_iterator it, TxtMiru::TxtLayoutList::const_iterator e)
{
}

void CGrTxtLayout::print(FILE *fp) const
{
}

////////////////////////
CGrCustomTxtLayout::CGrCustomTxtLayout()
{
	SetInitialize();
}

CGrCustomTxtLayout::~CGrCustomTxtLayout()
{
}
void CGrCustomTxtLayout::SetInitialize()
{
	Clear();
	if(m_szPaper.cx == 20600 && m_szPaper.cy == 18200){
		m_nombreFormatType = NombreFormatType::outside;
		m_szPaper    .cx = 20600; m_szPaper    .cy = 18200;
		m_szPageCount.cx =    68; m_szPageCount.cy =    20;
		m_ls[static_cast<int>(LineSizeType::Text        )].width = 310; m_ls[static_cast<int>(LineSizeType::Text        )].space = 20;
		m_ls[static_cast<int>(LineSizeType::Ruby        )].width = 160; m_ls[static_cast<int>(LineSizeType::Ruby        )].space = 20;
		m_ls[static_cast<int>(LineSizeType::Nombre      )].width = 300; m_ls[static_cast<int>(LineSizeType::Nombre      )].space =  0;
		m_ls[static_cast<int>(LineSizeType::Note        )].width = 200; m_ls[static_cast<int>(LineSizeType::Note        )].space = 20;
		m_ls[static_cast<int>(LineSizeType::RunningHeads)].width = 300; m_ls[static_cast<int>(LineSizeType::RunningHeads)].space =  0;
		m_format[static_cast<int>(FormatType::Nombre1)] = _T("%1!d!");
		m_format[static_cast<int>(FormatType::Nombre2)] = _T("%1!d!");
		TxtMiru::TxtLayout tl;
		/**/tl.left = 11100;   tl.top = 1500;  tl.right = 19600;   tl.bottom =  9100;    tl.lines = 17;    tl.characters = 20;   m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
		/*  tl.left = 11100;*/ tl.top = 9600;/*tl.right = 19600;*/ tl.bottom = 17200;  /*tl.lines = 17;*//*tl.characters = 20;*/ m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
		/**/tl.left =  1000;   tl.top = 1500;  tl.right =  9500;   tl.bottom =  9100;  /*tl.lines = 17;*//*tl.characters = 20;*/ m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
		/*  tl.left =  1000;*/ tl.top = 9600;/*tl.right =  9500;*/ tl.bottom = 17200;  /*tl.lines = 17;*//*tl.characters = 20;*/ m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
		/*  tl.left =  1000;*/ tl.top =  300;  tl.right =  1900;   tl.bottom =   600;  /*tl.lines = 17;*/  tl.characters = 40;   m_ll[static_cast<int>(LayoutListType::Nombre      )].push_back(tl);
		/**/tl.left = 18900; /*tl.top =  300;*/tl.right = 19800; /*tl.bottom =   600;*//*tl.lines = 17;*//*tl.characters = 40;*/ m_ll[static_cast<int>(LayoutListType::Nombre      )].push_back(tl);
		/**/tl.left =  2500; /*tl.top =  300;*/tl.right = 10300; /*tl.bottom =   600;*//*tl.lines = 17;*//*tl.characters = 40;*/ m_ll[static_cast<int>(LayoutListType::RunningHeads)].push_back(tl);
		/**/tl.left =     0;   tl.top = 1500;  tl.right =  1000;   tl.bottom = 17200;    tl.lines =  4;    tl.characters = 64;   m_ll[static_cast<int>(LayoutListType::Note        )].push_back(tl);
	} else {
		m_nombreFormatType = NombreFormatType::outside;
		m_szPaper    .cx = 21000; m_szPaper    .cy = 14800;
		m_szPageCount.cx =    34; m_szPageCount.cy =    40;
		m_ls[static_cast<int>(LineSizeType::Text        )].width = 310; m_ls[static_cast<int>(LineSizeType::Text        )].space = 20;
		m_ls[static_cast<int>(LineSizeType::Ruby        )].width = 160; m_ls[static_cast<int>(LineSizeType::Ruby        )].space = 20;
		m_ls[static_cast<int>(LineSizeType::Nombre      )].width = 300; m_ls[static_cast<int>(LineSizeType::Nombre      )].space =  0;
		m_ls[static_cast<int>(LineSizeType::Note        )].width = 200; m_ls[static_cast<int>(LineSizeType::Note        )].space = 20;
		m_ls[static_cast<int>(LineSizeType::RunningHeads)].width = 300; m_ls[static_cast<int>(LineSizeType::RunningHeads)].space =  0;
		m_format[static_cast<int>(FormatType::Nombre1)] = _T("%1!d!");
		m_format[static_cast<int>(FormatType::Nombre2)] = _T("%1!d!");
		TxtMiru::TxtLayout tl;
		tl.left = 11500; tl.top = 1000; tl.right = 20000; tl.bottom = 13800; tl.lines = 17; tl.characters = 40; m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
		tl.left =  1000; tl.top = 1000; tl.right =  9500; tl.bottom = 13800; tl.lines = 17; tl.characters = 40; m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
		tl.left =  1000; tl.top =  300; tl.right =  1900; tl.bottom =   600; tl.lines = 17; tl.characters = 40; m_ll[static_cast<int>(LayoutListType::Nombre      )].push_back(tl);
		tl.left = 19242; tl.top =  300; tl.right = 20142; tl.bottom =   600; tl.lines = 17; tl.characters = 40; m_ll[static_cast<int>(LayoutListType::Nombre      )].push_back(tl);
		tl.left =  2500; tl.top =  300; tl.right = 10500; tl.bottom =   600; tl.lines = 17; tl.characters = 40; m_ll[static_cast<int>(LayoutListType::RunningHeads)].push_back(tl);
		tl.left =     0; tl.top = 1000; tl.right =  1000; tl.bottom = 13800; tl.lines =  4; tl.characters = 64; m_ll[static_cast<int>(LayoutListType::Note        )].push_back(tl);
	}
}
bool CGrCustomTxtLayout::IsSupported(LPCTSTR lpFileName) const { return true; }
bool CGrCustomTxtLayout::IsSupported(CGrCSVText &csv) const { return true; };

LPCTSTR CGrCustomTxtLayout::LayoutType() const
{
	if(m_openLayoutType.empty()){
		return _T("Custom 1.0");
	}
	return m_openLayoutType.c_str();
}

void CGrCustomTxtLayout::SetFileName(LPCTSTR lpFileName)
{
	m_fileName = lpFileName;
}
void CGrCustomTxtLayout::SetOpenLayoutType(LPCTSTR lpType)
{
	m_openLayoutType = lpType;
}

LPCTSTR CGrBunkoTxtLayout::Name(){ return _T("BUNKO 1.0"); }
////////////////////////////////////////////////////
CGrBunkoTxtLayout::CGrBunkoTxtLayout()
{
	SetInitialize();
}

CGrBunkoTxtLayout::~CGrBunkoTxtLayout()
{
}

void CGrBunkoTxtLayout::SetInitialize()
{
	Clear();
	m_nombreFormatType = NombreFormatType::outside;
	m_szPaper    .cx = 21000; m_szPaper    .cy = 14800;
	m_szPageCount.cx =    34; m_szPageCount.cy =    40;
	m_ls[static_cast<int>(LineSizeType::Text        )].width = 310; m_ls[static_cast<int>(LineSizeType::Text        )].space = 20;
	m_ls[static_cast<int>(LineSizeType::Ruby        )].width = 160; m_ls[static_cast<int>(LineSizeType::Ruby        )].space = 20;
	m_ls[static_cast<int>(LineSizeType::Nombre      )].width = 300; m_ls[static_cast<int>(LineSizeType::Nombre      )].space =  0;
	m_ls[static_cast<int>(LineSizeType::Note        )].width = 200; m_ls[static_cast<int>(LineSizeType::Note        )].space = 20;
	m_ls[static_cast<int>(LineSizeType::RunningHeads)].width = 300; m_ls[static_cast<int>(LineSizeType::RunningHeads)].space =  0;
	m_format[static_cast<int>(FormatType::Nombre1)] = _T("%1!d!");
	m_format[static_cast<int>(FormatType::Nombre2)] = _T("%1!d!");
	TxtMiru::TxtLayout tl;
	/**/tl.left = 11500;    tl.top = 1000;   tl.right = 20000;  tl.bottom = 13800;    tl.lines = 17;    tl.characters = 40;   m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
	/**/tl.left =  1000;  /*tl.top = 1000;*/ tl.right =  9500;/*tl.bottom = 13800;*//*tl.lines = 17;*//*tl.characters = 40;*/ m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
	/*  tl.left =  1000;*/  tl.top =  300;   tl.right =  1900;  tl.bottom =   600;  /*tl.lines = 17;*//*tl.characters = 40;*/ m_ll[static_cast<int>(LayoutListType::Nombre      )].push_back(tl);
	/**/tl.left = 19242;  /*tl.top =  300;*/ tl.right = 20142;/*tl.bottom =   600;*//*tl.lines = 17;*//*tl.characters = 40;*/ m_ll[static_cast<int>(LayoutListType::Nombre      )].push_back(tl);
	/**/tl.left =  2500;  /*tl.top =  300;*/ tl.right = 10500;/*tl.bottom =   600;*//*tl.lines = 17;*//*tl.characters = 40;*/ m_ll[static_cast<int>(LayoutListType::RunningHeads)].push_back(tl);
	/**/tl.left =     0;    tl.top = 1000;   tl.right =  1000;  tl.bottom = 13800;    tl.lines =  4;    tl.characters = 64;   m_ll[static_cast<int>(LayoutListType::Note        )].push_back(tl);
}

LPCTSTR CGrShinShoTate2TxtLayout::Name(){ return _T("SHINSHOTATE2 1.0"); }
////////////////////////////////////////////////////
CGrShinShoTate2TxtLayout::CGrShinShoTate2TxtLayout()
{
	SetInitialize();
}

CGrShinShoTate2TxtLayout::~CGrShinShoTate2TxtLayout()
{
}

void CGrShinShoTate2TxtLayout::SetInitialize()
{
	Clear();
	m_nombreFormatType = NombreFormatType::outside;
	m_szPaper    .cx = 20600; m_szPaper    .cy = 18200;
	m_szPageCount.cx =    68; m_szPageCount.cy =    20;
	m_ls[static_cast<int>(LineSizeType::Text        )].width = 310; m_ls[static_cast<int>(LineSizeType::Text        )].space = 20;
	m_ls[static_cast<int>(LineSizeType::Ruby        )].width = 160; m_ls[static_cast<int>(LineSizeType::Ruby        )].space = 20;
	m_ls[static_cast<int>(LineSizeType::Nombre      )].width = 300; m_ls[static_cast<int>(LineSizeType::Nombre      )].space =  0;
	m_ls[static_cast<int>(LineSizeType::Note        )].width = 200; m_ls[static_cast<int>(LineSizeType::Note        )].space = 20;
	m_ls[static_cast<int>(LineSizeType::RunningHeads)].width = 300; m_ls[static_cast<int>(LineSizeType::RunningHeads)].space =  0;
	m_format[static_cast<int>(FormatType::Nombre1)] = _T("%1!d!");
	m_format[static_cast<int>(FormatType::Nombre2)] = _T("%1!d!");
	TxtMiru::TxtLayout tl;
	/**/tl.left = 11100;   tl.top = 1500;  tl.right = 19600;   tl.bottom =  9100;    tl.lines = 17;    tl.characters = 20;   m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
	/*  tl.left = 11100;*/ tl.top = 9600;/*tl.right = 19600;*/ tl.bottom = 17200;  /*tl.lines = 17;*//*tl.characters = 20;*/ m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
	/**/tl.left =  1000;   tl.top = 1500;  tl.right =  9500;   tl.bottom =  9100;  /*tl.lines = 17;*//*tl.characters = 20;*/ m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
	/*  tl.left =  1000;*/ tl.top = 9600;/*tl.right =  9500;*/ tl.bottom = 17200;  /*tl.lines = 17;*//*tl.characters = 20;*/ m_ll[static_cast<int>(LayoutListType::Text        )].push_back(tl);
	/*  tl.left =  1000;*/ tl.top =  300;  tl.right =  1900;   tl.bottom =   600;  /*tl.lines = 17;*/  tl.characters = 40;   m_ll[static_cast<int>(LayoutListType::Nombre      )].push_back(tl);
	/**/tl.left = 18900; /*tl.top =  300;*/tl.right = 19800; /*tl.bottom =   600;*//*tl.lines = 17;*//*tl.characters = 40;*/ m_ll[static_cast<int>(LayoutListType::Nombre      )].push_back(tl);
	/**/tl.left =  2500; /*tl.top =  300;*/tl.right = 10300; /*tl.bottom =   600;*//*tl.lines = 17;*//*tl.characters = 40;*/ m_ll[static_cast<int>(LayoutListType::RunningHeads)].push_back(tl);
	/**/tl.left =     0;   tl.top = 1500;  tl.right =  1000;   tl.bottom = 17200;    tl.lines =  4;    tl.characters = 64;   m_ll[static_cast<int>(LayoutListType::Note        )].push_back(tl);
}
