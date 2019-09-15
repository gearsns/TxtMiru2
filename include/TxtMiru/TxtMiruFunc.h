#ifndef __TXTMIRUFUNC_H__
#define __TXTMIRUFUNC_H__

#ifdef TXTMIRUFUNCNAMELIST
# undef TXTMIRUFUNCNAMELIST
# undef TXTMIRUFL
# undef TXTMIRUFUNCNAMELIST_END
# undef TXTMIRUFUNCNAMELIST_EXTERN
#endif

#ifdef TXTMIRUAPP
// TxtMiru.cpp で実体定義
# ifndef TXTMIRUFUNC_NAMELIST
#  define TXTMIRUFUNC_NAMELIST
#  define TXTMIRUFUNCNAMELIST LPCTSTR l_TxtMiruFuncNameList[] =
#  define TXTMIRUFL(__a__,__b__,__c__) _T(#__a__)
#  define TXTMIRUFUNCNAMELIST_END(__b__,__c__)
#  define TXTMIRUFUNCNAMELIST_EXTERN
# endif
#elif defined TXTMIRUMENU_STR_H
// Popup Menu用のString IDリスト
# ifndef TXTMIRUFUNC_STRIDLIST
#  define TXTMIRUFUNC_STRIDLIST
#  undef TXTMIRUMENU_STR_H
#  undef __TXTMIRUFUNC_H__
#  include "TxtMiruFunc.h"
#  define TXTMIRUFUNCNAMELIST static UINT l_TxtMiruFuncStrIDList[] =
#  define TXTMIRUFL(__a__,__b__,__c__) __b__
#  define TXTMIRUFUNCNAMELIST_END(__b__,__c__) __b__
#  define TXTMIRUFUNCNAMELIST_EXTERN
# endif
#elif defined TXTMIRUMENU_ID_H // Menu用のIDリスト
# ifndef TXTMIRUFUNC_MENUIDLIST
#  define TXTMIRUFUNC_MENUIDLIST
#  undef TXTMIRUMENU_ID_H
#  undef __TXTMIRUFUNC_H__
#  include "TxtMiruFunc.h"
#  define TXTMIRUFUNCNAMELIST static UINT l_TxtMiruFuncMenuIDList[] =
#  define TXTMIRUFL(__a__,__b__,__c__) __c__
#  define TXTMIRUFUNCNAMELIST_END(__b__,__c__) __c__
#  define TXTMIRUFUNCNAMELIST_EXTERN
# endif
#else
# ifndef TXTMIRUFUNC_ENUM
#  define TXTMIRUFUNC_ENUM
#  define TXTMIRUFUNCNAMELIST enum FuncNameID
#  define TXTMIRUFL(__a__,__b__,__c__) FnN_##__a__
#  define TXTMIRUFUNCNAMELIST_END(__b__,__c__)    FnN_MaxNum
#  define TXTMIRUFUNCNAMELIST_EXTERN extern LPCTSTR l_TxtMiruFuncNameList  [FnN_MaxNum];
# endif
#endif

