#pragma warning( disable : 4786 )

#include "TxtParam.h"
#include "Text.h"
#include "Shell.h"
#include "Font.h"
#include "CSVText.h"
#include "stlutil.h"
#include <stdarg.h>
//#define __DBG__
#include "Debug.h"

static bool ConvertToUTF16LE(LPCTSTR filename)
{
	bool ret = false;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hHeap = NULL;
	LPBYTE lpData = nullptr;
	do {
		std::tstring tmp_filename;
		CGrShell::ToPrettyFileName(filename, tmp_filename);
		filename = tmp_filename.c_str();
		DWORD sizeHigh;
		DWORD dw;
		DWORD fsize = 0;
		hFile = ::CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
		if(hFile != INVALID_HANDLE_VALUE){
			fsize = ::GetFileSize(hFile, &sizeHigh);
			if(fsize > 0){
				hHeap = ::HeapCreate(0, 0, 0);
				if (!hHeap) {
					break;
				}
				auto data = static_cast<LPBYTE>(::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BYTE)*(fsize+1)));
				lpData = data;
				if(!data){
					break;
				}
				if(0 == ::ReadFile(hFile, data, fsize, &dw, nullptr)){
					break;
				}
				data[dw] = _T('\0');
				if(fsize >= 2 && data[0] == 0xff && data[1] == 0xfe){
					break; // BOM‚ª‚ ‚ê‚Î UTF16LE‚Æ‚Ý‚È‚·
				} else {
					int nLen = ::MultiByteToWideChar(CP_THREAD_ACP, 0, reinterpret_cast<char*>(data), -1, nullptr, 0);
					if(nLen != 0){
						auto sjishHeap = ::HeapCreate(0, 0, 0);
						if (sjishHeap) {
							auto sjisdata = static_cast<LPBYTE>(::HeapAlloc(sjishHeap, HEAP_ZERO_MEMORY, sizeof(TCHAR) * (nLen + 1)));
							::MultiByteToWideChar(CP_THREAD_ACP, 0, reinterpret_cast<char*>(data), -1, reinterpret_cast<TCHAR*>(sjisdata), nLen);
							::HeapFree(hHeap, 0, data);
							::HeapDestroy(hHeap);
							hHeap = sjishHeap;
							lpData = sjisdata;
						}
					}
				}
			}
			::CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}
		if(fsize > 0){
			// •ÏŠ·‘O‚ÉƒoƒbƒNƒAƒbƒv
			std::tstring bkfilename = filename;
			bkfilename += _T(".bak");
			::CopyFile(filename, bkfilename.c_str(), TRUE);
		}
		hFile = ::CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
		if(hFile == INVALID_HANDLE_VALUE){
			break;
		}
		BYTE header[] = {0xff, 0xfe};
		::WriteFile(hFile, header, sizeof(header), &dw, nullptr);
		if(lpData){
			::WriteFile(hFile, lpData, _tcslen(reinterpret_cast<TCHAR*>(lpData))*sizeof(TCHAR), &dw, nullptr);
		}
		ret = true;
	} while(0);
	if(hFile != INVALID_HANDLE_VALUE){
		::CloseHandle(hFile);
	}
	if(hHeap){
		if(lpData){
			::HeapFree(hHeap, 0, lpData);
		}
		::HeapDestroy(hHeap);
	}
	return ret;
}


static void setStringArray(CGrTxtParam::StringArray &array, LPCTSTR lpNextSrc)
{
	array.clear();
	array.shrink_to_fit();
	while(*lpNextSrc){
		auto lpSrc = lpNextSrc;
		lpNextSrc = CGrText::CharNext(lpSrc);
		array.push_back(std::tstring(lpSrc, lpNextSrc-lpSrc));
	}
}
static LPCTSTR l_ValueTypeName[CGrTxtParam::VT_MaxNum] = {
	_T("HangingChar"  ),
	_T("ConfListStart"),
	_T("ConfListEnd"  ),
	_T("ConfListSkip" ),
	_T("SepNGChar"    ),
	_T("RotateChar"   ),
	_T("RRotateChar"  ),
};
static void initValueTypeMap(CGrTxtParam::Value *vtm, CGrTxtParam::ValueType type, LPCTSTR val)
{
	vtm[type].key = l_ValueTypeName[type];
	setStringArray(vtm[type].array, val);
}
static LPCTSTR l_CharTypeName[CGrTxtParam::CT_MaxNum] = {
	_T("TextFont"        ),
	_T("RubyFont"        ),
	_T("NoteFont"        ),
	_T("NombreFont"      ),
	_T("RunningHeadsFont"),
	_T("BoldFont"        ),
};
static void initCharInfoMap(CGrTxtParam::CharInfo *cim, CGrTxtParam::CharType type, LPCTSTR fontName)
{
	cim[type].key     = l_CharTypeName[type];
	_tcscpy_s(cim[type].fontName, fontName);
}
static LPCTSTR l_PointsTypeName[CGrTxtParam::PT_MaxNum] = {
	_T("PageColor"       ),
	_T("WindowSize"      ),
	_T("TateNakaNum"     ),
	_T("ShowHScroll"     ),
	_T("ShowVScroll"     ),
	_T("SearchLoop"      ),
	_T("BookMarkAutoSave"),
	_T("BookMarkToFolder"),
	_T("BookMarkNum"     ),
	_T("SaveWindowSize"  ),
	_T("BookmarkPos"     ),
	_T("SubtitlePos"     ),
	_T("FullScreen"      ),
	_T("WzMemoMode"      ),
	_T("PageFlip"        ),
	_T("PageFlipInterval"),
	_T("AntiAlias"       ),
	_T("SelectionMode"   ),
	_T("SkipHTMLImgSize" ),
	_T("UseIESetting"    ),
	_T("UseProxy"        ),
	_T("LinkTextColor"   ),
	_T("WhiteTrans"      ),
	_T("WhiteTransRate"  ),
	_T("UsePreParser"    ),
	_T("ArcMaxFileSize"  ),
	_T("LupePos"         ),
	_T("FileAutoReload"  ),
	_T("UseOverlapChar"  ),
	_T("PictPaddingNone" ),
	_T("SpSelectionMode" ),
	_T("AutoCopyMode"    ),
	_T("AutoHideMenu"    ),
	_T("LinkPos"         ),
	_T("RunExecCopyText" ),
	_T("CopyRuby"        ),
	_T("ImageNextLayout" ),
	_T("IEOption"        ),
	_T("UnicodeIni"      ),
	_T("RubyPosition"    ),
	_T("PageMode"        ),
	_T("FavoritePos"     ),
	_T("TouchMenu"       ),
	_T("TopMost"         ),
	_T("UseRegExp"       ),
	_T("KeyInterval"     ),
	_T("WebFilter"       ),
	_T("RunningHeadLevel"),
	_T("KeyRepeat"       ),
	_T("AozoraSetting   "),
	_T("UseFont"         ),
};

