// main.cpp
//
#define STRICT // 型を宣言および使用時に、より厳密な型チェックが行われます。
#define MAIN_H
#pragma warning(disable:4786)

#define OEMRESOURCE
#include <windows.h>
#include <windowsx.h>
#ifndef FACILITY_VISUALCPP
#define FACILITY_VISUALCPP  ((LONG)0x6d)
#endif
#include <DelayImp.h>
#include <ShlObj.h>
#include <wininet.h>
#include <time.h>
#include "resource.h"
#include "TxtMiru.h"
#include "WindowCtrl.h"
#include "stltchar.h"
#include "CSVText.h"
#include "Shell.h"
#include "stlutil.h"
#include "sqlite3/sqlite3.h"
#include "Shlwapi.h"
#include <process.h>
#include "BookMarkHtml.h"
#include "TxtMiruTheme.h"

class CGrMainWindow;
// メインウィンドウ
static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#include "shell.h"
#define TXTMIRU _T("TxtMiru")
#define NULL_INT_VALUE (-999)

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

static void GetBookmarkFolder(std::tstring &str)
{
	TCHAR file_path[512];
	TCHAR buf[1024];

	{
		TCHAR curPath[MAX_PATH];
		CGrShell::GetExePath(curPath);
		TCHAR iniFileName[MAX_PATH];
		_stprintf_s(iniFileName, _T("%s/%s.ini"), curPath, TXTMIRU);
		::TxtGetPrivateProfileString(_T("text"), _T("BookMarkFolder"), buf, sizeof(buf)/sizeof(TCHAR), iniFileName);
	}
	str = buf;
	_tcscpy_s(file_path, CGrTxtMiru::GetDataPath());
	switch(str.size()){
	case 0:
		CGrText::FormatMessage(str, _T("%1!s!/Bookmark"), file_path);
		break;
	case 1:
		if(CGrShell::IsBackSlash(str[0])){
			// 「\」ルート 指定
			if(CGrShell::IsBackSlash(file_path[0])){
				// ネットワークパスに対して、「\」 トップ指定は多分できないのでデフォルトパスに変更
				CGrText::FormatMessage(str, _T("%1!s!/Bookmark"), file_path);
			} else {
				file_path[2] = _T('\0');
				CGrText::FormatMessage(str, _T("%1!s!/"), file_path);
			}
		} else {
			// 相対パスから絶対パスに変更
			CGrText::FormatMessage(str, _T("%1!s!/%2!s!"), file_path, str.c_str());
		}
		break;
	default:
		if(CGrShell::IsBackSlash(str[0]) && !CGrShell::IsBackSlash(str[1])){
			// 「\」ルート 指定
			if(CGrShell::IsBackSlash(file_path[0])){
				// ネットワークパスに対して、「\」 トップ指定は多分できないのでデフォルトパスに変更
				CGrText::FormatMessage(str, _T("%1!s!/Bookmark"), file_path);
			} else {
				file_path[2] = _T('\0');
				CGrText::FormatMessage(str, _T("%1!s!/%2!s!"), file_path, str.c_str());
			}
		} else if(str[1] != _T(':')){
			// 相対パスから絶対パスに変更
			CGrText::FormatMessage(str, _T("%1!s!/%2!s!"), file_path, str.c_str());
		}
		break;
	}
}
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
	bool createTable(sqlite3 *pSqlite3, LPCTSTR table_name, LPCSTR sql)
	{
		// テーブルの作成
		int ret = sqlite3_exec(pSqlite3, sql, 0, nullptr, nullptr);
		if(ret != SQLITE_OK){
			return false;
		}
		return true;
	}
	bool createIndex(sqlite3 *pSqlite3, LPCTSTR index_name, LPCSTR sql)
	{
		int ret = sqlite3_exec(pSqlite3, sql, 0, nullptr, nullptr);
		if(ret != SQLITE_OK){
			return false;
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
	void GetSysNextDate(std::tstring &out_sysdate, int iDays)
	{
		time_t now;
		time(&now);
		now += 86400 * (time_t)iDays;
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

class CGrDBPlaces
{
public:
	CGrDBPlaces(){}
	virtual ~CGrDBPlaces(){}
	bool Create(sqlite3 *pSqlite3)
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
	bool Copy(sqlite3 *pSqlite3)
	{
		if(!pSqlite3){
			return false;
		}
		bool bret = true;
		sqlite3_stmt *pstmt_from = nullptr;
		do {
			std::tstring sysdate;
			CGrDBFunc::GetSysNextDate(sysdate, -30);
			int ret = sqlite3_prepare16(
				pSqlite3,
				_T("INSERT OR REPLACE INTO TXTMIRU_PLACES (ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,VISIT_COUNT,INSERT_DATE,LAST_VISIT_DATE) ")
				_T("SELECT ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,VISIT_COUNT,INSERT_DATE,LAST_VISIT_DATE FROM FROM_DB.TXTMIRU_PLACES A ")
				_T("WHERE (")
				_T("EXISTS (SELECT 1 FROM TXTMIRU_BOOKS B WHERE B.PLACE_ID=A.ID)")
				_T("OR EXISTS (SELECT 1 FROM FROM_DB.TXTMIRU_SUBTITLES S INNER JOIN TXTMIRU_BOOKS B ON B.PLACE_ID=S.PLACE_ID WHERE S.URL=A.URL)")
				_T("OR LAST_VISIT_DATE>@LAST_VISIT_DATE")
				_T(")"),
				-1, &pstmt_from, nullptr);
			if(ret != SQLITE_OK || !pstmt_from) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(pSqlite3)), _T("Place Copy ERROR"), MB_OK);
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt_from);
			CGrDBFunc::PutValue(pstmt_from, "@LAST_VISIT_DATE", sysdate.c_str());
			// 実行
			ret = sqlite3_step(pstmt_from);
			if(ret != SQLITE_ROW){
				break;
			}
		} while(0);

		if(pstmt_from){
			sqlite3_finalize(pstmt_from);
		}
		return bret;
	}
	bool GetByURL(sqlite3 *pSqlite3, Place &out_place, LPCTSTR lpURL)
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
	bool Put(sqlite3 *pSqlite3, Place &in_place)
	{
		if(!pSqlite3){
			return false;
		}
		bool bret = true;
		sqlite3_stmt *pstmt = nullptr;
		do {
			int ret = sqlite3_prepare16(pSqlite3,
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
};
class CGrDBHistory
{
public:
	CGrDBHistory(){}
	virtual ~CGrDBHistory(){}
	bool Create(sqlite3 *pSqlite3)
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
	bool Copy(sqlite3 *pSqlite3)
	{
		if(!pSqlite3){
			return false;
		}
		bool bret = true;
		sqlite3_stmt *pstmt_from = nullptr;
		sqlite3_stmt *pstmt_to   = nullptr;
		do {
			int ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,PLACE_ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,INSERT_DATE,LAST_VISIT_DATE FROM FROM_DB.TXTMIRU_HISTORY WHERE EXISTS(SELECT 1 FROM TXTMIRU_PLACES WHERE ID=PLACE_ID) ORDER BY PLACE_ID,ID DESC"), -1, &pstmt_from, nullptr);
			if(ret != SQLITE_OK || !pstmt_from) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt_from);
			ret = sqlite3_prepare16(pSqlite3,
									_T("INSERT OR REPLACE INTO TXTMIRU_HISTORY (ID,PLACE_ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,INSERT_DATE,LAST_VISIT_DATE)")
									_T(" VALUES (@ID,@PLACE_ID,@URL,@TITLE,@AUTHOR,@PAGE,@READ_PAGE,@INSERT_DATE,@LAST_VISIT_DATE)")
									, -1, &pstmt_to, nullptr);
			if(ret != SQLITE_OK || !pstmt_to) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				bret = false;
				break;
			}
			int place_id = -1;
			int num = 1;
			// 実行
			while(true){
				ret = sqlite3_step(pstmt_from);
				if(ret != SQLITE_ROW){
					break;
				}
				History history;
				CGrDBFunc::GetValue(pstmt_from, 0, history.id             );
				CGrDBFunc::GetValue(pstmt_from, 1, history.place_id       );
				CGrDBFunc::GetValue(pstmt_from, 2, history.url            );
				CGrDBFunc::GetValue(pstmt_from, 3, history.title          );
				CGrDBFunc::GetValue(pstmt_from, 4, history.author         );
				CGrDBFunc::GetValue(pstmt_from, 5, history.page           );
				CGrDBFunc::GetValue(pstmt_from, 6, history.read_page      );
				CGrDBFunc::GetValue(pstmt_from, 7, history.insert_date    );
				CGrDBFunc::GetValue(pstmt_from, 8, history.last_visit_date);
				if(place_id != history.place_id){
					place_id = history.place_id;
					++num;
					sqlite3_reset(pstmt_to);
					sqlite3_clear_bindings(pstmt_to);
					CGrDBFunc::PutValue(pstmt_to, "@ID"             , num                   );
					CGrDBFunc::PutValue(pstmt_to, "@PLACE_ID"       , history.place_id       );
					CGrDBFunc::PutValue(pstmt_to, "@URL"            , history.url            );
					CGrDBFunc::PutValue(pstmt_to, "@TITLE"          , history.title          );
					CGrDBFunc::PutValue(pstmt_to, "@AUTHOR"         , history.author         );
					CGrDBFunc::PutValue(pstmt_to, "@PAGE"           , history.page           );
					CGrDBFunc::PutValue(pstmt_to, "@READ_PAGE"      , history.read_page      );
					CGrDBFunc::PutValue(pstmt_to, "@INSERT_DATE"    , history.insert_date    );
					CGrDBFunc::PutValue(pstmt_to, "@LAST_VISIT_DATE", history.last_visit_date);
					// 実行
					ret = sqlite3_step(pstmt_to);
					if(ret != SQLITE_DONE){
						bret = false;
						break;
					}
				}
			};
		} while(0);

		if(pstmt_from){
			sqlite3_finalize(pstmt_from);
		}
		if(pstmt_to){
			sqlite3_finalize(pstmt_to);
		}
		return bret;
	}
};
class CGrDBBookmarks
{
public:
	CGrDBBookmarks(){}
	virtual ~CGrDBBookmarks(){}
	bool Create(sqlite3 *pSqlite3)
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
	bool Copy(sqlite3 *pSqlite3)
	{
		if(!pSqlite3){
			return false;
		}
		bool bret = true;
		sqlite3_stmt *pstmt_from = nullptr;
		do {
			int ret = sqlite3_prepare16(
				pSqlite3,
				_T("INSERT OR REPLACE INTO TXTMIRU_BOOKMARKS (ID,PLACE_ID,TITLE,PAGE,B_LINE,B_INDEX,B_POS,INSERT_DATE)")
				_T("SELECT ID,PLACE_ID,TITLE,PAGE,B_LINE,B_INDEX,B_POS,INSERT_DATE FROM FROM_DB.TXTMIRU_BOOKMARKS"),
				-1, &pstmt_from, nullptr);
			if(ret != SQLITE_OK || !pstmt_from) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt_from);
			// 実行
			ret = sqlite3_step(pstmt_from);
			if(ret != SQLITE_ROW){
				break;
			}
		} while(0);

		if(pstmt_from){
			sqlite3_finalize(pstmt_from);
		}
		return bret;
	}
};
class CGrDBSubtitles
{
public:
	CGrDBSubtitles(){}
	virtual ~CGrDBSubtitles(){}
	bool Create(sqlite3 *pSqlite3)
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
	bool Copy(sqlite3 *pSqlite3)
	{
		if(!pSqlite3){
			return false;
		}
		bool bret = true;
		sqlite3_stmt *pstmt_from = nullptr;
		do {
			int ret = sqlite3_prepare16(
				pSqlite3,
				_T("INSERT OR REPLACE INTO TXTMIRU_SUBTITLES (ID,PLACE_ID,B_ORDER,TITLE,URL,PAGE,B_LINE,B_INDEX,B_POS,LEVEL,INSERT_DATE)")
				_T("SELECT ID,PLACE_ID,B_ORDER,TITLE,URL,PAGE,B_LINE,B_INDEX,B_POS,LEVEL,INSERT_DATE FROM FROM_DB.TXTMIRU_SUBTITLES WHERE EXISTS(SELECT 1 FROM TXTMIRU_PLACES WHERE ID=PLACE_ID)"),
				-1, &pstmt_from, nullptr);
			if(ret != SQLITE_OK || !pstmt_from) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(pSqlite3)), _T("Books GetAll ERROR"), MB_OK);
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt_from);
			// 実行
			ret = sqlite3_step(pstmt_from);
			if(ret != SQLITE_ROW){
				break;
			}
		} while(0);

		if(pstmt_from){
			sqlite3_finalize(pstmt_from);
		}
		return bret;
	}
};
class CGrDBBooks
{
public:
	CGrDBBooks(){}
	virtual ~CGrDBBooks(){}
	bool Create(sqlite3 *pSqlite3)
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
	bool Copy(sqlite3 *pSqlite3)
	{
		std::vector<Book> book_list;
		if(!GetAll(pSqlite3, book_list)){
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
				MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(pSqlite3)), _T("Books Copy ERROR"), MB_OK);
				break;
			}
			for(const auto &book : book_list){
				sqlite3_reset(pstmt);
				sqlite3_clear_bindings(pstmt);
				CGrDBFunc::PutValue(pstmt, "@ID"         , book.id         );
				CGrDBFunc::PutValue(pstmt, "@PLACE_ID"   , book.place_id   );
				CGrDBFunc::PutValue(pstmt, "@PARENT"     , book.parent     );
				CGrDBFunc::PutValue(pstmt, "@TYPE"       , book.type       );
				CGrDBFunc::PutValue(pstmt, "@POSITION"   , book.position   );
				CGrDBFunc::PutValue(pstmt, "@BASE_URL"   , book.base_url   );
				CGrDBFunc::PutValue(pstmt, "@TITLE"      , book.title      );
				CGrDBFunc::PutValue(pstmt, "@AUTHOR"     , book.author     );
				CGrDBFunc::PutValue(pstmt, "@INSERT_DATE", book.insert_date);
				// 実行
				if(sqlite3_step(pstmt) != SQLITE_DONE){
					bret = false;
					break;
				}
			}
		} while(0);

		if(pstmt){
			sqlite3_finalize(pstmt);
		}
		return bret;
	}

	bool GetAll(sqlite3 *pSqlite3, std::vector<Book> &book_list, int parent = NULL_INT_VALUE)
	{
		if(!pSqlite3){
			return false;
		}
		bool bret = true;
		sqlite3_stmt *pstmt = nullptr;
		std::vector<int> id_list;
		do {
			int ret = 0;
			if(parent == NULL_INT_VALUE){
				sqlite3_prepare16(
					pSqlite3,
					_T("SELECT ID,PLACE_ID,PARENT,TYPE,POSITION,BASE_URL,TITLE,AUTHOR,INSERT_DATE FROM FROM_DB.TXTMIRU_BOOKS WHERE PARENT IS NULL"),
					-1, &pstmt, nullptr);
			} else {
				sqlite3_prepare16(
					pSqlite3,
					_T("SELECT ID,PLACE_ID,PARENT,TYPE,POSITION,BASE_URL,TITLE,AUTHOR,INSERT_DATE FROM FROM_DB.TXTMIRU_BOOKS WHERE PARENT=@PARENT"),
					-1, &pstmt, nullptr);
			}
			if(ret != SQLITE_OK || !pstmt) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(pSqlite3)), _T("Books GetAll ERROR"), MB_OK);
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt);
			CGrDBFunc::PutValue(pstmt, "@PARENT", parent);
			// 実行
			while(true){
				ret = sqlite3_step(pstmt);
				if(ret != SQLITE_ROW){
					break;
				}
				Book book;
				CGrDBFunc::GetValue(pstmt, 0, book.id         );
				CGrDBFunc::GetValue(pstmt, 1, book.place_id   );
				CGrDBFunc::GetValue(pstmt, 2, book.parent     );
				CGrDBFunc::GetValue(pstmt, 3, book.type       );
				CGrDBFunc::GetValue(pstmt, 4, book.position   );
				CGrDBFunc::GetValue(pstmt, 5, book.base_url   );
				CGrDBFunc::GetValue(pstmt, 6, book.title      );
				CGrDBFunc::GetValue(pstmt, 7, book.author     );
				CGrDBFunc::GetValue(pstmt, 8, book.insert_date);
				book_list.push_back(book);
				id_list.push_back(book.id);
			};
		} while(0);

		if(pstmt){
			sqlite3_finalize(pstmt);
		}
		for(auto &id : id_list){
			if(!GetAll(pSqlite3, book_list, id)){
				return false;
			}
		}
		return bret;
	}
	bool Put(sqlite3 *pSqlite3, const Book &in_book)
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
	bool GetLastInsertRow(sqlite3 *pSqlite3, Book &book)
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
	bool MaxPosition(sqlite3 *pSqlite3, int iParent, int &out_position)
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
};


