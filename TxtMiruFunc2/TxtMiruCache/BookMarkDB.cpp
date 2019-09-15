#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "BookmarkDB.h"
#ifndef FACILITY_VISUALCPP
#define FACILITY_VISUALCPP  ((LONG)0x6d)
#endif
#include <DelayImp.h>
#include "stltchar.h"
#include "Shell.h"
#include "stlutil.h"
#include "sqlite3/sqlite3.h"
#include "Shlwapi.h"
#include <map>
#include <process.h>
#include <time.h>
#include "TxtMiru.h"
#include <sstream>
#include <regex>

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


namespace CGrTxtFunc
{
	void GetBookmarkFolder(std::tstring &str)
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
		if(str.size() > 0){
			CGrShell::CreateFolder(str.c_str());
		}
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
	bool isExistsType(sqlite3 *pSqlite3, LPCTSTR lpType, LPCTSTR lpTableName, bool bAozora = true)
	{
		bool bret = true;
		sqlite3_stmt *pstmt = nullptr;
		do {
			if(!pSqlite3){
				bret = false;
				break;
			}
			int ret = 0;
			if(bAozora){
				sqlite3_prepare16(pSqlite3, _T("SELECT COUNT(*) FROM SQLITE_MASTER WHERE TYPE=? AND NAME=?"), -1, &pstmt, nullptr);
			} else {
				sqlite3_prepare16(pSqlite3, _T("SELECT COUNT(*) FROM BM_DB.SQLITE_MASTER WHERE TYPE=? AND NAME=?"), -1, &pstmt, nullptr);
			}
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
			auto ret = sqlite3_exec(pSqlite3, sql, NULL, NULL, NULL);
			if(ret != SQLITE_OK){
				return false;
			}
		}
		return true;
	}
	bool createIndex(sqlite3 *pSqlite3, LPCTSTR index_name, LPCSTR sql)
	{
		if(!IsExistsIndex(pSqlite3, index_name)){
			auto ret = sqlite3_exec(pSqlite3, sql, NULL, NULL, NULL);
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
			_tcsftime(date_str, sizeof(date_str)/sizeof(TCHAR), _T("%Y/%m/%d %T"), &ltm);
			out_sysdate = date_str;
		}
	}
	void GetSysNextDate(std::tstring &out_sysdate, int iDays)
	{
		time_t now;
		time(&now);
		now += 86400 * static_cast<time_t>(iDays);
		struct tm ltm = {};
		if(0 == localtime_s(&ltm, &now)){
			TCHAR date_str[512] = {};
			_tcsftime(date_str, sizeof(date_str)/sizeof(TCHAR), _T("%Y/%m/%d %T"), &ltm);
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
			auto ret = sqlite3_prepare16(
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
			auto ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,VISIT_COUNT,INSERT_DATE,LAST_VISIT_DATE FROM TXTMIRU_PLACES WHERE URL=@URL"), -1, &pstmt, nullptr);
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
		auto ret = CGrDBFunc::createTable(pSqlite3,
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
			auto ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,PLACE_ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,INSERT_DATE,LAST_VISIT_DATE FROM FROM_DB.TXTMIRU_HISTORY WHERE EXISTS(SELECT 1 FROM TXTMIRU_PLACES WHERE ID=PLACE_ID) ORDER BY PLACE_ID,ID DESC"), -1, &pstmt_from, NULL);
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
			auto ret = sqlite3_prepare16(
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
			auto ret = sqlite3_prepare16(
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
			for(auto &book : book_list){
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
			auto ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,PLACE_ID,PARENT,TYPE,POSITION,BASE_URL,TITLE,AUTHOR,INSERT_DATE FROM TXTMIRU_BOOKS WHERE ROWID = last_insert_rowid()"), -1, &pstmt, nullptr);
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

std::tstring CGrDBCore::GetDBFileName() const
{
	std::tstring db_file;
	CGrText::FormatMessage(db_file, _T("%1!s!/%2!s!.db"), CGrTxtMiru::GetDataPath(), GetDBName());
	return db_file;
}

bool CGrDBCore::Open()
{
	return Open(GetDBFileName().c_str());
}
bool CGrDBCore::Open(LPCTSTR lpFileName)
{
	bool bret = true;
	__try {
		if (m_filename == lpFileName && m_pSqlite3) {
			bret = false;
			__leave;
		}
		int ret;
		// データベースのオープン
		// オープンに失敗しても、Closeは行う必要がある。
		sqlite3 *pSqlite3 = nullptr;
		ret = sqlite3_open16(lpFileName, &pSqlite3);
		__try {
			if (ret != SQLITE_OK) {
				bret = false;
				__leave;
			}
			m_pSqlite3 = pSqlite3;
			Create();
			m_filename = lpFileName;
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

CGrDBCache::CGrDBCache()
{
}

CGrDBCache::~CGrDBCache()
{
	if (m_pSqlite3) {
		try {
			Rollback();
		}
		catch (...) {

		}
	}
}

LPCTSTR CGrDBCache::GetDBName() const
{
	return _T("cache");
}

bool CGrDBCache::Create()
{
	bool ret = true;
	if (ret) {
		ret = CGrDBFunc::createTable(m_pSqlite3,
									 _T("TXTMIRU_CACHE"),
									 "CREATE TABLE TXTMIRU_CACHE ("
									 "ID INTEGER,"            // ID
									 "PARENT_ID INTEGER,"
									 "URL TEXT,"
									 "PATH TEXT,"
									 "TITLE TEXT,"
									 "AUTHOR TEXT,"
									 "INSERT_DATE TEXT,"
									 "UPDATE_DATE TEXT,"
									 "PRIMARY KEY (ID AUTOINCREMENT));");
	}
	if (ret) {
		ret = CGrDBFunc::createTable(m_pSqlite3,
									 _T("TXTMIRU_CACHE_ACTIVE"),
									 "CREATE TABLE TXTMIRU_CACHE_ACTIVE ("
									 "ID INTEGER,"            // ID
									 "PRIMARY KEY (ID));");
	}
	if (ret) {
		ret = CGrDBFunc::createIndex(m_pSqlite3,
									 _T("TXTMIRU_CACHE"),
									 "CREATE UNIQUE INDEX I1_TXTMIRU_CACHE ON TXTMIRU_CACHE (URL);");
	}
	return ret;
}

extern void PostWokerMessage(LPCTSTR message);

bool CGrDBCache::Get(std::vector<CacheItem> &out_list, const std::vector<std::tstring> &in_url_list)
{
	if (!m_pSqlite3) {
		return false;
	}
	bool bret = false;
	sqlite3_stmt *pstmt = nullptr;
	do {
		auto ret = sqlite3_prepare16(m_pSqlite3, _T("SELECT ID,PARENT_ID,URL,PATH,TITLE,AUTHOR,INSERT_DATE,UPDATE_DATE FROM")
									 _T(" TXTMIRU_CACHE")
									 _T(" WHERE")
									 _T(" URL=@URL")
									 , -1, &pstmt, nullptr);
		if (ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		for (const auto& url : in_url_list) {
			sqlite3_clear_bindings(pstmt);
			CGrDBFunc::PutValue(pstmt, "@URL", url.c_str());
			// 実行
			while (true) {
				ret = sqlite3_step(pstmt);
				if (ret != SQLITE_ROW) {
					break;
				}
				bret = true;
				CacheItem item;
				CGrDBFunc::GetValue(pstmt, 0, item.id);
				CGrDBFunc::GetValue(pstmt, 1, item.parent_id);
				CGrDBFunc::GetValue(pstmt, 2, item.url);
				CGrDBFunc::GetValue(pstmt, 3, item.path);
				CGrDBFunc::GetValue(pstmt, 4, item.title);
				CGrDBFunc::GetValue(pstmt, 5, item.author);
				CGrDBFunc::GetValue(pstmt, 6, item.insert_date);
				CGrDBFunc::GetValue(pstmt, 7, item.update_date);
				out_list.push_back(item);
			};
			sqlite3_reset(pstmt);
		}
	} while (0);

	if (pstmt) {
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBCache::GetLatest(std::vector<CacheItem> &out_list, int maxnum/* = 1000*/)
{
	if (!m_pSqlite3) {
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		auto ret = sqlite3_prepare16(m_pSqlite3
									 , _T("SELECT ID,PARENT_ID,URL,PATH,TITLE,AUTHOR,INSERT_DATE,UPDATE_DATE FROM TXTMIRU_CACHE A")
									 _T(" WHERE PARENT_ID<0")
									 _T(" AND EXISTS(SELECT * FROM TXTMIRU_CACHE_ACTIVE B WHERE B.ID=A.ID)")
									 _T(" ORDER BY UPDATE_DATE DESC, URL ASC")
									 , -1, &pstmt, nullptr);
		if (ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		// 実行
		bret = false;
		while (true) {
			ret = sqlite3_step(pstmt);
			if (ret != SQLITE_ROW) {
				break;
			}
			bret = true;
			CacheItem item;
			CGrDBFunc::GetValue(pstmt, 0, item.id);
			CGrDBFunc::GetValue(pstmt, 1, item.parent_id);
			CGrDBFunc::GetValue(pstmt, 2, item.url);
			CGrDBFunc::GetValue(pstmt, 3, item.path);
			CGrDBFunc::GetValue(pstmt, 4, item.title);
			CGrDBFunc::GetValue(pstmt, 5, item.author);
			CGrDBFunc::GetValue(pstmt, 6, item.insert_date);
			CGrDBFunc::GetValue(pstmt, 7, item.update_date);
			out_list.push_back(item);
			--maxnum;
			if (maxnum <= 0) {
				break;
			}
		};
	} while (0);

	if (pstmt) {
		sqlite3_finalize(pstmt);
	}
	return bret;
}

bool CGrDBCache::GetSearchWord(std::vector<CacheItem> &out_list, LPCTSTR lpWord, int maxnum/* = 1000*/)
{
	if (!m_pSqlite3) {
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		auto ret = sqlite3_prepare16(m_pSqlite3
									 , _T("SELECT ID,PARENT_ID,URL,PATH,TITLE,AUTHOR,INSERT_DATE,UPDATE_DATE")
									 _T(" FROM TXTMIRU_CACHE A")
									 _T(" WHERE (URL LIKE @URL ESCAPE '\\' OR TITLE LIKE @TITLE ESCAPE '\\' OR AUTHOR LIKE @AUTHOR ESCAPE '\\') AND PARENT_ID<0")
									 _T(" AND EXISTS(SELECT * FROM TXTMIRU_CACHE_ACTIVE B WHERE B.ID=A.ID)")
									 _T(" ORDER BY UPDATE_DATE DESC, URL ASC")
									 , -1, &pstmt, nullptr);
		if (ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		std::tstring buf(lpWord);
		std::replace(buf, _T("%"), _T("\\%"));
		std::replace(buf, _T("_"), _T("\\_"));
		std::replace(buf, _T("\\"), _T("\\\\"));
		std::tstring word;
		word += L"%";
		word += buf;
		word += L"%";
		CGrDBFunc::PutValue(pstmt, "@URL", word.c_str());
		CGrDBFunc::PutValue(pstmt, "@TITLE", word.c_str());
		CGrDBFunc::PutValue(pstmt, "@AUTHOR", word.c_str());
		// 実行
		bret = false;
		while (true) {
			ret = sqlite3_step(pstmt);
			if (ret != SQLITE_ROW) {
				break;
			}
			bret = true;
			CacheItem item;
			CGrDBFunc::GetValue(pstmt, 0, item.id);
			CGrDBFunc::GetValue(pstmt, 1, item.parent_id);
			CGrDBFunc::GetValue(pstmt, 2, item.url);
			CGrDBFunc::GetValue(pstmt, 3, item.path);
			CGrDBFunc::GetValue(pstmt, 4, item.title);
			CGrDBFunc::GetValue(pstmt, 5, item.author);
			CGrDBFunc::GetValue(pstmt, 6, item.insert_date);
			CGrDBFunc::GetValue(pstmt, 7, item.update_date);
			out_list.push_back(item);
			--maxnum;
			if (maxnum <= 0) {
				break;
			}
		};
	} while (0);

	if (pstmt) {
		sqlite3_finalize(pstmt);
	}
	return bret;
}

bool CGrDBCache::Get(std::vector<CacheItem> &out_list, LPCTSTR lpurl)
{
	if (!m_pSqlite3) {
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		auto ret = sqlite3_prepare16(m_pSqlite3, _T("SELECT ID,PARENT_ID,URL,PATH,TITLE,AUTHOR,INSERT_DATE,UPDATE_DATE FROM TXTMIRU_CACHE A WHERE")
									 _T(" EXISTS(SELECT * FROM TXTMIRU_CACHE_ACTIVE B WHERE B.ID=A.ID)")
									 _T(" AND (")
									 _T("(URL=@URL AND PARENT_ID<0)")
									 _T(" OR (PARENT_ID IN (SELECT ID FROM TXTMIRU_CACHE WHERE URL=@URL AND PARENT_ID<0))")
									 _T(")")
									 , -1, &pstmt, nullptr);
		if (ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@URL", lpurl);
		// 実行
		bret = false;
		while (true) {
			ret = sqlite3_step(pstmt);
			if (ret != SQLITE_ROW) {
				break;
			}
			bret = true;
			CacheItem item;
			CGrDBFunc::GetValue(pstmt, 0, item.id);
			CGrDBFunc::GetValue(pstmt, 1, item.parent_id);
			CGrDBFunc::GetValue(pstmt, 2, item.url);
			CGrDBFunc::GetValue(pstmt, 3, item.path);
			CGrDBFunc::GetValue(pstmt, 4, item.title);
			CGrDBFunc::GetValue(pstmt, 5, item.author);
			CGrDBFunc::GetValue(pstmt, 6, item.insert_date);
			CGrDBFunc::GetValue(pstmt, 7, item.update_date);
			out_list.push_back(item);
		};
	} while (0);

	if (pstmt) {
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBCache::Get(std::tstring &outpath, LPCTSTR lpurl)
{
	if (!m_pSqlite3) {
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		auto ret = sqlite3_prepare16(m_pSqlite3, _T("SELECT PATH FROM TXTMIRU_CACHE WHERE URL=@URL"), -1, &pstmt, nullptr);
		if (ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@URL", lpurl);
		// 実行
		bret = false;
		while (true) {
			ret = sqlite3_step(pstmt);
			if (ret != SQLITE_ROW) {
				break;
			}
			bret = true;
			CGrDBFunc::GetValue(pstmt, 0, outpath);
			break;
		};
	} while (0);

	if (pstmt) {
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBCache::Get(CacheItem &out_item, LPCTSTR lpurl)
{
	if (!m_pSqlite3) {
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		auto ret = sqlite3_prepare16(m_pSqlite3, _T("SELECT ID,PARENT_ID,URL,PATH,TITLE,AUTHOR,INSERT_DATE,UPDATE_DATE FROM TXTMIRU_CACHE WHERE URL=@URL"), -1, &pstmt, nullptr);
		if (ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@URL", lpurl);
		// 実行
		bret = false;
		while (true) {
			ret = sqlite3_step(pstmt);
			if (ret != SQLITE_ROW) {
				break;
			}
			bret = true;
			CGrDBFunc::GetValue(pstmt, 0, out_item.id);
			CGrDBFunc::GetValue(pstmt, 1, out_item.parent_id);
			CGrDBFunc::GetValue(pstmt, 2, out_item.url);
			CGrDBFunc::GetValue(pstmt, 3, out_item.path);
			CGrDBFunc::GetValue(pstmt, 4, out_item.title);
			CGrDBFunc::GetValue(pstmt, 5, out_item.author);
			CGrDBFunc::GetValue(pstmt, 6, out_item.insert_date);
			CGrDBFunc::GetValue(pstmt, 7, out_item.update_date);
			break;
		};
	} while (0);

	if (pstmt) {
		sqlite3_finalize(pstmt);
	}
	return bret;
}
bool CGrDBCache::Put(const CacheItem &in_item)
{
	if (!m_pSqlite3) {
		return false;
	}
	bool bret = true;
	// TXTMIRU_CACHE
	{
		sqlite3_stmt *pstmt = nullptr;
		do {
			int ret = 0;
			ret = sqlite3_prepare16(m_pSqlite3,
									_T("INSERT OR REPLACE INTO TXTMIRU_CACHE (ID,PARENT_ID,URL,PATH,TITLE,AUTHOR,INSERT_DATE,UPDATE_DATE)")
									_T(" VALUES (@ID,@PARENT_ID,@URL,@PATH,@TITLE,@AUTHOR,@INSERT_DATE,@UPDATE_DATE)")
									, -1, &pstmt, nullptr);
			if (ret != SQLITE_OK || !pstmt) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt);
			CGrDBFunc::PutValue(pstmt, "@ID", in_item.id);
			CGrDBFunc::PutValue(pstmt, "@PARENT_ID", in_item.parent_id);
			CGrDBFunc::PutValue(pstmt, "@URL", in_item.url);
			CGrDBFunc::PutValue(pstmt, "@PATH", in_item.path);
			CGrDBFunc::PutValue(pstmt, "@TITLE", in_item.title);
			CGrDBFunc::PutValue(pstmt, "@AUTHOR", in_item.author);
			CGrDBFunc::PutValue(pstmt, "@INSERT_DATE", in_item.insert_date);
			CGrDBFunc::PutValue(pstmt, "@UPDATE_DATE", in_item.update_date);
			// 実行
			ret = sqlite3_step(pstmt);
			if (ret != SQLITE_DONE) {
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt);
		} while (0);

		if (pstmt) {
			sqlite3_finalize(pstmt);
		}
	}
	// TXTMIRU_CACHE_ACTIVE
	{
		sqlite3_stmt *pstmt = nullptr;
		do {
			auto ret = sqlite3_prepare16(m_pSqlite3,
										 _T("INSERT OR REPLACE INTO TXTMIRU_CACHE_ACTIVE (ID)")
										 _T(" SELECT ID FROM TXTMIRU_CACHE WHERE URL=@URL")
										 , -1, &pstmt, nullptr);
			if (ret != SQLITE_OK || !pstmt) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt);
			CGrDBFunc::PutValue(pstmt, "@URL", in_item.url);
			// 実行
			ret = sqlite3_step(pstmt);
			if (ret != SQLITE_DONE) {
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt);
		} while (0);

		if (pstmt) {
			sqlite3_finalize(pstmt);
		}
	}
	return bret;
}
bool CGrDBCache::Put(LPCTSTR lpurl, LPCTSTR lppath, LPCTSTR lptitle, LPCTSTR lpautho)
{
	std::tstring sysdate;
	CGrDBFunc::GetSysDate(sysdate);
	CacheItem item;
	if (Get(item, lpurl)) {
		item.parent_id = -1;
		item.path = lppath;
		item.title = lptitle;
		item.author = lpautho;
		item.update_date = sysdate;
	}
	else {
		item.id = NULL_INT_VALUE;
		item.parent_id = -1;
		item.url = lpurl;
		item.path = lppath;
		item.title = lptitle;
		item.author = lpautho;
		item.insert_date = sysdate;
		item.update_date = sysdate;
	}
	return Put(item);
}
bool CGrDBCache::Put(const CacheItem &item_p, LPCTSTR lpurl, LPCTSTR lppath)
{
	CacheItem item;
	if (Get(item, lpurl)) {
		item.path = lppath;
		item.update_date = item_p.update_date;
	}
	else {
		item.id = NULL_INT_VALUE;
		item.parent_id = item_p.id;
		item.url = lpurl;
		item.path = lppath;
		item.insert_date = item_p.update_date;
		item.update_date = item_p.update_date;
	}
	return Put(item);
}
bool CGrDBCache::Delete(int id)
{
	if (!m_pSqlite3) {
		return false;
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		auto ret = sqlite3_prepare16(m_pSqlite3,
									 _T("DELETE FROM TXTMIRU_CACHE_ACTIVE WHERE ID IN (SELECT ID FROM TXTMIRU_CACHE WHERE ID=@ID OR PARENT_ID=@ID)")
									 , -1, &pstmt, nullptr);
		if (ret != SQLITE_OK || !pstmt) {
			// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
		CGrDBFunc::PutValue(pstmt, "@ID", id);
		// 実行
		ret = sqlite3_step(pstmt);
		if (ret != SQLITE_DONE) {
			bret = false;
			break;
		}
		sqlite3_clear_bindings(pstmt);
	} while (0);

	if (pstmt) {
		sqlite3_finalize(pstmt);
	}
	return bret;
}

class CGrBookmarkDB::CGrBookmarkDBImpl
{
	bool m_bAttachBookmark = false;
public:
	sqlite3 *GetSqlite3(){ return m_pSqlite3; }
public:
	CGrDBPlaces m_places;
	CGrDBHistory m_history;
	CGrDBBookmarks m_bookmarks;
	CGrDBSubtitles m_subtitles;
	CGrDBBooks m_books;
	sqlite3 *m_pSqlite3 = nullptr;
	std::tstring m_filename;
private:
	bool attachBookmarkDB()
	{
		bool ret = false;
		std::tstring str;
		CGrTxtFunc::GetBookmarkFolder(str);
		std::tstring db_file_from;
		CGrText::FormatMessage(db_file_from, _T("%1!s!/bookmark.db"), str.c_str());
		//
		std::tstring sql_attach = _T("ATTACH DATABASE \"");
		sql_attach += db_file_from;
		sql_attach += _T("\" AS BM_DB");
		sqlite3_stmt *pstmt = nullptr;
		do {
			int ret = sqlite3_prepare16(m_pSqlite3, sql_attach.c_str(), -1, &pstmt, nullptr);
			if(ret != SQLITE_OK || !pstmt){
				break;
			}
			ret = sqlite3_step(pstmt);
			if(ret != SQLITE_DONE){
				break;
			}
		} while(0);
		if(pstmt){
			sqlite3_finalize(pstmt);
		}
		m_bAttachBookmark = CGrDBFunc::isExistsType(m_pSqlite3, _T("table"), L"TXTMIRU_BOOKS", false);
		return ret;
	}

public:
	bool Close()
	{
		bool bret = true;
		if(m_pSqlite3){
			if(SQLITE_OK != sqlite3_exec(m_pSqlite3, "DETACH DATABASE BM_DB", 0, nullptr, nullptr)){
			}
			if(SQLITE_OK != sqlite3_close(m_pSqlite3)){
				MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(m_pSqlite3)), _T("ERROR(Close)"), MB_OK);
				bret = false;
			}
		}
		m_pSqlite3 = nullptr;
		m_filename.clear();
		return bret;
	}
	bool OpenNew()
	{
		// 栞フォルダ
		std::tstring str;
		CGrTxtFunc::GetBookmarkFolder(str);
		std::tstring sysdate;
		CGrDBFunc::GetSysDate(sysdate);
		std::replace(sysdate, _T("\\"), _T(""));
		std::replace(sysdate, _T(":"), _T(""));
		std::replace(sysdate, _T(" "), _T(""));
		std::replace(sysdate, _T("/"), _T(""));

		std::tstring db_file_backup;
		CGrText::FormatMessage(db_file_backup, _T("%1!s!/aozora.%2!s!.db"), str.c_str(), sysdate.c_str());
		CGrShell::ToPrettyFileName(db_file_backup);

		std::tstring db_file_from;
		CGrText::FormatMessage(db_file_from, _T("%1!s!/aozora.db"), str.c_str());
		CGrShell::ToPrettyFileName(db_file_from);

		if(CGrShell::GetSearchDir(db_file_from.c_str()) && !MoveFileEx(db_file_from.c_str(), db_file_backup.c_str(), MOVEFILE_REPLACE_EXISTING)){
			TCHAR buf[4096];
			_stprintf_s(buf, _T("Copy Error:\nFrom:%s\nTo:%s\n"), db_file_from.c_str(), db_file_backup.c_str());
			MessageBox(NULL, buf, _T("ERROR"), MB_OK);
			return false;
		}
		return Open(db_file_from.c_str());
	}
	bool Open()
	{
		// 栞フォルダ
		std::tstring str;
		CGrTxtFunc::GetBookmarkFolder(str);
		std::tstring bookmark_db_file;
		CGrText::FormatMessage(bookmark_db_file, _T("%1!s!/aozora.db"), str.c_str());
		return Open(bookmark_db_file.c_str());
	}

	bool Open(LPCTSTR lpFileName)
	{
		bool bret = true;
		__try {
			if(m_filename == lpFileName && m_pSqlite3){
				bret = false;
				__leave;
			}
			// データベースのオープン
			// オープンに失敗しても、Closeは行う必要がある。
			sqlite3 *pSqlite3 = nullptr;
			auto ret = sqlite3_open16(lpFileName, &pSqlite3);
			__try {
				if(ret != SQLITE_OK){
					bret = false;
					__leave;
				}
				m_pSqlite3 = pSqlite3;
				attachBookmarkDB();
			} __finally {

			}
		} __except(DelayLoadExFilter(GetExceptionCode(), GetExceptionInformation())) {
			Close();
			bret = false;
		}
		return bret;
	}
	int BeginSession()
	{
		auto ret = sqlite3_exec(m_pSqlite3, "BEGIN", 0, nullptr, nullptr);
		if(ret != SQLITE_OK){
			return ret;
		}
		return ret;
	}

	int Commit()
	{
		auto ret = sqlite3_exec(m_pSqlite3, "COMMIT", 0, nullptr, nullptr);
		if(ret != SQLITE_OK){
			return ret;
		}
		return ret;
	}

	int Rollback()
	{
		auto ret = sqlite3_exec(m_pSqlite3, "ROLLBACK", 0, nullptr, nullptr);
		if(ret != SQLITE_OK){
			return ret;
		}
		return ret;
	}
};


CGrBookmarkDB::CGrBookmarkDB()
{
	m_pimpl = std::make_shared<CGrBookmarkDBImpl>();
}

CGrBookmarkDB::~CGrBookmarkDB()
{
	Close();
}

bool CGrBookmarkDB::Close()
{
	return m_pimpl->Close();
}

bool CGrBookmarkDB::Open()
{
	return m_pimpl->Open();
}

bool CGrBookmarkDB::Open(LPCTSTR lpFileName)
{
	return m_pimpl->Open(lpFileName);
}
bool CGrBookmarkDB::OpenNew()
{
	return m_pimpl->OpenNew();
}

int CGrBookmarkDB::BeginSession()
{
	return m_pimpl->BeginSession();
}

int CGrBookmarkDB::Commit()
{
	return m_pimpl->Commit();
}

int CGrBookmarkDB::Rollback()
{
	return m_pimpl->Rollback();
}