static void initPointMap(CGrTxtParam::Points *pm, CGrTxtParam::PointsType type, int n, ...)
{
	va_list argptr;
	auto &&points = pm[type];
	points.key = l_PointsTypeName[type];
	va_start(argptr, n);
	for(;n > 0; --n){
		points.array.push_back(va_arg(argptr, int));
	}
	va_end(argptr);
}
static LPCTSTR l_TextTypeName[CGrTxtParam::TT_MaxNum] = {
	_T("LastFolder"        ),
	_T("LastFile"          ),
	_T("HistFile1"         ),
	_T("HistFile2"         ),
	_T("HistFile3"         ),
	_T("HistFile4"         ),
	_T("HistFile5"         ),
	_T("HistFile6"         ),
	_T("HistFile7"         ),
	_T("HistFile8"         ),
	_T("HistFile9"         ),
	_T("HistFileTitle1"    ),
	_T("HistFileTitle2"    ),
	_T("HistFileTitle3"    ),
	_T("HistFileTitle4"    ),
	_T("HistFileTitle5"    ),
	_T("HistFileTitle6"    ),
	_T("HistFileTitle7"    ),
	_T("HistFileTitle8"    ),
	_T("HistFileTitle9"    ),
	_T("HistSearchWord1"   ),
	_T("HistSearchWord2"   ),
	_T("HistSearchWord3"   ),
	_T("HistSearchWord4"   ),
	_T("HistSearchWord5"   ),
	_T("HistSearchWord6"   ),
	_T("HistSearchWord7"   ),
	_T("HistSearchWord8"   ),
	_T("HistSearchWord9"   ),
	_T("LayoutFile"        ),
	_T("LayoutType"        ),
	_T("BookMarkFolder"    ),
	_T("NoteFormat"        ),
	_T("RunningHeadsFormat"),
	_T("BackgroundImage"   ),
	_T("SpiPluginFolder"   ),
	_T("ProxyServer"       ),
	_T("NoProxyAddress"    ),
	_T("PreParserFolder"   ),
	_T("BrowserAppName"    ),
	_T("FileTypeHtml"      ),
	_T("FileTypeArc7z"     ),
	_T("FileTypeArcCab"    ),
	_T("FileTypeArcLzh"    ),
	_T("FileTypeArcRar"    ),
	_T("FileTypeArcZip"    ),
	_T("CopyTextExe"       ),
	_T("CopyTextPrm"       ),
	_T("OpenFileExe"       ),
	_T("OpenFilePrm"       ),
	_T("OpenFileExe1"      ),
	_T("OpenFilePrm1"      ),
	_T("OpenFileExe2"      ),
	_T("OpenFilePrm2"      ),
	_T("GuessEncodingDLL"  ),
	_T("TitleFormat"       ),
	_T("OpenLinkExe"       ),
	_T("OpenLinkPrm"       ),
	_T("RubyListIgnore"    ),
};
static void initTextInfoMap(CGrTxtParam::TextInfo *tim, CGrTxtParam::TextType type, LPCTSTR str = nullptr)
{
	tim[type].key = l_TextTypeName[type];
	if(str){ tim[type].str = str; }
}
static void initCharOffsetMap(CGrTxtParam::CharOffsetMap &com, LPCTSTR name, int x, int y)
{
	com[name].x = x;
	com[name].y = y;
}
static void initCharOffsetTypeMap(CGrTxtParam::CharOffsetTypeMap &cotm, LPCTSTR name, WORD type)
{
	cotm[name] = type;
}
static void initCharOffsetTypeMap(CGrTxtParam::CharOffsetTypeMap &cotm, TCHAR ch, WORD type)
{
	TCHAR name[2] = {};
	name[0] = ch;
	cotm[name] = type;
}
////////////////////
int CGrTxtParam::Points::toIntArray(int points[], int size) const
{
	size = min(size, static_cast<int>(array.size()));
	for(int idx=0;idx<size; ++idx){
		points[idx] = array[idx];
	}
	return size;
}

void CGrTxtParam::Value::toString(std::tstring &out) const
{
	out.clear();
	for(const auto &item : array){
		out += item;
	}
}

