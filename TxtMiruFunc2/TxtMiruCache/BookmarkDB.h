#ifndef __BOOKMARKDB_H__
#define __BOOKMARKDB_H__

#include <memory>
#include <vector>
#include <list>
#include <string>
#include "stltchar.h"
#include "sqlite3/sqlite3.h"

#define TXTMIRU _T("TxtMiru")
#define NULL_INT_VALUE (-999)

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
struct CacheItem {
	int          id = 0;
	int          parent_id = 0;
	std::tstring url;
	std::tstring path;
	std::tstring title;
	std::tstring author;
	std::tstring insert_date;
	std::tstring update_date;
};

namespace CGrDBFunc
{
	void GetSysDate(std::tstring &out_sysdate);
	void GetSysNextDate(std::tstring &out_sysdate, int iDays);
};

namespace CGrTxtFunc
{
	void GetBookmarkFolder(std::tstring &str);
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
	virtual std::tstring GetDBFileName() const;
protected:
	sqlite3 *m_pSqlite3 = nullptr;
	std::tstring m_filename;
};

class CGrDBCache : public CGrDBCore
{
public:
	CGrDBCache();
	virtual ~CGrDBCache();

	bool Create() override;
	LPCTSTR GetDBName() const override;
	bool Get(std::vector<CacheItem> &out_list, const std::vector<std::tstring> &in_url_list);
	bool GetLatest(std::vector<CacheItem> &out_list, int maxnum = 1000);
	bool GetSearchWord(std::vector<CacheItem> &out_list, LPCTSTR lpWord, int maxnum = 3000);
	bool Get(std::vector<CacheItem> &out_list, LPCTSTR lpurl);
	bool Get(std::tstring &outpath, LPCTSTR lpurl);
	bool Get(CacheItem &out_item, LPCTSTR lpurl);
	bool Put(const CacheItem &in_item);
	bool Put(LPCTSTR lpurl, LPCTSTR lppath, LPCTSTR lptitle, LPCTSTR lpautho);
	bool Put(const CacheItem &item_p, LPCTSTR lpurl, LPCTSTR lppath);
	bool Delete(int id);
	int BeginSession()
	{
		int ret = sqlite3_exec(m_pSqlite3, "BEGIN", 0, nullptr, nullptr);
		if (ret != SQLITE_OK) {
			return ret;
		}
		return ret;
	}

	int Commit()
	{
		int ret = sqlite3_exec(m_pSqlite3, "COMMIT", 0, nullptr, nullptr);
		if (ret != SQLITE_OK) {
			return ret;
		}
		return ret;
	}

	int Rollback()
	{
		int ret = sqlite3_exec(m_pSqlite3, "ROLLBACK", 0, nullptr, nullptr);
		if (ret != SQLITE_OK) {
			return ret;
		}
		return ret;
	}
};

class CGrBookmarkDB
{
public:
	CGrBookmarkDB();
	virtual ~CGrBookmarkDB();

	bool Close();
	bool Open();
	bool Open(LPCTSTR lpFileName);
	bool OpenNew();

	int BeginSession();
	int Commit();
	int Rollback();

private:
	class CGrBookmarkDBImpl;
	std::shared_ptr<CGrBookmarkDBImpl> m_pimpl;
};

#endif // __BOOKMARKDB_H__