class CGrBookmarkDB
{
public:
	CGrBookmarkDB();
	virtual ~CGrBookmarkDB();

	bool Regist(BookmarkDB::BookList &book_list);
	bool Close();
	bool Open(LPCTSTR lpFileName);

	int BeginSession();
	int Commit();
	int Rollback();

	sqlite3 *GetSqlite3(){ return m_pSqlite3; }
private:
	bool regist(BookmarkDB::BookList &book_list, LPCTSTR fromFileName, LPCTSTR toFileName);
protected:
	CGrDBPlaces m_places;
	CGrDBHistory m_history;
	CGrDBBookmarks m_bookmarks;
	CGrDBSubtitles m_subtitles;
	CGrDBBooks m_books;
	sqlite3 *m_pSqlite3;
	std::tstring m_filename;
};
CGrBookmarkDB::CGrBookmarkDB() : m_pSqlite3(nullptr)
{
}

CGrBookmarkDB::~CGrBookmarkDB()
{
	Close();
}

bool CGrBookmarkDB::Regist(BookmarkDB::BookList &book_list)
{
	TCHAR buf[4096];
	// 栞フォルダ
	std::tstring str;
	GetBookmarkFolder(str);
	std::tstring db_file_from;
	CGrText::FormatMessage(db_file_from, _T("%1!s!/bookmark.db"), str.c_str());
	CGrShell::ToPrettyFileName(db_file_from);
	std::tstring db_file_to;
	CGrText::FormatMessage(db_file_to, _T("%1!s!/bookmark.%2!d!.db"), str.c_str(), _getpid());
	CGrShell::ToPrettyFileName(db_file_to);
	//
	regist(book_list, db_file_from.c_str(), db_file_to.c_str());
	//
	std::tstring sysdate;
	CGrDBFunc::GetSysDate(sysdate);
	std::replace(sysdate, _T("\\"), _T(""));
	std::replace(sysdate, _T(":"), _T(""));
	std::replace(sysdate, _T(" "), _T(""));
	std::replace(sysdate, _T("/"), _T(""));
	std::tstring db_file_backup;
	CGrText::FormatMessage(db_file_backup, _T("%1!s!/bookmark.%2!s!.db"), str.c_str(), sysdate.c_str());
	CGrShell::ToPrettyFileName(db_file_backup);
	if(!MoveFileEx(db_file_from.c_str(), db_file_backup.c_str(), MOVEFILE_REPLACE_EXISTING)){
		_stprintf_s(buf, _T("Copy Error:\nFrom:%s\nTo:%s\n"), db_file_from.c_str(), db_file_backup.c_str());
		MessageBox(NULL, buf, _T("ERROR"), MB_OK);
		return false;
	}
	if(!MoveFileEx(db_file_to.c_str(), db_file_from.c_str(), MOVEFILE_REPLACE_EXISTING)){
		_stprintf_s(buf, _T("Copy Error:\nFrom:%s\nTo:%s\n"), db_file_to.c_str(), db_file_from.c_str());
		MessageBox(NULL, buf, _T("ERROR"), MB_OK);
		return false;
	}
	{
		std::tstring buf;
		CGrText::FormatMessage(buf, IDS_SUCCESS, db_file_backup.c_str());
		MessageBox(NULL, buf.c_str(), _T("OK"), MB_OK);
	}
	return true;
}

