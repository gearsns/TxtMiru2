#ifndef __BOOKMARKDB_H__
#define __BOOKMARKDB_H__

#include "stltchar.h"
#include "TxtFuncBookmarkDef.h"
#include "sqlite3/sqlite3.h"
#include <vector>

#define NULL_INT_VALUE (-999)

struct Place {
	int          id              = 0;
	std::tstring url            ;
	std::tstring title          ;
	std::tstring author         ;
	int          page            = 0;
	int          read_page       = 0;
	int          visit_count     = 0;
	std::tstring insert_date    ;
	std::tstring last_visit_date;
};
struct History {
	int          id              = 0;
	int          place_id        = 0;
	std::tstring url            ;
	std::tstring title          ;
	std::tstring author         ;
	int          page            = 0;
	int          read_page       = 0;
	std::tstring insert_date    ;
	std::tstring last_visit_date;
};
struct Bookmark {
	int          id              = 0;
	int          place_id        = 0;
	std::tstring title          ;
	int          page            = 0;
	int          b_line          = 0;
	int          b_index         = 0;
	int          b_pos           = 0;
	std::tstring insert_date    ;
};
struct Subtitle {
	int          id              = 0;
	int          place_id        = 0;
	int          b_order         = 0;
	std::tstring title          ;
	std::tstring url            ;
	int          page            = 0;
	int          b_line          = 0;
	int          b_index         = 0;
	int          b_pos           = 0;
	int          level           = 0;
	std::tstring insert_date    ;
};
struct Book {
	int          id              = 0;
	int          place_id        = 0;
	int          parent          = 0; // NULL IS TOP ITEM
	int          type            = 0; // 0:ITEM, 1:FOLDER
	int          position        = 0;
	std::tstring base_url       ;
	std::tstring title          ;
	std::tstring author         ;
	std::tstring insert_date    ;
};

namespace CGrDBFunc
{
	bool isExistsType(sqlite3 *pSqlite3, LPCTSTR lpType, LPCTSTR lpTableName);
	bool IsExistsTable(sqlite3 *pSqlite3, LPCTSTR lpTableName);
	bool IsExistsIndex(sqlite3 *pSqlite3, LPCTSTR lpTableName);
	bool createTable(sqlite3 *pSqlite3, LPCTSTR table_name, LPCSTR sql);
	bool createIndex(sqlite3 *pSqlite3, LPCTSTR index_name, LPCSTR sql);
	void GetSysDate(std::tstring &out_sysdate);
	void GetValue(sqlite3_stmt *pstmt, int col, std::tstring &out_value);
	void GetValue(sqlite3_stmt *pstmt, int col, int &out_value);
	void PutValue(sqlite3_stmt *pstmt, LPCSTR field, LPCTSTR lpValue);
	void PutValue(sqlite3_stmt *pstmt, LPCSTR field, const std::tstring &str);
	void PutValue(sqlite3_stmt *pstmt, LPCSTR field, int iValue);
}

class CGrDBPlaces
{
public:
	CGrDBPlaces();
	virtual ~CGrDBPlaces();
	bool Create(sqlite3 *pSqlite3);
	bool GetByURL(sqlite3 *pSqlite3, Place &out_place, LPCTSTR lpURL);
	bool Put(sqlite3 *pSqlite3, Place &in_place);
};

class CGrDBHistory
{
public:
	CGrDBHistory();
	virtual ~CGrDBHistory();
	bool Create(sqlite3 *pSqlite3);
	bool Put(sqlite3 *pSqlite3, History &in_history);
};

class CGrDBBookmarks
{
public:
	CGrDBBookmarks();
	virtual ~CGrDBBookmarks();
	bool Create(sqlite3 *pSqlite3);
	bool GetByPlaceID(sqlite3 *pSqlite3, std::vector<Bookmark> &out_bookmark_list, int iPlaceID);
	bool Put(sqlite3 *pSqlite3, Bookmark &in_bookmark);
	bool Delete(sqlite3 *pSqlite3, int iID);
};

