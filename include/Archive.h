#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

class CGrArchive
{
private:
	TCHAR m_last_error_archiver[1024];
public:
	enum ArchiveType {
		ARTYPE_LZH ,
		ARTYPE_ZIP ,
		ARTYPE_7Z  ,
		ARTYPE_RAR ,
		ARTYPE_CAB ,
		ARTYPE_NONE
	};
	enum ErrorCode {
		ERCODE_SUCCESS         ,
		ERCODE_ERROR           ,
		ERCODE_NOSUPPORT       ,
		ERCODE_FILENOTFOUND    ,
		ERCODE_ARCHIVERNOTFOUND,
		ERCODE_ARCHIVERRUNNING ,
	};
	CGrArchive();
	~CGrArchive();
	bool IsSupportFile(LPCTSTR filename);
	ErrorCode ExtractFull(LPCTSTR filename, LPCTSTR to_dir, UINT iLimitMaxSize);
	ErrorCode ExtractFull(ArchiveType at, LPCTSTR filename, LPCTSTR to_dir, UINT iLimitMaxSize);
	LPCTSTR GetLastErrorArchiverName();
};

#endif // __ARCHIVE_H__
