#include "stdafx.h"
/*
  DBで栞を管理する場合
    ファイルが
      栞ファイル
        栞の読み込み、保存はファイルの内容を優先
      テキストファイル
        DBを優先

  DBで栞を管理しない場合
    ファイルが
      栞ファイル
        栞の読み込み、保存はファイルに
      テキストファイル
        栞から読み込み
        栞ファイルの自動保存の場合のみ保存
 */
#include <windows.h>
#include <windowsx.h>
#include "Shlwapi.h"
#include "Shellapi.h"
#include "stltchar.h"
#include "TxtFuncIBookmark.h"
#include "TxtFuncBookmark.h"
#include "CSVText.h"
#include "Shell.h"
#include "stlutil.h"
#include "resource.h"
#include "BookmarkDB.h"
#include "FunctionKeyMap.h"
#include "KeyState.h"

#define TXTMIRUAPP
#include "TxtMiruFunc.h"

#include <map>
//#define __DBG__
#include "Debug.h"
#ifdef __DBG__
#define BEGIN_FUNC {\
	TCHAR szLanguageName[100];\
	LCID idLocal = GetSystemDefaultLCID();\
	::GetLocaleInfo(idLocal, LOCALE_SENGLANGUAGE, szLanguageName, sizeof(szLanguageName));\
	_tsetlocale(LC_ALL,szLanguageName);\
}
#else
#define BEGIN_FUNC
#endif

#define TXTMIRUAPP

static const TCHAR *ExtSiori = _T(".siori");
enum BM_ATTR { BM_NONE = 0x00, BM_LAST = 0x01, BM_LAST_PAGE = 0x02 };

LPCTSTR g_dataDir = nullptr;
CGrTxtFuncIParam    *g_pParam = nullptr;
CGrTxtFuncIBookmark *g_pBookmark = nullptr;
static UINT       g_wm_funcCall = 0L;
namespace CGrTxtFunc
{
	void MoveDataDir()
	{
		SetCurrentDirectory(g_dataDir);
	}
	LPCTSTR GetDataPath()
	{
		return g_dataDir;
	}
	CGrTxtFuncIParam &Param()
	{
		return *g_pParam;
	}
	LPCTSTR AppName()
	{
		return _T("TxtMiru");
	}
	bool IsWaitting()
	{
		return g_pBookmark->GetParamInteger(STR_PARAMTYPE_IS_WAITTING) == TRUE;
	}