class CGrDBSubtitles
{
public:
	CGrDBSubtitles();
	virtual ~CGrDBSubtitles();
	bool Create(sqlite3 *pSqlite3);
	bool GetByPlaceID(sqlite3 *pSqlite3, std::vector<Subtitle> &out_subtitle_list, int iPlaceID);
	bool Put(sqlite3 *pSqlite3, Subtitle &in_subtitle);
	bool Delete(sqlite3 *pSqlite3, int iID);
};
class CGrDBBooks
{
public:
	CGrDBBooks();
	virtual ~CGrDBBooks();
	bool Create(sqlite3 *pSqlite3);
	bool Get(sqlite3 *pSqlite3, Book &book, int iID);
	bool GetLastInsertRow(sqlite3 *pSqlite3, Book &book);
	bool GetByBaseURL(sqlite3 *pSqlite3, Book &book, LPCTSTR lpBaseURL, int iType);
	bool MaxPosition(sqlite3 *pSqlite3, int iParent, int &out_position);
	bool Put(sqlite3 *pSqlite3, const Book &in_book);
	bool Delete(sqlite3 *pSqlite3, int iID);
	bool GetPositionList(sqlite3 *pSqlite3, int parent_id, int insert_id, int id, int type, std::vector<::Book> &id_list);
	bool SetPositionList(sqlite3 *pSqlite3, const std::vector<::Book> &id_list);
	bool SortPosition(sqlite3 *pSqlite3, int parent_id, int insert_id = NULL_INT_VALUE, int id = NULL_INT_VALUE, int type = 1);
};

class CGrDBCore
{
public:
	CGrDBCore();
	virtual ~CGrDBCore();

	void Close();
	bool Open();
	bool Open(LPCTSTR lpFileName);
	virtual bool Create() = 0;

	sqlite3 *GetSqlite3() { return m_pSqlite3; }

	virtual LPCTSTR GetDBName() const = 0;
	virtual LPCTSTR GetDBFileName() const;
protected:
	sqlite3 *m_pSqlite3 = nullptr;
	std::tstring m_filename;
};

class CGrBookmarkDB : public CGrDBCore
{
public:
	CGrBookmarkDB();
	virtual ~CGrBookmarkDB();

	bool Create() override;
	LPCTSTR GetDBName() const override;
	void Close();
	bool Open();
	bool Open(LPCTSTR lpFileName);

	int BeginSession();
	int Commit();
	int Rollback();

	bool GetPlace(Place &out_place, LPCTSTR lpURL);
	bool PutPlace(Place &in_place);
	bool PutHistory(History &in_history);
	bool GetBookmarks(std::vector<Bookmark> &out_bookmark_list, int iPlaceID);
	bool PutBookmark(Bookmark &bookmark);
	bool DeleteBookmark(int iID);
	bool PutSubtitle(Subtitle &in_subtitle);
	bool DeleteSubtitle(int iID);
	bool GetSubtitles(std::vector<Subtitle> &out_subtitle_list, int iPlaceID);

	bool GetLastVisitDate(std::tstring &out_visit_date, int iPlaceID, int iPage);
protected:
	CGrDBPlaces m_places;
	CGrDBHistory m_history;
	CGrDBBookmarks m_bookmarks;
	CGrDBSubtitles m_subtitles;
	CGrDBBooks m_books;
};

struct SearchHistory {
	int          id = 0;
	std::tstring fieldname;
	std::tstring value;
	std::tstring insert_date;
	std::tstring update_date;
};

class CGrDBSearchHistory : public CGrDBCore
{
public:
	CGrDBSearchHistory();
	virtual ~CGrDBSearchHistory();

	bool Create() override;
	LPCTSTR GetDBName() const override;
	bool Get(std::vector<SearchHistory> &out_history_list, LPCTSTR fieldname, int maxnum);
	bool Get(SearchHistory &out_history, LPCTSTR fieldname, LPCTSTR value);
	bool Put(SearchHistory &in_history);
	bool Put(LPCTSTR fieldname, LPCTSTR value);
};

#endif // __BOOKMARKDB_H__
