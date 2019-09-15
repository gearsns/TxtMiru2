// Shell.c
//
// ファイル操作
//
#define STRICT // 型を宣言および使用時に、より厳密な型チェックが行われます。

#include <windows.h>
#include <shobjidl.h>
#include <shlguid.h>
#include "stlutil.h"
#ifdef _USRDLL
#else
#include "stdwin.h"
#endif
#include "Shell.h"
#pragma warning (push)
#pragma warning(disable:4768)
#pragma warning(disable:4091)
#include <shlobj.h>
#pragma warning (pop)

#define IS_ALPH(__ch__) (__ch__ >= _T('A') && __ch__ <= _T('Z'))
#define IS_NUM(__ch__) (__ch__ >= _T('0') && __ch__ <= _T('9'))

namespace CGrShell
{
	bool GetPathFromLink(LPCTSTR linkFilename, std::tstring &filename)
	{
		IShellLink   *psl = nullptr;
		IPersistFile *ppf = nullptr;
		bool ret = false;

		do {
			auto hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, reinterpret_cast<void**>(&psl));
			if(FAILED(hres)){
				break;
			}
			hres = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf));
			if(FAILED(hres)){
				break;
			}
			hres = ppf->Load(linkFilename, STGM_READ);
			if(FAILED(hres)){
				break;
			}
			TCHAR fullPath[_MAX_PATH] = {};
			hres = psl->GetPath(fullPath, _MAX_PATH, nullptr, 0);
			if(FAILED(hres)){
				break;
			}
			filename = fullPath;
			ret = true;
		} while(0);
		if(ppf){
			ppf->Release();
		}
		if(psl){
			psl->Release();
		}
		return ret;
	}
	// フォルダ選択ダイアログのCallback関数
	//   初期選択フォルダを指定するために追加
	//static int CALLBACK browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	// ファイル名の表記を整えます。 /./,\ -> /
	void ToPrettyFileName(LPCTSTR fromFileName, std::tstring &toFileName)
	{
		toFileName = fromFileName;
		ToPrettyFileName(toFileName);
		if(toFileName.size() > 2 && toFileName[0] == _T('"') && toFileName[toFileName.length()-1] == _T('"')){
			toFileName = toFileName.substr(1, toFileName.length()-2);
		}
	}
	void ToPrettyFileName(std::tstring &fileName)
	{
		std::replace(fileName, _T("\n"), _T(""));
		std::replace(fileName, _T("\r"), _T(""));
		std::replace(fileName, _T("\\"), _T("/"));
		std::replace(fileName, _T("/./"), _T("/"));
	}

	// 指定ファイルが実行ファイルかどうかを取得します。
	//   fileName : ファイル名
	//   戻り値   : 実行ファイルならTRUE
	BOOL IsExecuteFile(TCHAR *fileName)
	{
		// 拡張子の取得
		auto *ext = GetFileExt(fileName);
		if(!ext){
			return FALSE;
		}
		if(CGrText::niCmp(_T("exe"), ext, 3) == 0 ||
		   CGrText::niCmp(_T("com"), ext, 3) == 0 ||
		   CGrText::niCmp(_T("bat"), ext, 3) == 0){
			return TRUE;
		}
		return FALSE;
	}

	// 拡張子開始位置を取得します。
	//   fileName : ファイル名
	//   戻り値   : 拡張子開始位置
	LPCTSTR GetFileExtConst(LPCTSTR fileName)
	{
		return GetFileExt(const_cast<TCHAR*>(fileName));
	}
	// 拡張子開始位置を取得します。
	//   fileName : ファイル名
	//   戻り値   : 拡張子開始位置
	TCHAR *GetFileExt(TCHAR *fileName)
	{
		const auto *lpNextSrc = fileName;
		while(*lpNextSrc){ lpNextSrc = CGrText::CharNext(lpNextSrc); }
		lpNextSrc = CharPrev(fileName, lpNextSrc);
		auto lpSrc = lpNextSrc;
		while(lpNextSrc != fileName && lpNextSrc && *lpNextSrc && !IsBackSlash(*lpNextSrc)){
			if(CGrText::niCmp(_T("."), lpNextSrc, 1) == 0){
				return const_cast<TCHAR*>(lpSrc);
			}
			lpSrc = lpNextSrc;
			lpNextSrc = CharPrev(fileName, lpNextSrc);
		}
		return nullptr;
	}

	// ファイル名開始位置を取得します。
	//   fileName  [in]: ファイル名
	//   戻り値   [out]: ファイル名開始位置
	const TCHAR *GetFileName(const TCHAR *fileName)
	{
		auto *lpNextSrc = fileName;
		while(*lpNextSrc){ lpNextSrc = CGrText::CharNext(lpNextSrc); }
		lpNextSrc = CharPrev(fileName, lpNextSrc);
		auto lpSrc = lpNextSrc;
		while(lpNextSrc != fileName && lpNextSrc && *lpNextSrc && !IsBackSlash(*lpNextSrc)){
			lpSrc = lpNextSrc;
			lpNextSrc = CharPrev(fileName, lpNextSrc);
		}
		return lpSrc;
	}

	// パスからファイル名を削除
	//   path : パス
	BOOL RemoveFileName(TCHAR *path)
	{
		auto *p = const_cast<TCHAR*>(GetFileName(path));
		if(p){
			if(!IsBackSlash(*p)){
				p = CharPrev(path, p);
			}
			*p = _T('\0');
			return TRUE;
		}
		return FALSE;
	}

	// パス区切り記号かどうか
	//   c      : 文字
	//   戻り値 : パス区切り文字の場合、TRUE
	bool IsBackSlash(TCHAR c)
	{
		return (c == _T('\\') || c == _T('/'));
	}
	// ファイルの情報取得
	bool getFileInfo(LPCTSTR lpFileName, WIN32_FIND_DATA &wfd)
	{
		auto hFile = ::FindFirstFile(lpFileName, &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return false;
		}
		::FindClose(hFile);
		return true;
	}

	// データ保存パスの取得
	void GetDataPath(TCHAR *path, int max_path, LPCTSTR appName)
	{
		// LOCALAPPDATAのフォルダを得る。
		TCHAR dataPath[MAX_PATH];
		if(SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, dataPath))){
			_stprintf_s(path, max_path, _T("%s/%s"), dataPath, appName);
		} else {
			if(SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, dataPath))){
				_stprintf_s(path, max_path, _T("%s/%s"), dataPath, appName);
			} else {
				GetExePath(path);
			}
		}
	}
	// ファイルを関連づけ実行します。
	//   hwnd : HWND
	//   path : ファイル名
	void Execute(HWND hwnd, const TCHAR *path)
	{
		TCHAR curPath[MAX_PATH];

		GetExePath(curPath);

		ShellExecute(hwnd, NULL, path, NULL, curPath, SW_SHOWDEFAULT);
	}