	void GetBookmarkFolder(CGrTxtFuncIParam *pParam, std::tstring &str)
	{
		TCHAR file_path[512];
		TCHAR buf[1024];
		pParam->GetText(CGrTxtFuncIParam::BookMarkFolder, buf, sizeof(buf)/sizeof(TCHAR));
		str = buf;
		_tcscpy_s(file_path, CGrTxtFunc::GetDataPath());
		switch(str.size()){
		case 0:
			CGrText::FormatMessage(str, _T("%1!s!/Bookmark"), file_path);
			break;
		case 1:
			if(CGrShell::IsBackSlash(str[0])){
				// 「\」ルート 指定
				if(CGrShell::IsBackSlash(file_path[0])){
					// ネットワークパスに対して、「\」 トップ指定は多分できないのでデフォルトパスに変更
					CGrText::FormatMessage(str, _T("%1!s!/Bookmark"), file_path);
				} else {
					file_path[2] = _T('\0');
					CGrText::FormatMessage(str, _T("%1!s!/"), file_path);
				}
			} else {
				// 相対パスから絶対パスに変更
				CGrText::FormatMessage(str, _T("%1!s!/%2!s!"), file_path, str.c_str());
			}
			break;
		default:
			if(CGrShell::IsBackSlash(str[0]) && !CGrShell::IsBackSlash(str[1])){
				// 「\」ルート 指定
				if(CGrShell::IsBackSlash(file_path[0])){
					// ネットワークパスに対して、「\」 トップ指定は多分できないのでデフォルトパスに変更
					CGrText::FormatMessage(str, _T("%1!s!/Bookmark"), file_path);
				} else {
					file_path[2] = _T('\0');
					CGrText::FormatMessage(str, _T("%1!s!/%2!s!"), file_path, str.c_str());
				}
			} else if(str[1] != _T(':')){
				// 相対パスから絶対パスに変更
				CGrText::FormatMessage(str, _T("%1!s!/%2!s!"), file_path, str.c_str());
			}
			break;
		}
	}
	//UINT GetFuncCallWindowMessage(){ return g_wm_funcCall; }
	void OpenFile(HWND hWnd, LPCTSTR lpUrl, int page)
	{
		std::tstring url;
		if(page >= 0){
			TCHAR buf[512];
			_stprintf_s(buf, _T("-p=%d "), page);
			url = buf;
		}
		{
			std::vector<TCHAR> buf;
			buf.resize(_tcslen(lpUrl)*2+1);
			lstrcpy(&(buf[0]), lpUrl);
			PathQuoteSpaces(&(buf[0]));
			url += &buf[0];
		}
		url.push_back(_T('\0'));

		COPYDATASTRUCT cpydata;

		cpydata.dwData = 0;
		cpydata.cbData = url.size()*sizeof(TCHAR)+1;
		cpydata.lpData = static_cast<void*>(&(url[0]));

		SendMessage(hWnd, WM_COPYDATA, NULL/*hwnd*/, reinterpret_cast<LPARAM>(&cpydata));
		SetFocus(hWnd);
	}
	int GetCurrentPage(HWND hWnd)
	{
		return SendMessage(hWnd, g_wm_funcCall, static_cast<WPARAM>(CGrTxtFuncIParam::FncCT_GetCurrentPage)/*FuncNo*/, 0);
	}
	int GetCurrentDisplayPage(HWND hWnd)
	{
		return SendMessage(hWnd, g_wm_funcCall, static_cast<WPARAM>(CGrTxtFuncIParam::FncCT_GetCurrentDisplayPage)/*FuncNo*/, 0);
	}
	bool GetSinglePage(HWND hWnd)
	{
		return SendMessage(hWnd, g_wm_funcCall, static_cast<WPARAM>(CGrTxtFuncIParam::FncCT_GetSinglePage)/*FuncNo*/, 0) == 1 ? true : false;
	}
	int AddBookmark(HWND hWnd)
	{
		return PostMessage(hWnd, g_wm_funcCall, static_cast<WPARAM>(CGrTxtFuncIParam::FunCT_AddBookmark)/*FuncNo*/, reinterpret_cast<LPARAM>(hWnd));
	}
	int DeleteBookmark(HWND hWnd, int idx)
	{
		return PostMessage(hWnd, g_wm_funcCall, static_cast<WPARAM>(CGrTxtFuncIParam::FunCT_DeleteBookmark)/*FuncNo*/, static_cast<LPARAM>(idx));
	}
	bool BackupImgFile(int id)
	{
		auto &&param = CGrTxtFunc::Param();
		std::tstring base_path;
		CGrTxtFunc::GetBookmarkFolder(&param, base_path);
		base_path += _T("/cover/");
		CGrShell::ToPrettyFileName(base_path);
		std::tstring backup_path(base_path);
		backup_path += _T("backup/");

		TCHAR find_path[MAX_PATH];
		_stprintf_s(find_path, _T("%s%d.*"), base_path.c_str(), id);
		WIN32_FIND_DATA wfd = {0};
		auto hFile = ::FindFirstFile(find_path, &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return false;
		}
		do {
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				;
			} else {
				CGrShell::CreateFolder(backup_path.c_str());
				std::tstring from_filename(base_path);
				std::tstring backupname(backup_path);
				from_filename += wfd.cFileName;
				backupname += wfd.cFileName;
				::MoveFileEx(from_filename.c_str(), backupname.c_str(), MOVEFILE_REPLACE_EXISTING);
			}
		} while(::FindNextFile(hFile, &wfd));
		::FindClose(hFile);
		return true;
	}
	bool RemoveBackupImgFolder(HWND hWnd)
	{
		auto &&param = CGrTxtFunc::Param();
		std::tstring path;
		CGrTxtFunc::GetBookmarkFolder(&param, path);
		path += _T("/cover/backup");
		CGrShell::ToPrettyFileName(path);
		if(CGrShell::GetSearchDir(path.c_str())){
			std::vector<TCHAR> folder(path.begin(), path.end());
			folder.push_back(_T('\0'));
			folder.push_back(_T('\0'));
			SHFILEOPSTRUCT os = {};
			os.hwnd   = hWnd;
			os.wFunc  = FO_DELETE;
			os.fFlags = FOF_ALLOWUNDO|FOF_NOCONFIRMATION;
			os.pFrom  = &folder[0];

			bool ret = (SHFileOperation(&os) == 0);

			return ret;
		}
		return true;
	}
	bool GetBookByUrl(TxtFuncBookmark::Book& book, LPCTSTR lpURL)
	{
		bool bRet = false;
		do {
			CGrBookmarkDB db;
			if (!db.Open()) {
				break;
			}
			Place place;
			bRet = db.GetPlace(place, lpURL);
			if (!bRet) {
				break;
			}
			book.place_id = place.id;
			book.url = place.url;
			book.title = place.title;
			book.author = place.author;
			bRet = true;
		} while (0);
		return bRet;
	}
}