static void setFileTypeMap(CGrTxtParam::FileTypeMap &ftMap,
						   LPCTSTR valHtml  ,
						   LPCTSTR valText  ,
						   LPCTSTR valLink  ,
						   LPCTSTR valSiori ,
						   LPCTSTR valArc7z ,
						   LPCTSTR valArcCab,
						   LPCTSTR valArcLzh,
						   LPCTSTR valArcRar,
						   LPCTSTR valArcZip)
{
	int len = 0;
	struct FTMapWork {
		CGrTxtParam::FileType ft       ;
		LPCTSTR               val      ;
		CSV_COLMN             csv_colmn;
	} workList[] = {
		{CGrTxtParam::FT_Html  ,valHtml  ,CSV_COLMN()},
		{CGrTxtParam::FT_Text  ,valText  ,CSV_COLMN()},
		{CGrTxtParam::FT_Link  ,valLink  ,CSV_COLMN()},
		{CGrTxtParam::FT_Siori ,valSiori ,CSV_COLMN()},
		{CGrTxtParam::FT_Arc7z ,valArc7z ,CSV_COLMN()},
		{CGrTxtParam::FT_ArcCab,valArcCab,CSV_COLMN()},
		{CGrTxtParam::FT_ArcLzh,valArcLzh,CSV_COLMN()},
		{CGrTxtParam::FT_ArcRar,valArcRar,CSV_COLMN()},
		{CGrTxtParam::FT_ArcZip,valArcZip,CSV_COLMN()},
	};
	for(auto &&item : workList){
		CGrCSVText::toColumns(item.csv_colmn, item.val, _T(','));
		len += item.csv_colmn.size();
	}
	ftMap.resize(len);
	int i = 0;
	for(const auto &item : workList){
		for(const auto &ext : item.csv_colmn){
			ftMap[i].ft  = item.ft;
			ftMap[i].ext = ext    ;
			++i;
		}
	}
	if(len > 0){
		qsort(&(ftMap[0]), len, sizeof(CGrTxtParam::FileTypeInfo), [](const void *p1, const void *p2){
			const auto* pfti1 = static_cast<const CGrTxtParam::FileTypeInfo*>(p1);
			const auto* pfti2 = static_cast<const CGrTxtParam::FileTypeInfo*>(p2);
			return _tcsicmp(pfti1->ext.c_str(), pfti2->ext.c_str());
		});
	}
}