bool CGrBookmarkDB::regist(BookmarkDB::BookList &book_list, LPCTSTR fromFileName, LPCTSTR toFileName)
{
	if(!Open(toFileName)){
		return false;
	}
	std::tstring sql_attach = _T("ATTACH DATABASE \"");
	sql_attach += fromFileName;
	sql_attach += _T("\" AS FROM_DB");
	{
		sqlite3_stmt *pstmt = nullptr;
		int ret = sqlite3_prepare16(m_pSqlite3, sql_attach.c_str(), -1, &pstmt, nullptr);
		if(ret != SQLITE_OK || !pstmt){
			MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(m_pSqlite3)), _T("ATTACH ERROR"), MB_OK);
			return false;
		}
		ret = sqlite3_step(pstmt);
		if(ret != SQLITE_DONE){
			MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(m_pSqlite3)), _T("ATTACH ERROR"), MB_OK);
			return false;
		}
		if(pstmt){
			sqlite3_finalize(pstmt);
		}
	}
	//
	BeginSession();
	m_books.Copy(m_pSqlite3);
	Commit();

	BeginSession();
	m_bookmarks.Copy(m_pSqlite3);
	Commit();

	BeginSession();
	m_places.Copy(m_pSqlite3);
	Commit();

	BeginSession();
	m_subtitles.Copy(m_pSqlite3);
	Commit();

	BeginSession();
	m_history.Copy(m_pSqlite3);
	Commit();
	//
	if(SQLITE_OK != sqlite3_exec(m_pSqlite3, "DETACH DATABASE FROM_DB", 0, nullptr, nullptr)){
	}
	bool bret = true;
	std::tstring sysdate;
	CGrDBFunc::GetSysDate(sysdate);
	std::map<int,int> id_list;
	std::map<int,int> position_list;
	BeginSession();
	for(auto &&pBook : book_list){
		if(pBook->type == 0/*ITEM*/){
			Place place;
			if(!m_places.GetByURL(m_pSqlite3, place, pBook->base_url.c_str())){
				place.id          = NULL_INT_VALUE;
				place.url         = pBook->base_url;
				place.title       = pBook->title;
				place.author      = pBook->author;
				place.page        = 1;
				place.read_page   = -1;
				place.visit_count = 0;
				place.insert_date = sysdate;
				if(!m_places.Put(m_pSqlite3, place)){
					bret = false;
					break;
				}
				if(!m_places.GetByURL(m_pSqlite3, place, pBook->base_url.c_str())){
					bret = false;
					break;
				}
			}
			int parent_id = NULL_INT_VALUE;
			{
				auto it = id_list.find(pBook->parent);
				if(it == id_list.end()){
					continue;
				}
				parent_id = it->second;
			}
			int position = 0;
			{
				auto it = position_list.find(parent_id);
				if(it != position_list.end()){
					position = it->second;
				}
				position_list[parent_id] = position + 1;
			}
			Book book;
			book.id          = NULL_INT_VALUE ;
			book.place_id    = place.id       ;
			book.parent      = parent_id      ;
			book.type        = 0              ;
			book.position    = position       ;
			book.base_url    = pBook->base_url;
			book.title       = pBook->title   ;
			book.author      = _T("")         ;
			book.insert_date = sysdate        ;

			bret = m_books.Put(m_pSqlite3, book);
			if(!bret){
				break;
			}
		} else {
			// Folder
			int position = 0;
			int parent_id = NULL_INT_VALUE;
			if(pBook->parent != -1){
				auto it = id_list.find(pBook->parent);
				if(it == id_list.end()){
					continue;
				}
				parent_id = it->second;
			}
			{
				if(pBook->parent == -1){
					if(m_books.MaxPosition(m_pSqlite3, NULL_INT_VALUE, position)){
						position_list[parent_id] = position;
					}
				}
				auto it = position_list.find(parent_id);
				if(it != position_list.end()){
					position = it->second;
				}
				position_list[parent_id] = position + 1;
			}
			Book book;
			book.id          = NULL_INT_VALUE ;
			book.place_id    = NULL_INT_VALUE ;
			book.parent      = parent_id      ;
			book.type        = 1              ;
			book.position    = position       ;
			book.base_url    = pBook->base_url;
			book.title       = pBook->title   ;
			book.author      = _T("")         ;
			book.insert_date = sysdate        ;

			bret = m_books.Put(m_pSqlite3, book);
			if(!bret){
				break;
			}
			bret = m_books.GetLastInsertRow(m_pSqlite3, book);
			if(!bret){
				break;
			}
			id_list[pBook->id] = book.id;
		}
		fflush(stderr);
	}
	Commit();
	if(!Close()){
		bret = false;
	}
	return bret;
}

