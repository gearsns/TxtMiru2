#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "shell.h"
#include "stlutil.h"
#include "text.h"
#include "Archive.h"
#include "Win32Wrap.h"
//#define __DBG__
#include "Debug.h"

#define FNAME_MAX32 512

class CGrArchiveBase
{
protected:
	typedef HGLOBAL HARC;
	typedef struct {
		DWORD           dwOriginalSize;
		DWORD           dwCompressedSize;
		DWORD           dwCRC;
		UINT            uFlag;
		UINT            uOSType;
		WORD            wRatio;
		WORD            wDate;
		WORD            wTime;
		char            szFileName[FNAME_MAX32 + 1];
		char            dummy1[3];
		char            szAttribute[8];
		char            szMode[8];
	} INDIVIDUALINFO, *LPINDIVIDUALINFO;
	WORD (WINAPI *GetVersion)() = nullptr;
	BOOL (WINAPI *GetRunning)() = nullptr;
	BOOL (WINAPI *CheckArchive)(LPCSTR, const int) = nullptr;
	int (WINAPI *UnArchive)(const HWND, LPCSTR, LPSTR, const DWORD) = nullptr;
	HARC (WINAPI *OpenArchive)(const HWND _hwnd, LPCSTR _szFileName, const DWORD _dwMode) = nullptr;
	int (WINAPI *CloseArchive)(HARC _harc) = nullptr;
	int (WINAPI *FindFirst)(HARC _harc, LPCSTR _szWildName, INDIVIDUALINFO FAR *lpSubInfo) = nullptr;
	int (WINAPI *FindNext)(HARC _harc, INDIVIDUALINFO FAR *_lpSubInfo) = nullptr;
	int (WINAPI *SetUnicodeMode)(BOOL _unicode) = nullptr;

