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
#include "TxtMiruTheme.h"
#include <process.h>

#define WM_VACUUM_END  WM_APP+1
#define WM_VACUUM_SETP WM_APP+2

enum TABLE_TYPE {
	TT_BOOK,
	TT_BOOKMARK,
	TT_PLACE,
	TT_SUBTITLES,
	TT_HISTORY,
	TT_END,
};

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
		auto ret = sqlite3_exec(pSqlite3, sql, 0, nullptr, nullptr);
		if(ret != SQLITE_OK){
			return false;
		}
		return true;
	}
	bool createIndex(sqlite3 *pSqlite3, LPCTSTR index_name, LPCSTR sql)
	{
		auto ret = sqlite3_exec(pSqlite3, sql, 0, nullptr, nullptr);
		if(ret != SQLITE_OK){
			return false;
		}
		return true;
	}
	void GetSysDate(std::tstring &out_sysdate)
	{
		time_t now;

		time(&now);
		struct tm ltm = {0};
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
		now += 86400 * static_cast<time_t>(iDays);
		struct tm ltm = {0};
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
		//out_value = sqlite3_column_int(pstmt, col);
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
		return ret;
	}
	bool Copy(HWND hWnd, sqlite3 *pSqlite3)
	{
		if(!pSqlite3){
			return false;
		}
		bool bret = true;
		sqlite3_stmt *pstmt_from = nullptr;
		std::vector<int> id_list;
		// お気に入り一覧
		do {
			auto ret = sqlite3_prepare16(
				pSqlite3, _T("SELECT PLACE_ID FROM TXTMIRU_BOOKS"),
				-1, &pstmt_from, nullptr);
			if(ret != SQLITE_OK || !pstmt_from) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(pSqlite3)), _T("Place Copy ERROR"), MB_OK);
				bret = false;
				break;
			}
			while(true){
				ret = sqlite3_step(pstmt_from);
				if(ret != SQLITE_ROW){
					break;
				}
				int id;
				CGrDBFunc::GetValue(pstmt_from, 0, id);
				id_list.push_back(id);
			};
			if(pstmt_from){
				sqlite3_finalize(pstmt_from);
			}
		} while(0);
		PostMessage(hWnd, WM_VACUUM_SETP, static_cast<LPARAM>(TT_PLACE), 1);
		// お気に入り タイトル一覧
		do {
			auto ret = sqlite3_prepare16(
				pSqlite3, _T("SELECT A.ID FROM")
				_T(" FROM_DB.TXTMIRU_BOOKS B")
				_T(" INNER JOIN FROM_DB.TXTMIRU_SUBTITLES S ON S.PLACE_ID=B.PLACE_ID")
				_T(" INNER JOIN FROM_DB.TXTMIRU_PLACES A ON A.URL=S.URL"),
				-1, &pstmt_from, nullptr);
			if(ret != SQLITE_OK || !pstmt_from) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(pSqlite3)), _T("Place Copy ERROR"), MB_OK);
				bret = false;
				break;
			}
			while(true){
				ret = sqlite3_step(pstmt_from);
				if(ret != SQLITE_ROW){
					break;
				}
				int id;
				CGrDBFunc::GetValue(pstmt_from, 0, id);
				id_list.push_back(id);
			};
			if(pstmt_from){
				sqlite3_finalize(pstmt_from);
			}
		} while(0);
		PostMessage(hWnd, WM_VACUUM_SETP, static_cast<LPARAM>(TT_PLACE), 2);
		// 30日以内のデータ
		do {
			std::tstring sysdate;
			CGrDBFunc::GetSysNextDate(sysdate, -30);
			auto ret = sqlite3_prepare16(
				pSqlite3, _T("SELECT ID FROM FROM_DB.TXTMIRU_PLACES WHERE LAST_VISIT_DATE>@LAST_VISIT_DATE"),
				-1, &pstmt_from, nullptr);
			if(ret != SQLITE_OK || !pstmt_from) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(pSqlite3)), _T("Place Copy ERROR"), MB_OK);
				bret = false;
				break;
			}
			sqlite3_reset(pstmt_from);
			sqlite3_clear_bindings(pstmt_from);
			CGrDBFunc::PutValue(pstmt_from, "@LAST_VISIT_DATE", sysdate.c_str());
			while(true){
				ret = sqlite3_step(pstmt_from);
				if(ret != SQLITE_ROW){
					break;
				}
				int id;
				CGrDBFunc::GetValue(pstmt_from, 0, id);
				id_list.push_back(id);
			};
			if(pstmt_from){
				sqlite3_finalize(pstmt_from);
			}
		} while(0);
		PostMessage(hWnd, WM_VACUUM_SETP, static_cast<LPARAM>(TT_PLACE), 3);
		std::sort(id_list.begin(), id_list.end());
		id_list.erase(std::unique(id_list.begin(), id_list.end()), id_list.end());
		do {
			auto ret = sqlite3_prepare16(
				pSqlite3,
				_T("INSERT OR REPLACE INTO TXTMIRU_PLACES (ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,VISIT_COUNT,INSERT_DATE,LAST_VISIT_DATE)")
				_T(" SELECT ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,VISIT_COUNT,INSERT_DATE,LAST_VISIT_DATE FROM FROM_DB.TXTMIRU_PLACES A")
				_T(" WHERE A.ID=@ID"),
				-1, &pstmt_from, nullptr);
			if(ret != SQLITE_OK || !pstmt_from) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(pSqlite3)), _T("Place Copy ERROR"), MB_OK);
				bret = false;
				break;
			}
			for(const auto id : id_list){
				sqlite3_reset(pstmt_from);
				sqlite3_clear_bindings(pstmt_from);
				CGrDBFunc::PutValue(pstmt_from, "@ID", id);
				ret = sqlite3_step(pstmt_from);
				if(ret != SQLITE_DONE){
					break;
				}
			}
			if(pstmt_from){
				sqlite3_finalize(pstmt_from);
			}
		} while(0);
		if(bret){
			bret = CGrDBFunc::createIndex(pSqlite3,
										  _T("I1_TXTMIRU_PLACES"),
										  "CREATE INDEX I1_TXTMIRU_PLACES ON TXTMIRU_PLACES (URL);");
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
		return ret;
	}
	bool Copy(HWND hWnd, sqlite3 *pSqlite3)
	{
		if(!pSqlite3){
			return false;
		}
		bool bret = true;
		sqlite3_stmt *pstmt_from = nullptr;
		sqlite3_stmt *pstmt_to   = nullptr;
		do {
			auto ret = sqlite3_prepare16(pSqlite3, _T("SELECT ID,PLACE_ID,URL,TITLE,AUTHOR,PAGE,READ_PAGE,INSERT_DATE,LAST_VISIT_DATE FROM FROM_DB.TXTMIRU_HISTORY WHERE EXISTS(SELECT 1 FROM TXTMIRU_PLACES WHERE ID=PLACE_ID) ORDER BY PLACE_ID,ID DESC"), -1, &pstmt_from, nullptr);
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
			PostMessage(hWnd, WM_VACUUM_SETP, static_cast<LPARAM>(TT_HISTORY), 1);
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
		if(bret){
			bret = CGrDBFunc::createIndex(pSqlite3,
										  _T("I1_TXTMIRU_HISTORYS"),
										  "CREATE INDEX I1_TXTMIRU_HISTORYS ON TXTMIRU_HISTORY (PLACE_ID,READ_PAGE);");
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
	bool Copy(HWND hWnd, sqlite3 *pSqlite3)
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
			PostMessage(hWnd, WM_VACUUM_SETP, static_cast<LPARAM>(TT_BOOKMARK), 1);
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
		return ret;
	}
	bool Copy(HWND hWnd, sqlite3 *pSqlite3)
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
			PostMessage(hWnd, WM_VACUUM_SETP, static_cast<LPARAM>(TT_SUBTITLES), 1);
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
		if(bret){
			bret = CGrDBFunc::createIndex(pSqlite3,
										  _T("I1_TXTMIRU_SUBTITLES"),
										  "CREATE INDEX I1_TXTMIRU_SUBTITLES ON TXTMIRU_SUBTITLES (PLACE_ID,URL,PAGE,B_ORDER);");
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
		return ret;
	}
	bool Copy(HWND hWnd, sqlite3 *pSqlite3)
	{
		std::vector<Book> book_list;
		if(!GetAll(pSqlite3, book_list)){
			return false;
		}
		bool bret = true;
		sqlite3_stmt *pstmt = nullptr;
		do {
			auto ret = sqlite3_prepare16(pSqlite3,
										 _T("INSERT OR REPLACE INTO TXTMIRU_BOOKS (ID,PLACE_ID,PARENT,TYPE,POSITION,BASE_URL,TITLE,AUTHOR,INSERT_DATE)")
										 _T(" VALUES (@ID,@PLACE_ID,@PARENT,@TYPE,@POSITION,@BASE_URL,@TITLE,@AUTHOR,@INSERT_DATE)")
										 , -1, &pstmt, nullptr);
			if(ret != SQLITE_OK || !pstmt) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				MessageBox(NULL, reinterpret_cast<LPCTSTR>(sqlite3_errmsg16(pSqlite3)), _T("Books Copy ERROR"), MB_OK);
				bret = false;
				break;
			}
			PostMessage(hWnd, WM_VACUUM_SETP, static_cast<LPARAM>(TT_BOOK), 1);
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
		if(bret){
			bret = CGrDBFunc::createIndex(pSqlite3,
										  _T("I1_TXTMIRU_BOOKS"),
										  "CREATE INDEX I1_TXTMIRU_BOOKS ON TXTMIRU_BOOKS (PARENT);");
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
};


class CGrBookmarkDB
{
public:
	CGrBookmarkDB();
	virtual ~CGrBookmarkDB();

	bool Vacuum(HWND hWndProgress);
	bool Copy(HWND hWndProgress, LPCTSTR fromFileName, LPCTSTR toFileName);
	bool Close();
	bool Open(LPCTSTR lpFileName);

	int BeginSession();
	int Commit();
	int Rollback();

	sqlite3 *GetSqlite3(){ return m_pSqlite3; }
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

bool CGrBookmarkDB::Vacuum(HWND hWndProgress)
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
	if(!Copy(hWndProgress, db_file_from.c_str(), db_file_to.c_str())){
		_stprintf_s(buf, _T("Copy Error:\n:%s\n"), db_file_from.c_str());
		MessageBox(NULL, buf, _T("ERROR"), MB_OK);
		return false;
	}
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
		_stprintf_s(buf, _T("Backup Copy Error:\nFrom:%s\nTo:%s\n"), db_file_from.c_str(), db_file_backup.c_str());
		MessageBox(NULL, buf, _T("ERROR"), MB_OK);
		return false;
	}
	if(!MoveFileEx(db_file_to.c_str(), db_file_from.c_str(), MOVEFILE_REPLACE_EXISTING)){
		_stprintf_s(buf, _T("Replace Error:\nFrom:%s\nTo:%s\n"), db_file_to.c_str(), db_file_from.c_str());
		MessageBox(NULL, buf, _T("ERROR"), MB_OK);
		return false;
	}
	{
		std::tstring buf;
		CGrText::FormatMessage(buf, IDS_SUCCESS, db_file_backup.c_str());
		MessageBox(hWndProgress, buf.c_str(), _T("OK"), MB_OK);
	}
	return true;
}

bool CGrBookmarkDB::Copy(HWND hWndProgress, LPCTSTR fromFileName, LPCTSTR toFileName)
{
	if(!Open(toFileName)){
		return false;
	}
	std::tstring sql_attach = _T("ATTACH DATABASE \"");
	sql_attach += fromFileName;
	sql_attach += _T("\" AS FROM_DB");
	{
		sqlite3_stmt *pstmt = nullptr;
		auto ret = sqlite3_prepare16(m_pSqlite3, sql_attach.c_str(), -1, &pstmt, NULL);
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
	bool bret = true;
	do {
		PostMessage(hWndProgress, WM_VACUUM_SETP, static_cast<LPARAM>(TT_BOOK), 0);
		BeginSession();
		bret = m_books.Copy(hWndProgress, m_pSqlite3);
		Commit();
		if(!bret){
			break;
		}

		PostMessage(hWndProgress, WM_VACUUM_SETP, static_cast<LPARAM>(TT_BOOKMARK), 0);
		BeginSession();
		bret = m_bookmarks.Copy(hWndProgress, m_pSqlite3);
		Commit();
		if(!bret){
			break;
		}

		PostMessage(hWndProgress, WM_VACUUM_SETP, static_cast<LPARAM>(TT_PLACE), 0);
		BeginSession();
		bret = m_places.Copy(hWndProgress, m_pSqlite3);
		Commit();
		if(!bret){
			break;
		}

		PostMessage(hWndProgress, WM_VACUUM_SETP, static_cast<LPARAM>(TT_SUBTITLES), 0);
		BeginSession();
		bret = m_subtitles.Copy(hWndProgress, m_pSqlite3);
		Commit();
		if(!bret){
			break;
		}

		PostMessage(hWndProgress, WM_VACUUM_SETP, static_cast<LPARAM>(TT_HISTORY), 0);
		BeginSession();
		bret = m_history.Copy(hWndProgress, m_pSqlite3);
		Commit();
		if(!bret){
			break;
		}
		PostMessage(hWndProgress, WM_VACUUM_SETP, static_cast<LPARAM>(TT_END), 0);
	} while(0);
	//
	if(SQLITE_OK != sqlite3_exec(m_pSqlite3, "DETACH DATABASE FROM_DB", 0, nullptr, nullptr)){
	}
	return Close() && bret;
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
			sqlite3_exec(m_pSqlite3, "PRAGMA journal_mode = WAL", 0, nullptr, nullptr);
			sqlite3_exec(m_pSqlite3, "PRAGMA synchronous = NORMAL", 0, nullptr, nullptr);
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
	auto ret = sqlite3_exec(m_pSqlite3, "BEGIN", 0, nullptr, nullptr);
	if(ret != SQLITE_OK){
		return ret;
	}
	return ret;
}

int CGrBookmarkDB::Commit()
{
	auto ret = sqlite3_exec(m_pSqlite3, "COMMIT", 0, nullptr, nullptr);
	if(ret != SQLITE_OK){
		return ret;
	}
	return ret;
}

int CGrBookmarkDB::Rollback()
{
	auto ret = sqlite3_exec(m_pSqlite3, "ROLLBACK", 0, nullptr, nullptr);
	if(ret != SQLITE_OK){
		return ret;
	}
	return ret;
}

struct VacuumData {
	HWND hWnd;
	HWND hWndProgress;
	bool bBreak;
};
static unsigned int __stdcall VacuumProc(void *lpParam)
{
	auto *lpData = static_cast<VacuumData*>(lpParam);
	CGrBookmarkDB db;
	db.Vacuum(lpData->hWnd);
	if(!lpData->bBreak){
		PostMessage(lpData->hWnd, WM_VACUUM_END, 0, 0);
	}
	return 0;
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
	}
	HWND Create(HINSTANCE hInstance)
	{
		WNDCLASSEX wc = { sizeof(WNDCLASSEX) };

		wc.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP));
		wc.hIconSm = reinterpret_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
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
		case WM_VACUUM_END:
			{
				EnableWindow(GetDlgItem(hWnd, IDREBUILDDB), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDCANCEL   ), TRUE);
			}
			break;
		case WM_VACUUM_SETP:
			{
				auto id = static_cast<int>(wParam);
				auto hWndProgress = GetDlgItem(m_hWnd, IDC_PROGRESS);
				auto pos = static_cast<int>(lParam);
				switch(static_cast<TABLE_TYPE>(wParam)){
				case TT_BOOK     : pos +=  1; break;
				case TT_BOOKMARK : pos +=  5; break;
				case TT_PLACE    : pos += 10; break;
				case TT_SUBTITLES: pos += 15; break;
				case TT_HISTORY  : pos += 20; break;
				case TT_END      : pos += 24; break;
				}
				SendMessage(hWndProgress, PBM_SETPOS, pos, 0);
			}
			break;
		case WM_CLOSE:
			if(m_hThread){
				PostThreadMessage(m_threadid, WM_QUIT, 0, 0);
				::WaitForSingleObject(m_hThread, INFINITE);
				CloseHandle(m_hThread);
				m_hThread = 0;
				m_threadid = 0;
			}
			::DestroyWindow(hWnd);
			break;
		case WM_DESTROY: OnDestory(); break;
		default:
			return DefWindowProc(hWnd,uMsg,wParam,lParam);
		}
		return 0L;
	}
private:
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		switch(id){
		case IDREBUILDDB:
			{
				EnableWindow(GetDlgItem(hwnd, IDREBUILDDB), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDCANCEL   ), FALSE);

				auto hWndProgress = GetDlgItem(hwnd, IDC_PROGRESS);
				SendMessage(hWndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 25-1));
				SendMessage(hWndProgress, PBM_SETSTEP, 1, 0);
				SendMessage(hWndProgress, PBM_SETPOS, 0, 0);

				m_vacuumData.hWnd = m_hWnd;
				m_vacuumData.hWndProgress = hWndProgress;
				m_vacuumData.bBreak = false;
				m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0 , VacuumProc, reinterpret_cast<void*>(&m_vacuumData)/*param*/, 0, &(m_threadid)));
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
		TxtMiruTheme_SetWindowSubclass(m_hWnd);
		return TRUE;
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
	HANDLE m_hThread = NULL;
	unsigned int m_threadid = 0;
	VacuumData m_vacuumData = {};
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

	auto hwnd = CGrMainWindow::theApp().Create(hCurInst);
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