bool CGrBookmarkDB::Close()
{
	bool bret = true;
	if(m_pSqlite3){
		if(SQLITE_OK != sqlite3_close(m_pSqlite3)){
			MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(m_pSqlite3)), _T("ERROR(Close)"), MB_OK);
			bret = false;
		}
	}
	m_pSqlite3 = nullptr;
	m_filename.clear();
	return bret;
}

bool CGrBookmarkDB::Open(LPCTSTR lpFileName)
{
	bool bret = true;
	__try {
		if(m_filename == lpFileName && m_pSqlite3){
			bret = false;
			__leave;
		}
		int ret;
		// データベースのオープン
		// オープンに失敗しても、Closeは行う必要がある。
		sqlite3 *pSqlite3 = nullptr;
		ret = sqlite3_open16(lpFileName, &pSqlite3);
		__try {
			if(ret != SQLITE_OK){
				bret = false;
				__leave;
			}
			m_pSqlite3 = pSqlite3;
			m_places.Create(m_pSqlite3);
			m_history.Create(m_pSqlite3);
			m_bookmarks.Create(m_pSqlite3);
			m_subtitles.Create(m_pSqlite3);
			m_books.Create(m_pSqlite3);
		} __finally {

		}
	} __except(DelayLoadExFilter(GetExceptionCode(), GetExceptionInformation())) {
		Close();
		bret = false;
	}
	return bret;
}
int CGrBookmarkDB::BeginSession()
{
	int ret = sqlite3_exec(m_pSqlite3, "BEGIN", 0, nullptr, nullptr);
	if(ret != SQLITE_OK){
		return ret;
	}
	return ret;
}

