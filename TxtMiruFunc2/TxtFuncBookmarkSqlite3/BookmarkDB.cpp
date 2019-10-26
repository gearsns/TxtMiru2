#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include <commctrl.h>
#ifndef FACILITY_VISUALCPP
#define FACILITY_VISUALCPP  ((LONG)0x6d)
#endif
#include <DelayImp.h>
#include "resource.h"
#include "Shlwapi.h"
#include "BookmarkDB.h"
#include "TxtFuncIBookmark.h"
#include "TxtFuncBookmark.h"
#include "TxtFunc.h"
#include "Text.h"
#include "CSVText.h"
#include "Shell.h"
//#define __DBG__
#include "Debug.h"

////////////////////////////////////////////////////////////
// lib /def:sqlite3.def /machine:x86
static DWORD DelayLoadExFilter(DWORD ec, EXCEPTION_POINTERS *ep)
{
	DelayLoadInfo *dli = nullptr;
	DelayLoadProc *dlp = nullptr;

	if (ec == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND)) {
	} else if (ec == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND)) {
	} else {
		// 遅延読み込みの例外でない場合は上位の例外ハンドラへ
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// 遅延読み込み用の例外情報を取得
	dli = reinterpret_cast<DelayLoadInfo*>(ep->ExceptionRecord->ExceptionInformation[0]);
	dlp = &(dli->dlp);
	if (dlp->fImportByName) {
	} else {
	}

	// 処理続行
	return EXCEPTION_EXECUTE_HANDLER;
}

namespace CGrDBFunc
{
	bool isExistsType(sqlite3 *pSqlite3, LPCTSTR lpType, LPCTSTR lpTableName)
	{
		bool bret = true;
		sqlite3_stmt *pstmt = nullptr;
		do {
			if(!pSqlite3){
				bret = false;
				break;
			}
			int ret = sqlite3_prepare16(pSqlite3, _T("SELECT COUNT(*) FROM SQLITE_MASTER WHERE TYPE=? AND NAME=?"), -1, &pstmt, nullptr);
			if(ret != SQLITE_OK || !pstmt) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt);
			ret = sqlite3_bind_text16(pstmt, 1, lpType     , -1, SQLITE_STATIC);
			ret = sqlite3_bind_text16(pstmt, 2, lpTableName, -1, SQLITE_STATIC);
			if(SQLITE_OK != ret){
				bret = false;
				break;
			}
			// 実行
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				bret = false;
				break;
			}
			int cnt = sqlite3_column_int(pstmt, 0);
			if(cnt > 0){
				bret = true;
			} else {
				bret = false;
			}
			sqlite3_clear_bindings(pstmt);
		} while(0);

		if(pstmt){
			sqlite3_finalize(pstmt);
		}
		return bret;
	}
	bool IsExistsTable(sqlite3 *pSqlite3, LPCTSTR lpTableName)
	{
		return isExistsType(pSqlite3, _T("table"), lpTableName);
	}
	bool IsExistsIndex(sqlite3 *pSqlite3, LPCTSTR lpTableName)
	{
		return isExistsType(pSqlite3, _T("index"), lpTableName);
	}
	bool createTable(sqlite3 *pSqlite3, LPCTSTR table_name, LPCSTR sql)
	{
		if(!IsExistsTable(pSqlite3, table_name)){
			// テーブルの作成
			int ret = sqlite3_exec(pSqlite3, sql, nullptr, nullptr, nullptr);
			if(ret != SQLITE_OK){
				return false;
			}
		}
		return true;
	}
	bool createIndex(sqlite3 *pSqlite3, LPCTSTR index_name, LPCSTR sql)
	{
		if(!IsExistsIndex(pSqlite3, index_name)){
			int ret = sqlite3_exec(pSqlite3, sql, nullptr, nullptr, nullptr);
			if(ret != SQLITE_OK){
				return false;
			}
		}
		return true;
	}
	void GetSysDate(std::tstring &out_sysdate)
	{
		time_t now;

		time(&now);
		struct tm ltm = {};
		if(0 == localtime_s(&ltm, &now)){
			TCHAR date_str[512] = {};
			_tcsftime(date_str, sizeof(date_str)/sizeof(TCHAR), _T("%Y/%m/%d %X"), &ltm);
			out_sysdate = date_str;
		}
	}
	void GetValue(sqlite3_stmt *pstmt, int col, std::tstring &out_value)
	{
		const void* val = sqlite3_column_text16(pstmt, col);
		if(val){
			out_value = static_cast<LPCTSTR>(val);
		} else {
			out_value = _T("");
		}
	}
	void GetValue(sqlite3_stmt *pstmt, int col, int &out_value)
	{
		auto* val = sqlite3_column_value(pstmt, col);
		if(sqlite3_value_type(val) == SQLITE_NULL){
			out_value = NULL_INT_VALUE;
		} else {
			out_value = sqlite3_value_int(val);
		}
	}
	void PutValue(sqlite3_stmt *pstmt, LPCSTR field, LPCTSTR lpValue)
	{
		if(lpValue){
			sqlite3_bind_text16(pstmt, sqlite3_bind_parameter_index(pstmt, field), lpValue, -1, SQLITE_STATIC);
		} else {
			sqlite3_bind_null(pstmt, sqlite3_bind_parameter_index(pstmt, field));
		}
	}
	void PutValue(sqlite3_stmt *pstmt, LPCSTR field, const std::tstring &str)
	{
		PutValue(pstmt, field, str.c_str());
	}
	void PutValue(sqlite3_stmt *pstmt, LPCSTR field, int iValue)
	{
		if(iValue == NULL_INT_VALUE){
			sqlite3_bind_null(pstmt, sqlite3_bind_parameter_index(pstmt, field));
		} else {
			sqlite3_bind_int(pstmt, sqlite3_bind_parameter_index(pstmt, field), iValue);
		}
	}
}

