#ifndef __TXTFUNCBOOKMARKDEF_H__
#define __TXTFUNCBOOKMARKDEF_H__

namespace TxtFuncBookmark
{
	enum ImageTreeIcon {
		iti_stat_tree_collapsed         ,
		iti_stat_tree_expanded          ,
		iti_stat_tree_none              ,
		iti_max
	};
	enum ImageIcon {
		ii_stat_tag_favorite           ,
		ii_stat_book_normal            ,
		ii_stat_book_normal_update     ,
		ii_stat_book_current           ,
		ii_stat_book_current_update    ,
		ii_stat_book_check             ,
		ii_stat_bookmark_normal        ,
		ii_stat_bookmark_normal_update ,
		ii_stat_bookmark_current       ,
		ii_stat_bookmark_current_update,
		ii_stat_link_normal            ,
		ii_stat_link_normal_update     ,
		ii_stat_link_current           ,
		ii_stat_link_current_update    ,
		ii_tag_add                     ,
		ii_tag_modify                  ,
		ii_tag_delete                  ,
		ii_tag_up                      ,
		ii_tag_down                    ,
		ii_book_add                    ,
		ii_book_modify                 ,
		ii_book_delete                 ,
		ii_book_update                 ,
		ii_book_update_all             ,
		ii_book_up                     ,
		ii_book_down                   ,
		ii_book_open                   ,
		ii_book_abort                  ,
		ii_subtitle_open               ,
		ii_stay_on_top                 ,
		ii_bookmark                    ,
		ii_max
	};
	struct Tag {
		int          id             = 0;
		int          place_id       = 0;
		std::tstring title          ;
		int          position       = 0;
	};
	struct Book {
		int          id             = 0;
		int          place_id       = 0;
		std::tstring url            ;
		std::tstring title          ;
		std::tstring author         ;
		int          page           = 0;
		int          read_page      = 0;
		int          visit_count    = 0;
		int          read_cnt       = 0;
		int          total_cnt      = 0;
		int          position       = 0;
		std::tstring insert_date    ;
		std::tstring last_visit_date;
	};
	struct Subtitle {
		int          id             = 0;
		int          b_order        = 0;
		std::tstring title          ;
		std::tstring url            ;
		int          page           = 0;
		int          read_page      = 0;
		int          level          = 0;
		std::tstring last_visit_date;
	};
	struct TagDragInfo {
		int num = 0;
		int id_list[1] = {};
	};
	struct BookDragInfo {
		int num = 0;
		int tag_id = 0;
		int id_list[1] = {};
	};
	enum EventID {
		IDGOTOPAGE = 40300,
		IDADDBOOKMARK,
		IDDELBOOKMARK,
		IDLINKSTAY   ,
		IDBOOKMARK   ,
		IDSUBTITLE   ,
		IDAUTOSCROLL ,
	};
	enum ModelWindowID {
		MWID_LINKDLG,
		MWID_BOOKDLG,
		MWID_SEARCHDLG,
		MWID_RUBYDLG,
	};
};

#define STR_PARAMTYPE_AUTHOR        _T("Author"       )
#define STR_PARAMTYPE_BMFILENAME    _T("BMFileName"   )
#define STR_PARAMTYPE_FILENAME      _T("FileName"     )
#define STR_PARAMTYPE_IS_WAITTING   _T("IsWaitting"   )
#define STR_PARAMTYPE_LASTWRITETIME _T("LastWriteTime")
#define STR_PARAMTYPE_PAGE          _T("Page"         )
#define STR_PARAMTYPE_TITLE         _T("Title"        )
#define STR_PARAMTYPE_VER           _T("Ver"          )

#include "ModelessWnd.h"
CGrModelessWnd* GetInstallWnd(TxtFuncBookmark::ModelWindowID id);
bool InstallWnd(TxtFuncBookmark::ModelWindowID id, CGrModelessWnd *pWnd);
bool UnInstallWnd(TxtFuncBookmark::ModelWindowID id);

#endif // __TXTFUNCBOOKMARKDEF_H__