////////////////////////////////////////////////////////////
// 書式  :
// siori : 001>#Style:スタイル名
// siroi : 002>#Ver:TxtMiru 2.0
// siori : 003>#FileName:ファイル名
// siori : 004>#Title:タイトル
// siori : 005>#Author:作者
// siori : 006>#Page:最後に表示していたページ/ページ数
// siori : 007>#LastWriteTime:最終更新日
// siori : 008>属性[=0x00,0x01],iLine,iIndex,iPos,名称
// siori : 009>属性[=0x02],iPage,0,0,名称
//             ※Open時には、iLine,iIndex,iPosに -1 を入れる
// 属性  : 0x00 = 無し
//         0x01 = 最後に表示していた位置(非表示)
//         0x02 = 最後に表示していたページ(非表示)
// iLine : 物理行
// iIndex: 文字タイプごとのインデックス
// iPos  : iIndex内の位置
// iPage : ページ

static bool isBookmarkFile(LPCTSTR lpFileName)
{
	if(!CGrText::isMatchILast(lpFileName, ExtSiori)){
		return false;
	}
	auto hFile = ::CreateFile(lpFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return false;
	}
	char header[100] = {};
	DWORD dwCanReadSize = 0;
	if (::ReadFile(hFile, header, sizeof(header) / sizeof(char), &dwCanReadSize, 0)) {
		;
	}
	::CloseHandle(hFile);
	return CGrText::Find(reinterpret_cast<LPCTSTR>(header), _T("#Ver:TxtMiru")) != nullptr;
}

static bool openBookmark(CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpFileName, std::tstring &out_open_text_filename, LPCTSTR lpTextFileName = nullptr)
{
	if(CGrShell::IsURI(lpFileName)){
		return false;
	}
	CGrCSVText csv;
	bool bLoadBookMark = false;
	std::tstring open_bm_filename;
	std::tstring open_text_filename;
	if(CGrText::isMatchLast(lpFileName, ExtSiori)){
		CGrShell::ToPrettyFileName(lpFileName, open_text_filename);
		open_text_filename.resize(open_text_filename.size() - _tcslen(ExtSiori));
	} else {
		CGrShell::ToPrettyFileName(lpFileName, open_text_filename);
	}
	if(!csv.Open(lpFileName)){
		return false;
	}
	pBookmark->SetParamString(STR_PARAMTYPE_LASTWRITETIME, nullptr);
	const auto &csv_row = csv.GetCSVROW();
	int len = csv_row.size();
	if(len > 0){
		const auto *it=&csv_row[0];
		for(int row=0;len>0; --len, ++it, ++row){
			const auto &csv_col = (*it);
			int num = csv_col.size();
			if(num <= 0){
				continue;
			}
			auto lpSrc = csv_col[0].c_str();
			if(*lpSrc == _T('#')){
				// パラメータ
				if(CGrText::isMatchChar(lpSrc, _T("#Ver:TxtMiru"))){
					if(lpTextFileName == nullptr){
						bLoadBookMark = true;
					}
				} else if(CGrText::isMatchChar(lpSrc, _T("#FileName:"))){
					std::tstring filename(lpSrc + _tcslen(_T("#FileName:")));
					if(filename.empty()){ return false; }
					CGrShell::ToPrettyFileName(filename.c_str(), open_text_filename);
					if(lpTextFileName && open_text_filename == lpTextFileName){
						bLoadBookMark = true;
					}
				} else if(CGrText::isMatchChar(lpSrc, _T("#LastWriteTime:"))){
					std::tstring lastWriteTime(lpSrc + _tcslen(_T("#LastWriteTime:")));
					pBookmark->SetParamString(STR_PARAMTYPE_LASTWRITETIME, lastWriteTime.c_str());
				}
			} else if(bLoadBookMark && num >= 5){
				int array[4];
				csv.toIntArray(row,  array, sizeof(array)/sizeof(int));
				if(array[1] < 0 || array[2] < 0 || array[3] < 0){
					continue;
				}
				// 栞データ
				switch(array[0]){
				case BM_LAST     : pBookmark->SetLatest(-1, array[1],array[2],array[3], nullptr); break;
				case BM_LAST_PAGE: pBookmark->SetLatest(array[1],-1,-1,-1, nullptr); break;
				case BM_NONE     : pBookmark->AddLeaf(-1, array[1],array[2],array[3], nullptr); break;
				}
			}
		}
	}
	if(bLoadBookMark){
		out_open_text_filename = open_text_filename;
	}
	return bLoadBookMark;
}

static bool openDB(CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpFileName)
{
	if(!lpFileName || lpFileName[0] == '\0'){
		return false;
	}
	CGrBookmarkDB db;
	if(!db.Open()){
		return false;
	}
	Place place;
	if(!db.GetPlace(place, lpFileName)){
		return false;
	}
	pBookmark->SetLatest(place.read_page,-1,-1,-1, nullptr);
	std::vector<Bookmark> bookmark_list;
	if(db.GetBookmarks(bookmark_list, place.id)){
		for(const auto &item : bookmark_list){
			pBookmark->AddLeaf(-1, item.b_line, item.b_index, item.b_pos, item.title.c_str());
		}
	}
	return true;
}