int CGrBookmarkDB::Commit()
{
	int ret = sqlite3_exec(m_pSqlite3, "COMMIT", 0, nullptr, nullptr);
	if(ret != SQLITE_OK){
		return ret;
	}
	return ret;
}

int CGrBookmarkDB::Rollback()
{
	int ret = sqlite3_exec(m_pSqlite3, "ROLLBACK", 0, nullptr, nullptr);
	if(ret != SQLITE_OK){
		return ret;
	}
	return ret;
}

//////////////////////////////////////////////////
class CGrMainWindow
{
public:
	static inline CGrMainWindow &theApp()
	{
		static CGrMainWindow mainWindow;
		return mainWindow;
	}
	virtual ~CGrMainWindow()
	{
		DeleteAllItem();
	}
	HWND Create(HINSTANCE hInstance)
	{
		WNDCLASSEX wc = { sizeof(WNDCLASSEX) };

		wc.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP));
		wc.hIconSm = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
		wc.lpszClassName = _T("TxtMiruDBTool");
		wc.cbWndExtra = DLGWINDOWEXTRA;
		wc.hInstance = hInstance;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = ::MainWndProc;

		if (!RegisterClassEx(&wc)){
			return NULL;
		}

		return CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, reinterpret_cast<DLGPROC>(::MainWndProc));
	}
	LRESULT MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg){
			HANDLE_MSG(hWnd, WM_COMMAND          , OnCommand    );
			HANDLE_MSG(hWnd, WM_INITDIALOG       , OnInitDialog );
			HANDLE_MSG(hWnd, WM_SIZE             , OnSize       );
		case WM_CLOSE:
			::DestroyWindow(hWnd);
			break;
		case WM_NOTIFY:
			{
				auto lpnmh = reinterpret_cast<LPNMHDR>(lParam);
				if(wParam == IDC_TREE_BM){
					TVHITTESTINFO ht = {};
					if(lpnmh->code == NM_CLICK && lpnmh->idFrom == IDC_TREE_BM){
						auto dwpos = GetMessagePos();
						ht.pt.x = GET_X_LPARAM(dwpos);
						ht.pt.y = GET_Y_LPARAM(dwpos);
						MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);
						auto hItem = TreeView_HitTest(lpnmh->hwndFrom, &ht);
						if(TVHT_ONITEMSTATEICON & ht.flags){
							TreeView_SelectItem(lpnmh->hwndFrom, hItem);
							int state = TreeView_GetCheckState(lpnmh->hwndFrom, hItem);
							if(state == 1){
								SetCheckAllParent(lpnmh->hwndFrom, hItem, FALSE);
								SetCheckAllChild(lpnmh->hwndFrom, hItem, FALSE);
							} else {
								SetCheckAllChild(lpnmh->hwndFrom, hItem, TRUE);
							}
						}
					}
				}
			}
			break;
		case WM_DESTROY: OnDestory(); break;
		default:
			return DefWindowProc(hWnd,uMsg,wParam,lParam);
		}
		return 0L;
	}