#ifdef TXTMIRUFUNCNAMELIST
namespace TxtMiru
{
	TXTMIRUFUNCNAMELIST
	{
		TXTMIRUFL(ReadFile            ,IDS_READFILE            ,IDOPEN                ),TXTMIRUFL(ReadClipboard   ,IDS_READCLIPBOAD        ,IDOPENCLIPBOARD       ),
		TXTMIRUFL(Reload              ,IDS_RELOAD              ,IDRELOAD              ),TXTMIRUFL(URLOpen         ,IDS_URLOPEN             ,IDURLOPEN             ),
		TXTMIRUFL(OpenBrowser         ,IDS_OPENBROWSER         ,IDOPENBROWSER         ),
		TXTMIRUFL(NextPage            ,IDS_NEXTPAGE            ,IDNEXTPAGE            ),TXTMIRUFL(PrevPage        ,IDS_PREVPAGE            ,IDPREVPAGE            ),
		TXTMIRUFL(FirstPage           ,IDS_FIRSTPAGE           ,IDTOPPAGE             ),TXTMIRUFL(EndPage         ,IDS_ENDPAGE             ,IDENDPAGE             ),
		TXTMIRUFL(GotoPage            ,IDS_GOTOPAGE            ,IDGOTOPAGE            ),
		TXTMIRUFL(Copy                ,IDS_COPY                ,IDCOPY                ),TXTMIRUFL(Save            ,0                       ,IDSAVE                ),
		TXTMIRUFL(SaveAsBookmark      ,0                       ,IDBOOKMARKSAVEAS      ),TXTMIRUFL(AddBookmark     ,IDS_ADDBOOKMARK         ,IDADDBOOKMARK         ),
		TXTMIRUFL(GotoBookmark1       ,IDS_GOTOBOOKMARK1       ,IDGOBOOKMARK1         ),TXTMIRUFL(GotoBookmark2   ,IDS_GOTOBOOKMARK2       ,IDGOBOOKMARK2         ),
		TXTMIRUFL(GotoBookmark3       ,IDS_GOTOBOOKMARK3       ,IDGOBOOKMARK3         ),TXTMIRUFL(GotoBookmark4   ,IDS_GOTOBOOKMARK4       ,IDGOBOOKMARK4         ),
		TXTMIRUFL(GotoBookmark5       ,IDS_GOTOBOOKMARK5       ,IDGOBOOKMARK5         ),TXTMIRUFL(GotoBookmark6   ,IDS_GOTOBOOKMARK6       ,IDGOBOOKMARK6         ),
		TXTMIRUFL(GotoBookmark7       ,IDS_GOTOBOOKMARK7       ,IDGOBOOKMARK7         ),TXTMIRUFL(GotoBookmark8   ,IDS_GOTOBOOKMARK8       ,IDGOBOOKMARK8         ),
		TXTMIRUFL(GotoBookmark9       ,IDS_GOTOBOOKMARK9       ,IDGOBOOKMARK9         ),TXTMIRUFL(GotoBookmark0   ,IDS_GOTOBOOKMARK0       ,IDGOBOOKMARK0         ),
		TXTMIRUFL(ShowBookmark        ,IDS_SHOWBOOKMARK        ,IDBOOKMARK            ),
		TXTMIRUFL(Search              ,IDS_SEARCH              ,IDSEARCH              ),TXTMIRUFL(SearchNext      ,IDS_SEARCHNEXT          ,IDSEARCHNEXT          ),
		TXTMIRUFL(SearchPrev          ,IDS_SEARCHPREV          ,IDSEARCHPREV          ),TXTMIRUFL(Config          ,IDS_CONFIG              ,IDCONFIG              ),
		TXTMIRUFL(Exit                ,IDS_EXIT                ,IDCLOSE               ),TXTMIRUFL(Version         ,IDS_SHOW_VERSION        ,IDVERSION             ),
		TXTMIRUFL(ShowHScrollBar      ,IDS_SHOW_HSCROLLBAR     ,0                     ),TXTMIRUFL(ShowVScrollBar  ,IDS_SHOW_VSCROLLBAR     ,0                     ),
		TXTMIRUFL(HideHScrollBar      ,IDS_HIDE_HSCROLLBAR     ,0                     ),TXTMIRUFL(HideVScrollBar  ,IDS_HIDE_VSCROLLBAR     ,0                     ),
		TXTMIRUFL(ToggleHScrollBar    ,IDS_TOGGLE_HSCROLLBAR   ,IDVISIBLEHSCROLLBAR   ),TXTMIRUFL(ToggleVScrollBar,IDS_TOGGLE_VSCROLLBAR   ,IDVISIBLEVSCROLLBAR   ),
		TXTMIRUFL(Help                ,IDS_SHOW_HELP           ,IDTXTMIRUHELP         ),TXTMIRUFL(ShowSubtitle    ,IDS_SHOWSUBTITLE        ,IDSUBTITLE            ),
		TXTMIRUFL(ToggleLupe          ,IDS_TOGGLE_LUPE         ,IDLUPE                ),TXTMIRUFL(ToggleLupePos   ,0                       ,IDLUPEWINDOWPOS       ),
		TXTMIRUFL(ShowLupe            ,IDS_SHOW_LUPE           ,0                     ),TXTMIRUFL(HideLupe        ,IDS_HIDE_LUPE           ,0                     ),
		TXTMIRUFL(SetLupeZoom100      ,IDS_LUPE100             ,IDLUPE100             ),TXTMIRUFL(SetLupeZoom150  ,IDS_LUPE150             ,IDLUPE150             ),
		TXTMIRUFL(SetLupeZoom200      ,IDS_LUPE200             ,IDLUPE200             ),TXTMIRUFL(SetLupeZoom400  ,IDS_LUPE400             ,IDLUPE400             ),
		TXTMIRUFL(FullScreen          ,IDS_FULLSCREEN          ,IDFULLSCREEN          ),
		TXTMIRUFL(ForwardPage         ,IDS_FORWARDPAGE         ,IDFORWARDPAGE         ),TXTMIRUFL(BackPage        ,IDS_BACKPAGE            ,IDBACKPAGE            ),
		TXTMIRUFL(LayoutSet           ,IDS_LAYOUTSET           ,IDLAYOUTSET           ),TXTMIRUFL(ShowProperty    ,IDS_SHOWPROPERTY        ,IDSHOWPROPERTY        ),
		TXTMIRUFL(ShowDocInfo         ,IDS_SHOWDOCINFO         ,IDSHOWDOCINFO         ),TXTMIRUFL(Nop             ,0                       ,0                     ),
		TXTMIRUFL(RefreshPreParserList,IDS_REFRESHPREPARSERLIST,IDREFRESHPREPARSERLIST),
		TXTMIRUFL(NextFile            ,IDS_NEXTFILE            ,IDNEXTFILE            ),TXTMIRUFL(PrevFile        ,IDS_PREVFILE            ,IDPREVFILE            ),
		TXTMIRUFL(ShowSubtitleBookmark,IDS_SHOWSUBTITLEBOOKMARK,IDSUBTITLEBOOKMARK    ),
		TXTMIRUFL(ShowBookList        ,IDS_FAVORITE            ,IDFAVORITE            ),
		TXTMIRUFL(ToggleCopyRuby      ,IDS_TOGGLECOPYRUBY      ,IDCOPYRUBY            ),
		TXTMIRUFL(ExecOpenFiile       ,IDS_OPENFILEEXE         ,IDOPENFILEEXE         ),
		TXTMIRUFL(ExecOpenFiile1      ,IDS_OPENFILEEXE1        ,IDOPENFILEEXE1        ),
		TXTMIRUFL(ExecOpenFiile2      ,IDS_OPENFILEEXE2        ,IDOPENFILEEXE2        ),
		TXTMIRUFL(ShowAozoraList      ,IDS_BMAOZORA            ,IDBMAOZORA            ),
		TXTMIRUFL(TM_FreeLibrary      ,0                       ,0                     ),
		TXTMIRUFL(LinkGoto            ,IDS_LINKGOTO            ,0                     ),
		TXTMIRUFL(LinkOpen            ,IDS_LINKOPEN            ,0                     ),
		TXTMIRUFL(ToggleSelectionMode ,IDS_SELECTIONMODE       ,IDSELECTIONMODE       ),
		TXTMIRUFL(ShowContextMenu     ,0                       ,0                     ),
		TXTMIRUFL(SearchFiles         ,IDS_SEARCHFILES         ,IDSEARCHFILES         ),
		TXTMIRUFL(ShowRubyList        ,IDS_SHOWRUBYLIST        ,IDRUBYLIST            ),
		TXTMIRUFL(OpenFolder          ,IDS_OPENFOLDER          ,IDOPENFOLDER          ),
		TXTMIRUFL(AddFavorite         ,IDS_ADDFAVORITE         ,0                     ),
		TXTMIRUFUNCNAMELIST_END(       IDS_CANCEL              ,0                     )
		};
	TXTMIRUFUNCNAMELIST_EXTERN;
};
#endif

#ifdef TXTMIRUFUNCNAMELIST
# undef TXTMIRUFUNCNAMELIST
# undef TXTMIRUFL
# undef TXTMIRUFUNCNAMELIST_END
# undef TXTMIRUFUNCNAMELIST_EXTERN
#endif

#endif // __TXTMIRUFUNC_H__
