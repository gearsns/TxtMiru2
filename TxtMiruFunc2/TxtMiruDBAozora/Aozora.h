#ifndef __AOZORA_H__
#define __AOZORA_H__

#include "stltchar.h"

enum AozoarDBError {
	ADBERR_SUCCESS     = 0,
	ADBERR_CSVFILE_OPEN   ,
	ADBERR_CSVFILE_NODATA ,
	ADBERR_ZIPFILE_OPEN   ,
	ADBERR_ZIPFILE_GETINFO,
	ADBERR_ZIPFILE_CUROPEN,
	ADBERR_ZIPFILE_NODATA ,
	ADBERR_WEBFILE_OPEN   ,
};

AozoarDBError AozoraUpdate(LPCTSTR strURL);
void GetAozoraUpdateDate(std::tstring &str_date);

#endif // __AOZORA_H__