private:
	void SetCheckAllChild(HWND hWnd, HTREEITEM hParentItem, BOOL bCheck){
		auto hItem = TreeView_GetChild(hWnd, hParentItem);
		while(hItem) {
			TreeView_SetCheckState(hWnd, hItem, bCheck);
			SetCheckAllChild(hWnd, hItem, bCheck);
			hItem = TreeView_GetNextSibling(hWnd, hItem);
		}
	}
	void SetCheckAllParent(HWND hWnd, HTREEITEM hParentItem, BOOL bCheck){
		auto hItem = TreeView_GetParent(hWnd, hParentItem);
		if(hItem){
			TreeView_SetCheckState(hWnd, hItem, bCheck);
			SetCheckAllParent(hWnd, hItem, bCheck);
		}
	}
	bool isChildCheck(HWND hWnd, HTREEITEM hParentItem)
	{
		auto hItem = TreeView_GetChild(hWnd, hParentItem);
		while(hItem) {
			int state = TreeView_GetCheckState(hWnd, hItem);
			if(state == 1 || isChildCheck(hWnd, hItem)){
				return true;
			}
			hItem = TreeView_GetNextSibling(hWnd, hItem);
		}
		return false;
	}
	void getRegistBookList(BookmarkDB::BookList &book_list, HWND hWnd, HTREEITEM hParentItem)
	{
		bool bCheck = false;
		TV_ITEM item = {TVIF_PARAM|TVIF_STATE};
		item.hItem = hParentItem;
		item.stateMask = TVIS_STATEIMAGEMASK;
		TreeView_GetItem(hWnd, &item);
		int state = (item.state >> 12) - 1;
		if(state == 0){
			bCheck = isChildCheck(hWnd, hParentItem);
		} else {
			bCheck = true;
		}
		if(bCheck){
			auto *pBook = reinterpret_cast<BookmarkDB::Book*>(item.lParam);
			if(pBook){
				book_list.push_back(pBook);
				auto hItem = TreeView_GetChild(hWnd, hParentItem);
				while(hItem) {
					getRegistBookList(book_list, hWnd, hItem);
					hItem = TreeView_GetNextSibling(hWnd, hItem);
				}
			}
		}

	}
	void DeleteAllItem()
	{
		for(auto &&pitem : m_book_list){
			if(pitem){
				delete pitem;
			}
		}
		m_book_list.clear();
	}
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		switch(id){
		case IDIMPORT:
			{
				auto hBookmarkTree = GetDlgItem(m_hWnd, IDC_TREE_BM);
				auto hItem = TreeView_GetRoot(hBookmarkTree);
				if(hItem){
					BookmarkDB::BookList book_list;
					getRegistBookList(book_list, hBookmarkTree, hItem);
					CGrBookmarkDB db;
					db.Regist(book_list);
				}
			}
			break;
		case IDFILE:
			{
				TCHAR filter[2048] = {};
				CGrText::LoadString(IDS_HTMLFILE, filter, sizeof(filter)/sizeof(TCHAR));
				std::tstring str;
				CGrText::LoadString(IDS_OPENFILE, str);

				OPENFILENAME of = {sizeof(OPENFILENAME)};
				TCHAR fileName[MAX_PATH] = {};

				of.hwndOwner       = m_hWnd;
				of.lpstrFilter     = filter;
				of.lpstrTitle      = str.c_str();
				of.nMaxCustFilter  = 40;
				of.lpstrFile       = fileName;
				of.nMaxFile        = MAX_PATH - 1;
				of.Flags           = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
				// カレントディレクトリをデフォルトパスに
				TCHAR cur_dir[MAX_PATH];
				::GetCurrentDirectory(sizeof(cur_dir)/sizeof(TCHAR), cur_dir);
				auto lpchr = cur_dir;
				for(; *lpchr; ++lpchr){
					if(*lpchr == _T('/')){
						*lpchr = _T('\\');
					}
				}
				if(CGrShell::IsBackSlash(*(lpchr-1))){
					*(lpchr-1) = '\0';
				}
				*(lpchr+1) = '\0'; // 末尾を \0\0にする
				of.lpstrInitialDir = cur_dir;
				if(!::GetOpenFileName(&of)){
					break;
				}
				auto hBookmarkTree = GetDlgItem(m_hWnd, IDC_TREE_BM);
				TreeView_DeleteAllItems(hBookmarkTree);
				DeleteAllItem();
				CGrBookMarkHtml html;
				html.ReadFile(fileName, m_book_list);
				TVINSERTSTRUCT is = {0};
				std::map<int, HTREEITEM> tree_map;
				for(auto const &pitem : m_book_list){
					SHFILEINFO fileInfo = {0};
					if(pitem->type == 0){
						SHGetFileInfo(_T("*.html"), 0, &fileInfo, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_OPENICON | SHGFI_USEFILEATTRIBUTES);
					} else {
						SHGetFileInfo(_T("."), 0, &fileInfo, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_OPENICON | SHGFI_OVERLAYINDEX);
					}
					is.hParent      = NULL;
					is.hInsertAfter = TVI_LAST;
					is.itemex.mask           = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					is.itemex.state          = INDEXTOSTATEIMAGEMASK(2);
					is.itemex.stateMask      = TVIS_STATEIMAGEMASK;
					is.itemex.pszText        = const_cast<LPTSTR>(pitem->title.c_str());
					is.itemex.cchTextMax     = pitem->title.size();
					is.itemex.iImage         = fileInfo.iIcon;
					is.itemex.iSelectedImage = fileInfo.iIcon;
					is.itemex.lParam         = reinterpret_cast<LPARAM>(pitem);
					is.itemex.iExpandedImage = fileInfo.iIcon;

					auto it = tree_map.find(pitem->parent);
					if(pitem->parent != -1 && it != tree_map.end()){
						is.hParent = it->second;
					}

					auto hItem = TreeView_InsertItem(hBookmarkTree, &is);
					tree_map[pitem->id] = hItem;
				}
			}
			break;
		case IDCANCEL:
		case ID_EXIT:
			::PostMessage(m_hWnd, WM_CLOSE, 0, 0L);
			break;
		}
	}
	BOOL OnInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lParam)
	{
		m_hWnd = hWnd;
		RECT rect;
		GetClientRect(hWnd, &rect);
		setWindowSize(rect.right-rect.left, rect.bottom-rect.top);
		//
		TxtMiruTheme_SetWindowSubclass(m_hWnd);

		SHFILEINFO fileInfo = {0};
		auto hBookmarkTree = GetDlgItem(m_hWnd, IDC_TREE_BM);
		auto himl = reinterpret_cast<HIMAGELIST>(SHGetFileInfo(_T("C:\\"), 0, &fileInfo, sizeof(fileInfo), SHGFI_SYSICONINDEX | SHGFI_ICON));
		TreeView_SetImageList(hBookmarkTree, himl, TVSIL_NORMAL);
		FORWARD_WM_COMMAND(hWnd, IDFILE, 0, 0, PostMessage);
		return TRUE;
	}
