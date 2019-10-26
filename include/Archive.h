#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

class CGrArchive
{
private:
	TCHAR m_last_error_archiver[1024];
public:
	enum class ArchiveType {
		LZH ,
		ZIP ,
		SevenZ,
		RAR ,
		CAB ,
		NONE
	};
	enum class ErrorCode {
		SUCCESS         ,
		Error           ,
		NOSUPPORT       ,
		FILENOTFOUND    ,
		ARCHIVERNOTFOUND,
		ARCHIVERRUNNING ,
	};
	CGrArchive();
	~CGrArchive();
	bool IsSupportFile(LPCTSTR filename);
	ErrorCode ExtractFull(LPCTSTR filename, LPCTSTR to_dir, UINT iLimitMaxSize);
	ErrorCode ExtractFull(ArchiveType at, LPCTSTR filename, LPCTSTR to_dir, UINT iLimitMaxSize);
	LPCTSTR GetLastErrorArchiverName();
};

#endif // __ARCHIVE_H__