#ifdef _USRDLL
	// 自分自身のフルパス名
	void GetOwnFileName(TCHAR *path, int len)
	{
		::GetModuleFileName(NULL, path, len);
	}
	// 実行ファイルのパスを取得します。
	//   path : パスを取得するアドレス
	void GetExePath(TCHAR *path)
	{
		::GetModuleFileName(NULL, path, MAX_PATH-1);
		RemoveFileName(path);
	}
#else
	// 自分自身のフルパス名
	void GetOwnFileName(TCHAR *path, int len)
	{
		::GetModuleFileName(CGrStdWin::GetInst(), path, len);
	}
	// 実行ファイルのパスを取得します。
	//   path : パスを取得するアドレス
	void GetExePath(TCHAR *path)
	{
		::GetModuleFileName(CGrStdWin::GetInst(), path, MAX_PATH-1);
		RemoveFileName(path);
	}
#endif

	// パスの最後に\を追加します。
	//   path : パス
	void AddBackSlash(TCHAR *path)
	{
		int last = _tcslen(path)-1;
		if(last < 0){
			return;
		}
		if(!IsBackSlash(path[last])){
			path[last+1] = _T('\\');
			path[last+2] = _T('\0');
		}
	}

	// 文字がファイル名の一部として相応しいかどうか
	//   ch     : 文字
	//   戻り値 : 相応しければ TRUE
	BOOL IsAppropriateFileName(TCHAR ch)
	{
		ch = toupper(ch);
		return (IS_ALPH(ch) || IS_NUM(ch) || ch == _T('@'));
	}

	// double null-terminatedで格納されている文字列の次の文字列を取得します。
	//   strings : 文字列
	//   戻り値  : 次の文字列の開始位置
	TCHAR *GetNextString(TCHAR *strings)
	{
		if(!strings || *strings == _T('\0')){
			return nullptr;
		}
		return strings + _tcslen(strings) + 1;
	}

	// double null-terminatedで格納されている文字列の文字列数を返す。
	//   strings : 文字列
	//   戻り値  : 文字列数
	int GetStringCount(TCHAR *strings)
	{
		int  i = 0;

		auto p = strings;
		while(*p != _T('\0')){
			i++;
			p += _tcslen(strings)+1;
		}
		return i;
	}

	// double null-terminatedで格納されている文字列に文字列を追加します。
	//   str    : 文字列
	//   addStr : 追加文字列
	void AddString(TCHAR *str, int maxlen, TCHAR *addStr)
	{
		auto *p = str;
		while(*p != _T('\0')){
			p += _tcslen(p)+1;
		}

		//lstrcat(p, addStr);
		_tcscat_s(p, maxlen, addStr);
		p += _tcslen(p)+1;
		*p = _T('\0');
	}

	// fileNameからpathを削除します。
	//   fileName : ファイル名のフルパス
	//   path     : 削除するパス
	//   戻り値   : fileNameにpathが含まれていない場合、NULL
	//              成功時は､fileNameからpathを削除した位置
	//   ex.
	//       path     = "path/path/path"
	//       fileName = "path/path/path/fileName"
	//     ->return   = "fileName"
	//       path     = "path2/path2/path2"
	//       fileName = "path2/path2/path1/fileName"
	//     ->return   = NULL
	LPCTSTR RemovePath(LPCTSTR fileName, LPCTSTR path)
	{
		if(!fileName || !path){
			return nullptr;
		}

		auto name = fileName;
		while(*path != _T('\0')){
			if(IsBackSlash(*path)){
				if(!IsBackSlash(*name)){
					return nullptr;
				}
			} else if(*path != *name){
				return nullptr;
			}
			path = CharNext(path);
			name = CharNext(name);
		}

		return name;
	}

	// ファイル名で最初に現れるパス区切り文字を取得します。
	//   fileName : ファイル名
	//   戻り値   : 成功時、区切り文字位置
	//              失敗時、NULL
	TCHAR *GetFirstBackSlash(TCHAR *fileName)
	{
		if(!fileName){
			return nullptr;
		}

		while(*fileName != _T('\0')){
			if(IsBackSlash(*fileName)){
				return fileName;
			}
			fileName = CharNext(fileName);
		}
		return nullptr;
	}

	// パス区切り文字で終わっているかどうか
	//   fileName : ファイル名
	BOOL EndBackSlash(TCHAR *fileName)
	{
		if(!fileName){
			return FALSE;
		}

		auto len = _tcslen(fileName);
		if(len <= 0){
			return FALSE;
		}
		return IsBackSlash(fileName[len-1]);
	}

	// パス区切り文字を "/" から "\\" に変更します。
	//   fileName : ファイル名
	void ModifyBackSlash(TCHAR *fileName)
	{
		TCHAR *p;
		TCHAR c = _T('/');

		while((p = const_cast<TCHAR*>(CGrText::strChar(fileName, &c))) != nullptr){
			*p = _T('\\');
		}
	}

	void GetFileList(std::vector<WIN32_FIND_DATA> &flist, LPCTSTR path)
	{
		WIN32_FIND_DATA wfd = {};
		std::tstring buf(path);
		if(buf.empty()){
			return;
		}
		if(IsBackSlash(buf[buf.size()-1])){
			buf.resize(buf.size()-1);
		}
		path = buf.c_str();
		std::tstring find_path;
		// ディレクトリ一覧取得
		CGrText::FormatMessage(find_path, _T("%1!s!\\*.*"), path);
		//
		auto hFile = ::FindFirstFile(find_path.c_str(), &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		do {
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(_tcscmp(wfd.cFileName, _T(".")) == 0 ||
				   _tcscmp(wfd.cFileName, _T("..")) == 0){
					continue;
				}
			}
			flist.push_back(wfd);
		} while(::FindNextFile(hFile, &wfd));
		::FindClose(hFile);
	}
	// フォルダ(サブフォルダを含む)以下のファイルを検索
	// flist      [out]: std::vector<tstring>
	// path        [in]: 検索フォルダ
	void GetFolderList(std::vector<std::tstring> &flist, LPCTSTR path)
	{
		WIN32_FIND_DATA wfd = {};
		std::tstring buf(path);
		if(buf.empty()){
			return;
		}
		if(IsBackSlash(buf[buf.size()-1])){
			buf.resize(buf.size()-1);
		}
		path = buf.c_str();
		std::tstring find_path;
		// ディレクトリ一覧取得
		CGrText::FormatMessage(find_path, _T("%1!s!\\*.*"), path);
		//
		auto hFile = ::FindFirstFile(find_path.c_str(), &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		do {
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(_tcscmp(wfd.cFileName, _T(".")) == 0 ||
				   _tcscmp(wfd.cFileName, _T("..")) == 0){
					continue;
				}
				std::tstring next_path;
				CGrText::FormatMessage(next_path, _T("%1!s!\\%2!s!"), path, wfd.cFileName);
				flist.emplace_back(next_path);
			}
		} while(::FindNextFile(hFile, &wfd));
		::FindClose(hFile);
	}
	// フォルダ(サブフォルダを含む)以下のファイルを検索
	// flist      [out]: std::vector<tstring>
	// path        [in]: 検索フォルダ
	// filter      [in]: 検索パターン
	// bNeedFolder [in]: フォルダをflistに加えるか?
	void GetFullPathList(std::vector<std::tstring> &flist, LPCTSTR path, LPCTSTR filter, bool bNeedFolder)
	{
		WIN32_FIND_DATA wfd = {};
		std::tstring buf(path);
		if(buf.empty()){
			return;
		}
		if(IsBackSlash(buf[buf.size()-1])){
			buf.resize(buf.size()-1);
		}
		path = buf.c_str();
		std::tstring find_path;
		// ディレクトリ一覧取得
		CGrText::FormatMessage(find_path, _T("%1!s!\\*.*"), path);
		//
		auto hFile = ::FindFirstFile(find_path.c_str(), &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		do {
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(_tcscmp(wfd.cFileName, _T(".")) == 0 ||
				   _tcscmp(wfd.cFileName, _T("..")) == 0){
					continue;
				}
				std::tstring next_path;
				CGrText::FormatMessage(next_path, _T("%1!s!\\%2!s!"), path, wfd.cFileName);
				GetFullPathList(flist, next_path.c_str(), filter, bNeedFolder);
				if(bNeedFolder){
					flist.push_back(next_path);
				}
			}
		} while(::FindNextFile(hFile, &wfd));
		::FindClose(hFile);
		// ファイル一覧取得
		CGrText::FormatMessage(find_path, _T("%1!s!\\%2!s!"), path, filter);
		hFile = ::FindFirstFile(find_path.c_str(), &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		do {
			if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				;
			} else {
				std::tstring next_path;
				CGrText::FormatMessage(next_path, _T("%1!s!\\%2!s!"), path, wfd.cFileName);
				flist.push_back(next_path);
			}
		} while(::FindNextFile(hFile, &wfd));
		::FindClose(hFile);
	}

	// 指定ファイルの有無を取得します。
	//   path   : ファイル名
	//   戻り値 : 存在した場合は、TRUE
	BOOL GetSearchDir(LPCTSTR path)
	{
		WIN32_FIND_DATA wfd;
		auto hFile = FindFirstFile(path, &wfd);
		if(hFile == INVALID_HANDLE_VALUE){
			// 見つかりませんでした。
			return FALSE;
		}
		FindClose(hFile);
		return TRUE;
	}

	// フォルダを作成します。
	//   path   : フォルダ名
	//   戻り値 : 作成した場合は､TRUE
	BOOL CreateFolder(LPCTSTR path)
	{
		if(!GetSearchDir(path)){
			if(_tcslen(path) <= 2 && path[1] == _T(':')){
				// ドライブ名
				return FALSE;
			}
			if(!CreateDirectory(path, NULL)){
				TCHAR folder[MAX_PATH];
				_tcscpy_s(folder, path);
				ModifyBackSlash(folder);
				if(RemoveFileName(folder)){
					CreateFolder(folder);
				}
				auto bOK = CreateDirectory(path, NULL);
				return bOK;
			} else {
				return TRUE;
			}
		}
		return FALSE;
	}

	// 親ディレクトリを取得します。
	//   fileName : フルパス
	//   parent   : 親ディレクトリを受け取るアドレス
	//   戻り値   : 親ディレクトリが取得できない場合は､NULL
	//              取得した場合は､子ディレクトリのアドレス
	//   ※parent == fileName 又は panret == NULL の場合は､fileNameに上書き
	TCHAR *GetParentDir(TCHAR *fileName, TCHAR *parent)
	{
		TCHAR *path, *prePath = nullptr;
		if(!parent){
			parent = fileName;
		} else if(parent != fileName){
#ifdef _DEBUG
			_tcscpy_s(parent, 128, fileName);
#else
			_tcscpy_s(parent, 512, fileName);
#endif // DEBUG
		}
		path = GetFirstBackSlash(parent);
		if(!path){
			return nullptr;
		}
		while(path){
			path = const_cast<TCHAR*>(CGrText::CharNext(path));
			if(*path == _T('\0')){
				break;
			}
			prePath = path;
			path = GetFirstBackSlash(path);
		}
		if(prePath){
			*prePath = _T('\0');
			return fileName + _tcslen(parent);
		}
		return nullptr;
	}
	bool IsURI(LPCTSTR lpFileName)
	{
		auto lpFind = CGrText::Find(lpFileName, _T(":"));
		return (lpFind && lpFind > (lpFileName+1));
	}
