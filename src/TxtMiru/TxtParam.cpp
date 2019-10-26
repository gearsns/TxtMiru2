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
					break; // BOMÇ™Ç†ÇÍÇŒ UTF16LEÇ∆Ç›Ç»Ç∑
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
			// ïœä∑ëOÇ…ÉoÉbÉNÉAÉbÉv
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
static LPCTSTR l_ValueTypeName[static_cast<int>(CGrTxtParam::ValueType::MaxNum)] = {
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
	vtm[static_cast<int>(type)].key = l_ValueTypeName[static_cast<int>(type)];
	setStringArray(vtm[static_cast<int>(type)].array, val);
}
static LPCTSTR l_CharTypeName[static_cast<int>(CGrTxtParam::CharType::MaxNum)] = {
	_T("TextFont"        ),
	_T("RubyFont"        ),
	_T("NoteFont"        ),
	_T("NombreFont"      ),
	_T("RunningHeadsFont"),
	_T("BoldFont"        ),
};
static void initCharInfoMap(CGrTxtParam::CharInfo *cim, CGrTxtParam::CharType type, LPCTSTR fontName)
{
	cim[static_cast<int>(type)].key     = l_CharTypeName[static_cast<int>(type)];
	_tcscpy_s(cim[static_cast<int>(type)].fontName, fontName);
}
static LPCTSTR l_PointsTypeName[static_cast<int>(CGrTxtParam::PointsType::MaxNum)] = {
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
	auto &&points = pm[static_cast<int>(type)];
	points.key = l_PointsTypeName[static_cast<int>(type)];
	va_start(argptr, n);
	for(;n > 0; --n){
		points.array.push_back(va_arg(argptr, int));
	}
	va_end(argptr);
}
static LPCTSTR l_TextTypeName[static_cast<int>(CGrTxtParam::TextType::MaxNum)] = {
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
	tim[static_cast<int>(type)].key = l_TextTypeName[static_cast<int>(type)];
	if(str){ tim[static_cast<int>(type)].str = str; }
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
		{CGrTxtParam::FileType::Html  ,valHtml  ,CSV_COLMN()},
		{CGrTxtParam::FileType::Text  ,valText  ,CSV_COLMN()},
		{CGrTxtParam::FileType::Link  ,valLink  ,CSV_COLMN()},
		{CGrTxtParam::FileType::Siori ,valSiori ,CSV_COLMN()},
		{CGrTxtParam::FileType::Arc7z ,valArc7z ,CSV_COLMN()},
		{CGrTxtParam::FileType::ArcCab,valArcCab,CSV_COLMN()},
		{CGrTxtParam::FileType::ArcLzh,valArcLzh,CSV_COLMN()},
		{CGrTxtParam::FileType::ArcRar,valArcRar,CSV_COLMN()},
		{CGrTxtParam::FileType::ArcZip,valArcZip,CSV_COLMN()},
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
	::initValueTypeMap(m_valueTypeMap, ValueType::HangingCharacters      , _T(""/*",.§°ÅAÅBÅCÅD"*/                                                               )); // Ç‘ÇÁâ∫Ç∞ï∂éö
	::initValueTypeMap(m_valueTypeMap, ValueType::LineStartNGCharacters  , _T(",.§°ÅAÅBÅCÅD£ÅvÅx]}):;?!ﬁﬂ•~ÅEÅFÅGÅHÅIÅJÅKÅ]ÅfÅhÅjÅlÅnÅpÅrÅtÅz¨≠ÆÇ·Ç„ÇÂÉÉÉÖÉáÇ¡Éb")); // çsì™ã÷é~
	::initValueTypeMap(m_valueTypeMap, ValueType::LineEndNGCharacters    , _T("ÅuÅwÅiÅoÅkÅmÅy(["                                                                 )); // çsññã÷é~ï∂éö
	::initValueTypeMap(m_valueTypeMap, ValueType::LineStartSkipCharacters, _T(" Å@"                                                                              )); // çsì™ÉXÉLÉbÉvï∂éö
	::initValueTypeMap(m_valueTypeMap, ValueType::SeparateNGCharacters   , _T("ÅcÅd"                                                                             )); // ï™äÑã÷é~ï∂éö
	::initValueTypeMap(m_valueTypeMap, ValueType::RotateCharacters       , _T("ÅFÅGÅeÅfÅgÅhÅ|ÅÄÅÇÅÉÅÑÅÖÅÜ"                                                       )); // âÒì]Ç≥ÇπÇƒï\é¶Ç∑ÇÈï∂éö
	::initValueTypeMap(m_valueTypeMap, ValueType::RRotateCharacters      , _T(""                                                                                 )); // ãtâÒì]Ç≥ÇπÇƒï\é¶Ç∑ÇÈï∂éö
	//
	::initCharInfoMap(m_charInfoMap, CharType::Text        , _T("@ÇlÇr ñæí©"));
	::initCharInfoMap(m_charInfoMap, CharType::Ruby        , _T("@ÇlÇr ñæí©"));
	::initCharInfoMap(m_charInfoMap, CharType::Note        , _T("@ÇlÇr ñæí©"));
	::initCharInfoMap(m_charInfoMap, CharType::Bold        , _T("@ÇlÇr ÉSÉVÉbÉN"));
	// à»â∫ÇÕÅAâ°èëÇ´
	::initCharInfoMap(m_charInfoMap, CharType::Nombre      , _T("ÇlÇr ñæí©"));
	::initCharInfoMap(m_charInfoMap, CharType::RunningHeads, _T("ÇlÇr ñæí©"));
	//
	::initPointMap(m_pointsMap, PointsType::PageColor       , 3, 0xffffff,0xe0e0e0,0xd0d0d0);
	::initPointMap(m_pointsMap, PointsType::WindowSize      , 0         );
	::initPointMap(m_pointsMap, PointsType::TateChuNum      , 1,  3     );
	::initPointMap(m_pointsMap, PointsType::ShowHScroll     , 1,  0     );
	::initPointMap(m_pointsMap, PointsType::ShowVScroll     , 1,  0     );
	::initPointMap(m_pointsMap, PointsType::SearchLoop      , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::BookMarkAutoSave, 2,  1,-1  );
	::initPointMap(m_pointsMap, PointsType::BookMarkToFolder, 1,  1     );
	::initPointMap(m_pointsMap, PointsType::BookMarkNum     , 1,  10    );
	::initPointMap(m_pointsMap, PointsType::SaveWindowSize  , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::BookmarkPos     , 5,  0,-1,-1,-1,-1); // Bookmar,left,top,width,height
	::initPointMap(m_pointsMap, PointsType::SubtitlePos     , 5,  0,-1,-1,-1,-1); // Subtitle,left,top,width,height
	::initPointMap(m_pointsMap, PointsType::FullScreen      , 1,  0     );
	::initPointMap(m_pointsMap, PointsType::WzMemoMode      , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::PageFlip        , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::PageFlipInterval, 1,  30    );
	::initPointMap(m_pointsMap, PointsType::AntiAlias       , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::SelectionMode   , 1,  0     );
	::initPointMap(m_pointsMap, PointsType::SkipHTMLImgSize , 2,  100,100);
	::initPointMap(m_pointsMap, PointsType::UseIESetting    , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::UseProxy        , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::LinkTextColor   , 1,  RGB(0x00,0x00,0xff));
	::initPointMap(m_pointsMap, PointsType::WhiteTrans      , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::WhiteTransRate  , 1,  20    );
	::initPointMap(m_pointsMap, PointsType::UsePreParser    , 3,  1, 1, 0  );
	::initPointMap(m_pointsMap, PointsType::ArcMaxFileSize  , 1,  10    );
	::initPointMap(m_pointsMap, PointsType::LupePos         , 5, 0x08,-1,-1,-1,-1); // 200%Ç…ïœçX
	::initPointMap(m_pointsMap, PointsType::FileAutoReload  , 1,  0,0,1000);
	::initPointMap(m_pointsMap, PointsType::UseOverlapChar  , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::PictPaddingNone , 1,  0     );
	::initPointMap(m_pointsMap, PointsType::SpSelectionMode , 2,  1,0   );
	::initPointMap(m_pointsMap, PointsType::AutoCopyMode    , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::AutoHideMenu    , 1,  0     );
	::initPointMap(m_pointsMap, PointsType::LinkPos         , 6,  0,1,-1,-1,-1,-1); // LinkDlg(Showed),StayOn,left,top,width,height
	::initPointMap(m_pointsMap, PointsType::RunExecCopyText , 1,  0     );
	::initPointMap(m_pointsMap, PointsType::CopyRuby        , 1,  0     );
	::initPointMap(m_pointsMap, PointsType::ImageNextLayout , 1,  1     );
	::initPointMap(m_pointsMap, PointsType::IEOption        , 2,  1,1   ); // IEÇÃê›íË, DLCTL_SILENT,DLCTL_NO_SCRIPTS
	::initPointMap(m_pointsMap, PointsType::UnicodeIni      , 1,  0     ); 
	::initPointMap(m_pointsMap, PointsType::RubyPosition    , 1,  0     ); 
	::initPointMap(m_pointsMap, PointsType::PageMode        , 1,  0     ); 
	::initPointMap(m_pointsMap, PointsType::FavoritePos     , 1,  0     ); 
	::initPointMap(m_pointsMap, PointsType::TouchMenu       , 1,  1     ); 
	::initPointMap(m_pointsMap, PointsType::TopMost         , 1,  0     ); 
	::initPointMap(m_pointsMap, PointsType::UseRegExp       , 1,  0     ); 
	::initPointMap(m_pointsMap, PointsType::KeyInterval     , 1,  1000  ); 
	::initPointMap(m_pointsMap, PointsType::WebFilter       , 2,  0,0   ); // WebFilterÇÃégóp, CacheÇÃégóp
	::initPointMap(m_pointsMap, PointsType::RunningHeadLevel, 2,  1,1   ); 
	::initPointMap(m_pointsMap, PointsType::KeyRepeat       , 1,  1     ); 
	::initPointMap(m_pointsMap, PointsType::AozoraSetting   , 1,  0     ); 
	::initPointMap(m_pointsMap, PointsType::UseFont         , 1,  0     ); 
	//
	::initTextInfoMap(m_textInfoMap, TextType::LastFolder        );
	::initTextInfoMap(m_textInfoMap, TextType::LastFile          );
	::initTextInfoMap(m_textInfoMap, TextType::HistFile1         );
	::initTextInfoMap(m_textInfoMap, TextType::HistFile2         );
	::initTextInfoMap(m_textInfoMap, TextType::HistFile3         );
	::initTextInfoMap(m_textInfoMap, TextType::HistFile4         );
	::initTextInfoMap(m_textInfoMap, TextType::HistFile5         );
	::initTextInfoMap(m_textInfoMap, TextType::HistFile6         );
	::initTextInfoMap(m_textInfoMap, TextType::HistFile7         );
	::initTextInfoMap(m_textInfoMap, TextType::HistFile8         );
	::initTextInfoMap(m_textInfoMap, TextType::HistFile9         );
	::initTextInfoMap(m_textInfoMap, TextType::HistFileTitle1    );
	::initTextInfoMap(m_textInfoMap, TextType::HistFileTitle2    );
	::initTextInfoMap(m_textInfoMap, TextType::HistFileTitle3    );
	::initTextInfoMap(m_textInfoMap, TextType::HistFileTitle4    );
	::initTextInfoMap(m_textInfoMap, TextType::HistFileTitle5    );
	::initTextInfoMap(m_textInfoMap, TextType::HistFileTitle6    );
	::initTextInfoMap(m_textInfoMap, TextType::HistFileTitle7    );
	::initTextInfoMap(m_textInfoMap, TextType::HistFileTitle8    );
	::initTextInfoMap(m_textInfoMap, TextType::HistFileTitle9    );
	::initTextInfoMap(m_textInfoMap, TextType::HistSearchWord1   );
	::initTextInfoMap(m_textInfoMap, TextType::HistSearchWord2   );
	::initTextInfoMap(m_textInfoMap, TextType::HistSearchWord3   );
	::initTextInfoMap(m_textInfoMap, TextType::HistSearchWord4   );
	::initTextInfoMap(m_textInfoMap, TextType::HistSearchWord5   );
	::initTextInfoMap(m_textInfoMap, TextType::HistSearchWord6   );
	::initTextInfoMap(m_textInfoMap, TextType::HistSearchWord7   );
	::initTextInfoMap(m_textInfoMap, TextType::HistSearchWord8   );
	::initTextInfoMap(m_textInfoMap, TextType::HistSearchWord9   );
	::initTextInfoMap(m_textInfoMap, TextType::LayoutFile        , _T("Layout/Bunko.lay"));
	::initTextInfoMap(m_textInfoMap, TextType::LayoutType        , _T("BUNKO 1.0"       ));
	::initTextInfoMap(m_textInfoMap, TextType::BookMarkFolder    , _T("./Bookmark"      ));
	::initTextInfoMap(m_textInfoMap, TextType::NoteFormat        , _T("Å¶%d"            ));
	::initTextInfoMap(m_textInfoMap, TextType::RunningHeadsFormat);
	::initTextInfoMap(m_textInfoMap, TextType::BackgroundImage   , _T("Image/background.png"));
	::initTextInfoMap(m_textInfoMap, TextType::SpiPluginFolder   );
	::initTextInfoMap(m_textInfoMap, TextType::ProxyServer       , _T("localhost:8080"  ));
	::initTextInfoMap(m_textInfoMap, TextType::NoProxyAddress    , _T("128.0.0.1;"      ));
	::initTextInfoMap(m_textInfoMap, TextType::PreParserFolder   , _T("Script"          ));
	::initTextInfoMap(m_textInfoMap, TextType::BrowserAppName    , _T("ApplicationFrameWindow,Chrome_WidgetWin_0,Chrome_WidgetWin_1,Maxthon3Cls_MainFrm,Slimjet_WidgetWin_1,MozillaWindowClass,IEXPLORE,Firefox,Sleipnir,Sleipnir2,Mozilla,Opera,Flock,NETSCAPE"));
	::initTextInfoMap(m_textInfoMap, TextType::FileTypeHtml      , _T("HTM,HTML,MHT,MHTML,XHTML"));
	::initTextInfoMap(m_textInfoMap, TextType::FileTypeArc7z     , _T("7Z"                      ));
	::initTextInfoMap(m_textInfoMap, TextType::FileTypeArcCab    , _T("CAB"                     ));
	::initTextInfoMap(m_textInfoMap, TextType::FileTypeArcLzh    , _T("LZH"                     ));
	::initTextInfoMap(m_textInfoMap, TextType::FileTypeArcRar    , _T("RAR,R01"                 ));
	::initTextInfoMap(m_textInfoMap, TextType::FileTypeArcZip    , _T("ZIP"                     ));
	::initTextInfoMap(m_textInfoMap, TextType::CopyTextExe       );
	::initTextInfoMap(m_textInfoMap, TextType::CopyTextPrm       );
	::initTextInfoMap(m_textInfoMap, TextType::OpenFileExe       , _T("notepad"));
	::initTextInfoMap(m_textInfoMap, TextType::OpenFilePrm       , _T("\"%1\""));
	::initTextInfoMap(m_textInfoMap, TextType::OpenFileExe1      , _T(""));
	::initTextInfoMap(m_textInfoMap, TextType::OpenFilePrm1      , _T(""));
	::initTextInfoMap(m_textInfoMap, TextType::OpenFileExe2      , _T(""));
	::initTextInfoMap(m_textInfoMap, TextType::OpenFilePrm2      , _T(""));
	::initTextInfoMap(m_textInfoMap, TextType::GuessEncodingDLL  );
	::initTextInfoMap(m_textInfoMap, TextType::TitleFormat       , _T("%p %T - %A (%P/%N)"));
	::initTextInfoMap(m_textInfoMap, TextType::OpenLinkExe       , _T("microsoft-edge:%1"));
	::initTextInfoMap(m_textInfoMap, TextType::OpenLinkPrm       , _T(""));
	::initTextInfoMap(m_textInfoMap, TextType::RubyListIgnore    , _T("^[ÅE\\.Åc\\*Å`Å|\\-Å]Å\~]+$"));
	// %p: ÉvÉçÉOÉâÉÄñº
	// %v: ÉoÅ[ÉWÉáÉì
	// %T: É^ÉCÉgÉã
	// %A: íòé“
	// %P: ÉJÉåÉìÉgÉyÅ[ÉW
	// %N: ÉyÅ[ÉWêî
	// %F: ÉtÉ@ÉCÉãñº
	// %f: ÉtÉ@ÉCÉãñº
	// %%: %
	setFileTypeMap(m_fileTypeMap,
				   m_textInfoMap[static_cast<int>(TextType::FileTypeHtml  )].str.c_str(),
				   _T("TXT"),
				   _T("LNK"),
				   _T("SIORI"),
				   m_textInfoMap[static_cast<int>(TextType::FileTypeArc7z )].str.c_str(),
				   m_textInfoMap[static_cast<int>(TextType::FileTypeArcCab)].str.c_str(),
				   m_textInfoMap[static_cast<int>(TextType::FileTypeArcLzh)].str.c_str(),
				   m_textInfoMap[static_cast<int>(TextType::FileTypeArcRar)].str.c_str(),
				   m_textInfoMap[static_cast<int>(TextType::FileTypeArcZip)].str.c_str());
	//
	::initCharOffsetMap(m_charOffsetMap, _T("ÅK"), 75,   0);
	::initCharOffsetMap(m_charOffsetMap, _T("ÅJ"), 75,   0);
	// ÅeÅgÅiÅkÅmÅoÅqÅsÅuÅwÅy??Å·áÄÅfÅhÅjÅlÅnÅpÅrÅtÅvÅxÅzÅ‚áÅÅEÅA
	// bitÇ≈ï∂éöÇÃóLå¯ïîï™Çê›íË    x         0x07 x
	// è„à  x â∫à  y ÇéwÇ∑       y Å†Å†Å†Å†     y Å†Å†Å†Å†
	//        x    y                Å†Å†Å†Å†       Å°Å°Å°Å°
	// 0xff : 1111 1111             Å†Å†Å†Å†       Å°Å°Å°Å°
	//                              Å†Å†Å†Å†       Å°Å°Å°Å°
	// 1    2    3    4    6    7    8    c    e    f
	// 0001 0010 0011 0100 0110 0111 1000 1100 1110 1111
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åe"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åg"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åi"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åk"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åm"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åo"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åq"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Ås"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åu"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åw"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åy"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("áÄ"), 0x07);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åf"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åh"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åj"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Ål"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Ån"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åp"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("År"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åt"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åv"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åx"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("Åz"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("áÅ"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("ÅE"), 0x06);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("ÅA"), 0x0E);
	::initCharOffsetTypeMap(m_charOffsetTypeMap, _T("ÅB"), 0x0E);
#ifdef _UNICODE
	TCHAR ch[2] = {};
	ch[0] = 0x2985; ::initCharOffsetTypeMap(m_charOffsetTypeMap, ch, 0x07); // 2985  LEFT WHITE PARENTHESIS  énÇﬂìÒèdÉpÅ[ÉåÉì  ÅCénÇﬂìÒèdäáå 
	ch[0] = 0x2986; ::initCharOffsetTypeMap(m_charOffsetTypeMap, ch, 0x0E); // 2986  RIGHT WHITE PARENTHESIS èIÇÌÇËìÒèdÉpÅ[ÉåÉìÅCèIÇÌÇËìÒèdäáå 
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
			if(val[0] == ' ' || val[len-1] == ' '       // ëOå„Ç™ãÛîí
			   || val[0] == '\'' || val[len-1] == '\''  //       '
			   || val[0] == '"'  || val[len-1] == '"'){ //       "Ç≈Ç»ÇØÇÍÇŒÅAÉGÉXÉPÅ[ÉvÇµÇ»Ç¢
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
	if(!GetBoolean(PointsType::UnicodeIni)){
		if(ConvertToUTF16LE(lpFileName)){
			SetBoolean(PointsType::UnicodeIni, true);
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
				   m_textInfoMap[static_cast<int>(TextType::FileTypeHtml  )].str.c_str(),
				   _T("TXT"),
				   _T("LNK"),
				   _T("SIORI"),
				   m_textInfoMap[static_cast<int>(TextType::FileTypeArc7z )].str.c_str(),
				   m_textInfoMap[static_cast<int>(TextType::FileTypeArcCab)].str.c_str(),
				   m_textInfoMap[static_cast<int>(TextType::FileTypeArcLzh)].str.c_str(),
				   m_textInfoMap[static_cast<int>(TextType::FileTypeArcRar)].str.c_str(),
				   m_textInfoMap[static_cast<int>(TextType::FileTypeArcZip)].str.c_str());

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
	for(const auto &item : m_valueTypeMap[static_cast<int>(type)].array){
		if(sc == item.c_str()){
			return 1;
		}
	}
	return -1;
}

void CGrTxtParam::SetFontName(CGrTxtParam::CharType type, LPCTSTR fontName)
{
	_tcscpy_s(m_charInfoMap[static_cast<int>(type)].fontName, fontName);
}

LPCTSTR CGrTxtParam::GetFontName(CGrTxtParam::CharType type)
{
	return m_charInfoMap[static_cast<int>(type)].fontName;
}

void CGrTxtParam::SetColor(CharType type, COLORREF color)
{
	m_charInfoMap[static_cast<int>(type)].color = color;
}

void CGrTxtParam::SetFontSpacing(CharType type, double fontSpacing)
{
	m_charInfoMap[static_cast<int>(type)].fontSpacing = fontSpacing;
}

void CGrTxtParam::SetFontCentering(CharType type, bool fontCentering)
{
	m_charInfoMap[static_cast<int>(type)].fontCentering = fontCentering;
}

void CGrTxtParam::SetFontWeight(CharType type, long weight)
{
	m_charInfoMap[static_cast<int>(type)].weight = weight;
}

void CGrTxtParam::SetText(TextType type, LPCTSTR lpStr)
{
	m_textInfoMap[static_cast<int>(type)].str = lpStr;
	auto &&str = m_textInfoMap[static_cast<int>(type)].str;
	if(!str.empty()){
		std::replaceCRLF(str, _T(" "));
	}
	if(type == TextType::FileTypeArcZip){ // ÉtÉ@ÉCÉãÇÃägí£éqèCê≥ÇµÇƒÇ‡ éüâÒãNìÆÇ‹Ç≈îΩâfÇ≥ÇÍÇ»Ç¢ÇÃÇ≈
		setFileTypeMap(m_fileTypeMap,
					   m_textInfoMap[static_cast<int>(TextType::FileTypeHtml  )].str.c_str(),
					   _T("TXT"),
					   _T("LNK"),
					   _T("SIORI"),
					   m_textInfoMap[static_cast<int>(TextType::FileTypeArc7z )].str.c_str(),
					   m_textInfoMap[static_cast<int>(TextType::FileTypeArcCab)].str.c_str(),
					   m_textInfoMap[static_cast<int>(TextType::FileTypeArcLzh)].str.c_str(),
					   m_textInfoMap[static_cast<int>(TextType::FileTypeArcRar)].str.c_str(),
					   m_textInfoMap[static_cast<int>(TextType::FileTypeArcZip)].str.c_str());
	}
}

COLORREF CGrTxtParam::GetColor(CharType type)
{
	return m_charInfoMap[static_cast<int>(type)].color;
}

double CGrTxtParam::GetFontSpacing(CharType type)
{
	return m_charInfoMap[static_cast<int>(type)].fontSpacing;
}

bool CGrTxtParam::GetFontCentering(CharType type)
{
	return m_charInfoMap[static_cast<int>(type)].fontCentering;
}

long CGrTxtParam::GetFontWeight(CharType type)
{
	return m_charInfoMap[static_cast<int>(type)].weight;
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
	m_pointsMap[static_cast<int>(type)].array.assign(points, points+size);
}

int CGrTxtParam::GetPoints(PointsType type, int points[], int size) const
{
	return m_pointsMap[static_cast<int>(type)].toIntArray(points, size);
}

void CGrTxtParam::SetTextList(ValueType type, LPCTSTR lpStr)
{
	::setStringArray(m_valueTypeMap[static_cast<int>(type)].array, lpStr);
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
	m_valueTypeMap[static_cast<int>(type)].toString(out);
}

void CGrTxtParam::GetText(TextType type, std::tstring &out) const
{
	out = m_textInfoMap[static_cast<int>(type)].str.c_str();
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
	return FileType::MaxNum;
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
