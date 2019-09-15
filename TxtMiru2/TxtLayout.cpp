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
#include "TxtMiru.h"

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
	layout.push_back({
		/**/CGrText::toInt(csv_col[1])    ,
		/**/CGrText::toInt(csv_col[2])    ,
		/**/CGrText::toInt(csv_col[3])    ,
		/**/CGrText::toInt(csv_col[4])    ,
		max(CGrText::toInt(csv_col[5]), 2), /* パーサー上 1以下の場合、無限ループになるので */
		/**/CGrText::toInt(csv_col[6])
		});
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
	for(const auto &csv_col : csv.GetCSVROW()){
		if(csv_col.size() != 2){
			continue;
		}
		auto lpSrc = csv_col[0].c_str();
		if(CGrText::isMatchChar(csv_col[0].c_str(), _T("LayoutType"))){
			return CGrText::isMatchChar(csv_col[1].c_str(), LayoutType());
		}
	}
	return false;
}

void CGrTxtLayout::getItem(const CSV_COLMN &csv_col)
{
	if(csv_col.size() <= 2){
		return;
	}
	auto lpSrc = csv_col[0].c_str();
	/*   */if(CGrText::isMatchChar(lpSrc, _T("PaperSize"         ))){ addSize(csv_col, m_szPaper    );
	} else if(CGrText::isMatchChar(lpSrc, _T("PageCharCount"     ))){ addSize(csv_col, m_szPageCount);
	} else if(CGrText::isMatchChar(lpSrc, _T("TextSize"          ))){ addLineSize(csv_col, m_ls[LST_Text        ]);
	} else if(CGrText::isMatchChar(lpSrc, _T("RubySize"          ))){ addLineSize(csv_col, m_ls[LST_Ruby        ]);
	} else if(CGrText::isMatchChar(lpSrc, _T("NombreSize"        ))){ addLineSize(csv_col, m_ls[LST_Nombre      ]);
	} else if(CGrText::isMatchChar(lpSrc, _T("NoteSize"          ))){ addLineSize(csv_col, m_ls[LST_Note        ]);
	} else if(CGrText::isMatchChar(lpSrc, _T("RunningHeadsSize"  ))){ addLineSize(csv_col, m_ls[LST_RunningHeads]);
	} else if(CGrText::isMatchChar(lpSrc, _T("TextLayout"        ))){ addLayout(csv_col, m_ll[LLT_Text        ]);
	} else if(CGrText::isMatchChar(lpSrc, _T("NombreLayout"      ))){ addLayout(csv_col, m_ll[LLT_Nombre      ]);
	} else if(CGrText::isMatchChar(lpSrc, _T("RunningHeadsLayout"))){ addLayout(csv_col, m_ll[LLT_RunningHeads]);
	} else if(CGrText::isMatchChar(lpSrc, _T("NoteLayout"        ))){ addLayout(csv_col, m_ll[LLT_Note        ]);
	} else if(CGrText::isMatchChar(lpSrc, _T("LayoutType"        ))){ m_openLayoutType = csv_col[1];
	} else if(CGrText::isMatchChar(lpSrc, _T("LayoutName"        ))){ m_layoutName     = csv_col[1];
	} else if(CGrText::isMatchChar(lpSrc, _T("Nombre1Format"     ))){ m_format[FT_Nombre1] = csv_col[1];
	} else if(CGrText::isMatchChar(lpSrc, _T("Nombre2Format"     ))){ m_format[FT_Nombre2] = csv_col[1];
	} else if(CGrText::isMatchChar(lpSrc, _T("NumbreFormatType"  ))){ m_nombreFormatType = static_cast<NombreFormatType>(CGrText::toInt(csv_col[1]));
	}
}

bool CGrTxtLayout::Open(LPCTSTR lpFileName)
{
	CGrCSVText csv;

	CGrCurrentDirectory cd;
	CGrTxtMiru::MoveDataDir(); // Application Dataにカレントを移す
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
	m_format[FT_Nombre1] = _T("%1!d!");
	m_format[FT_Nombre2] = _T("%1!d!");

	for(const auto &csv_col : csv.GetCSVROW()){
		if(csv_col.size() <= 1){
			continue;
		}
		getItem(csv_col);
	}
	struct counter { int operator()(int lines, const TxtMiru::TxtLayout &l){ return lines+l.lines; } };
	m_szPageCount.cx = std::accumulate(m_ll[LLT_Text].begin(), m_ll[LLT_Text].end(), 0, counter());

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
		std::tstring filename = m_fileName;
		return Open(filename.c_str());
	}
}