static void add_data(CGrCSVText &csv, enum BM_ATTR attr, CGrTxtFuncIBookmarkLeaf *pLeaf)
{
	csv.AddTail(_T(",,,,"));
	int row = csv.GetRowSize() - 1;
	csv.SetIntegerExAdd(row, 0, attr             );
	csv.SetIntegerExAdd(row, 1, pLeaf->GetLine ());
	csv.SetIntegerExAdd(row, 2, pLeaf->GetIndex());
	csv.SetIntegerExAdd(row, 3, pLeaf->GetPos  ());
	csv.SetTextExAdd   (row, 4, pLeaf->GetName ());
}

static void add_row(CGrCSVText &csv, LPCTSTR lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);

	std::tstring buf;
	CGrText::FormatMessage(buf, lpszFormat, &argList);
	csv.SetTextExAdd(csv.GetRowSize(), 0, buf.c_str());

	va_end(argList);
}
static void add_row(CGrTxtFuncIBookmark *pBookmark, CGrCSVText &csv, LPCTSTR id)
{
	add_row(csv, _T("#%1!s!:%2!s!"), id, pBookmark->GetParamString(id));
}

static bool saveBookmark(CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpBookmarkFileName)
{
	if(_tcslen(lpBookmarkFileName) <= 0 || CGrShell::IsURI(lpBookmarkFileName)){
		return false;
	}
	auto *pLatestLeaf = pBookmark->GetLatest();
	if (!pLatestLeaf) {
		return false;
	}
	CGrCSVText csv;
	add_row(pBookmark, csv, STR_PARAMTYPE_VER          );
	add_row(pBookmark, csv, STR_PARAMTYPE_FILENAME     );
	add_row(pBookmark, csv, STR_PARAMTYPE_TITLE        );
	add_row(pBookmark, csv, STR_PARAMTYPE_AUTHOR       );
	add_row(pBookmark, csv, STR_PARAMTYPE_LASTWRITETIME);
	add_row(csv, _T("#%1!s!:%2!d!/%3!d!"   ), STR_PARAMTYPE_PAGE, pLatestLeaf->GetPage(), pBookmark->GetParamInteger(STR_PARAMTYPE_PAGE));
	add_data(csv, BM_LAST, pLatestLeaf);
	int len = pBookmark->Count();
	if(len > 0){
		for(int idx=0; idx<len; ++idx){
			add_data(csv, BM_NONE, pBookmark->GetLeaf(idx));
		}
	}
	return csv.Save(lpBookmarkFileName) == TRUE;
}

static Bookmark *getBookmark(std::vector<Bookmark> &bookmark_list, Bookmark *pBookmark)
{
	for(auto &&item : bookmark_list){
		if(item.page == pBookmark->page
		   && item.b_line  == pBookmark->b_line
		   && item.b_index == pBookmark->b_index
		   && item.b_pos   == pBookmark->b_pos  ){
			return &item;
		}
	}
	return nullptr;
}

static Subtitle *getSubtitle(std::vector<Subtitle> &subtitle_list, Subtitle *pSubtitle)
{
	for(auto &&item : subtitle_list){
		if(item.page == pSubtitle->page
		   && item.b_order == pSubtitle->b_order
		   && item.title   == pSubtitle->title
		   && item.url     == pSubtitle->url
		   && item.level   == pSubtitle->level
		   && item.b_line  == pSubtitle->b_line
		   && item.b_index == pSubtitle->b_index
		   && item.b_pos   == pSubtitle->b_pos  ){
			return &item;
		}
	}
	return nullptr;
}

