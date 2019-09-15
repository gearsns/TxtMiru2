#include "stdafx.h"
#include "CacheDB.h"

#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include <commctrl.h>
#include <regex>
#ifndef FACILITY_VISUALCPP
#define FACILITY_VISUALCPP  ((LONG)0x6d)
#endif
#include <DelayImp.h>
#include "Shlwapi.h"
#include "Text.h"
#include "CSVText.h"
#include "Shell.h"
//#define __DBG__
#include "Debug.h"

#pragma warning( disable : 4786 )


////////////////////////////////////////////////////////////
// lib /def:sqlite3.def /machine:x86
static DWORD DelayLoadExFilter(DWORD ec, EXCEPTION_POINTERS *ep)
{
	DelayLoadInfo *dli = nullptr;
	DelayLoadProc *dlp = nullptr;

	if (ec == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND)) {
	}
	else if (ec == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND)) {
	}
	else {
		// 遅延読み込みの例外でない場合は上位の例外ハンドラへ
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// 遅延読み込み用の例外情報を取得
	dli = reinterpret_cast<DelayLoadInfo*>(ep->ExceptionRecord->ExceptionInformation[0]);
	dlp = &(dli->dlp);
	if (dlp->fImportByName) {
	}
	else {
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
			if (!pSqlite3) {
				bret = false;
				break;
			}
			int ret = sqlite3_prepare16(pSqlite3, _T("SELECT COUNT(*) FROM SQLITE_MASTER WHERE TYPE=? AND NAME=?"), -1, &pstmt, nullptr);
			if (ret != SQLITE_OK || !pstmt) {
				// コメント等、有効なSQLステートメントでないと、戻り値はOKだがstmはNULLになる。
				bret = false;
				break;
			}
			sqlite3_clear_bindings(pstmt);
			ret = sqlite3_bind_text16(pstmt, 1, lpType, -1, SQLITE_STATIC);
			ret = sqlite3_bind_text16(pstmt, 2, lpTableName, -1, SQLITE_STATIC);
			if (SQLITE_OK != ret) {
				bret = false;
				break;
			}
			// 実行
			ret = sqlite3_step(pstmt);
			if (ret != SQLITE_ROW) {
				bret = false;
				break;
			}
			int cnt = sqlite3_column_int(pstmt, 0);
			if (cnt > 0) {
				bret = true;
			}
			else {
				bret = false;
			}
			sqlite3_clear_bindings(pstmt);
		} while (0);

		if (pstmt) {
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
		if (!IsExistsTable(pSqlite3, table_name)) {
			// テーブルの作成
			int ret = sqlite3_exec(pSqlite3, sql, nullptr, nullptr, nullptr);
			if (ret != SQLITE_OK) {
				return false;
			}
		}
		return true;
	}
	bool createIndex(sqlite3 *pSqlite3, LPCTSTR index_name, LPCSTR sql)
	{
		if (!IsExistsIndex(pSqlite3, index_name)) {
			int ret = sqlite3_exec(pSqlite3, sql, nullptr, nullptr, nullptr);
			if (ret != SQLITE_OK) {
				return false;
			}
		}
		return true;
	}
	void GetSysDate(std::tstring &out_sysdate)
	{
		time_t now;

		time(&now);
		struct tm ltm = { 0 };
		if (0 == localtime_s(&ltm, &now)) {
			TCHAR date_str[512] = { 0 };
			_tcsftime(date_str, sizeof(date_str) / sizeof(TCHAR), _T("%Y/%m/%d %X"), &ltm);
			out_sysdate = date_str;
		}
	}
	void GetValue(sqlite3_stmt *pstmt, int col, std::tstring &out_value)
	{
		const void* val = sqlite3_column_text16(pstmt, col);
		if (val) {
			out_value = static_cast<LPCTSTR>(val);
		}
		else {
			out_value = _T("");
		}
	}
	void GetValue(sqlite3_stmt *pstmt, int col, int &out_value)
	{
		auto* val = sqlite3_column_value(pstmt, col);
		if (sqlite3_value_type(val) == SQLITE_NULL) {
			out_value = NULL_INT_VALUE;
		}
		else {
			out_value = sqlite3_value_int(val);
		}
	}
	void PutValue(sqlite3_stmt *pstmt, LPCSTR field, LPCTSTR lpValue)
	{
		if (lpValue) {
			sqlite3_bind_text16(pstmt, sqlite3_bind_parameter_index(pstmt, field), lpValue, -1, SQLITE_STATIC);
		}
		else {
			sqlite3_bind_null(pstmt, sqlite3_bind_parameter_index(pstmt, field));
		}
	}
	void PutValue(sqlite3_stmt *pstmt, LPCSTR field, const std::tstring &str)
	{
		PutValue(pstmt, field, str.c_str());
	}
	void PutValue(sqlite3_stmt *pstmt, LPCSTR field, int iValue)
	{
		if (iValue == NULL_INT_VALUE) {
			sqlite3_bind_null(pstmt, sqlite3_bind_parameter_index(pstmt, field));
		}
		else {
			sqlite3_bind_int(pstmt, sqlite3_bind_parameter_index(pstmt, field), iValue);
		}
	}
}

#include "TxtFuncBookmark.h"
namespace CGrTxtMiruFunc
{
	extern LPCTSTR GetDataPath();
	extern CGrTxtFuncIParam &Param();
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
	std::tstring db_file;
	CGrText::FormatMessage(db_file, _T("%1!s!/%2!s!.db"), CGrTxtMiruFunc::GetDataPath(), GetDBName());
	return db_file.c_str();
}

bool CGrDBCore::Open()
{
	return Open(GetDBFileName());
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
		if (!CGrShell::GetSearchDir(lpFileName)) {
			bret = false;
			__leave;
		}
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
}

LPCTSTR CGrDBCache::GetDBName() const
{
	return _T("cache");
}

bool CGrDBCache::Create()
{
	return false;
}
bool CGrDBCache::Get(std::tstring &outpath, LPCTSTR lpurl)
{
	if (!m_pSqlite3) {
		if (!Open()) {
			return false;
		} else if (!m_pSqlite3) {
			return false;
		}
	}
	bool bret = true;
	sqlite3_stmt *pstmt = nullptr;
	do {
		int ret = sqlite3_prepare16(m_pSqlite3
			, _T("SELECT PATH FROM TXTMIRU_CACHE A WHERE URL=@URL")
			_T(" AND EXISTS(SELECT * FROM TXTMIRU_CACHE_ACTIVE B WHERE B.ID=A.ID)")
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

			std::tstring path;
			CGrDBFunc::GetValue(pstmt, 0, path);
			outpath = std::regex_replace(path.c_str(), std::wregex(L"%DATADIR%"), CGrTxtMiruFunc::GetDataPath(), std::regex_constants::format_first_only);
			break;
		};
	} while (0);

	if (pstmt) {
		sqlite3_finalize(pstmt);
	}
	return bret;
}
