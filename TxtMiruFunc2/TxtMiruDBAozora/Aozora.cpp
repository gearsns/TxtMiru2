#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <wininet.h>
#include "resource.h"
#include "Aozora.h"
#include "TxtMiru.h"
#include "CSVText.h"
#include "stlutil.h"
#include "BookMarkDB.h"
#include "shell.h"
#include <map>

struct AozoraCSVColumn {
	LPCTSTR name;
	LPCTSTR field;
} AozoraCSVColumnList[] = {
	{_T("作品ID"                      ),_T(""                 )}, //  0
	{_T("作品名"                      ),_T("TITLE"            )}, //  1
	{_T("作品名読み"                  ),_T("TITLE_R"          )}, //  2
	{_T("ソート用読み"                ),_T("TITLE_S"          )}, //  3
	{_T("副題"                        ),_T("SUBTITLE"         )}, //  4
	{_T("副題読み"                    ),_T("SUBTITLE_R"       )}, //  5
	{_T("原題"                        ),_T("ORGTITLE"         )}, //  6
	{_T("初出"                        ),_T("NOTE_F"           )}, //  7
	{_T("分類番号"                    ),_T("CLASSIFICATION"   )}, //  8
	{_T("文字遣い種別"                ),_T("CHAR_TYPE"        )}, //  9
	{_T("作品著作権フラグ"            ),_T(""                 )}, // 10
	{_T("公開日"                      ),_T("RELEASE_DATE"     )}, // 11
	{_T("最終更新日"                  ),_T(""                 )}, // 12
	{_T("図書カードURL"               ),_T(""                 )}, // 13
	{_T("人物ID"                      ),_T(""                 )}, // 14
	{_T("姓"                          ),_T("FAMILY_NAME"      )}, // 15
	{_T("名"                          ),_T("GIVEN_NAME"       )}, // 16
	{_T("姓読み"                      ),_T("FAMILY_NAME_R"    )}, // 17
	{_T("名読み"                      ),_T("GIVEN_NAME_R"     )}, // 18
	{_T("姓読みソート用"              ),_T("FAMILY_NAME_S"    )}, // 19
	{_T("名読みソート用"              ),_T("GIVEN_NAME_S"     )}, // 20
	{_T("姓ローマ字"                  ),_T("FAMILY_NAME_ROMEN")}, // 21
	{_T("名ローマ字"                  ),_T("GIVEN_NAME_ROMEN" )}, // 22
	{_T("役割フラグ"                  ),_T("ROLE_FLAG"        )}, // 23
	{_T("生年月日"                    ),_T(""                 )}, // 24
	{_T("没年月日"                    ),_T(""                 )}, // 25
	{_T("人物著作権フラグ"            ),_T(""                 )}, // 26
	{_T("底本名1"                     ),_T(""                 )}, // 27
	{_T("底本出版社名1"               ),_T(""                 )}, // 28
	{_T("底本初版発行年1"             ),_T(""                 )}, // 29
	{_T("入力に使用した版1"           ),_T(""                 )}, // 30
	{_T("校正に使用した版1"           ),_T(""                 )}, // 31
	{_T("底本の親本名1"               ),_T(""                 )}, // 32
	{_T("底本の親本出版社名1"         ),_T(""                 )}, // 33
	{_T("底本の親本初版発行年1"       ),_T(""                 )}, // 34
	{_T("底本名2"                     ),_T(""                 )}, // 35
	{_T("底本出版社名2"               ),_T(""                 )}, // 36
	{_T("底本初版発行年2"             ),_T(""                 )}, // 37
	{_T("入力に使用した版2"           ),_T(""                 )}, // 38
	{_T("校正に使用した版2"           ),_T(""                 )}, // 39
	{_T("底本の親本名2"               ),_T(""                 )}, // 40
	{_T("底本の親本出版社名2"         ),_T(""                 )}, // 41
	{_T("底本の親本初版発行年2"       ),_T(""                 )}, // 42
	{_T("入力者"                      ),_T(""                 )}, // 43
	{_T("校正者"                      ),_T(""                 )}, // 44
	{_T("テキストファイルURL"         ),_T(""                 )}, // 45
	{_T("テキストファイル最終更新日"  ),_T(""                 )}, // 46
	{_T("テキストファイル符号化方式"  ),_T(""                 )}, // 47
	{_T("テキストファイル文字集合"    ),_T(""                 )}, // 48
	{_T("テキストファイル修正回数"    ),_T(""                 )}, // 49
	{_T("XHTML/HTMLファイルURL"       ),_T("HTML_URL"         )}, // 50
	{_T("XHTML/HTMLファイル最終更新日"),_T("HTML_UPDATE_DATE" )}, // 51
	{_T("XHTML/HTMLファイル符号化方式"),_T(""                 )}, // 52
	{_T("XHTML/HTMLファイル文字集合"  ),_T(""                 )}, // 53
	{_T("XHTML/HTMLファイル修正回数"  ),_T(""                 )}, // 54
};