static bool saveDB(LPCTSTR lpOpenTextFileName, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, CGrTxtFuncIParam *pParam)
{
	if(!lpOpenTextFileName || lpOpenTextFileName[0] == '\0'){
		return false;
	}
	// 栞フォルダ
	std::tstring str;
	CGrTxtFunc::GetBookmarkFolder(pParam, str);
	std::tstring bookmark_db_file;
	if(!CGrShell::GetSearchDir(str.c_str())){
		if(!CGrShell::CreateFolder(str.c_str())){
			return false;
		}
	}
	CGrBookmarkDB db;
	if(!db.Open()){
		return false;
	}
	if(db.BeginSession() != SQLITE_OK){
		return false;
	}
	std::tstring sysdate;
	CGrDBFunc::GetSysDate(sysdate);
	Place place;
	if(db.GetPlace(place, lpOpenTextFileName)){
	} else {
		place.id          = NULL_INT_VALUE    ;
		place.url         = lpOpenTextFileName;
		place.insert_date = sysdate;
		place.visit_count = 0;
	}
	place.title           = pBookmark->GetParamString(STR_PARAMTYPE_TITLE );
	place.author          = pBookmark->GetParamString(STR_PARAMTYPE_AUTHOR);
	place.page            = pBookmark->GetParamInteger(STR_PARAMTYPE_PAGE);
	place.read_page       = pBookmark->GetLastPage();
	place.visit_count     = place.visit_count + 1;
	place.last_visit_date = sysdate;
	if(!db.PutPlace(place)){
		return false;
	}
	if(db.GetPlace(place, lpOpenTextFileName)){
		History history;
		history.id              = NULL_INT_VALUE       ;
		history.place_id        = place.id             ;
		history.url             = place.url            ;
		history.title           = place.title          ;
		history.author          = place.author         ;
		history.page            = place.page           ;
		history.read_page       = place.read_page      ;
		history.insert_date     = place.insert_date    ;
		history.last_visit_date = place.last_visit_date;
		//
		if(pBookmark->GetParamInteger(_T("IsUpdateCheck")) == 1){
			// 更新チェックモードの時は、last_visit_dateを空白に
			history.insert_date     = place.last_visit_date;
			history.last_visit_date = _T("");
		}
		if(!db.PutHistory(history)){
			return false;
		}
	}
	std::vector<Bookmark> bookmark_list;
	db.GetBookmarks(bookmark_list, place.id);
	int len = pBookmark->Count();
	if(len > 0){
		Bookmark bookmark;
		bookmark.place_id = place.id;
		for(int idx=0; idx<len; ++idx){
			auto *pLeaf = pBookmark->GetLeaf(idx);
			if(!pLeaf){
				continue;
			}
			bookmark.page        = pLeaf->GetPage ();
			bookmark.b_line      = pLeaf->GetLine ();
			bookmark.b_index     = pLeaf->GetIndex();
			bookmark.b_pos       = pLeaf->GetPos  ();
			auto *pDBBoomark = getBookmark(bookmark_list, &bookmark);
			if(pDBBoomark){
				pDBBoomark->id = NULL_INT_VALUE;
				continue;
			}
			bookmark.id          = NULL_INT_VALUE;
			bookmark.title       = pLeaf->GetName ();
			bookmark.insert_date = sysdate;
			db.PutBookmark(bookmark);
		}
	}
	for(const auto &item_bm : bookmark_list){
		if(item_bm.id != NULL_INT_VALUE){
			db.DeleteBookmark(item_bm.id);
		}
	}
	if(pSubTitle){
		std::vector<Subtitle> subtitle_list;
		db.GetSubtitles(subtitle_list, place.id);
		len = pSubTitle->Count();
		for(int idx=0; idx<len; ++idx){
			auto *pLeaf = pSubTitle->GetLeaf(idx);
			if(!pLeaf){
				continue;
			}
			int level = pLeaf->GetLevel();
			if(level >= 999){ // ID ページ内リンク用(非表示:NAMEタグ)
				continue;
			}
			if(level < 0 && pLeaf->GetOrder() == 0){
				continue; // カレントファイルへのリンク
			}
			Subtitle subtitle;
			subtitle.place_id    = place.id            ;
			subtitle.b_order     = idx                 ;
			subtitle.title       = pLeaf->GetName    ();
			subtitle.url         = pLeaf->GetFileName();
			subtitle.page        = pLeaf->GetPage    ();
			subtitle.b_line      = pLeaf->GetLine    ();
			subtitle.b_index     = pLeaf->GetIndex   ();
			subtitle.b_pos       = pLeaf->GetPos     ();
			subtitle.level       = pLeaf->GetLevel   ();

			auto *pDBSubtitle = getSubtitle(subtitle_list, &subtitle);
			if(pDBSubtitle){
				pDBSubtitle->id = NULL_INT_VALUE;
				continue;
			} else {
				subtitle.id     = NULL_INT_VALUE; // Auto
				subtitle.insert_date = sysdate;
			}
			db.PutSubtitle(subtitle);
		}
		for(const auto &item : subtitle_list){
			if(item.id != NULL_INT_VALUE){
				db.DeleteSubtitle(item.id);
			}
		}
	}
	if(db.Commit() != SQLITE_OK){
		return false;
	}
	return true;
}
static void ShowErrorMessag(LONG lerrno)
{
	LPVOID lpMsgBuf = nullptr;
	//ここにチェックしたい処理を書く
	FormatMessage(
		//エラー表示文字列作成
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, lerrno,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, NULL);
	MessageBox(NULL, reinterpret_cast<LPCTSTR>(lpMsgBuf), _T("TxtMiru"), MB_OK);
	LocalFree(lpMsgBuf);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncBookmarkOpen(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam)
{
	BEGIN_FUNC;
	//
	g_dataDir   = lpDataDir;
	g_pParam    = pParam;
	if(!lpDataDir || !pParam){
		return false;
	}
	TCHAR in_filename[MAX_PATH+1] = {};
	if(lpFileName && lpFileName[0] == _T('"')){
		// 引数で、ブランクが含まれている場合 「"」で括られてくるので CGrCSVTextを使って 「"」を外す
		CGrCSVText csv(lpFileName);
		lstrcpy(in_filename, csv.GetText(0,0));
		lpFileName = in_filename;
	}
	//
	pBookmark->ClearLeaf();
	std::tstring open_text_filename;
	std::tstring open_bm_filename  ;
	bool bLoadBookMark = false;
	if(isBookmarkFile(lpFileName) && openBookmark(pBookmark, lpFileName, open_text_filename)){
		// 栞ファイル
		bLoadBookMark = true;
		CGrShell::ToPrettyFileName(lpFileName, open_bm_filename);
	} else {
		// 栞ファイルではない
		CGrShell::ToPrettyFileName(lpFileName, open_text_filename);
		if(pParam->GetBoolean(CGrTxtFuncIParam::BookMarkToFolder)){
			// DBで栞を管理する場合 [栞ファイルは栞フォルダに保存]
			bLoadBookMark = openDB(pBookmark, lpFileName); // DBから栞取得
		}
		if(!bLoadBookMark){
			// テキストファイル名.siori で栞ファイルが無いかチェック
			std::tstring bookmark_filename;
			CGrShell::ToPrettyFileName(lpFileName, bookmark_filename);
			bookmark_filename += ExtSiori;
			if(isBookmarkFile(bookmark_filename.c_str())){
				bLoadBookMark = openBookmark(pBookmark, bookmark_filename.c_str(), open_text_filename, lpFileName);
			}
		}
	}
	pBookmark->SetParamString(STR_PARAMTYPE_FILENAME  , open_text_filename.c_str());
	pBookmark->SetParamString(STR_PARAMTYPE_BMFILENAME, open_bm_filename.c_str());
	return bLoadBookMark;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncBookmarkSaveAs(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam)
{
	BEGIN_FUNC;
	//
	g_dataDir   = lpDataDir;
	g_pParam    = pParam;
	if(!lpDataDir || !pParam){
		return false;
	}
	std::tstring save_bm_filenmae;
	if(lpFileName){
		CGrShell::ToPrettyFileName(lpFileName, save_bm_filenmae);
	} else {
		CGrShell::ToPrettyFileName(pBookmark->GetParamString(STR_PARAMTYPE_BMFILENAME), save_bm_filenmae);
	}
	std::tstring open_text_filename = pBookmark->GetParamString(STR_PARAMTYPE_FILENAME);
	//
	if(pParam->GetBoolean(CGrTxtFuncIParam::BookMarkToFolder)){
		// DBで栞を管理する場合
		if(!saveDB(open_text_filename.c_str(), pBookmark, pSubTitle, pParam)){
			return false;
		}
	}
	// 栞ファイルが指定されている場合のみ栞ファイルに保存
	// ※栞ファイルから開いている場合と名前をつけて栞ファイルを保存する場合
	return saveBookmark(pBookmark, save_bm_filenmae.c_str());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
#include "BookListDlg.h"

extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncShowBookList(HWND hWnd, LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam)
{
	BEGIN_FUNC;
	if(hWnd == NULL){
		auto* pWnd = static_cast<CGrBookListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_BOOKDLG));
		if(pWnd && pWnd->GetWnd()){
			ShowWindow(pWnd->GetWnd(), SW_HIDE);
		}
		return true;
	}
	g_dataDir   = lpDataDir;
	g_pParam    = pParam;
	g_pBookmark = pBookmark;
	if(!lpDataDir || !pParam){
		return false;
	}
	if(g_wm_funcCall == 0){
		g_wm_funcCall = RegisterWindowMessage(WM_TXTMIRU_FUNC_CALL);
	}
	auto* pWnd = static_cast<CGrBookListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_BOOKDLG));
	if(pWnd && !pWnd->GetWnd()){
		UnInstallWnd(TxtFuncBookmark::MWID_BOOKDLG);
	}
	if(pWnd){
		pWnd->SetFilename(lpFileName);
		pWnd->Show(hWnd);
	} else {
		pWnd = new CGrBookListDlg(hWnd, lpFileName, pBookmark);
		InstallWnd(TxtFuncBookmark::MWID_BOOKDLG, pWnd);
	}
	return true;
}

extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncRefreshBookList()
{
	BEGIN_FUNC;
	auto* pWnd = static_cast<CGrBookListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_BOOKDLG));
	if(pWnd){
		auto hWnd = pWnd->GetWnd();
		if(hWnd && IsWindowVisible(hWnd)){
			if(g_pBookmark){
				pWnd->SetFilename(g_pBookmark->GetParamString(_T("FileName")));
			}
		}
		pWnd->PostRefresh();
	}
	return true;
}

extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsShowBookList()
{
	BEGIN_FUNC;
	//
	auto* pWnd = static_cast<CGrBookListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_BOOKDLG));
	return (pWnd && pWnd->GetWnd() && IsWindowVisible(pWnd->GetWnd()));
}

extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsBookListMember(HWND hWnd)
{
	BEGIN_FUNC;
	//
	auto* pWnd = static_cast<CGrBookListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_BOOKDLG));
	if(pWnd){
		auto hLinkWnd = pWnd->GetWnd();
		return (hLinkWnd && IsWindowVisible(hLinkWnd) && (hLinkWnd == hWnd || IsChild(hLinkWnd, hWnd)));
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LinkDlg.h"

extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncShowLinkList(HWND hWnd, int mode, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam)
{
	BEGIN_FUNC;
	//
	g_dataDir   = lpDataDir;
	g_pParam    = pParam;
	g_pBookmark = pBookmark;
	if(!lpDataDir || !pParam){
		return false;
	}
	if(g_wm_funcCall == 0){
		g_wm_funcCall = RegisterWindowMessage(WM_TXTMIRU_FUNC_CALL);
	}
	auto* pWnd = static_cast<CGrLinkDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_LINKDLG));
	if(pWnd && !pWnd->GetWnd()){
		UnInstallWnd(TxtFuncBookmark::MWID_LINKDLG);
	}
	if(!pWnd && mode != 3){
		pWnd = new CGrLinkDlg(hWnd, mode, pBookmark, pSubTitle);
		InstallWnd(TxtFuncBookmark::MWID_LINKDLG, pWnd);
	} else if(pWnd){
		switch(mode){
		case 1: pWnd->ShowBookmark(hWnd, true); break;
		case 2: pWnd->ShowSubtitle(hWnd, true); break;
		case 0: pWnd->ShowNormal(hWnd); break;
		default:
			if(pWnd->GetWnd()){
				ShowWindow(pWnd->GetWnd(), SW_HIDE);
			}
			break;
		}
	}
	return true;
}

extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncRefreshLinkList()
{
	BEGIN_FUNC;
	auto* pWnd = static_cast<CGrLinkDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_LINKDLG));
	if(pWnd){
		pWnd->PostRefresh();
	}

	return true;
}

extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsShowLinkList()
{
	BEGIN_FUNC;
	//
	auto* pWnd = static_cast<CGrLinkDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_LINKDLG));
	return (pWnd && pWnd->GetWnd() && IsWindowVisible(pWnd->GetWnd()));
}

extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsLinkListMember(HWND hWnd)
{
	BEGIN_FUNC;
	//
	auto* pWnd = static_cast<CGrLinkDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_LINKDLG));
	if(pWnd){
		auto hLinkWnd = pWnd->GetWnd();
		return (hLinkWnd && IsWindowVisible(hLinkWnd) && (hLinkWnd == hWnd || IsChild(hLinkWnd, hWnd)));
	}
	return false;
}

extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncUpdatePage()
{
	BEGIN_FUNC;
	{
		auto* pWnd = static_cast<CGrBookListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_BOOKDLG));
		if(pWnd){
			pWnd->PostRefreshSubtitleList();
		}
	}
	{
		auto* pWnd = static_cast<CGrLinkDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_LINKDLG));
		if(pWnd){
			pWnd->PtstUpdatePage();
		}
	}
	return true;
}

extern "C" TXTFUNCBOOKMARK_API HWND cdecl TxtFuncGetWnd(int id)
{
	auto* pWnd = static_cast<CGrModelessWnd*>(GetInstallWnd((TxtFuncBookmark::ModelWindowID)id));
	if(pWnd){
		return pWnd->GetWnd();
	}
	return NULL;
}