#define IS_DIRECTORY(_pwfd_) ((_pwfd_)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	struct SortWFDCompare {
		SortWFDCompare(){}
		bool operator()(const WIN32_FIND_DATA &wfd1, const WIN32_FIND_DATA &wfd2) const {
			int f1 = 0;
			int f2 = 0;
			std::tstring name1;
			std::tstring name2;
			std::tstring ext1;
			std::tstring ext2;
			if(IS_DIRECTORY(&wfd1)){
				f1 = 1;
				name1 = wfd1.cFileName;
			} else {
				auto lpExt = GetFileExtConst(wfd1.cFileName);
				if(lpExt){
					name1 = std::tstring(wfd1.cFileName, lpExt);
					ext1  = lpExt + 1;
				} else {
					name1 = wfd1.cFileName;
				}
			}
			if(IS_DIRECTORY(&wfd2)){
				f2 = 1;
				name2 = wfd2.cFileName;
			} else {
				auto lpExt = GetFileExtConst(wfd2.cFileName);
				if(lpExt){
					name2 = std::tstring(wfd2.cFileName, lpExt);
					ext2  = lpExt + 1;
				} else {
					name2 = wfd2.cFileName;
				}
			}
			int ret = f1 - f2;
			if(ret == 0){
				ret = _tcsicmp(name1.c_str(), name2.c_str());
				if(ret == 0){
					ret = _tcsicmp(ext1.c_str(), ext2.c_str());
				}
			}
			return ret < 0;
		}
	};
	struct FileData {
		LPCTSTR lpFileName;
	};
	static int FileExtCompare(const void *p1, const void *p2){
		auto *lpFile1 = static_cast<const FileData*>(p1);
		auto *lpFile2 = static_cast<const FileData*>(p2);
		return _tcsicmp(lpFile1->lpFileName, lpFile2->lpFileName);
	}
	static bool GetPrevFile(std::tstring &outFileName, LPCTSTR lpFileName, LPCTSTR lpFolderName, LPCTSTR lpFullPath, FileData lpExtList[], int extListNum, int type = 1)
	{
		bool bFile   = false;
		bool bFolder = false;
		bool bParent = false;
		std::vector<WIN32_FIND_DATA> flist;
		GetFileList(flist, lpFullPath);
		std::sort(flist.begin(), flist.end(), SortWFDCompare());
		int len = flist.size();
		if(lpFileName){
			// 一つ前のファイルがあるか確認
			bFile   = true;
			bParent = true;
		} else if(lpFolderName){
			// フォルダに対象ファイルがあるフォルダがあるか
			bFolder = true;
			bParent = true;
		} else {
			bFolder = true;
		}
		if(bFolder){
			len = flist.size();
			if(len > 0){
				auto *pwfd = &flist[len-1];
				for(; len>0; --len, --pwfd){
					if(IS_DIRECTORY(pwfd)){
						if(lpFolderName && _tcsicmp(pwfd->cFileName, lpFolderName) >= 0){
							continue;
						}
						std::tstring new_foldername;
						CGrText::FormatMessage(new_foldername, _T("%1!s!\\%2!s!"), lpFullPath, pwfd->cFileName);
						if(GetPrevFile(outFileName, nullptr, nullptr, new_foldername.c_str(), lpExtList, extListNum)){
							return true;
						}
					}
				}
				bFile = true; // 対象ファイルがあるか
			}
		}
		if(bFile){
			// 対象ファイルがあるか
			len = flist.size();
			if(len > 0){
				auto *pwfd = &flist[len-1];
				for(; len>0; --len, --pwfd){
					if(pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
						continue;
					}
					auto lpFileExt = GetFileExt(pwfd->cFileName);
					if(!lpFileExt){ // 2.0.11.0
						continue;
					}
					FileData fd = {lpFileExt};
					if(!bsearch(&fd, lpExtList, extListNum, sizeof(FileData), FileExtCompare)){
						continue;
					}
					if(lpFileName && _tcsicmp(pwfd->cFileName, lpFileName) >= 0){
						continue;
					}
					CGrText::FormatMessage(outFileName, _T("%1!s!\\%2!s!"), lpFullPath, pwfd->cFileName);
					return true;
				}
			}
		}
		if(bParent){
			// 親フォルダをチェック
			std::tstring parent_foldername(lpFullPath);
			int pos = parent_foldername.find_last_of(_T("\\/"));
			if(pos != std::string::npos){
				auto lpTartgetFolder = lpFullPath + (pos+1);
				parent_foldername[pos] = '\0';
				return GetPrevFile(outFileName, nullptr, lpTartgetFolder, parent_foldername.c_str(), lpExtList, extListNum);
			}
		}
		return false;
	}
	bool GetPrevFile(std::tstring &outFileName, LPCTSTR lpFileName, LPCTSTR lpExtList[], int extListNum)
	{
		if(extListNum <= 0){
			return false;
		}
		LPCTSTR lpTartget = nullptr;
		std::tstring find_path(lpFileName);
		int pos = find_path.find_last_of(_T("\\/"));
		if(pos != std::string::npos){
			find_path[pos] = '\0';
			lpTartget = lpFileName + (pos+1);
		}
		auto *fdlist = new FileData[extListNum];
		for(int i=0; i<extListNum; ++i){
			fdlist[i].lpFileName = lpExtList[i];
		}
		qsort(fdlist, extListNum, sizeof(FileData), FileExtCompare);
		auto ret = GetPrevFile(outFileName, lpTartget, nullptr, find_path.c_str(), fdlist, extListNum);
		delete [] fdlist;
		return ret;
		return false;
	}
	static bool GetNextFile(std::tstring &outFileName, LPCTSTR lpFileName, LPCTSTR lpFolderName, LPCTSTR lpFullPath, FileData lpExtList[], int extListNum)
	{
		bool bFile   = false;
		bool bFolder = false;
		bool bParent = false;
		std::vector<WIN32_FIND_DATA> flist;
		GetFileList(flist, lpFullPath);
		std::sort(flist.begin(), flist.end(), SortWFDCompare());
		int len = 0;
		if(lpFileName){
			// 一つ先のファイルがあるか確認
			bFile   = true;
			bParent = true;
		} else if(lpFolderName){
			// フォルダに対象ファイルがあるフォルダがあるか
			bFolder = true;
			bParent = true;
		} else {
			// 対象ファイルがあるか
			bFile   = true;
		}
		if(bFile){
			// 対象ファイルがあるか
			len = flist.size();
			if(len > 0){
				auto *pwfd = &flist[0];
				for(; len>0; --len, ++pwfd){
					if(IS_DIRECTORY(pwfd)){
						continue;
					}
					auto lpFileExt = GetFileExt(pwfd->cFileName);
					if(!lpFileExt){
						continue;
					}
					FileData fd = {lpFileExt};
					if(!bsearch(&fd, lpExtList, extListNum, sizeof(FileData), FileExtCompare)){
						continue;
					}
					if(lpFileName && _tcsicmp(pwfd->cFileName, lpFileName) <= 0){
						continue;
					}
					CGrText::FormatMessage(outFileName, _T("%1!s!\\%2!s!"), lpFullPath, pwfd->cFileName);
					return true;
				}
				bFolder = true; // 対象ファイルがあるフォルダがあるか
			}
		}
		if(bFolder){
			// 対象ファイルがあるフォルダがあるか
			len = flist.size();
			if(len > 0){
				auto *pwfd = &flist[0];
				for(; len>0; --len, ++pwfd){
					if(IS_DIRECTORY(pwfd)){
						if(lpFolderName && _tcsicmp(pwfd->cFileName, lpFolderName) <= 0){
							continue;
						}
						std::tstring new_foldername;
						CGrText::FormatMessage(new_foldername, _T("%1!s!\\%2!s!"), lpFullPath, pwfd->cFileName);
						if(GetNextFile(outFileName, nullptr, nullptr, new_foldername.c_str(), lpExtList, extListNum)){
							return true;
						}
					}
				}
			}
		}
		if(bParent){
			// 親フォルダをチェック
			std::tstring parent_foldername(lpFullPath);
			auto pos = parent_foldername.find_last_of(_T("\\/"));
			if(pos != std::string::npos){
				auto lpTartgetFolder = lpFullPath + (pos+1);
				parent_foldername[pos] = '\0';
				return GetNextFile(outFileName, nullptr, lpTartgetFolder, parent_foldername.c_str(), lpExtList, extListNum);
			}
		}
		return false;
	}
	bool GetNextFile(std::tstring &outFileName, LPCTSTR lpFileName, LPCTSTR lpExtList[], int extListNum)
	{
		if(extListNum <= 0){
			return false;
		}
		LPCTSTR lpTartget = nullptr;
		std::tstring find_path(lpFileName);
		auto pos = find_path.find_last_of(_T("\\/"));
		if(pos != std::string::npos){
			find_path[pos] = '\0';
			lpTartget = lpFileName + (pos+1);
		}
		auto *fdlist = new FileData[extListNum];
		for(int i=0; i<extListNum; ++i){
			fdlist[i].lpFileName = lpExtList[i];
		}
		qsort(fdlist, extListNum, sizeof(FileData), FileExtCompare);
		auto ret = GetNextFile(outFileName, lpTartget, nullptr, find_path.c_str(), fdlist, extListNum);
		delete [] fdlist;
		return ret;
	}
	bool MakeFullPath(std::tstring &outpath, LPCTSTR lpParent, LPCTSTR lpFolder)
	{
		TCHAR *lpfilepart;
		TCHAR fullpath[MAX_PATH+1];
		TCHAR exepath[MAX_PATH] = {};
		if(!lpParent){
			GetExePath(exepath);
			if(exepath[0] == _T('\0')){
				return false;
			}
			lpParent = exepath;
		}
		::GetFullPathName(lpParent, sizeof(fullpath)/sizeof(TCHAR), fullpath, &lpfilepart);
		CGrShell::AddBackSlash(fullpath);
		CGrText::FormatMessage(outpath, _T("%1!s!%2!s!\\"), fullpath, lpFolder);
		return true;
	};
};