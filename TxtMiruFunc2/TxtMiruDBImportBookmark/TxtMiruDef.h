#ifndef __TXTMIRUDEF_H__
#define __TXTMIRUDEF_H__

#include <windows.h>
#include "stltchar.h"
#include <vector>
#include <list>
#include <map>

#define ARCFILE_SPLIT_CHAR _T("|") // URI, ファイル名で使用できない文字を区切り文字にする

namespace BookmarkDB
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
		ii_subtitle_open               ,
		ii_max
	};
	struct Place {
		int          id             = 0;
		std::tstring url            ;
		std::tstring title          ;
		std::tstring author         ;
		int          page           = 0;
		int          read_page      = 0;
		int          visit_count    = 0;
		std::tstring insert_date    ;
		std::tstring last_visit_date;
	};
	struct History {
		int          id             = 0;
		int          place_id       = 0;
		std::tstring url            ;
		std::tstring title          ;
		std::tstring author         ;
		int          page           = 0;
		int          read_page      = 0;
		std::tstring insert_date    ;
		std::tstring last_visit_date;
	};
	struct Bookmark {
		int          id             = 0;
		int          place_id       = 0;
		std::tstring title          ;
		int          page           = 0;
		int          b_line         = 0;
		int          b_index        = 0;
		int          b_pos          = 0;
		std::tstring insert_date    ;
	};
	struct Subtitle {
		int          id             = 0;
		int          place_id       = 0;
		int          b_order        = 0;
		std::tstring title          ;
		std::tstring url            ;
		int          page           = 0;
		int          b_line         = 0;
		int          b_index        = 0;
		int          b_pos          = 0;
		int          level          = 0;
		std::tstring insert_date    ;
	};
	struct Book {
		int          id             = 0;
		int          place_id       = 0;
		int          parent         = 0; // NULL IS TOP ITEM
		int          type           = 0; // 0:ITEM, 1:FOLDER
		int          position       = 0;
		std::tstring base_url       ;
		std::tstring title          ;
		std::tstring author         ;
		std::tstring insert_date    ;
	};
	using BookList = std::list<Book *>;
};

#endif // __TXTMIRUDEF_H__