#include "TxtSearchFiles.h"
extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncSearchFiles(HWND hWnd, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam)
{
	BEGIN_FUNC;

	if(hWnd == NULL){
		auto* pWnd = static_cast<CGrTxtSearchFiles*>(GetInstallWnd(TxtFuncBookmark::MWID_SEARCHDLG));
		if(pWnd && pWnd->GetWnd()){
			ShowWindow(pWnd->GetWnd(), SW_HIDE);
		}
		return true;
	}
	g_dataDir   = lpDataDir;
	g_pParam    = pParam;
	if(!lpDataDir || !pParam){
		return false;
	}
	if(g_wm_funcCall == 0){
		g_wm_funcCall = RegisterWindowMessage(WM_TXTMIRU_FUNC_CALL);
	}
	auto* pWnd = static_cast<CGrTxtSearchFiles*>(GetInstallWnd(TxtFuncBookmark::MWID_SEARCHDLG));
	if(pWnd && !pWnd->GetWnd()){
		UnInstallWnd(TxtFuncBookmark::MWID_SEARCHDLG);
	}
	if(pWnd){
		pWnd->Show(hWnd);
	} else {
		pWnd = new CGrTxtSearchFiles(hWnd);
		InstallWnd(TxtFuncBookmark::MWID_SEARCHDLG, pWnd);
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
#include "RubyListDlg.h"
extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncShowRubyList(HWND hWnd, void *pDoc, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam)
{
	BEGIN_FUNC;
	if(hWnd == NULL){
		auto* pWnd = static_cast<CGrRubyListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_RUBYDLG));
		if(pWnd && pWnd->GetWnd()){
			ShowWindow(pWnd->GetWnd(), SW_HIDE);
		}
		return true;
	}
	g_dataDir   = lpDataDir;
	g_pParam    = pParam;
	g_pBookmark = pBookmark;
	if(!lpDataDir || !pParam){
		return false;
	}
	if(g_wm_funcCall == 0){
		g_wm_funcCall = RegisterWindowMessage(WM_TXTMIRU_FUNC_CALL);
	}
	auto* pWnd = static_cast<CGrRubyListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_RUBYDLG));
	if(pWnd && !pWnd->GetWnd()){
		UnInstallWnd(TxtFuncBookmark::MWID_RUBYDLG);
	}
	if(pWnd){
		pWnd->Show(hWnd);
	} else {
		pWnd = new CGrRubyListDlg(hWnd, pDoc);
		InstallWnd(TxtFuncBookmark::MWID_RUBYDLG, pWnd);
	}
	return true;
}
extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsShowRubyList()
{
	BEGIN_FUNC;
	auto* pWnd = static_cast<CGrRubyListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_RUBYDLG));
	return (pWnd && pWnd->GetWnd() && IsWindowVisible(pWnd->GetWnd()));
}
extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncRefreshRubyList()
{
	BEGIN_FUNC;
	auto* pWnd = static_cast<CGrRubyListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_RUBYDLG));
	if(pWnd){
		pWnd->PostRefresh();
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
#include "TxtFuncSaveText.h"
extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncSaveText(HWND hWnd, void *pDoc, LPCTSTR lpFileName, LPCTSTR lpRealFileName, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam)
{
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
#include "TxtSearchDlg.h"
extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncSearch(HWND hWnd, void *pDoc, const TxtMiru::TextPoint &point, TxtMiru::TextPoint &out_point, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam)
{
	BEGIN_FUNC;
	g_dataDir   = lpDataDir;
	g_pParam    = pParam;
	g_pBookmark = pBookmark;
	if(!lpDataDir || !pParam){
		return false;
	}
	CGrTxtSearchDlg dlg(pDoc, point);
	if(IDOK == dlg.DoModal(hWnd)){
		out_point = dlg.GetTextPoint();
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" TXTFUNCBOOKMARK_API bool cdecl TxtFuncRefresh(int id)
{
	BEGIN_FUNC;
	switch(id){
	case TxtFuncBookmark::MWID_BOOKDLG:
		{
			auto* pWnd = static_cast<CGrBookListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_BOOKDLG));
			if(pWnd){
				auto hWnd = pWnd->GetWnd();
				if(hWnd && IsWindowVisible(hWnd)){
					if(g_pBookmark){
						pWnd->SetFilename(g_pBookmark->GetParamString(_T("FileName")));
					}
				}
				pWnd->PostRefresh();
			}
		}
		break;
	case TxtFuncBookmark::MWID_LINKDLG:
		{
			auto* pWnd = static_cast<CGrLinkDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_LINKDLG));
			if(pWnd){
				pWnd->PostRefresh();
			}
		}
		break;
	case TxtFuncBookmark::MWID_RUBYDLG:
		{
			auto* pWnd = static_cast<CGrRubyListDlg*>(GetInstallWnd(TxtFuncBookmark::MWID_RUBYDLG));
			if(pWnd){
				pWnd->PostRefresh();
			}
		}
		break;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
#include "BookDlg.h"
extern "C" TXTFUNCBOOKMARK_API int cdecl TxtFuncAddFavorite(HWND hWnd, LPCTSTR lpURL)
{
	TxtFuncBookmark::Book book;
	book.id = -1;
	book.place_id = -1;
	book.page = -1;
	book.read_page = -1;
	book.visit_count = -1;
	book.read_cnt = -1;
	book.total_cnt = -1;
	book.position = -1;
	CGrTxtFunc::GetBookByUrl(book, lpURL);
	CGrBookDlg dlg(book, -1);
	return dlg.DoModal(hWnd);
}
