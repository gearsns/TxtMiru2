#pragma once
#include "stltchar.h"
#include "sqlite3/sqlite3.h"
#include <vector>

#define NULL_INT_VALUE (-999)

struct CacheItem {
	int          id;
	int          parent_id;
	std::tstring url;
	std::tstring path;
	std::tstring insert_date;
	std::tstring update_date;
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

class CGrDBCache : public CGrDBCore
{
public:
	CGrDBCache();
	virtual ~CGrDBCache();

	bool Create() override;
	LPCTSTR GetDBName() const override;
	bool Get(std::tstring &outpath, LPCTSTR lpurl);
};