void CGrTxtLayout::setItem(CGrCSVText &csv)
{
	csv.AddFormatTail(_T("ss" ), _T("LayoutType"      ), LayoutType()                );
	csv.AddFormatTail(_T("ss" ), _T("LayoutName"      ), m_layoutName.c_str()        );
	csv.AddFormatTail(_T("sdd"), _T("PaperSize"       ), m_szPaper                   );
	csv.AddFormatTail(_T("sdd"), _T("PageCharCount"   ), m_szPageCount               );
	csv.AddFormatTail(_T("sdd"), _T("TextSize"        ), m_ls[LST_Text        ]      );
	csv.AddFormatTail(_T("sdd"), _T("RubySize"        ), m_ls[LST_Ruby        ]      );
	csv.AddFormatTail(_T("sdd"), _T("NombreSize"      ), m_ls[LST_Nombre      ]      );
	csv.AddFormatTail(_T("sdd"), _T("NoteSize"        ), m_ls[LST_Note        ]      );
	csv.AddFormatTail(_T("sdd"), _T("RunningHeadsSize"), m_ls[LST_RunningHeads]      );
	csv.AddFormatTail(_T("ss") , _T("Nombre1Format"   ), m_format[FT_Nombre1].c_str());
	csv.AddFormatTail(_T("ss") , _T("Nombre2Format"   ), m_format[FT_Nombre2].c_str());
	csv.AddFormatTail(_T("sd") , _T("NumbreFormatType"), m_nombreFormatType          );
	struct LayoutListConvert
	{
		CGrCSVText &csv;
		LayoutListConvert(CGrCSVText &c) : csv(c){}
		void AddTail(LPCTSTR name, const TxtMiru::TxtLayoutList &tll){
			for(const auto &tl : tll){
				csv.AddFormatTail(_T("sdddddd"), name, tl.left, tl.top, tl.right, tl.bottom, tl.lines, tl.characters);
			}
		}
	} llc(csv);
	llc.AddTail(_T("TextLayout"        ), m_ll[LLT_Text        ]);
	llc.AddTail(_T("NombreLayout"      ), m_ll[LLT_Nombre      ]);
	llc.AddTail(_T("RunningHeadsLayout"), m_ll[LLT_RunningHeads]);
	llc.AddTail(_T("NoteLayout"        ), m_ll[LLT_Note        ]);
}

bool CGrTxtLayout::Save(LPCTSTR lpFileName)
{
	CGrCurrentDirectory cd;
	CGrTxtMiru::MoveDataDir(); // Application Dataにカレントを移す
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
	for(auto &&item : m_ll){
		item.clear();
		item.shrink_to_fit();
	}
}

void CGrTxtLayout::SetNombreFormatType(NombreFormatType lft)
{
	switch(lft){
	case NFT_center :
	case NFT_inside :
	case NFT_outside:
		m_nombreFormatType = lft;
		break;
	default:
		m_nombreFormatType = NFT_outside;
		break;
	}
}

void CGrTxtLayout::operator=(const CGrTxtLayout &layout)
{
	for(int idx=0; idx<sizeof(m_ll)/sizeof(TxtMiru::TxtLayoutList); ++idx){
		m_ll[idx] = layout.m_ll[idx];
	}
	CopyMemory(&m_ls, &layout.m_ls, sizeof(m_ls));
	m_nombreFormatType = layout.m_nombreFormatType;

	m_szPaper     = layout.m_szPaper    ;
	m_szPageCount = layout.m_szPageCount;
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

int CGrCustomTxtLayout::ConfigurationDlg(HWND hWnd, CGrTxtDocument &doc)
{
	return 0;
}

void CGrCustomTxtLayout::SetInitialize()
{
	Clear();
	m_nombreFormatType = NFT_outside;
	m_szPaper    .cx = 21000; m_szPaper    .cy = 14800;
	m_szPageCount.cx =    34; m_szPageCount.cy =    40;
	m_ls[LST_Text        ].width = 310; m_ls[LST_Text        ].space = 20;
	m_ls[LST_Ruby        ].width = 160; m_ls[LST_Ruby        ].space = 20;
	m_ls[LST_Nombre      ].width = 300; m_ls[LST_Nombre      ].space =  0;
	m_ls[LST_Note        ].width = 200; m_ls[LST_Note        ].space = 20;
	m_ls[LST_RunningHeads].width = 300; m_ls[LST_RunningHeads].space =  0;
	m_format[FT_Nombre1] = _T("%1!d!");
	m_format[FT_Nombre2] = _T("%1!d!");
	m_ll[LLT_Text        ].push_back({11500,1000,20000,13800,17,40});
	m_ll[LLT_Text        ].push_back({ 1000,1000, 9500,13800,17,40});
	m_ll[LLT_Nombre      ].push_back({ 1000, 300, 1900,  600,17,40});
	m_ll[LLT_Nombre      ].push_back({19242, 300,20142,  600,17,40});
	m_ll[LLT_RunningHeads].push_back({ 2500, 300,10500,  600,17,40});
	m_ll[LLT_Note        ].push_back({    0,1000, 1000,13800, 4,64});
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