static AozoarDBError aozoraCSV2DB(CGrCSVText &csv)
{
	auto &&row = csv.GetCSVROW();
	int n_column[sizeof(AozoraCSVColumnList)/sizeof(AozoraCSVColumn)] = {};
	bool bHead = true;
	std::map<std::tstring,int> field_map;

	auto ret = ADBERR_CSVFILE_NODATA;

	std::tstring sysdate;
	CGrDBFunc::GetSysDate(sysdate);
	CGrBookmarkDB db;
	db.Open();
	db.BeginSession();
	db.DeleteAozoraBooks();
	for(const auto &line : row){
		if(bHead){
			bHead = false;
			int col = 0;
			for(const auto &item: line){
				field_map[item] = col;
				++col;
			}
			col = 0;
			bool bBreak = false;
			for(const auto &item : AozoraCSVColumnList){
				if(field_map.count(item.name) > 0){
					n_column[col] = field_map[item.name];
				} else {
					bBreak = true;
					break;
				}
				++col;
			}
			if(bBreak){
				break;
			}
			continue;
		}
		AozoraBooks books;
		books.id                = NULL_INT_VALUE;
		books.title             = line[n_column[ 1]];
		books.title_r           = line[n_column[ 2]];
		books.title_s           = line[n_column[ 3]];
		books.subtitle          = line[n_column[ 4]];
		books.subtitle_r        = line[n_column[ 5]];
		books.orgtitle          = line[n_column[ 6]];
		books.note_f            = line[n_column[ 7]];
		books.classification    = line[n_column[ 8]];
		books.char_type         = line[n_column[ 9]];
		books.release_date      = line[n_column[11]];
		books.family_name       = line[n_column[15]];
		books.given_name        = line[n_column[16]];
		books.family_name_r     = line[n_column[17]];
		books.given_name_r      = line[n_column[18]];
		books.family_name_s     = line[n_column[19]];
		books.given_name_s      = line[n_column[20]];
		books.family_name_romen = line[n_column[21]];
		books.given_name_romen  = line[n_column[22]];
		books.role_flag         = line[n_column[23]];
		books.html_url          = line[n_column[50]];
		books.html_update_date  = line[n_column[51]];
		books.insert_date       = sysdate;
		std::replace(books.release_date    , _T("-"), _T("/"));
		std::replace(books.html_update_date, _T("-"), _T("/"));
		db.Put(books);
		ret = ADBERR_SUCCESS;
	}
	db.Commit();
	db.Close();
	return ret;
}
static std::tstring l_str_updatedate;
static void setUpdateDate(LPCTSTR lpFileName)
{
	WIN32_FIND_DATA wfd = {};
	if(CGrShell::getFileInfo(lpFileName, wfd)){
		SYSTEMTIME st = {};
		FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &wfd.ftLastWriteTime);
		FileTimeToSystemTime(&wfd.ftLastWriteTime, &st);
		TCHAR val[2048];
		wsprintf(val, _T("%04d/%02d/%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		l_str_updatedate = val;
	}
}
void GetAozoraUpdateDate(std::tstring &str_date)
{
	if(l_str_updatedate.empty()){
		std::tstring aozora_db_filename;
		CGrTxtFunc::GetBookmarkFolder(aozora_db_filename);
		aozora_db_filename += _T("/aozora.db");
		setUpdateDate(aozora_db_filename.c_str());
	}
	str_date = l_str_updatedate;
}
static AozoarDBError AozoraCSV2DB(LPCTSTR strCSVFilename)
{
	CGrCSVText csv;
	if(!csv.Open(strCSVFilename)){
		return ADBERR_CSVFILE_OPEN;
	}
	setUpdateDate(strCSVFilename);
	if(csv.GetRowSize() <= 0){
		return ADBERR_CSVFILE_NODATA;
	}
	return aozoraCSV2DB(csv);
}
#include "libzippp.h"
#pragma comment(lib, "libzippp_static.lib")
#pragma comment(lib, "zlibstatic.lib")
#pragma comment(lib, "zip.lib")
static AozoarDBError AozoraZip2DB(LPCTSTR strZipFilename)
{
	auto ret = ADBERR_SUCCESS;

	std::string strZipFilenameUTF8;
	w2utf8(strZipFilename, strZipFilenameUTF8, CP_ACP);
	libzippp::ZipArchive zf(strZipFilenameUTF8.c_str());
	if (!zf.open(libzippp::ZipArchive::READ_ONLY)) {
		return ADBERR_ZIPFILE_OPEN;
	}
	std::tstring str;
	libzippp::ZipEntry entry1 = zf.getEntry("list_person_all_extended_utf8.csv");
	if (!entry1.isNull()) {
		std::string data = entry1.readAsText();
		if (data.size() > 3 && (data[0] & 0xff) == 0xef && (data[1] & 0xff) == 0xbb && (data[2] & 0xff) == 0xbf) {
			utf82w(&data[3], str);
		}
		else {
			utf82w(&data[0], str);
		}
		auto localtime = entry1.getDate();
		struct tm localT;
		gmtime_s(&localT, &localtime);
		TCHAR val[2048];
		wsprintf(val, _T("%04d/%02d/%02d %02d:%02d:%02d"), localT.tm_year+ 1900, localT.tm_mon + 1, localT.tm_mday, localT.tm_hour, localT.tm_min, localT.tm_sec);
		l_str_updatedate = val;
	}

	zf.close();
	zf.unlink();

	CGrCSVText csv;
	csv.AddTailCSVData(str.c_str());
	ret = aozoraCSV2DB(csv);

	return ret;
}

/*
 lpwUrlで指定されたURIのキャッシュの情報を取得します。
 取得後、ファイル名をlpszwFileNameに対して書き込みます。
 */
static bool GetUrlCacheFileName(LPCWSTR lpwUrl, LPWSTR lpszwFileName, DWORD dwFileNameInBytes)
{
	if(CGrText::isMatchChar(lpwUrl, _T("file:///"))){
		if (lstrcpynW(lpszwFileName, &(lpwUrl[sizeof(_T("file:///")) / sizeof(TCHAR) - 1]), dwFileNameInBytes)) {

		}
		return true;
	}
	DWORD dwCbCacheInfo = 1024;
	auto lpCacheInfo = reinterpret_cast<LPINTERNET_CACHE_ENTRY_INFO>(new char[dwCbCacheInfo]);
	if (!lpCacheInfo) {
		return false;
	}
	bool bret = true;

	do {
		ZeroMemory(lpCacheInfo, dwCbCacheInfo);
		if(!GetUrlCacheEntryInfo(lpwUrl, lpCacheInfo, &dwCbCacheInfo)){
			switch(GetLastError()){
			case ERROR_FILE_NOT_FOUND:
				if(S_OK == URLDownloadToCacheFile(NULL, lpwUrl, lpszwFileName, dwFileNameInBytes, BINDF_IGNORESECURITYPROBLEM/*SSLエラーが出るので*/, NULL)){
					delete [] lpCacheInfo;
					return true;
				}
				bret = false;
				break;
			case ERROR_INSUFFICIENT_BUFFER:
				delete [] lpCacheInfo;
				lpCacheInfo = reinterpret_cast<LPINTERNET_CACHE_ENTRY_INFO>(new char[dwCbCacheInfo]);
				continue;
			default:
				bret = false;
				break;
			}
		}
		break;
	} while(true);
	if(bret && FAILED(_tcscpy_s(lpszwFileName, dwFileNameInBytes, lpCacheInfo->lpszLocalFileName))){
		bret = false;
	}
	delete [] lpCacheInfo;

	if (bret) {
		// gzip圧縮のファイル場合、内容が 1F 8B 08 になるhtmファイルが取得されるので URLDownloadToCacheFileでファイルを取得しなおす
		auto lpExt = CGrShell::GetFileExt(lpszwFileName);
		if (lpExt && (0 == _tcsicmp(lpExt, _T("html"))|| 0 == _tcsicmp(lpExt, _T("htm")))) {
			if (S_OK == URLDownloadToCacheFile(NULL, lpwUrl, lpszwFileName, dwFileNameInBytes, 0, NULL)) {
				return true;
			}
		}
	}
	return bret;
}

static AozoarDBError AozoraWeb2DB(LPCTSTR strURL)
{
	TCHAR fileName[2048] = {};
	DeleteUrlCacheEntry(strURL);
	if(::GetUrlCacheFileName(strURL, fileName, sizeof(fileName)/sizeof(TCHAR))){
		return AozoraZip2DB(fileName);
	} else {
		return ADBERR_WEBFILE_OPEN;
	}
}

AozoarDBError AozoraUpdate(LPCTSTR strURL)
{
	if(CGrShell::IsURI(strURL)){
		return AozoraWeb2DB(strURL);
	} else if(_tcsicmp(CGrShell::GetFileExtConst(strURL), _T("zip")) == 0){
		return AozoraZip2DB(strURL);
	} else {
		return AozoraCSV2DB(strURL);
	}
}