	enum FunctionNameIndex {
		FNI_FILE_NAME     ,
		FNI_GETVERSION    ,
		FNI_GETRUNNING    ,
		FNI_CHECKARCHIVE  ,
		FNI_UNARCHIVE     ,
		FNI_COMMAND_LINE  ,
		FNI_OPENARCHIVE   ,
		FNI_CLOSEARCHIVE  ,
		FNI_FINDFIRST     ,
		FNI_FINDNEXT      ,
		FNI_SETUNICODEMODE,
		FNI_NUM            /* End */
	};
	LPCTSTR m_lpFName[FNI_NUM] = {};
	FARPROC m_fncList[FNI_NUM] = {};
	HMODULE m_hDLL = NULL;
	CGrArchiveBase(){}
public:
	virtual ~CGrArchiveBase(){ Unload(); }
	LPCTSTR GetName(){ return m_lpFName[FNI_FILE_NAME]; }
	virtual bool Load()
	{
		if(!m_hDLL && m_lpFName[FNI_FILE_NAME]){
			m_hDLL = ::LoadLibrary(m_lpFName[FNI_FILE_NAME]);
			if(!m_hDLL){
				TCHAR path[512];
				TCHAR fileName[512];
				CGrShell::GetExePath(path);
				_stprintf_s(fileName, _T("%s/%s"), path, m_lpFName[FNI_FILE_NAME]);
				m_hDLL = ::LoadLibrary(fileName);
			}
		}
		return (m_hDLL != NULL);
	}
	virtual void Unload()
	{
		if(m_hDLL){
			::FreeLibrary(m_hDLL);
			m_hDLL = NULL;
			::memset(m_fncList, 0x00, sizeof(m_fncList));
		}
	}
	void w2c(LPCTSTR w, std::string &c)
	{
		size_t wLen = 0;
		c.resize(_tcslen(w) * 3);
		if(!c.empty()){
			auto err = wcstombs_s(&wLen, &c[0], c.size()-1, w, _TRUNCATE);
		}
	}
	void c2w(LPCSTR c, std::tstring &w)
	{
		size_t cLen = 0;
		w.resize(strlen(c) * 3);
		if(!w.empty()){
			auto err =  mbstowcs_s(&cLen, &w[0], w.size()-1, c, _TRUNCATE);
		}
	}
	bool utf82w(LPCSTR c, std::tstring &w)
	{
		if(c[0]==0xEF && c[1]==0xBB && c[2]==0xBF){ //BOM check
			c+=3;
		}
		bool result = false;

		int wlen = ::MultiByteToWideChar(CP_UTF8, 0, c, -1, nullptr, 0);
		if(wlen == 0){
			return false;
		}
		auto* buff = new WCHAR[wlen + 1];
		if(::MultiByteToWideChar(CP_UTF8, 0, c, -1, buff, wlen)){
			result = true;
			buff[wlen] = L'\0';
			w = buff;
		}
		delete[] buff;
		return result;
	}
	bool w2utf8(LPCTSTR w, std::string &c)
	{
		bool result = false;

		int clen = ::WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
		if(clen == 0){
			return false;
		}
		auto* buff = new char[clen + 1];
		if(::WideCharToMultiByte(CP_UTF8, 0, w, -1, buff, clen, nullptr, nullptr)){
			result = true;
			buff[clen] = L'\0';
			c = buff;
		}
		delete[] buff;
		return result;
	}
	virtual void c2conv(LPCTSTR w, std::string &c)
	{
		w2c(w, c);
	}
	virtual void writeResponseFileLine(HANDLE hFile, const INDIVIDUALINFO &ii, UINT iLimitMaxSize)
	{
		DWORD dw = 0;
		if(ii.dwOriginalSize > 0 && (iLimitMaxSize == 0 || ii.dwOriginalSize < 1024/*KB*/ * 1024/*MB*/ * iLimitMaxSize)){
			WriteFile(hFile, "\"", 1, &dw, nullptr);
			WriteFile(hFile, ii.szFileName, strlen(ii.szFileName), &dw, nullptr);
			WriteFile(hFile, "\"\r\n", 3, &dw, nullptr);
		}
	}
	FARPROC Function(FunctionNameIndex fn_idx)
	{
		if(!Load() || !m_lpFName[fn_idx]){
			return nullptr;
		}
		if(m_fncList[fn_idx]){
			return m_fncList[fn_idx];
		}
		std::string f;
		w2c(m_lpFName[fn_idx], f);
		m_fncList[fn_idx] = ::GetProcAddress(m_hDLL, f.c_str());
		return m_fncList[fn_idx];
	}
	int UnArchiveResFile(LPCTSTR filename, PCTSTR to_dir, LPCTSTR tempFile)
	{
		int ret = 0;
		// 展開
		if(m_lpFName[FNI_UNARCHIVE] && m_lpFName[FNI_COMMAND_LINE]){
			SetProcPtr(UnArchive, Function(FNI_UNARCHIVE));
			if(!UnArchive){
				return CGrArchive::ERCODE_ARCHIVERNOTFOUND;
			}
			CGrShell::CreateFolder(to_dir);
			char output[1024] = {0};
			std::tstring cmdline;
			CGrText::FormatMessage(cmdline, m_lpFName[FNI_COMMAND_LINE], filename, to_dir, tempFile);
			std::replace(cmdline, _T("/"), _T("\\"));
			std::string cmdline_c;
			c2conv(cmdline.c_str(), cmdline_c);
			ret = UnArchive(NULL, cmdline_c.c_str(), output, sizeof(output)/sizeof(char));
		}
		::DeleteFile(tempFile);
		return ret;
	}
	CGrArchive::ErrorCode ArchiveDLLCheck(LPCTSTR filename)
	{
		//
		auto dwAttr = ::GetFileAttributes(filename);
		if(dwAttr == 0xFFFFFFFF){
			return CGrArchive::ERCODE_FILENOTFOUND;
		}
		//
		if(!Load()){
			return CGrArchive::ERCODE_ARCHIVERNOTFOUND;
		}
		// バージョン取得
		if(m_lpFName[FNI_GETVERSION]){
			SetProcPtr(GetVersion, Function(FNI_GETVERSION));
			if(!GetVersion){
				return CGrArchive::ERCODE_ARCHIVERNOTFOUND;
			}
		}
		// 動作中かチェック
		if(m_lpFName[FNI_GETRUNNING]){
			SetProcPtr(GetRunning, Function(FNI_GETRUNNING));
			if(!GetRunning){
				return CGrArchive::ERCODE_ARCHIVERNOTFOUND;
			}
			if(GetRunning()){
				return CGrArchive::ERCODE_ARCHIVERRUNNING;
			}
		}
		return CGrArchive::ERCODE_SUCCESS;
	}
	virtual CGrArchive::ErrorCode ExtractFull(LPCTSTR filename, LPCTSTR to_dir, UINT iLimitMaxSize)
	{
		auto errcode = ArchiveDLLCheck(filename);
		if(errcode != CGrArchive::ERCODE_SUCCESS){
			return errcode;
		}
		if(m_lpFName[FNI_SETUNICODEMODE]){
			SetProcPtr(SetUnicodeMode, Function(FNI_SETUNICODEMODE));
			if(SetUnicodeMode){
				if(!SetUnicodeMode(TRUE)){
					return CGrArchive::ERCODE_ARCHIVERNOTFOUND;
				}
			}
		}
		bool bNoLimit = false;
		//
		if(iLimitMaxSize == 0){
			bNoLimit = true;
		}
		if(!bNoLimit && m_lpFName[FNI_OPENARCHIVE]){
			SetProcPtr(OpenArchive, Function(FNI_OPENARCHIVE));
			if(!OpenArchive){
				bNoLimit = true;
			}
		}
		//
		std::string filename_c;
		c2conv(filename, filename_c);
		if(bNoLimit){
			// iLimitMaxSize = 0 又は、OpenArchive未サポートのDLLの場合は、全て展開する
			// 展開チェック
			if(m_lpFName[FNI_CHECKARCHIVE]){
				SetProcPtr(CheckArchive, Function(FNI_CHECKARCHIVE));
				if(!CheckArchive){
					return CGrArchive::ERCODE_ARCHIVERNOTFOUND;
				}
				if(!CheckArchive(filename_c.c_str(), 0)){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
			}
			{
				TCHAR tempPath[MAX_PATH] = {};
				if(::GetTempPath(sizeof(tempPath)/sizeof(TCHAR), tempPath) == 0){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				TCHAR tempFile[MAX_PATH] = {};
				if(::GetTempFileName(tempPath, _T("txt"), 0, tempFile) == 0){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				auto hFile = ::CreateFile(tempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if(INVALID_HANDLE_VALUE == hFile){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				DWORD dw = 0;
				WriteFile(hFile, "*\r\n", 3, &dw, NULL);
				::CloseHandle(hFile);
				return (UnArchiveResFile(filename, to_dir, tempFile) == 0) ? CGrArchive::ERCODE_SUCCESS : CGrArchive::ERCODE_ERROR;
			}
		} else {
			if(m_lpFName[FNI_CLOSEARCHIVE]){
				SetProcPtr(CloseArchive, Function(FNI_CLOSEARCHIVE));
				if(!CloseArchive){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
			}
			if(m_lpFName[FNI_FINDFIRST]){
				SetProcPtr(FindFirst, Function(FNI_FINDFIRST));
				if(!FindFirst){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
			}
			if(m_lpFName[FNI_FINDNEXT]){
				SetProcPtr(FindNext, Function(FNI_FINDNEXT));
				if(!FindNext){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
			}
			//
			auto harc = OpenArchive(NULL, filename_c.c_str(), 0);
			if(harc){
				TCHAR tempPath[MAX_PATH] = {};
				if(::GetTempPath(sizeof(tempPath)/sizeof(TCHAR), tempPath) == 0){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				TCHAR tempFile[MAX_PATH] = {};
				if(::GetTempFileName(tempPath, _T("txt"), 0, tempFile) == 0){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				auto hFile = ::CreateFile(tempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if(INVALID_HANDLE_VALUE == hFile){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				INDIVIDUALINFO ii = {};
				if(FindFirst(harc, "*", &ii) == 0){
					do {
						writeResponseFileLine(hFile, ii, iLimitMaxSize);
					} while(FindNext(harc, &ii) == 0);
				}
				CloseArchive(harc);
				::CloseHandle(hFile);
				return (UnArchiveResFile(filename, to_dir, tempFile) == 0) ? CGrArchive::ERCODE_SUCCESS : CGrArchive::ERCODE_ERROR;
			}
		}
		return CGrArchive::ERCODE_ERROR;
	}
};
class CGrArchiveUnLha : public CGrArchiveBase
{
	typedef struct {
		DWORD           dwOriginalSize;
		DWORD           dwCompressedSize;
		DWORD           dwCRC;
		UINT            uFlag;
		UINT            uOSType;
		WORD            wRatio;
		WORD            wDate;
		WORD            wTime;
		WCHAR           szFileName[FNAME_MAX32 + 1];
		WCHAR           dummy1[3];
		WCHAR           szAttribute[8];
		WCHAR           szMode[8];
	} INDIVIDUALINFOW, *LPINDIVIDUALINFOW;
	BOOL (WINAPI *CheckArchiveW)(LPCTSTR, const int) = nullptr;
	int (WINAPI *UnArchiveW)(const HWND, LPCTSTR, LPTSTR, const DWORD) = nullptr;
	HARC (WINAPI *OpenArchiveW)(const HWND _hwnd, LPCTSTR _szFileName, const DWORD _dwMode) = nullptr;
	int (WINAPI *FindFirstW)(HARC _harc, LPCTSTR _szWildName, INDIVIDUALINFOW FAR *lpSubInfo) = nullptr;
	int (WINAPI *FindNextW)(HARC _harc, INDIVIDUALINFOW FAR *_lpSubInfo) = nullptr;
public:
	CGrArchiveUnLha() : CGrArchiveBase(){
		m_lpFName[FNI_FILE_NAME   ] = _T("UNLHA32.DLL");
		m_lpFName[FNI_GETVERSION  ] = _T("UnlhaGetVersion");
		m_lpFName[FNI_GETRUNNING  ] = _T("UnlhaGetRunning");
		m_lpFName[FNI_CHECKARCHIVE] = _T("UnlhaCheckArchiveW");
		m_lpFName[FNI_UNARCHIVE   ] = _T("UnlhaW");
		m_lpFName[FNI_COMMAND_LINE] = _T("x \"%1!s!\" \"%2!s!\" \"@%3!s!\"");
		m_lpFName[FNI_OPENARCHIVE ] = _T("UnlhaOpenArchiveW");
		m_lpFName[FNI_CLOSEARCHIVE] = _T("UnlhaCloseArchive");
		m_lpFName[FNI_FINDFIRST   ] = _T("UnlhaFindFirstW");
		m_lpFName[FNI_FINDNEXT    ] = _T("UnlhaFindNextW");
	}
	virtual void writeResponseFileLine(HANDLE hFile, const INDIVIDUALINFOW &ii, UINT iLimitMaxSize)
	{
		DWORD dw = 0;
		if(ii.dwOriginalSize > 0 && (iLimitMaxSize == 0 || ii.dwOriginalSize < 1024/*KB*/ * 1024/*MB*/ * iLimitMaxSize)){
			WriteFile(hFile, _T("\""), 1*sizeof(TCHAR), &dw, nullptr);
			WriteFile(hFile, ii.szFileName, _tcslen(ii.szFileName)*sizeof(TCHAR), &dw, nullptr);
			WriteFile(hFile, _T("\"\r\n"), 3*sizeof(TCHAR), &dw, nullptr);
		}
	}
	int UnArchiveResFile(LPCTSTR filename, PCTSTR to_dir, LPCTSTR tempFile)
	{
		int ret = 0;
		// 展開
		if(m_lpFName[FNI_UNARCHIVE] && m_lpFName[FNI_COMMAND_LINE]){
			SetProcPtr(UnArchiveW, Function(FNI_UNARCHIVE));
			if(!UnArchiveW){
				return CGrArchive::ERCODE_ARCHIVERNOTFOUND;
			}
			CGrShell::CreateFolder(to_dir);
			TCHAR output[1024] = {0};
			std::tstring cmdline;
			CGrText::FormatMessage(cmdline, m_lpFName[FNI_COMMAND_LINE], filename, to_dir, tempFile);
			std::replace(cmdline, _T("/"), _T("\\"));
			ret = UnArchiveW(NULL, cmdline.c_str(), output, sizeof(output)/sizeof(TCHAR));
		}
		::DeleteFile(tempFile);
		return ret;
	}
	virtual CGrArchive::ErrorCode ExtractFull(LPCTSTR filename, LPCTSTR to_dir, UINT iLimitMaxSize)
	{
		auto errcode = ArchiveDLLCheck(filename);
		if(errcode != CGrArchive::ERCODE_SUCCESS){
			return errcode;
		}
		bool bNoLimit = false;
		//
		if(iLimitMaxSize == 0){
			bNoLimit = true;
		}
		if(!bNoLimit && m_lpFName[FNI_OPENARCHIVE]){
			SetProcPtr(OpenArchiveW, Function(FNI_OPENARCHIVE));
			if(!OpenArchiveW){
				bNoLimit = true;
			}
		}
		//
		if(bNoLimit){
			// iLimitMaxSize = 0 又は、OpenArchive未サポートのDLLの場合は、全て展開する
			// 展開チェック
			if(m_lpFName[FNI_CHECKARCHIVE]){
				SetProcPtr(CheckArchiveW, Function(FNI_CHECKARCHIVE));
				if(!CheckArchiveW){
					return CGrArchive::ERCODE_ARCHIVERNOTFOUND;
				}
				if(!CheckArchiveW(filename, 0)){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
			}
			{
				TCHAR tempPath[MAX_PATH] = {0};
				if(::GetTempPath(sizeof(tempPath)/sizeof(TCHAR), tempPath) == 0){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				TCHAR tempFile[MAX_PATH] = {0};
				if(::GetTempFileName(tempPath, _T("txt"), 0, tempFile) == 0){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				auto hFile = ::CreateFile(tempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if(INVALID_HANDLE_VALUE == hFile){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				DWORD dw = 0;
				BYTE header[] = {0xff, 0xfe};
				::WriteFile(hFile, header, sizeof(header), &dw, nullptr);
				WriteFile(hFile, _T("*\r\n"), 3*sizeof(TCHAR), &dw, nullptr);
				::CloseHandle(hFile);
				return (UnArchiveResFile(filename, to_dir, tempFile) == 0) ? CGrArchive::ERCODE_SUCCESS : CGrArchive::ERCODE_ERROR;
			}
		} else {
			if(m_lpFName[FNI_CLOSEARCHIVE]){
				SetProcPtr(CloseArchive, Function(FNI_CLOSEARCHIVE));
				if(!CloseArchive){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
			}
			if(m_lpFName[FNI_FINDFIRST]){
				SetProcPtr(FindFirstW, Function(FNI_FINDFIRST));
				if(!FindFirstW){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
			}
			if(m_lpFName[FNI_FINDNEXT]){
				SetProcPtr(FindNextW, Function(FNI_FINDNEXT));
				if(!FindNextW){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
			}
			auto harc = OpenArchiveW(NULL, filename, 0);
			if(harc){
				TCHAR tempPath[MAX_PATH] = {0};
				if(::GetTempPath(sizeof(tempPath)/sizeof(TCHAR), tempPath) == 0){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				TCHAR tempFile[MAX_PATH] = {0};
				if(::GetTempFileName(tempPath, _T("txt"), 0, tempFile) == 0){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				auto hFile = ::CreateFile(tempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if(INVALID_HANDLE_VALUE == hFile){
					return CGrArchive::ERCODE_NOSUPPORT;
				}
				DWORD dw = 0;
				BYTE header[] = {0xff, 0xfe};
				::WriteFile(hFile, header, sizeof(header), &dw, nullptr);
				INDIVIDUALINFOW ii = {0};
				if(FindFirstW(harc, _T("*"), &ii) == 0){
					do {
						writeResponseFileLine(hFile, ii, iLimitMaxSize);
					} while(FindNextW(harc, &ii) == 0);
				}
				CloseArchive(harc);
				::CloseHandle(hFile);
				return (UnArchiveResFile(filename, to_dir, tempFile) == 0) ? CGrArchive::ERCODE_SUCCESS : CGrArchive::ERCODE_ERROR;
			}
		}
		return CGrArchive::ERCODE_ERROR;
	}
};
class CGrArchiveUnZip : public CGrArchiveBase
{
public:
	CGrArchiveUnZip() : CGrArchiveBase(){
		m_lpFName[FNI_FILE_NAME   ] = _T("UNZIP32.DLL");
		m_lpFName[FNI_GETVERSION  ] = _T("UnZipGetVersion");
		m_lpFName[FNI_GETRUNNING  ] = _T("UnZipGetRunning");
		m_lpFName[FNI_CHECKARCHIVE] = _T("UnZipCheckArchive");
		m_lpFName[FNI_UNARCHIVE   ] = _T("UnZip");
		m_lpFName[FNI_COMMAND_LINE] = _T("-x -s \"%1!s!\" \"%2!s!\" \"@%3!s!\"");
		m_lpFName[FNI_OPENARCHIVE ] = _T("UnZipOpenArchive");
		m_lpFName[FNI_CLOSEARCHIVE] = _T("UnZipCloseArchive");
		m_lpFName[FNI_FINDFIRST   ] = _T("UnZipFindFirst");
		m_lpFName[FNI_FINDNEXT    ] = _T("UnZipFindNext");
	}
	//
	virtual void writeResponseFileLine(HANDLE hFile, const INDIVIDUALINFO &ii, UINT iLimitMaxSize)
	{
		DWORD dw = 0;
		if(ii.dwOriginalSize > 0 && (iLimitMaxSize == 0 || ii.dwOriginalSize < 1024/*KB*/ * 1024/*MB*/ * iLimitMaxSize)){
			WriteFile(hFile, "\"", 1, &dw, nullptr);
			std::tstring srcFileNameTmp;
			c2w(ii.szFileName, srcFileNameTmp);
			std::replace(srcFileNameTmp, _T("["), _T("\\["));
			std::replace(srcFileNameTmp, _T("]"), _T("\\]"));
			std::string srcFileName;
			w2c(srcFileNameTmp.c_str(), srcFileName);
			WriteFile(hFile, srcFileName.c_str(), strlen(srcFileName.c_str()), &dw, nullptr);
			WriteFile(hFile, "\"\r\n", 3, &dw, nullptr);
		}
	}
};

class CGrArchiveUn7Zip : public CGrArchiveBase
{
public:
	CGrArchiveUn7Zip() : CGrArchiveBase(){
		m_lpFName[FNI_FILE_NAME     ] = _T("7-ZIP32.DLL");
		m_lpFName[FNI_GETVERSION    ] = _T("SevenZipGetVersion");
		m_lpFName[FNI_GETRUNNING    ] = _T("SevenZipGetRunning");
		m_lpFName[FNI_CHECKARCHIVE  ] = _T("SevenZipCheckArchive");
		m_lpFName[FNI_UNARCHIVE     ] = _T("SevenZip");
		m_lpFName[FNI_COMMAND_LINE  ] = _T("x -scsUTF-8 -aoa \"%1!s!\" \"%2!s!\" \"@%3!s!\"");
		m_lpFName[FNI_OPENARCHIVE   ] = _T("SevenZipOpenArchive");
		m_lpFName[FNI_CLOSEARCHIVE  ] = _T("SevenZipCloseArchive");
		m_lpFName[FNI_FINDFIRST     ] = _T("SevenZipFindFirst");
		m_lpFName[FNI_FINDNEXT      ] = _T("SevenZipFindNext");
		m_lpFName[FNI_SETUNICODEMODE] = _T("SevenZipSetUnicodeMode");
	}
	virtual void c2conv(LPCTSTR w, std::string &c)
	{
		w2utf8(w, c);
	}
};

// RAR,R01
class CGrArchiveUnRAR : public CGrArchiveBase
{
public:
	CGrArchiveUnRAR() : CGrArchiveBase(){
		m_lpFName[FNI_FILE_NAME   ] = _T("UNRAR32.DLL");
		m_lpFName[FNI_GETVERSION  ] = _T("UnrarGetVersion");
		m_lpFName[FNI_GETRUNNING  ] = _T("UnrarGetRunning");
		m_lpFName[FNI_CHECKARCHIVE] = _T("UnrarCheckArchive");
		m_lpFName[FNI_UNARCHIVE   ] = _T("Unrar");
		m_lpFName[FNI_COMMAND_LINE] = _T("-x \"%1!s!\" \"%2!s!\" \"@%3!s!\"");
		m_lpFName[FNI_OPENARCHIVE ] = _T("UnrarOpenArchive");
		m_lpFName[FNI_CLOSEARCHIVE] = _T("UnrarCloseArchive");
		m_lpFName[FNI_FINDFIRST   ] = _T("UnrarFindFirst");
		m_lpFName[FNI_FINDNEXT    ] = _T("UnrarFindNext");
	}
};

// CAB
class CGrArchiveCab : public CGrArchiveBase
{
public:
	CGrArchiveCab() : CGrArchiveBase(){
		m_lpFName[FNI_FILE_NAME   ] = _T("CAB32.DLL");
		m_lpFName[FNI_GETVERSION  ] = _T("CabGetVersion");
		m_lpFName[FNI_GETRUNNING  ] = _T("CabGetRunning");
		m_lpFName[FNI_CHECKARCHIVE] = _T("CabCheckArchive");
		m_lpFName[FNI_UNARCHIVE   ] = _T("Cab");
		m_lpFName[FNI_COMMAND_LINE] = _T("-x \"%1!s!\" -o \"%2!s!\" \"@%3!s!\"");
		m_lpFName[FNI_OPENARCHIVE ] = _T("CabOpenArchive");
		m_lpFName[FNI_CLOSEARCHIVE] = _T("CabCloseArchive");
		m_lpFName[FNI_FINDFIRST   ] = _T("CabFindFirst");
		m_lpFName[FNI_FINDNEXT    ] = _T("CabFindNext");
	}
};
//
CGrArchive::CGrArchive()
{
	m_last_error_archiver[0] = 0;
}

CGrArchive::~CGrArchive()
{
}

struct ArchiveTypeMap {
	LPCTSTR pext;
	CGrArchive::ArchiveType at;
} l_archivetypemap[] = { // pextでソートしておくこと
	{_T("7Z" ), CGrArchive::ARTYPE_7Z },
	{_T("CAB"), CGrArchive::ARTYPE_CAB},
	{_T("LZH"), CGrArchive::ARTYPE_LZH},
	{_T("RAR"), CGrArchive::ARTYPE_RAR},
	{_T("R01"), CGrArchive::ARTYPE_RAR},
	{_T("ZIP"), CGrArchive::ARTYPE_ZIP},
};
static int ArchiveTypeMapCompare(const void *key, const void *pdata)
{
	auto pext  = static_cast<LPCTSTR>(key);
	auto *patm = static_cast<const ArchiveTypeMap*>(pdata);
	return _tcsicmp(pext, patm->pext);
}
bool CGrArchive::IsSupportFile(LPCTSTR filename)
{
	auto pext = CGrShell::GetFileExtConst(filename);
	if(!pext){
		return false;
	}
	auto *patm = static_cast<ArchiveTypeMap*>(bsearch(pext, l_archivetypemap, sizeof(l_archivetypemap)/sizeof(ArchiveTypeMap), sizeof(ArchiveTypeMap), ArchiveTypeMapCompare));
	if(patm){
		return true;
	}
	return false;
}
CGrArchive::ErrorCode CGrArchive::ExtractFull(CGrArchive::ArchiveType at, LPCTSTR filename, LPCTSTR to_dir, UINT iLimitMaxSize)
{
	auto ec = ERCODE_NOSUPPORT;
	switch(at){
	case ARTYPE_LZH:
		{
			CGrArchiveUnLha lha;
			ec=lha.ExtractFull(filename, to_dir, iLimitMaxSize);
			if(ec != ERCODE_SUCCESS){
				_tcscpy_s(m_last_error_archiver, lha.GetName());
			}
		}
		break;
	case ARTYPE_ZIP:
		{
			CGrArchiveUn7Zip sevenzip;
			if(sevenzip.Load()){
				ec = sevenzip.ExtractFull(filename, to_dir, iLimitMaxSize);
				if(ec != ERCODE_SUCCESS){
					_tcscpy_s(m_last_error_archiver, sevenzip.GetName());
				}
			} else {
				CGrArchiveUnZip zip;
				ec = zip.ExtractFull(filename, to_dir, iLimitMaxSize);
				if(ec != ERCODE_SUCCESS){
					_tcscpy_s(m_last_error_archiver, zip.GetName());
				}
			}
		}
		break;
	case ARTYPE_7Z:
		{
			CGrArchiveUn7Zip zip;
			ec = zip.ExtractFull(filename, to_dir, iLimitMaxSize);
			if(ec != ERCODE_SUCCESS){
				_tcscpy_s(m_last_error_archiver, zip.GetName());
			}
		}
		break;
	case ARTYPE_RAR:
		{
			CGrArchiveUnRAR rar;
			ec = rar.ExtractFull(filename, to_dir, iLimitMaxSize);
			if(ec != ERCODE_SUCCESS){
				_tcscpy_s(m_last_error_archiver, rar.GetName());
			}
		}
		break;
	case ARTYPE_CAB:
		{
			CGrArchiveCab cab;
			ec = cab.ExtractFull(filename, to_dir, iLimitMaxSize);
			if(ec != ERCODE_SUCCESS){
				_tcscpy_s(m_last_error_archiver, cab.GetName());
			}
		}
		break;
	}
	return ec;
}
CGrArchive::ErrorCode CGrArchive::ExtractFull(LPCTSTR filename, LPCTSTR to_dir, UINT iLimitMaxSize)
{
	auto pext = CGrShell::GetFileExtConst(filename);
	if(pext){
		auto *patm = static_cast<ArchiveTypeMap*>(bsearch(pext, l_archivetypemap, sizeof(l_archivetypemap)/sizeof(ArchiveTypeMap), sizeof(ArchiveTypeMap), ArchiveTypeMapCompare));
		if(patm){
			return ExtractFull(patm->at, filename, to_dir, iLimitMaxSize);
		}
	}
	return ERCODE_NOSUPPORT;
}

LPCTSTR CGrArchive::GetLastErrorArchiverName()
{
	return m_last_error_archiver;
}