CGrDBPlaces::CGrDBPlaces(){}
CGrDBPlaces::~CGrDBPlaces(){}
bool CGrDBPlaces::Create(sqlite3 *pSqlite3)
{
	bool ret = true;
	if(ret){
		ret = CGrDBFunc::createTable(pSqlite3,
									 _T("TXTMIRU_PLACES"),
									 "CREATE TABLE TXTMIRU_PLACES ("
									 "ID INTEGER,"            // ID
									 "URL TEXT,"              // URL
									 "TITLE TEXT,"            // TITLE
									 "AUTHOR TEXT,"           // AUTHOR
									 "PAGE INTEGER,"          // 総数
									 "READ_PAGE INTEGER,"     // 読んだページ
									 "VISIT_COUNT INTEGER,"   // 回数
									 "INSERT_DATE TEXT,"      //
									 "LAST_VISIT_DATE TEXT,"  //
									 "PRIMARY KEY (ID AUTOINCREMENT));");
	}
	if(ret){
		ret = CGrDBFunc::createIndex(pSqlite3,
									 _T("I1_TXTMIRU_PLACES"),
									 "CREATE INDEX I1_TXTMIRU_PLACES ON TXTMIRU_PLACES (URL);");
	}
	return ret;
}

bool CGrDBPlaces::GetByURL(sqlite3 *pSqlite3, Place &out_place, LPCTSTR lpURL)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,VISIT_COUNT,INSERT_DATE,LAST_VISIT_DATE FROM TXTMIRU_PLACES WHERE URL=@URL"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@URL", lpURL);
		// 実行
		bret = false;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			CGrDBFunc::GetValue(pstmt, 0, out_place.id             );
			CGrDBFunc::GetValue(pstmt, 1, out_place.url            );
			CGrDBFunc::GetValue(pstmt, 2, out_place.title          );
			CGrDBFunc::GetValue(pstmt, 3, out_place.author         );
			CGrDBFunc::GetValue(pstmt, 4, out_place.page           );
			CGrDBFunc::GetValue(pstmt, 5, out_place.read_page      );
			CGrDBFunc::GetValue(pstmt, 6, out_place.visit_count    );
			CGrDBFunc::GetValue(pstmt, 7, out_place.insert_date    );
			CGrDBFunc::GetValue(pstmt, 8, out_place.last_visit_date);
			bret = true;
			break;
		};
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBPlaces::Put(sqlite3 *pSqlite3, Place &in_place)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = 0;
		ret = sqlite3_prepare16(pSqlite3,
								_T("INSERT OR REPLACE INTO TXTMIRU_PLACES (ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,VISIT_COUNT,INSERT_DATE,LAST_VISIT_DATE)")
								_T(" VALUES (@ID,@URL,@TITLE,@AUTHOR,@PAGE,@READ_PAGE,@VISIT_COUNT,@INSERT_DATE,@LAST_VISIT_DATE)")
								, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID"             , in_place.id             );
		CGrDBFunc::PutValue(pstmt, "@URL"            , in_place.url            );
		CGrDBFunc::PutValue(pstmt, "@TITLE"          , in_place.title          );
		CGrDBFunc::PutValue(pstmt, "@AUTHOR"         , in_place.author         );
		CGrDBFunc::PutValue(pstmt, "@PAGE"           , in_place.page           );
		CGrDBFunc::PutValue(pstmt, "@READ_PAGE"      , in_place.read_page      );
		CGrDBFunc::PutValue(pstmt, "@VISIT_COUNT"    , in_place.visit_count    );
		CGrDBFunc::PutValue(pstmt, "@INSERT_DATE"    , in_place.insert_date    );
		CGrDBFunc::PutValue(pstmt, "@LAST_VISIT_DATE", in_place.last_visit_date);
		// 実行
		ret = sqlite3_step(pstmt);
		if(ret != SQLITE_DONE){
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}

CGrDBHistory::CGrDBHistory(){}
CGrDBHistory::~CGrDBHistory(){}
bool CGrDBHistory::Create(sqlite3 *pSqlite3)
{
	bool ret = CGrDBFunc::createTable(pSqlite3,
									  _T("TXTMIRU_HISTORY"),
									  "CREATE TABLE TXTMIRU_HISTORY ("
									  "ID INTEGER,"            // ID
									  "PLACE_ID INTEGER,"      // PLACE_ID
									  "URL TEXT,"              // URL
									  "TITLE TEXT,"            // TITLE
									  "AUTHOR TEXT,"           // AUTHOR
									  "PAGE INTEGER,"          // 総数
									  "READ_PAGE INTEGER,"     // 読んだページ
									  "INSERT_DATE TEXT,"      //
									  "LAST_VISIT_DATE TEXT,"  //
									  "PRIMARY KEY (ID AUTOINCREMENT));");
	if(ret){
		ret = CGrDBFunc::createIndex(pSqlite3,
									 _T("I1_TXTMIRU_HISTORYS"),
									 "CREATE INDEX I1_TXTMIRU_HISTORYS ON TXTMIRU_HISTORY (PLACE_ID,READ_PAGE);");
	}
	return ret;
}

bool CGrDBHistory::Put(sqlite3 *pSqlite3, History &in_history)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3,
								_T("INSERT OR REPLACE INTO TXTMIRU_HISTORY (ID,PLACE_ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,INSERT_DATE,LAST_VISIT_DATE)")
								_T(" VALUES (@ID,@PLACE_ID,@URL,@TITLE,@AUTHOR,@PAGE,@READ_PAGE,@INSERT_DATE,@LAST_VISIT_DATE)")
								, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID"             , in_history.id             );
		CGrDBFunc::PutValue(pstmt, "@PLACE_ID"       , in_history.place_id       );
		CGrDBFunc::PutValue(pstmt, "@URL"            , in_history.url            );
		CGrDBFunc::PutValue(pstmt, "@TITLE"          , in_history.title          );
		CGrDBFunc::PutValue(pstmt, "@AUTHOR"         , in_history.author         );
		CGrDBFunc::PutValue(pstmt, "@PAGE"           , in_history.page           );
		CGrDBFunc::PutValue(pstmt, "@READ_PAGE"      , in_history.read_page      );
		CGrDBFunc::PutValue(pstmt, "@INSERT_DATE"    , in_history.insert_date    );
		CGrDBFunc::PutValue(pstmt, "@LAST_VISIT_DATE", in_history.last_visit_date);
		// 実行
		ret = sqlite3_step(pstmt);
		if(ret != SQLITE_DONE){
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
CGrDBBookmarks::CGrDBBookmarks(){}
CGrDBBookmarks::~CGrDBBookmarks(){}
bool CGrDBBookmarks::Create(sqlite3 *pSqlite3)
{
	return CGrDBFunc::createTable(pSqlite3,
								  _T("TXTMIRU_BOOKMARKS"),
								  "CREATE TABLE TXTMIRU_BOOKMARKS ("
								  "ID INTEGER,"            // ID
								  "PLACE_ID INTEGER,"      // PLACE_ID
								  "TITLE TEXT,"            // TITLE
								  "PAGE INTEGER,"          // PAGE
								  "B_LINE INTEGER,"        // LINE
								  "B_INDEX INTEGER,"       // INDEX
								  "B_POS INTEGER,"         // POS
								  "INSERT_DATE TEXT,"      //
								  "PRIMARY KEY (ID AUTOINCREMENT));");
}
bool CGrDBBookmarks::GetByPlaceID(sqlite3 *pSqlite3, std::vector<Bookmark> &out_bookmark_list, int iPlaceID)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,PLACE_ID,TITLE,PAGE,B_LINE,B_INDEX,B_POS,INSERT_DATE FROM TXTMIRU_BOOKMARKS WHERE PLACE_ID=@PLACE_ID"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@PLACE_ID", iPlaceID);
		// 実行
		bret = false;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			Bookmark item;
			CGrDBFunc::GetValue(pstmt, 0, item.id         );
			CGrDBFunc::GetValue(pstmt, 1, item.place_id   );
			CGrDBFunc::GetValue(pstmt, 2, item.title      );
			CGrDBFunc::GetValue(pstmt, 3, item.page       );
			CGrDBFunc::GetValue(pstmt, 4, item.b_line     );
			CGrDBFunc::GetValue(pstmt, 5, item.b_index    );
			CGrDBFunc::GetValue(pstmt, 6, item.b_pos      );
			CGrDBFunc::GetValue(pstmt, 7, item.insert_date);
			out_bookmark_list.push_back(item);
			bret = true;
		};
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBBookmarks::Put(sqlite3 *pSqlite3, Bookmark &in_bookmark)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3,
								_T("INSERT OR REPLACE INTO TXTMIRU_BOOKMARKS (ID,PLACE_ID,TITLE,PAGE,B_LINE,B_INDEX,B_POS,INSERT_DATE)")
								_T(" VALUES (@ID,@PLACE_ID,@TITLE,@PAGE,@B_LINE,@B_INDEX,@B_POS,@INSERT_DATE)")
								, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID"         , in_bookmark.id         );
		CGrDBFunc::PutValue(pstmt, "@PLACE_ID"   , in_bookmark.place_id   );
		CGrDBFunc::PutValue(pstmt, "@TITLE"      , in_bookmark.title      );
		CGrDBFunc::PutValue(pstmt, "@PAGE"       , in_bookmark.page       );
		CGrDBFunc::PutValue(pstmt, "@B_LINE"     , in_bookmark.b_line     );
		CGrDBFunc::PutValue(pstmt, "@B_INDEX"    , in_bookmark.b_index    );
		CGrDBFunc::PutValue(pstmt, "@B_POS"      , in_bookmark.b_pos      );
		CGrDBFunc::PutValue(pstmt, "@INSERT_DATE", in_bookmark.insert_date);
		// 実行
		ret = sqlite3_step(pstmt);
		if(ret != SQLITE_DONE){
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBBookmarks::Delete(sqlite3 *pSqlite3, int iID)
{
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;

	do {
		int ret = sqlite3_prepare16(pSqlite3,
									_T("DELETE FROM TXTMIRU_BOOKMARKS")
									_T(" WHERE ID=@ID")
									, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID"         , iID);
		// 実行
		ret = sqlite3_step(pstmt);
		if(ret != SQLITE_DONE){
			bret = false;
			break;
		}
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
//
CGrDBSubtitles::CGrDBSubtitles(){}
CGrDBSubtitles::~CGrDBSubtitles(){}
bool CGrDBSubtitles::Create(sqlite3 *pSqlite3)
{
	bool ret = true;
	if(ret){
		ret = CGrDBFunc::createTable(pSqlite3,
									 _T("TXTMIRU_SUBTITLES"),
									 "CREATE TABLE TXTMIRU_SUBTITLES ("
									 "ID INTEGER,"            // ID
									 "PLACE_ID INTEGER,"      // PLACE_ID
									 "B_ORDER INTEGER,"       // ORDER
									 "TITLE TEXT,"            // TITLE
									 "URL TEXT,"              // URL
									 "PAGE INTEGER,"          // PAGE
									 "B_LINE INTEGER,"        // LINE
									 "B_INDEX INTEGER,"       // INDEX
									 "B_POS INTEGER,"         // POS
									 "LEVEL INTEGER,"         // LEVEL
									 "INSERT_DATE TEXT,"      // INSERT_DATE
									 "PRIMARY KEY (ID AUTOINCREMENT));");
	}
	if(ret){
		ret = CGrDBFunc::createIndex(pSqlite3,
									 _T("I1_TXTMIRU_SUBTITLES"),
									 "CREATE INDEX I1_TXTMIRU_SUBTITLES ON TXTMIRU_SUBTITLES (PLACE_ID,URL,PAGE,B_ORDER);");
	}
	return ret;
}
bool CGrDBSubtitles::GetByPlaceID(sqlite3 *pSqlite3, std::vector<Subtitle> &out_subtitle_list, int iPlaceID)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,PLACE_ID,B_ORDER,TITLE,URL,PAGE,B_LINE,B_INDEX,B_POS,LEVEL,INSERT_DATE FROM TXTMIRU_SUBTITLES WHERE PLACE_ID=@PLACE_ID"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@PLACE_ID", iPlaceID);
		// 実行
		bret = false;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			Subtitle item;
			CGrDBFunc::GetValue(pstmt, 0, item.id         );
			CGrDBFunc::GetValue(pstmt, 1, item.place_id   );
			CGrDBFunc::GetValue(pstmt, 2, item.b_order    );
			CGrDBFunc::GetValue(pstmt, 3, item.title      );
			CGrDBFunc::GetValue(pstmt, 4, item.url        );
			CGrDBFunc::GetValue(pstmt, 5, item.page       );
			CGrDBFunc::GetValue(pstmt, 6, item.b_line     );
			CGrDBFunc::GetValue(pstmt, 7, item.b_index    );
			CGrDBFunc::GetValue(pstmt, 8, item.b_pos      );
			CGrDBFunc::GetValue(pstmt, 9, item.level      );
			CGrDBFunc::GetValue(pstmt,10, item.insert_date);
			out_subtitle_list.push_back(item);
			bret = true;
		};
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}

bool CGrDBSubtitles::Put(sqlite3 *pSqlite3, Subtitle &in_subtitle)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3,
								_T("INSERT OR REPLACE INTO TXTMIRU_SUBTITLES (ID,PLACE_ID,B_ORDER,TITLE,URL,PAGE,B_LINE,B_INDEX,B_POS,LEVEL,INSERT_DATE)")
								_T(" VALUES (@ID,@PLACE_ID,@B_ORDER,@TITLE,@URL,@PAGE,@B_LINE,@B_INDEX,@B_POS,@LEVEL,@INSERT_DATE)")
								, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID"         , in_subtitle.id         );
		CGrDBFunc::PutValue(pstmt, "@PLACE_ID"   , in_subtitle.place_id   );
		CGrDBFunc::PutValue(pstmt, "@B_ORDER"    , in_subtitle.b_order    );
		CGrDBFunc::PutValue(pstmt, "@TITLE"      , in_subtitle.title      );
		CGrDBFunc::PutValue(pstmt, "@URL"        , in_subtitle.url        );
		CGrDBFunc::PutValue(pstmt, "@PAGE"       , in_subtitle.page       );
		CGrDBFunc::PutValue(pstmt, "@B_LINE"     , in_subtitle.b_line     );
		CGrDBFunc::PutValue(pstmt, "@B_INDEX"    , in_subtitle.b_index    );
		CGrDBFunc::PutValue(pstmt, "@B_POS"      , in_subtitle.b_pos      );
		CGrDBFunc::PutValue(pstmt, "@LEVEL"      , in_subtitle.level      );
		CGrDBFunc::PutValue(pstmt, "@INSERT_DATE", in_subtitle.insert_date);
		// 実行
		ret = sqlite3_step(pstmt);
		if(ret != SQLITE_DONE){
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBSubtitles::Delete(sqlite3 *pSqlite3, int iID)
{
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;

	do {
		int ret = sqlite3_prepare16(pSqlite3,
									_T("DELETE FROM TXTMIRU_SUBTITLES")
									_T(" WHERE ID=@ID")
									, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID", iID);
		// 実行
		ret = sqlite3_step(pstmt);
		if(ret != SQLITE_DONE){
			bret = false;
			break;
		}
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
CGrDBBooks::CGrDBBooks(){}
CGrDBBooks::~CGrDBBooks(){}
bool CGrDBBooks::Create(sqlite3 *pSqlite3)
{
	bool ret = true;
	if(ret){
		ret = CGrDBFunc::createTable(pSqlite3,
									 _T("TXTMIRU_BOOKS"),
									 "CREATE TABLE TXTMIRU_BOOKS ("
									 "ID INTEGER,"            // ID
									 "PLACE_ID INTEGER,"      // PLACE_ID
									 "PARENT INTEGER,"        // PARENT
									 "TYPE INTEGER,"          // TYPE
									 "POSITION INTEGER,"      // POSITION
									 "BASE_URL TEXT,"         // BASE_URL
									 "TITLE TEXT,"            // TITLE
									 "AUTHOR TEXT,"           // AUTHOR
									 "INSERT_DATE TEXT,"      // INSERT_DATE
									 "PRIMARY KEY (ID AUTOINCREMENT));");
	}
	if(ret){
		ret = CGrDBFunc::createIndex(pSqlite3,
									 _T("I1_TXTMIRU_BOOKS"),
									 "CREATE INDEX I1_TXTMIRU_BOOKS ON TXTMIRU_BOOKS (PARENT);");
	}
	return ret;
}
bool CGrDBBooks::Get(sqlite3 *pSqlite3, Book &book, int iID)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,PLACE_ID,PARENT,TYPE,POSITION,BASE_URL,TITLE,AUTHOR,INSERT_DATE FROM TXTMIRU_BOOKS WHERE ID=@ID"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID", iID);
		// 実行
		bret = false;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			CGrDBFunc::GetValue(pstmt, 0, book.id         );
			CGrDBFunc::GetValue(pstmt, 1, book.place_id   );
			CGrDBFunc::GetValue(pstmt, 2, book.parent     );
			CGrDBFunc::GetValue(pstmt, 3, book.type       );
			CGrDBFunc::GetValue(pstmt, 4, book.position   );
			CGrDBFunc::GetValue(pstmt, 5, book.base_url   );
			CGrDBFunc::GetValue(pstmt, 6, book.title      );
			CGrDBFunc::GetValue(pstmt, 7, book.author     );
			CGrDBFunc::GetValue(pstmt, 8, book.insert_date);
			bret = true;
			break;
		};
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBBooks::GetLastInsertRow(sqlite3 *pSqlite3, Book &book)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,PLACE_ID,PARENT,TYPE,POSITION,BASE_URL,TITLE,AUTHOR,INSERT_DATE FROM TXTMIRU_BOOKS WHERE ROWID = last_insert_rowid()"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		// 実行
		bret = false;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			CGrDBFunc::GetValue(pstmt, 0, book.id         );
			CGrDBFunc::GetValue(pstmt, 1, book.place_id   );
			CGrDBFunc::GetValue(pstmt, 2, book.parent     );
			CGrDBFunc::GetValue(pstmt, 3, book.type       );
			CGrDBFunc::GetValue(pstmt, 4, book.position   );
			CGrDBFunc::GetValue(pstmt, 5, book.base_url   );
			CGrDBFunc::GetValue(pstmt, 6, book.title      );
			CGrDBFunc::GetValue(pstmt, 7, book.author     );
			CGrDBFunc::GetValue(pstmt, 8, book.insert_date);
			bret = true;
			break;
		};
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBBooks::GetByBaseURL(sqlite3 *pSqlite3, Book &book, LPCTSTR lpBaseURL, int iType)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,PLACE_ID,PARENT,TYPE,POSITION,BASE_URL,TITLE,AUTHOR,INSERT_DATE FROM TXTMIRU_BOOKS WHERE BASE_URL=@BASE_URL AND TYPE=@TYPE"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@BASE_URL", lpBaseURL);
		CGrDBFunc::PutValue(pstmt, "@TYPE"    , iType    );
		// 実行
		bret = false;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			CGrDBFunc::GetValue(pstmt, 0, book.id         );
			CGrDBFunc::GetValue(pstmt, 1, book.place_id   );
			CGrDBFunc::GetValue(pstmt, 2, book.parent     );
			CGrDBFunc::GetValue(pstmt, 3, book.type       );
			CGrDBFunc::GetValue(pstmt, 4, book.position   );
			CGrDBFunc::GetValue(pstmt, 5, book.base_url   );
			CGrDBFunc::GetValue(pstmt, 6, book.title      );
			CGrDBFunc::GetValue(pstmt, 7, book.author     );
			CGrDBFunc::GetValue(pstmt, 8, book.insert_date);
			bret = true;
			break;
		};
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBBooks::MaxPosition(sqlite3 *pSqlite3, int iParent, int &out_position)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = 0;
		if(iParent == NULL_INT_VALUE){
			ret = sqlite3_prepare16(pSqlite3, _T("SELECT MAX(POSITION) FROM TXTMIRU_BOOKS WHERE PARENT IS NULL"), -1, &pstmt, nullptr);
		} else {
			ret = sqlite3_prepare16(pSqlite3, _T("SELECT MAX(POSITION) FROM TXTMIRU_BOOKS WHERE PARENT=@PARENT"), -1, &pstmt, nullptr);
		}
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@PARENT", iParent);
		// 実行
		bret = false;
		out_position = 0;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			CGrDBFunc::GetValue(pstmt, 0, out_position);
			bret = true;
		};
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBBooks::Put(sqlite3 *pSqlite3, const Book &in_book)
{
	if(!pSqlite3){
		return false;
	}
	bool bret = false;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3,
								_T("INSERT OR REPLACE INTO TXTMIRU_BOOKS (ID,PLACE_ID,PARENT,TYPE,POSITION,BASE_URL,TITLE,AUTHOR,INSERT_DATE)")
								_T(" VALUES (@ID,@PLACE_ID,@PARENT,@TYPE,@POSITION,@BASE_URL,@TITLE,@AUTHOR,@INSERT_DATE)")
								, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID"         , in_book.id         );
		CGrDBFunc::PutValue(pstmt, "@PLACE_ID"   , in_book.place_id   );
		CGrDBFunc::PutValue(pstmt, "@PARENT"     , in_book.parent     );
		CGrDBFunc::PutValue(pstmt, "@TYPE"       , in_book.type       );
		CGrDBFunc::PutValue(pstmt, "@POSITION"   , in_book.position   );
		CGrDBFunc::PutValue(pstmt, "@BASE_URL"   , in_book.base_url   );
		CGrDBFunc::PutValue(pstmt, "@TITLE"      , in_book.title      );
		CGrDBFunc::PutValue(pstmt, "@AUTHOR"     , in_book.author     );
		CGrDBFunc::PutValue(pstmt, "@INSERT_DATE", in_book.insert_date);
		// 実行
		if(sqlite3_step(pstmt) != SQLITE_DONE){
			break;
		}
		sqlite3_clear_bindings(pstmt);
		bret = true;
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBBooks::Delete(sqlite3 *pSqlite3, int iID)
{
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;

	do {
		int ret = sqlite3_prepare16(pSqlite3,
									_T("DELETE FROM TXTMIRU_BOOKS")
									_T(" WHERE ID=@ID")
									, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID", iID);
		// 実行
		ret = sqlite3_step(pstmt);
		if(ret != SQLITE_DONE){
			bret = false;
			break;
		}
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBBooks::GetPositionList(sqlite3 *pSqlite3, int parent_id, int insert_id, int id, int type, std::vector<::Book> &id_list)
{
	bool bRet = true;
	sqlite3_stmt *pstmt = nullptr;

	do {
		LPCTSTR lpSql = nullptr;
		if(insert_id == NULL_INT_VALUE){
			if(parent_id == NULL_INT_VALUE){
				lpSql = _T("SELECT ID,POSITION FROM TXTMIRU_BOOKS WHERE PARENT IS NULL AND TYPE=@TYPE ORDER BY POSITION");
			} else {
				lpSql = _T("SELECT ID,POSITION FROM TXTMIRU_BOOKS WHERE PARENT=@PARENT AND TYPE=@TYPE ORDER BY POSITION");
			}
		} else {
			if(parent_id == NULL_INT_VALUE){
				lpSql = _T("SELECT ID,POSITION FROM TXTMIRU_BOOKS WHERE PARENT IS NULL AND TYPE=@TYPE AND ID!=@ID ORDER BY POSITION");
			} else {
				lpSql = _T("SELECT ID,POSITION FROM TXTMIRU_BOOKS WHERE PARENT=@PARENT AND TYPE=@TYPE AND ID!=@ID ORDER BY POSITION");
			}
		}
		int ret = sqlite3_prepare16(pSqlite3, lpSql, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bRet = false;
			break;
		}
		bRet = true;
		sqlite3_reset(pstmt);
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@PARENT"  , parent_id);
		CGrDBFunc::PutValue(pstmt, "@ID"      , id       );
		CGrDBFunc::PutValue(pstmt, "@TYPE"    , type     );
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			::Book book;
			CGrDBFunc::GetValue(pstmt, 0, book.id      );
			CGrDBFunc::GetValue(pstmt, 1, book.position);
			if(book.id == insert_id){
				::Book new_book;
				new_book.id       = id;
				new_book.position = -1;
				id_list.push_back(new_book);
			}
			id_list.push_back(book);
		};
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bRet;
}
bool CGrDBBooks::SetPositionList(sqlite3 *pSqlite3, const std::vector<::Book> &id_list)
{
	bool bRet = true;
	sqlite3_stmt *pstmt = nullptr;

	do {
		int ret = sqlite3_prepare16(pSqlite3, _T("UPDATE TXTMIRU_BOOKS SET POSITION=@POSITION WHERE ID=@ID"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bRet = false;
			break;
		}
		bRet = true;
		int position = 0;
		for(const auto &book : id_list){
			if(book.position != position){
				sqlite3_reset(pstmt);
				sqlite3_clear_bindings(pstmt);
				CGrDBFunc::PutValue(pstmt, "@ID"      , book.id );
				CGrDBFunc::PutValue(pstmt, "@POSITION", position);
				ret = sqlite3_step(pstmt);
				if(ret != SQLITE_DONE){
					bRet = false;
					break;
				}
			}
			++position;
		}
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bRet;
}
bool CGrDBBooks::SortPosition(sqlite3 *pSqlite3, int parent_id, int insert_id/* = NULL_INT_VALUE*/, int id/* = NULL_INT_VALUE*/, int type/* = 1*/)
{
	bool bRet = true;
	do {
		std::vector<::Book> id_list;
		bRet = GetPositionList(pSqlite3, parent_id, insert_id, id, type, id_list);
		if(!bRet){
			break;
		}
		bRet = SetPositionList(pSqlite3, id_list);
		if(!bRet){
			break;
		}
	} while(0);
	return bRet;
}

CGrBookmarkDB::CGrBookmarkDB()
{
}

CGrBookmarkDB::~CGrBookmarkDB()
{
}

bool CGrBookmarkDB::Create()
{
	return true;
}

LPCTSTR CGrBookmarkDB::GetDBName() const
{
	return _T("bookmark");
}


bool CGrBookmarkDB::Open()
{
	auto &param = CGrTxtFunc::Param();
	// 栞フォルダ
	std::tstring str;
	CGrTxtFunc::GetBookmarkFolder(&param, str);
	std::tstring bookmark_db_file;
	CGrText::FormatMessage(bookmark_db_file, _T("%1!s!/%2!s!.db"), str.c_str(), GetDBName());
	return Open(bookmark_db_file.c_str());
}

bool CGrBookmarkDB::Open(LPCTSTR lpFileName)
{
	if (CGrDBCore::Open(lpFileName)) {
		m_places.Create(m_pSqlite3);
		m_history.Create(m_pSqlite3);
		m_bookmarks.Create(m_pSqlite3);
		m_subtitles.Create(m_pSqlite3);
		m_books.Create(m_pSqlite3);
		return true;
	}
	return false;
}
int CGrBookmarkDB::BeginSession()
{
	int ret = sqlite3_exec(m_pSqlite3, "BEGIN", nullptr, nullptr, nullptr);
	if(ret != SQLITE_OK){
		return ret;
	}
	return ret;
}

int CGrBookmarkDB::Commit()
{
	int ret = sqlite3_exec(m_pSqlite3, "COMMIT", nullptr, nullptr, nullptr);
	if(ret != SQLITE_OK){
		return ret;
	}
	return ret;
}

int CGrBookmarkDB::Rollback()
{
	int ret = sqlite3_exec(m_pSqlite3, "ROLLBACK", nullptr, nullptr, nullptr);
	if(ret != SQLITE_OK){
		return ret;
	}
	return ret;
}
bool CGrBookmarkDB::GetPlace(Place &out_place, LPCTSTR lpURL)
{
	return m_places.GetByURL(m_pSqlite3, out_place, lpURL);
}
bool CGrBookmarkDB::PutPlace(Place &in_place)
{
	return m_places.Put(m_pSqlite3, in_place);
}
bool CGrBookmarkDB::PutHistory(History &in_history)
{
	return m_history.Put(m_pSqlite3, in_history);
}
bool CGrBookmarkDB::GetBookmarks(std::vector<Bookmark> &out_bookmark_list, int iPlaceID)
{
	return m_bookmarks.GetByPlaceID(m_pSqlite3, out_bookmark_list, iPlaceID);
}

bool CGrBookmarkDB::PutBookmark(Bookmark &bookmark)
{
	return m_bookmarks.Put(m_pSqlite3, bookmark);
}
bool CGrBookmarkDB::DeleteBookmark(int iID)
{
	return m_bookmarks.Delete(m_pSqlite3, iID);
}
bool CGrBookmarkDB::PutSubtitle(Subtitle &in_subtitle)
{
	return m_subtitles.Put(m_pSqlite3, in_subtitle);
}
bool CGrBookmarkDB::GetSubtitles(std::vector<Subtitle> &out_subtitle_list, int iPlaceID)
{
	return m_subtitles.GetByPlaceID(m_pSqlite3, out_subtitle_list, iPlaceID);
}
bool CGrBookmarkDB::DeleteSubtitle(int iID)
{
	return m_subtitles.Delete(m_pSqlite3, iID);
}
bool CGrBookmarkDB::GetLastVisitDate(std::tstring &out_visit_date, int iPlaceID, int iPage)
{
	auto *pSqlite3 = GetSqlite3();
	if(!pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(pSqlite3, _T("SELECT H.LAST_VISIT_DATE")
									_T(" FROM TXTMIRU_HISTORY H")
									_T(" WHERE H.PLACE_ID=@PLACE_ID AND H.READ_PAGE>=@READ_PAGE ORDER BY H.READ_PAGE,H.LAST_VISIT_DATE DESC"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@PLACE_ID" , iPlaceID);
		CGrDBFunc::PutValue(pstmt, "@READ_PAGE", iPage   );
		// 実行
		bret = false;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			CGrDBFunc::GetValue(pstmt, 0, out_visit_date);
			bret = true;
			break;
		}
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}

////////////////////////////////////////
bool CGrDBSearchHistory::Create()
{
	bool bExist = CGrDBFunc::IsExistsTable(m_pSqlite3, _T("TXTMIRU_SEARCH_HISTORY"));
	bool ret = CGrDBFunc::createTable(m_pSqlite3,
									  _T("TXTMIRU_SEARCH_HISTORY"),
									  "CREATE TABLE TXTMIRU_SEARCH_HISTORY ("
									  "ID INTEGER,"            // ID
									  "FIELDNAME TEXT,"        // FIELDNAME
									  "VALUE TEXT,"            // VALUE
									  "INSERT_DATE TEXT,"      //
									  "UPDATE_DATE TEXT,"      //
									  "PRIMARY KEY (ID AUTOINCREMENT));");
	if(ret){
		ret = CGrDBFunc::createIndex(m_pSqlite3,
									 _T("I1_TXTMIRU_SEARCH_HISTORY"),
									 "CREATE INDEX I1_TXTMIRU_SEARCH_HISTORY ON TXTMIRU_SEARCH_HISTORY (FIELDNAME,UPDATE_DATE);");
		ret = CGrDBFunc::createIndex(m_pSqlite3,
									 _T("I2_TXTMIRU_SEARCH_HISTORY"),
									 "CREATE INDEX I2_TXTMIRU_SEARCH_HISTORY ON TXTMIRU_SEARCH_HISTORY (FIELDNAME,VALUE);");
	}
	if(ret && !bExist){
		std::tstring sysdate;
		CGrDBFunc::GetSysDate(sysdate);
		SearchHistory history;
		history.id         = NULL_INT_VALUE;
		history.fieldname  = _T("cur")     ;
		history.insert_date= sysdate       ;
		history.update_date= sysdate       ;
		const auto &param = CGrTxtFunc::Param();
		for(int idx=0; idx<9; ++idx){
			TCHAR buf[2048] = {};
			param.GetText(static_cast<CGrTxtFuncIParam::TextType>(static_cast<int>(CGrTxtFuncIParam::TextType::HistSearchWord1)+idx), buf, _countof(buf));
			if(buf[0]){
				history.value = buf;
				if(!Put(history)){
					break;
				}
			}
		}
		std::tstring filename;
		CGrCSVText csv;
		filename = CGrTxtFunc::GetDataPath();
		if(!CGrShell::EndBackSlash(const_cast<TCHAR*>(filename.c_str()))){
			filename += _T("/");
		}
		filename += _T("search.csv");
		if(csv.Open(filename.c_str())){
			for(const auto &line : csv.GetCSVROW()){
				history.fieldname.clear();
				for(const auto &item : line){
					if(!history.fieldname.empty()){
						history.value = item;
						Put(history);
					} else if(item == _T("Text")){
						history.fieldname = _T("sf_text");
					} else if(item == _T("Path")){
						history.fieldname = _T("sf_path");
					} else if(item == _T("Type")){
						history.fieldname = _T("sf_type");
					} else {
						break;
					}
				}
			}
		}
	}
	return ret;
}
bool CGrDBSearchHistory::Get(std::vector<SearchHistory> &out_history_list, LPCTSTR fieldname, int maxnum)
{
	if(!m_pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(m_pSqlite3, _T("SELECT ID,FIELDNAME,VALUE,INSERT_DATE,UPDATE_DATE FROM TXTMIRU_SEARCH_HISTORY WHERE FIELDNAME=@FIELDNAME ORDER BY UPDATE_DATE DESC,ID"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@FIELDNAME", fieldname);
		// 実行
		bret = false;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			bret = true;
			SearchHistory history;
			CGrDBFunc::GetValue(pstmt, 0, history.id         );
			CGrDBFunc::GetValue(pstmt, 1, history.fieldname  );
			CGrDBFunc::GetValue(pstmt, 2, history.value      );
			CGrDBFunc::GetValue(pstmt, 3, history.insert_date);
			CGrDBFunc::GetValue(pstmt, 4, history.update_date);
			out_history_list.push_back(history);
			if(out_history_list.size() > static_cast<unsigned int>(maxnum)){
				break;
			}
		};
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBSearchHistory::Get(SearchHistory &out_history, LPCTSTR fieldname, LPCTSTR value)
{
	if(!m_pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(m_pSqlite3, _T("SELECT ID,FIELDNAME,VALUE,INSERT_DATE,UPDATE_DATE FROM TXTMIRU_SEARCH_HISTORY WHERE FIELDNAME=@FIELDNAME AND VALUE=@VALUE"), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@FIELDNAME", fieldname);
		CGrDBFunc::PutValue(pstmt, "@VALUE"    , value    );
		// 実行
		bret = false;
		while(true){
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_ROW){
				break;
			}
			bret = true;
			CGrDBFunc::GetValue(pstmt, 0, out_history.id         );
			CGrDBFunc::GetValue(pstmt, 1, out_history.fieldname  );
			CGrDBFunc::GetValue(pstmt, 2, out_history.value      );
			CGrDBFunc::GetValue(pstmt, 3, out_history.insert_date);
			CGrDBFunc::GetValue(pstmt, 4, out_history.update_date);
			break;
		};
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBSearchHistory::Put(LPCTSTR fieldname, LPCTSTR value)
{
	std::tstring sysdate;
	CGrDBFunc::GetSysDate(sysdate);
	SearchHistory history;
	if(Get(history, fieldname, value)){
		history.update_date = sysdate;
	} else {
		history.id         = NULL_INT_VALUE;
		history.fieldname  = fieldname     ;
		history.value      = value         ;
		history.insert_date= sysdate       ;
		history.update_date= sysdate       ;
	}
	return Put(history);
}
bool CGrDBSearchHistory::Put(SearchHistory &in_history)
{
	if(!m_pSqlite3){
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = 0;
		ret = sqlite3_prepare16(m_pSqlite3,
								_T("INSERT OR REPLACE INTO TXTMIRU_SEARCH_HISTORY (ID,FIELDNAME,VALUE,INSERT_DATE,UPDATE_DATE)")
								_T(" VALUES (@ID,@FIELDNAME,@VALUE,@INSERT_DATE,@UPDATE_DATE)")
								, -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID"          , in_history.id         );
		CGrDBFunc::PutValue(pstmt, "@FIELDNAME"   , in_history.fieldname  );
		CGrDBFunc::PutValue(pstmt, "@VALUE"       , in_history.value      );
		CGrDBFunc::PutValue(pstmt, "@INSERT_DATE" , in_history.insert_date);
		CGrDBFunc::PutValue(pstmt, "@UPDATE_DATE" , in_history.update_date);
		// 実行
		ret = sqlite3_step(pstmt);
		if(ret != SQLITE_DONE){
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
	} while(0);

	if(pstmt){
		sqlite3_finalize(pstmt);
	}
	return bret;
}
CGrDBSearchHistory::CGrDBSearchHistory()
{
}

CGrDBSearchHistory::~CGrDBSearchHistory()
{
}

LPCTSTR CGrDBSearchHistory::GetDBName() const
{
	return _T("searchhistory");
}

///////////////////////////////////
CGrDBCore::CGrDBCore()
{

}
CGrDBCore::~CGrDBCore()
{
	Close();
}

void CGrDBCore::Close()
{
	if (m_pSqlite3) {
		if (SQLITE_OK != sqlite3_close(m_pSqlite3)) {
		}
	}
	m_pSqlite3 = nullptr;
	m_filename.clear();
}

LPCTSTR CGrDBCore::GetDBFileName() const
{
	auto &param = CGrTxtFunc::Param();
	std::tstring db_file;
	CGrText::FormatMessage(db_file, _T("%1!s!/%2!s!s.db"), CGrTxtFunc::GetDataPath(), GetDBName());
	return db_file.c_str();
}

bool CGrDBCore::Open()
{
	std::tstring filename(GetDBFileName());
	return Open(filename.c_str());
}
bool CGrDBCore::Open(LPCTSTR lpFileName)
{
	bool bret = true;
	__try {
		if (m_filename == lpFileName && m_pSqlite3) {
			bret = false;
			__leave;
		}
		// データベースのオープン
		// オープンに失敗しても、Closeは行う必要がある。
		sqlite3 *pSqlite3 = nullptr;
		int ret = sqlite3_open16(lpFileName, &pSqlite3);
		__try {
			if (ret != SQLITE_OK) {
				bret = false;
				__leave;
			}
			m_pSqlite3 = pSqlite3;
			Create();
		}
		__finally {

		}
	}
	__except (DelayLoadExFilter(GetExceptionCode(), GetExceptionInformation())) {
		Close();
		bret = false;
	}
	return bret;
}