CGrTxtParam::CGrTxtParam()
{
	::initValueTypeMap(m_valueTypeMap, HangingCharacters      , _T(""                                                                                 )); // ‚Ô‚ç‰º‚°•¶Žš
	::initValueTypeMap(m_valueTypeMap, LineStartNGCharacters  , _T(",.¤¡ABCD£vx]}):;?!Þß¥~EFGHIJK]fhjlnprtz¬­®‚á‚ã‚åƒƒƒ…ƒ‡‚Áƒb")); // s“ª‹ÖŽ~
	::initValueTypeMap(m_valueTypeMap, LineEndNGCharacters    , _T("uwiokmy(["                                                                 )); // s––‹ÖŽ~•¶Žš
	::initValueTypeMap(m_valueTypeMap, LineStartSkipCharacters, _T(" @"                                                                              )); // s“ªƒXƒLƒbƒv•¶Žš
	::initValueTypeMap(m_valueTypeMap, SeparateNGCharacters   , _T("cd"                                                                             )); // •ªŠ„‹ÖŽ~•¶Žš
	::initValueTypeMap(m_valueTypeMap, RotateCharacters       , _T("FGefgh|€‚ƒ„…†"                                                       )); // ‰ñ“]‚³‚¹‚Ä•\Ž¦‚·‚é•¶Žš
	::initValueTypeMap(m_valueTypeMap, RRotateCharacters      , _T(""                                                                                 )); // ‹t‰ñ“]‚³‚¹‚Ä•\Ž¦‚·‚é•¶Žš
	//
	::initCharInfoMap(m_charInfoMap, Text        , _T("@‚l‚r –¾’©"));
	::initCharInfoMap(m_charInfoMap, Ruby        , _T("@‚l‚r –¾’©"));
	::initCharInfoMap(m_charInfoMap, Note        , _T("@‚l‚r –¾’©"));
	::initCharInfoMap(m_charInfoMap, Bold        , _T("@‚l‚r ƒSƒVƒbƒN"));
	// ˆÈ‰º‚ÍA‰¡‘‚«
	::initCharInfoMap(m_charInfoMap, Nombre      , _T("‚l‚r –¾’©"));
	::initCharInfoMap(m_charInfoMap, RunningHeads, _T("‚l‚r –¾’©"));
	//
	::initPointMap(m_pointsMap, PageColor       , 3, 0xffffff,0xe0e0e0,0xd0d0d0);
	::initPointMap(m_pointsMap, WindowSize      , 0         );
	::initPointMap(m_pointsMap, TateChuNum      , 1,  3     );
	::initPointMap(m_pointsMap, ShowHScroll     , 1,  0     );
	::initPointMap(m_pointsMap, ShowVScroll     , 1,  0     );
	::initPointMap(m_pointsMap, SearchLoop      , 1,  1     );
	::initPointMap(m_pointsMap, BookMarkAutoSave, 2,  1,-1  );
	::initPointMap(m_pointsMap, BookMarkToFolder, 1,  1     );
	::initPointMap(m_pointsMap, BookMarkNum     , 1,  10    );
	::initPointMap(m_pointsMap, SaveWindowSize  , 1,  1     );
	::initPointMap(m_pointsMap, BookmarkPos     , 5,  0,-1,-1,-1,-1); // Bookmar,left,top,width,height
	::initPointMap(m_pointsMap, SubtitlePos     , 5,  0,-1,-1,-1,-1); // Subtitle,left,top,width,height
	::initPointMap(m_pointsMap, FullScreen      , 1,  0     );
	::initPointMap(m_pointsMap, WzMemoMode      , 1,  1     );
	::initPointMap(m_pointsMap, PageFlip        , 1,  1     );
	::initPointMap(m_pointsMap, PageFlipInterval, 1,  30    );
	::initPointMap(m_pointsMap, AntiAlias       , 1,  1     );
	::initPointMap(m_pointsMap, SelectionMode   , 1,  0     );
	::initPointMap(m_pointsMap, SkipHTMLImgSize , 2,  100,100);
	::initPointMap(m_pointsMap, UseIESetting    , 1,  1     );
	::initPointMap(m_pointsMap, UseProxy        , 1,  1     );
	::initPointMap(m_pointsMap, LinkTextColor   , 1,  RGB(0x00,0x00,0xff));
	::initPointMap(m_pointsMap, WhiteTrans      , 1,  1     );
	::initPointMap(m_pointsMap, WhiteTransRate  , 1,  20    );
	::initPointMap(m_pointsMap, UsePreParser    , 3,  1, 1, 0  );
	::initPointMap(m_pointsMap, ArcMaxFileSize  , 1,  10    );
	::initPointMap(m_pointsMap, LupePos         , 5, 0x08,-1,-1,-1,-1);
	::initPointMap(m_pointsMap, FileAutoReload  , 1,  0,0,1000);
	::initPointMap(m_pointsMap, UseOverlapChar  , 1,  1     );
	::initPointMap(m_pointsMap, PictPaddingNone , 1,  0     );
	::initPointMap(m_pointsMap, SpSelectionMode , 2,  1,0   );
	::initPointMap(m_pointsMap, AutoCopyMode    , 1,  1     );
	::initPointMap(m_pointsMap, AutoHideMenu    , 1,  0     );
	::initPointMap(m_pointsMap, LinkPos         , 6,  0,1,-1,-1,-1,-1); // LinkDlg(Showed),StayOn,left,top,width,height
	::initPointMap(m_pointsMap, RunExecCopyText , 1,  0     );
	::initPointMap(m_pointsMap, CopyRuby        , 1,  0     );
	::initPointMap(m_pointsMap, ImageNextLayout , 1,  1     );
	::initPointMap(m_pointsMap, IEOption        , 2,  1,1   ); // IE‚ÌÝ’è, DLCTL_SILENT,DLCTL_NO_SCRIPTS
	::initPointMap(m_pointsMap, UnicodeIni      , 1,  0     );
	::initPointMap(m_pointsMap, RubyPosition    , 1,  0     );
	::initPointMap(m_pointsMap, PageMode        , 1,  0     );
	::initPointMap(m_pointsMap, FavoritePos     , 1,  0     );
	::initPointMap(m_pointsMap, TouchMenu       , 1,  1     );
	::initPointMap(m_pointsMap, TopMost         , 1,  0     );
	::initPointMap(m_pointsMap, UseRegExp       , 1,  0     );
	::initPointMap(m_pointsMap, KeyInterval     , 1,  1000  );
	::initPointMap(m_pointsMap, WebFilter       , 2,  0,0   ); // WebFilter‚ÌŽg—p, Cache‚ÌŽg—p
	::initPointMap(m_pointsMap, RunningHeadLevel, 2,  1,1   );
	::initPointMap(m_pointsMap, KeyRepeat       , 1,  1     );
	::initPointMap(m_pointsMap, AozoraSetting   , 1,  0     );
	::initPointMap(m_pointsMap, UseFont         , 1,  0     );

	::initTextInfoMap(m_textInfoMap, LastFolder        );
	::initTextInfoMap(m_textInfoMap, LastFile          );
	::initTextInfoMap(m_textInfoMap, HistFile1         );
	::initTextInfoMap(m_textInfoMap, HistFile2         );
	::initTextInfoMap(m_textInfoMap, HistFile3         );
	::initTextInfoMap(m_textInfoMap, HistFile4         );
	::initTextInfoMap(m_textInfoMap, HistFile5         );
	::initTextInfoMap(m_textInfoMap, HistFile6         );
	::initTextInfoMap(m_textInfoMap, HistFile7         );
	::initTextInfoMap(m_textInfoMap, HistFile8         );
	::initTextInfoMap(m_textInfoMap, HistFile9         );
	::initTextInfoMap(m_textInfoMap, HistFileTitle1    );
	::initTextInfoMap(m_textInfoMap, HistFileTitle2    );
	::initTextInfoMap(m_textInfoMap, HistFileTitle3    );
	::initTextInfoMap(m_textInfoMap, HistFileTitle4    );
	::initTextInfoMap(m_textInfoMap, HistFileTitle5    );
	::initTextInfoMap(m_textInfoMap, HistFileTitle6    );
	::initTextInfoMap(m_textInfoMap, HistFileTitle7    );
	::initTextInfoMap(m_textInfoMap, HistFileTitle8    );
	::initTextInfoMap(m_textInfoMap, HistFileTitle9    );
	::initTextInfoMap(m_textInfoMap, HistSearchWord1   );
	::initTextInfoMap(m_textInfoMap, HistSearchWord2   );
	::initTextInfoMap(m_textInfoMap, HistSearchWord3   );
	::initTextInfoMap(m_textInfoMap, HistSearchWord4   );
	::initTextInfoMap(m_textInfoMap, HistSearchWord5   );
	::initTextInfoMap(m_textInfoMap, HistSearchWord6   );
	::initTextInfoMap(m_textInfoMap, HistSearchWord7   );
	::initTextInfoMap(m_textInfoMap, HistSearchWord8   );
	::initTextInfoMap(m_textInfoMap, HistSearchWord9   );
	::initTextInfoMap(m_textInfoMap, LayoutFile        , _T("Layout/Bunko.lay"));
	::initTextInfoMap(m_textInfoMap, LayoutType        , _T("BUNKO 1.0"       ));
	::initTextInfoMap(m_textInfoMap, BookMarkFolder    , _T("./Bookmark"      ));
	::initTextInfoMap(m_textInfoMap, NoteFormat        , _T("¦%d"            ));
	::initTextInfoMap(m_textInfoMap, RunningHeadsFormat);
	::initTextInfoMap(m_textInfoMap, BackgroundImage   , _T("Image/background.png"));
	::initTextInfoMap(m_textInfoMap, SpiPluginFolder   );
	::initTextInfoMap(m_textInfoMap, ProxyServer       , _T("localhost:8080"  ));
	::initTextInfoMap(m_textInfoMap, NoProxyAddress    , _T("128.0.0.1;"      ));
	::initTextInfoMap(m_textInfoMap, PreParserFolder   , _T("Script"          ));
	::initTextInfoMap(m_textInfoMap, BrowserAppName    , _T("ApplicationFrameWindow,Chrome_WidgetWin_0,Chrome_WidgetWin_1,Maxthon3Cls_MainFrm,Slimjet_WidgetWin_1,MozillaWindowClass,IEXPLORE,Firefox,Sleipnir,Sleipnir2,Mozilla,Opera,Flock,NETSCAPE"));
	::initTextInfoMap(m_textInfoMap, FileTypeHtml      , _T("HTM,HTML,MHT,MHTML,XHTML"));
	::initTextInfoMap(m_textInfoMap, FileTypeArc7z     , _T("7Z"                      ));
	::initTextInfoMap(m_textInfoMap, FileTypeArcCab    , _T("CAB"                     ));
	::initTextInfoMap(m_textInfoMap, FileTypeArcLzh    , _T("LZH"                     ));
	::initTextInfoMap(m_textInfoMap, FileTypeArcRar    , _T("RAR,R01"                 ));
	::initTextInfoMap(m_textInfoMap, FileTypeArcZip    , _T("ZIP"                     ));
	::initTextInfoMap(m_textInfoMap, CopyTextExe       );
	::initTextInfoMap(m_textInfoMap, CopyTextPrm       );
	::initTextInfoMap(m_textInfoMap, OpenFileExe       , _T("notepad"));
	::initTextInfoMap(m_textInfoMap, OpenFilePrm       , _T("\"%1\""));
	::initTextInfoMap(m_textInfoMap, OpenFileExe1      , _T(""));
	::initTextInfoMap(m_textInfoMap, OpenFilePrm1      , _T(""));
	::initTextInfoMap(m_textInfoMap, OpenFileExe2      , _T(""));
	::initTextInfoMap(m_textInfoMap, OpenFilePrm2      , _T(""));
	::initTextInfoMap(m_textInfoMap, GuessEncodingDLL  );
	::initTextInfoMap(m_textInfoMap, TitleFormat       , _T("%p %T - %A (%P/%N)"));
	::initTextInfoMap(m_textInfoMap, OpenLinkExe       , _T("microsoft-edge:%1"));
	::initTextInfoMap(m_textInfoMap, OpenLinkPrm       , _T(""));
	::initTextInfoMap(m_textInfoMap, RubyListIgnore    , _T("^[E\\.c\\*`|\\-]\~]+$"));
	// %p: ƒvƒƒOƒ‰ƒ€–¼
	// %v: ƒo[ƒWƒ‡ƒ“
	// %T: ƒ^ƒCƒgƒ‹
	// %A: ’˜ŽÒ
	// %P: ƒJƒŒƒ“ƒgƒy[ƒW
	// %N: ƒy[ƒW”
	// %F: ƒtƒ@ƒCƒ‹–¼
	// %f: ƒtƒ@ƒCƒ‹–¼
	// %%: %
	setFileTypeMap(m_fileTypeMap,
				   m_textInfoMap[FileTypeHtml  ].str.c_str(),
				   _T("TXT"                                ),
				   _T("LNK"                                ),
				   _T("SIORI"                              ),
				   m_textInfoMap[FileTypeArc7z ].str.c_str(),
				   m_textInfoMap[FileTypeArcCab].str.c_str(),
				   m_textInfoMap[FileTypeArcLzh].str.c_str(),
				   m_textInfoMap[FileTypeArcRar].str.c_str(),
				   m_textInfoMap[FileTypeArcZip].str.c_str());
	//
	::initCharOffsetMap(m_charOffsetMap, _T("K"), 75,   0);
	::initCharOffsetMap(m_charOffsetMap, _T("J"), 75,   0);
	// egikmoqsuwy??á‡€fhjlnprtvxzâ‡EA
	// bit‚Å•¶Žš‚Ì—LŒø•”•ª‚ðÝ’è    x         0x07 x
	// ãˆÊ x ‰ºˆÊ y ‚ðŽw‚·       y          y     
	//        x    y                           ¡¡¡¡
	// 0xff : 1111 1111                        ¡¡¡¡
	//                                         ¡¡¡¡
	// 1    2    3    4    6    7    8    c    e    f
	// 0001 0010 0011 0100 0110 0111 1000 1100 1110 1111
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("e"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("g"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("i"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("k"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("m"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("o"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("q"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("s"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("u"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("w"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("y"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("‡€"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("f"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("h"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("j"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("l"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("n"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("p"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("r"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("t"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("v"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("x"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("z"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("‡"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("E"), 0x06);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("A"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("B"), 0x0E);
#ifdef _UNICODE
	TCHAR ch[2] = {};
	ch[0] = 0x2985; ::initCharOffsetTypeMap(m_charOffsetTypeMap, ch, 0x07); // 2985  LEFT WHITE PARENTHESIS  Žn‚ß“ñdƒp[ƒŒƒ“  CŽn‚ß“ñdŠ‡ŒÊ
	ch[0] = 0x2986; ::initCharOffsetTypeMap(m_charOffsetTypeMap, ch, 0x0E); // 2986  RIGHT WHITE PARENTHESIS I‚í‚è“ñdƒp[ƒŒƒ“CI‚í‚è“ñdŠ‡ŒÊ
#endif
}

CGrTxtParam::~CGrTxtParam()
{
}

void CGrTxtParam::ForEachValueType(ValueSetter &vs)
{
	for(auto &&item : m_valueTypeMap){ vs(item); }
}
void CGrTxtParam::ForEachCharInfo(ValueSetter &vs)
{
	for(auto &&item : m_charInfoMap){ vs(item); }
}
void CGrTxtParam::ForEachTextInfo(ValueSetter &vs)
{
	for(auto &&item : m_textInfoMap){ vs(item); }
}
void CGrTxtParam::ForEachPoints(ValueSetter &vs)
{
	for(auto &&item : m_pointsMap){ vs(item); }
}
void CGrTxtParam::ForEachCharOffset(ValueSetter &vs)
{
	for(auto &&item : m_charOffsetMap){ vs(item); }
}

static void TxtWritePrivateProfileString(LPCTSTR sec, LPCTSTR key, LPCTSTR val, LPCTSTR lpFileName)
{
	::WritePrivateProfileString(sec, key, NULL, lpFileName);
	if(val){
		int len = _tcslen(val);
		if(len > 0){
			if(val[0] == ' ' || val[len-1] == ' '       // ‘OŒã‚ª‹ó”’
			   || val[0] == '\'' || val[len-1] == '\''  //       '
			   || val[0] == '"'  || val[len-1] == '"'){ //       "‚Å‚È‚¯‚ê‚ÎAƒGƒXƒP[ƒv‚µ‚È‚¢
				std::tstring str(_T("\""));
				str += val;
				str += _T('"');
				::WritePrivateProfileString(sec, key, str.c_str(), lpFileName);
				return;
			}
		} else {
			::WritePrivateProfileString(sec, key, _T("\"\""), lpFileName);
			return;
		}
	}
	::WritePrivateProfileString(sec, key, val, lpFileName);
}

static bool TxtGetPrivateProfileString(LPCTSTR sec, LPCTSTR key, LPTSTR val, int cnt, LPCTSTR lpFileName)
{
	::GetPrivateProfileString(sec, key, _T(""), val, cnt, lpFileName);
	if(val[0] == _T('\0')){
		::GetPrivateProfileString(sec, key, _T("A"), val, cnt, lpFileName);
		if(val[0] == _T('A')){
			val[0] = '\0';
			return false;
		}
	}
	return true;
}

struct WriteValue : public CGrTxtParam::ValueSetter {
	LPCTSTR lpFileName = nullptr;
	TCHAR buf[64] = {};
	WriteValue(LPCTSTR n) : lpFileName(n){}
	virtual void operator()(CGrTxtParam::Value &value){
		if(!value.key){ return; }
		std::tstring str;
		value.toString(str);
		::TxtWritePrivateProfileString(_T("text"), value.key, str.c_str(), lpFileName);
	}
	virtual void operator()(CGrTxtParam::CharInfo &info){
		if(!info.key){ return; }
		::WritePrivateProfileString(_T("font"), info.key, info.fontName, lpFileName);
		_stprintf_s(buf, _T("#%06X"), info.color);
		::WritePrivateProfileString(_T("color"), info.key, buf, lpFileName);
		_stprintf_s(buf, _T("%g"), info.fontSpacing);
		::WritePrivateProfileString(_T("font_spacing"), info.key, buf, lpFileName);
		_stprintf_s(buf, _T("%d"), info.fontCentering);
		::WritePrivateProfileString(_T("font_centering"), info.key, buf, lpFileName);
		_stprintf_s(buf, _T("%d"), info.weight);
		::WritePrivateProfileString(_T("font_weight"), info.key, buf, lpFileName);
	}
	virtual void operator()(CGrTxtParam::Points &points){
		if(!points.key){ return; }
		int max_col = points.array.size();
		if (max_col > 0) {
			_stprintf_s(buf, _T("%d"), points.array[0]);
			std::tstring str(buf);
			for (int col = 1; col < max_col; ++col) {
				_stprintf_s(buf, _T(",%d"), points.array[col]);
				str += buf;
			}
			::WritePrivateProfileString(_T("points"), points.key, str.c_str(), lpFileName);
		}
	}
	virtual void operator()(CGrTxtParam::TextInfo &text){
		if(!text.key){ return; }
		::TxtWritePrivateProfileString(_T("text"), text.key, text.str.c_str(), lpFileName);
	}
};

bool CGrTxtParam::Save(LPCTSTR lpFileName)
{
	if(!GetBoolean(UnicodeIni)){
		if(ConvertToUTF16LE(lpFileName)){
			SetBoolean(UnicodeIni, true);
		}
	}
	WriteValue wv(lpFileName);
	ForEachValueType(wv);
	ForEachCharInfo (wv);
	ForEachTextInfo (wv);
	ForEachPoints   (wv);

	return true;
}

struct WriteStyleValue : public WriteValue {
	std::vector<std::tstring> style_list;
	WriteStyleValue(LPCTSTR n) : WriteValue(n){
		TCHAR curPath[MAX_PATH];
		CGrShell::GetExePath(curPath);
		TCHAR styleDefFileName[MAX_PATH];
		_stprintf_s(styleDefFileName, _T("%s/style.txt"), curPath);
		CGrCSVText csv;
		csv.Open(styleDefFileName);
		int max_row = csv.GetRowSize();
		for(int row=0; row<max_row; ++row){
			style_list.push_back(csv.GetText(row, 0));
		}
		std::sort(style_list.begin(), style_list.end());
	}
	bool isStyle(LPCTSTR val)
	{
		if(val && std::binary_search(style_list.begin(), style_list.end(), val)){
			return true;
		}
		return false;
	}
	virtual void operator()(CGrTxtParam::Value &value){
		if(isStyle(value.key)){
			WriteValue::operator()(value);
		}
	}
	virtual void operator()(CGrTxtParam::CharInfo &info){
		if(isStyle(info.key)){
			WriteValue::operator()(info);
		}
	}
	virtual void operator()(CGrTxtParam::Points &points){
		if(isStyle(points.key)){
			WriteValue::operator()(points);
		}
	}
	virtual void operator()(CGrTxtParam::TextInfo &text){
		if(isStyle(text.key)){
			WriteValue::operator()(text);
		}
	}
};

bool CGrTxtParam::SaveStyle(LPCTSTR lpFileName, LPCTSTR lpStyleName)
{
	auto hFile = ::CreateFile(lpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	BYTE header[] = {0xff, 0xfe};
	DWORD dw;
	::WriteFile(hFile, header, sizeof(header), &dw, nullptr);
	::CloseHandle(hFile);
	::TxtWritePrivateProfileString(_T("TxtMiru"), _T("StyleName"), lpStyleName, lpFileName);
	WriteStyleValue wv(lpFileName);
	ForEachValueType(wv);
	ForEachCharInfo (wv);
	ForEachTextInfo (wv);
	ForEachPoints   (wv);

	return true;
}

struct LoadValue : public CGrTxtParam::ValueSetter {
	TCHAR str[1024] = {};
	LPCTSTR lpFileName =nullptr;
	LoadValue(LPCTSTR n) : lpFileName(n){}
	virtual void operator()(CGrTxtParam::Value &value){
		if(!value.key){ return; }
		if(::TxtGetPrivateProfileString(_T("text"), value.key, str, sizeof(str)/sizeof(TCHAR), lpFileName)){
			::setStringArray(value.array, str);
		}
	}
	virtual void operator()(CGrTxtParam::CharInfo &info){
		if(!info.key){ return; }
		if(::TxtGetPrivateProfileString(_T("font"), info.key, str, sizeof(str)/sizeof(TCHAR), lpFileName)){
			if(_tcslen(str) < sizeof(info.fontName)/sizeof(TCHAR)){
				_tcscpy_s(info.fontName, str);
			}
		}
		if(::TxtGetPrivateProfileString(_T("color"), info.key, str, sizeof(str)/sizeof(TCHAR), lpFileName)){
			TCHAR *lpStop;
			info.color = _tcstol(str+1, &lpStop, 16);
		}
		if(::TxtGetPrivateProfileString(_T("font_spacing"), info.key, str, sizeof(str)/sizeof(TCHAR), lpFileName)){
			info.fontSpacing = _tstof(str);
		}
		if(::TxtGetPrivateProfileString(_T("font_centering"), info.key, str, sizeof(str)/sizeof(TCHAR), lpFileName)){
			info.fontCentering = (bool)(_tstol(str) == 1);
		}
		if(::TxtGetPrivateProfileString(_T("font_weight"), info.key, str, sizeof(str)/sizeof(TCHAR), lpFileName)){
			info.weight = _tstol(str);
		}
	}
	virtual void operator()(CGrTxtParam::Points &points){
		if(!points.key){ return; }
		if(::TxtGetPrivateProfileString(_T("points"), points.key, str, sizeof(str)/sizeof(TCHAR), lpFileName)){
			CGrCSVText csv;
			csv.AddTail(str);
			int row = csv.GetRowSize() - 1;
			if(row != 0){
				return;
			}
			int max_col = csv.GetColmnSize(row);
			if(max_col <= 0){
				return;
			}
			points.array.clear();
			points.array.shrink_to_fit();
			for(int col=0; col<max_col; ++col){
				points.array.push_back(csv.GetInteger(row, col, 0));
			}
		}
	}
	virtual void operator()(CGrTxtParam::TextInfo &text){
		if(!text.key){ return; }
		if(::TxtGetPrivateProfileString(_T("text"), text.key, str, sizeof(str)/sizeof(TCHAR), lpFileName)){
			text.str = str;
		}
	}
};

bool CGrTxtParam::Load(LPCTSTR lpFileName)
{
	LoadValue lv(lpFileName);
	ForEachValueType(lv);
	ForEachCharInfo (lv);
	ForEachTextInfo (lv);
	ForEachPoints   (lv);
	setFileTypeMap(m_fileTypeMap,
				   m_textInfoMap[FileTypeHtml  ].str.c_str(),
				   _T("TXT"                                ),
				   _T("LNK"                                ),
				   _T("SIORI"                              ),
				   m_textInfoMap[FileTypeArc7z ].str.c_str(),
				   m_textInfoMap[FileTypeArcCab].str.c_str(),
				   m_textInfoMap[FileTypeArcLzh].str.c_str(),
				   m_textInfoMap[FileTypeArcRar].str.c_str(),
				   m_textInfoMap[FileTypeArcZip].str.c_str());

	TCHAR path[512], offset_file[512];
	CGrShell::GetParentDir((TCHAR*)lpFileName, path);
	CGrShell::AddBackSlash(path);
	_stprintf_s(offset_file, _T("%soffset.lis"), path);
	CGrCSVText csv;
	if(csv.Open(offset_file)){
		int max_row = csv.GetRowSize();
		for(int row=0; row<max_row; ++row){
			int array[2];
			int col_num = csv.GetColmnSize(row);
			if(col_num == 3){
				if(!csv.toIntArray(row, 1, array, sizeof(array)/sizeof(int))){
					continue;
				}
				auto &&pos = m_charOffsetMap[csv.GetText(row, 0)];
				pos.x = array[0];
				pos.y = array[1];
			} else if(col_num == 2){
				if(!csv.toIntArray(row, 1, array, 1)){
					continue;
				}
				m_charOffsetTypeMap[csv.GetText(row, 0)] = array[0];
			}
		}
	}
	//
	TCHAR cookie_file[512];
	_stprintf_s(cookie_file, _T("%scookie.txt"), path);
	if(csv.Open(cookie_file)){
		int max_row = csv.GetRowSize();
		m_cookieInfoMap.resize(max_row);
		auto *p = m_cookieInfoMap.data();
		for(int row=0; row<max_row; ++row, ++p){
			int col_num = csv.GetColmnSize(row);
			if(col_num == 2){
				try {
					std::wregex re(csv.GetText(row, 0), std::regex_constants::icase);
					p->re = re;
					p->cookie = csv.GetText(row, 1);
				} catch(...){}
			}
		}
	}
	return true;
}

int CGrTxtParam::FindIF(CGrTxtParam::ValueType type, CGrTxtParam::CGrStringCompare &&sc) const
{
	for(const auto &item : m_valueTypeMap[type].array){
		if(sc == item.c_str()){
			return 1;
		}
	}
	return -1;
}

void CGrTxtParam::SetFontName(CGrTxtParam::CharType type, LPCTSTR fontName)
{
	_tcscpy_s(m_charInfoMap[type].fontName, fontName);
}

LPCTSTR CGrTxtParam::GetFontName(CGrTxtParam::CharType type)
{
	return m_charInfoMap[type].fontName;
}

void CGrTxtParam::SetColor(CharType type, COLORREF color)
{
	m_charInfoMap[type].color = color;
}

void CGrTxtParam::SetFontSpacing(CharType type, double fontSpacing)
{
	m_charInfoMap[type].fontSpacing = fontSpacing;
}

void CGrTxtParam::SetFontCentering(CharType type, bool fontCentering)
{
	m_charInfoMap[type].fontCentering = fontCentering;
}

void CGrTxtParam::SetFontWeight(CharType type, long weight)
{
	m_charInfoMap[type].weight = weight;
}

void CGrTxtParam::SetText(TextType type, LPCTSTR lpStr)
{
	m_textInfoMap[type].str = lpStr;
	auto &&str = m_textInfoMap[type].str;
	if(!str.empty()){
		std::replaceCRLF(str, _T(" "));
	}
	if(type == FileTypeArcZip){ // ƒtƒ@ƒCƒ‹‚ÌŠg’£ŽqC³‚µ‚Ä‚à ŽŸ‰ñ‹N“®‚Ü‚Å”½‰f‚³‚ê‚È‚¢‚Ì‚Å
		setFileTypeMap(m_fileTypeMap,
					   m_textInfoMap[FileTypeHtml  ].str.c_str(),
					   _T("TXT"                                ),
					   _T("LNK"                                ),
					   _T("SIORI"                              ),
					   m_textInfoMap[FileTypeArc7z ].str.c_str(),
					   m_textInfoMap[FileTypeArcCab].str.c_str(),
					   m_textInfoMap[FileTypeArcLzh].str.c_str(),
					   m_textInfoMap[FileTypeArcRar].str.c_str(),
					   m_textInfoMap[FileTypeArcZip].str.c_str());
	}
}

COLORREF CGrTxtParam::GetColor(CharType type)
{
	return m_charInfoMap[type].color;
}

double CGrTxtParam::GetFontSpacing(CharType type)
{
	return m_charInfoMap[type].fontSpacing;
}

bool CGrTxtParam::GetFontCentering(CharType type)
{
	return m_charInfoMap[type].fontCentering;
}

long CGrTxtParam::GetFontWeight(CharType type)
{
	return m_charInfoMap[type].weight;
}

void CGrTxtParam::SetBoolean(CGrTxtParam::PointsType type, bool val)
{
	int iVal = val ? 1 : 0;
	SetPoints(type, &iVal, 1);
}

bool CGrTxtParam::GetBoolean(CGrTxtParam::PointsType type) const
{
	int iVal = 0;
	GetPoints(type, &iVal, 1);
	return (iVal == 1);
}

void CGrTxtParam::SetPoints(PointsType type, const int points[], int size)
{
	m_pointsMap[type].array.assign(points, points+size);
}

int CGrTxtParam::GetPoints(PointsType type, int points[], int size) const
{
	return m_pointsMap[type].toIntArray(points, size);
}

void CGrTxtParam::SetTextList(ValueType type, LPCTSTR lpStr)
{
	::setStringArray(m_valueTypeMap[type].array, lpStr);
}

void CGrTxtParam::GetTextList(ValueType type, LPTSTR outstr, int numberOfElements)
{
	std::tstring str;
	GetTextList(type, str);
	_tcscpy_s(outstr, numberOfElements, str.c_str());
}

void CGrTxtParam::GetText(TextType type, LPTSTR outstr, int numberOfElements) const
{
	std::tstring str;
	GetText(type, str);
	_tcscpy_s(outstr, numberOfElements, str.c_str());
}

void CGrTxtParam::GetTextList(ValueType type, std::tstring &out) const
{
	m_valueTypeMap[type].toString(out);
}

void CGrTxtParam::GetText(TextType type, std::tstring &out) const
{
	out = m_textInfoMap[type].str.c_str();
}

POINT CGrTxtParam::GetCharOffset(LPCTSTR str) const
{
	return GetCharOffset(std::tstring(str));
}
WORD CGrTxtParam::GetCharOffsetType(LPCTSTR str) const
{
	return GetCharOffsetType(std::tstring(str));
}
POINT CGrTxtParam::GetCharOffset(const std::tstring &str) const
{
	auto it=m_charOffsetMap.find(str);
	if(it == m_charOffsetMap.end()){
		POINT pos = {0,0};
		return pos;
	}
	return it->second;
}

WORD CGrTxtParam::GetCharOffsetType(const std::tstring &str) const
{
	auto it=m_charOffsetTypeMap.find(str);
	if(it == m_charOffsetTypeMap.end()){
		return 0x00;
	}
	return it->second;
}

#include "TxtMiruDef.h"
void CGrTxtParam::UpdateKeybord(HWND hWnd)
{
	::SendMessage(hWnd, WM_COMMAND, static_cast<WPARAM>(TxtDocMessage::UPDATE_KEYBORD), 0);
}
void CGrTxtParam::UpdateConfig(HWND hWnd)
{
	::SendMessage(hWnd, WM_COMMAND, static_cast<WPARAM>(TxtDocMessage::UPDATE_CONFIG), 0);
}
void CGrTxtParam::UpdateLayout(HWND hWnd)
{
	::SendMessage(hWnd, WM_COMMAND, static_cast<WPARAM>(TxtDocMessage::UPDATE_LAYOUT), 0);
}
void CGrTxtParam::UpdateStyleList(HWND hWnd)
{
	::SendMessage(hWnd, WM_COMMAND, static_cast<WPARAM>(TxtDocMessage::UPDATE_STYLE_LIST), 0);
}

/////
CGrTxtParam::FileType CGrTxtParam::GetFileType(LPCTSTR lpExt) const
{
	int len = m_fileTypeMap.size();
	if(lpExt && len > 0){
		auto *pfti = static_cast<FileTypeInfo*>(bsearch(
			lpExt, m_fileTypeMap.data(), len, sizeof(FileTypeInfo),
			[](const void *key, const void *pdata){
				auto pext  = static_cast<LPCTSTR>(key);
				const auto *pfti = static_cast<const CGrTxtParam::FileTypeInfo*>(pdata);
				return _tcsicmp(pext, pfti->ext.c_str());
			}));
		if(pfti){
			return pfti->ft;
		}
	}
	return FT_MaxNum;
}

LPCTSTR CGrTxtParam::GetCookie(LPCTSTR lpURL) const
{
	int len = m_cookieInfoMap.size();
	if(lpURL && len > 0){
		const auto *p = m_cookieInfoMap.data();
		for(int i=0; i<len; ++i, ++p){
			if(!p->cookie.empty() && std::regex_search(lpURL, p->re)){
				return p->cookie.c_str();
			}
		}
	}
	return nullptr;
}
