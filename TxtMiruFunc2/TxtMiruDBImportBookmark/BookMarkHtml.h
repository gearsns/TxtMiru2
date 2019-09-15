#ifndef __BOOKMARKHTML_H__
#define __BOOKMARKHTML_H__

#include "TxtMiruDef.h"

class CGrBookMarkHtml
{
public:
	CGrBookMarkHtml();
	virtual ~CGrBookMarkHtml();
	bool ReadFile(LPCTSTR lpFileName, BookmarkDB::BookList &out_book_list);
};

#endif // __BOOKMARKHTML_H__