#define PADDING 10
	void setWindowSize(int cx, int cy)
	{
		if(cx <= 0 || cy <= 0){
			return;
		}
		RECT client_rect = {0,0,cx,cy};
		auto hFileBtn      = GetDlgItem(m_hWnd, IDFILE     );
		auto hImportBtn    = GetDlgItem(m_hWnd, IDIMPORT   );
		auto hCancelBtn    = GetDlgItem(m_hWnd, IDCANCEL   );
		auto hBookmarkTree = GetDlgItem(m_hWnd, IDC_TREE_BM);
		if(hFileBtn&& hImportBtn && hCancelBtn && hBookmarkTree){
			auto rect = client_rect;
			rect.bottom -= PADDING;
			RECT item_rect;
			{
				GetWindowRect(hFileBtn, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hFileBtn, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
			}
			{
				GetWindowRect(hImportBtn, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				MoveWindow(hImportBtn, pos.x, rect.bottom - nHeight, nWidth, nHeight, TRUE);
			}
			{
				GetWindowRect(hCancelBtn, &item_rect);
				int nWidth  = item_rect.right - item_rect.left;
				int nHeight = item_rect.bottom - item_rect.top;
				MoveWindow(hCancelBtn, rect.right - nWidth - PADDING, rect.bottom - nHeight, nWidth, nHeight, TRUE);

				rect.bottom -= nHeight;
			}
			{
				GetWindowRect(hBookmarkTree, &item_rect);
				POINT pos = {item_rect.left,item_rect.top};
				ScreenToClient(m_hWnd, &pos);
				int nWidth  = rect.right - rect.left - PADDING * 2;
				int nHeight = rect.bottom - pos.y - PADDING;
				MoveWindow(hBookmarkTree, rect.left + PADDING, pos.y, nWidth, nHeight, TRUE);
			}
		}
	}
	void OnSize(HWND hwnd, UINT state, int cx, int cy)
	{
		if(state == SIZE_MINIMIZED || state == SIZE_MAXHIDE){
			return;
		}
		setWindowSize(cx, cy);
	}
	void OnDestory()
	{
		::PostQuitMessage(0);
	}
private:
	CGrMainWindow()
	{
	}
	HWND m_hWnd = NULL;
	BookmarkDB::BookList m_book_list;
public:
	LPCTSTR m_lpsCmdLine = nullptr;
};

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CGrMainWindow::theApp().MainWndProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI _tWinMain(_In_ HINSTANCE hCurInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPTSTR lpsCmdLine, _In_ int nCmdShow)
{
	MSG    msg = {};
	HWND   hwnd = NULL;
	CGrTxtMiru::theApp().SetUseDialogMessage(true);

	TCHAR szLanguageName[100];
	auto idLocal = GetSystemDefaultLCID();
	::GetLocaleInfo(idLocal, LOCALE_SENGLANGUAGE, szLanguageName, _countof(szLanguageName));
	_tsetlocale(LC_ALL,szLanguageName);

	UNREFERENCED_PARAMETER(nCmdShow  );
	UNREFERENCED_PARAMETER(lpsCmdLine);
	UNREFERENCED_PARAMETER(hPrevInst );

	CGrMainWindow::theApp().m_lpsCmdLine = lpsCmdLine;

	if (::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) == S_OK) {
		;
	}

	hwnd = CGrMainWindow::theApp().Create(hCurInst);
	if(!hwnd){
		return FALSE;
	}
	// アクセラレータテーブル読み込み
	while(::GetMessage(&msg, NULL, 0, 0)){
		if(CGrTxtMiru::theApp().IsDialogMessage(hwnd, msg)){
			continue;
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	::CoUninitialize();
	::OleUninitialize();

	return msg.wParam;
}
