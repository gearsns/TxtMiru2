// Shell.h
//
// ファイル操作
//
#ifndef __SHELL_H__
#define __SHELL_H__

#include "Text.h"
#include <vector>

namespace CGrShell
{
	// リンク先ファイル名の取得
	bool GetPathFromLink(LPCTSTR linkFilename, std::tstring &filename);
	// ファイル名の表記を整えます。 /./,\ -> /
	void ToPrettyFileName(LPCTSTR fromFileName, std::tstring &toFileName);
	void ToPrettyFileName(std::tstring &fileName);
	// パス区切り記号かどうか
	//   c      : 文字
	//   戻り値 : パス区切り文字の場合、TRUE
	bool IsBackSlash(TCHAR c);
	// 指定ファイルが実行ファイルかどうかを取得します。
	//   fileName : ファイル名
	//   戻り値   : 実行ファイルならTRUE
	BOOL IsExecuteFile(TCHAR *fileName);
	// ファイル名開始位置を取得します。
	//   fileName  [in]: ファイル名
	//   戻り値   [out]: ファイル名開始位置
	const TCHAR *GetFileName(const TCHAR *fileName);
	// 拡張子開始位置を取得します。
	//   fileName : ファイル名
	//   戻り値   : 拡張子開始位置
	TCHAR *GetFileExt(TCHAR *fileName);
	// 拡張子開始位置を取得します。
	//   fileName : ファイル名
	//   戻り値   : 拡張子開始位置
	LPCTSTR GetFileExtConst(LPCTSTR fileName);
	// パスからファイル名を削除
	//   path : パス
	BOOL RemoveFileName(TCHAR *path);
	// ファイルを関連づけ実行します。
	//   hwnd : HWND
	//   path : ファイル名
	void Execute(HWND hwnd, const TCHAR *path);
	// 自分自身のフルパス名
	void GetOwnFileName(TCHAR *path, int len);
	// ファイルの情報取得
	bool getFileInfo(LPCTSTR lpFileName, WIN32_FIND_DATA &wfd);
	// データ保存パスの取得
	void GetDataPath(TCHAR *path, int max_path, LPCTSTR appName);
	// 実行ファイルのパスを取得します。
	//   path : パスを取得するアドレス
	void GetExePath(TCHAR *path);
	// パスの最後に\を追加します。
	//   path : パス
	void AddBackSlash(TCHAR *path);
	// 文字がファイル名の一部として相応しいかどうか
	//   ch     : 文字
	//   戻り値 : 相応しければ TRUE
	BOOL IsAppropriateFileName(TCHAR ch);
	// double null-terminatedで格納されている文字列の次の文字列を取得します。
	//   strings : 文字列
	//   戻り値  : 次の文字列の開始位置
	TCHAR *GetNextString(TCHAR *strings);
	// double null-terminatedで格納されている文字列の文字列数を返す。
	//   strings : 文字列
	//   戻り値  : 文字列数
	int GetStringCount(TCHAR *strings);
	// double null-terminatedで格納されている文字列に文字列を追加します。
	//   str    : 文字列
	//   addStr : 追加文字列
	void AddString(TCHAR *str, int maxlen, TCHAR *addStr);
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
	LPCTSTR RemovePath(LPCTSTR fileName, LPCTSTR path);
	// ファイル名で最初に現れるパス区切り文字を取得します。
	//   fileName : ファイル名
	//   戻り値   : 成功時、区切り文字位置
	//              失敗時、NULL
	TCHAR *GetFirstBackSlash(TCHAR *fileName);
	// パス区切り文字で終わっているかどうか
	//   fileName : ファイル名
	BOOL EndBackSlash(TCHAR *fileName);
	// パス区切り文字を "/" から "\\" に変更します。
	//   fileName : ファイル名
	void ModifyBackSlash(TCHAR *fileName);
	//
	void GetFileList(std::vector<WIN32_FIND_DATA> &flist, LPCTSTR path);
	// フォルダ(サブフォルダを含む)以下のファイルを検索
	// flist      [out]: std::vector<tstring>
	// path        [in]: 検索フォルダ
	void GetFolderList(std::vector<std::tstring> &flist, LPCTSTR path);
	// フォルダ(サブフォルダを含む)以下のファイルを検索
	// flist      [out]: std::vector<tstring>
	// path        [in]: 検索フォルダ
	// filter      [in]: 検索パターン
	// bNeedFolder [in]: フォルダをflistに加えるか?
	void GetFullPathList(std::vector<std::tstring> &flist, LPCTSTR path, LPCTSTR filter, bool bNeedFolder=false);
	// 指定ファイルの有無を取得します。
	//   path   : ファイル名
	//   戻り値 : 存在した場合は、TRUE
	BOOL GetSearchDir(LPCTSTR path);
	// フォルダを作成します。
	//   path   : フォルダ名
	//   戻り値 : 作成した場合は､TRUE
	BOOL CreateFolder(LPCTSTR path);
	// 親ディレクトリを取得します。
	//   fileName : フルパス
	//   parent   : 親ディレクトリを受け取るアドレス
	//   戻り値   : 親ディレクトリが取得できない場合は､NULL
	//              取得した場合は､子ディレクトリのアドレス
	//   ※parent == fileName 又は panret == NULL の場合は､fileNameに上書き
	TCHAR *GetParentDir(TCHAR *fileName, TCHAR *parent);
	// URIかどうかを判断：http://, https://, file:///, ftp://
	bool IsURI(LPCTSTR lpFileName);
	bool GetPrevFile(std::tstring &outFileNae, LPCTSTR lpFileName, LPCTSTR lpExtList[], int extListNum);
	bool GetNextFile(std::tstring &outFileNae, LPCTSTR lpFileName, LPCTSTR lpExtList[], int extListNum);
	//
	bool MakeFullPath(std::tstring &outpath, LPCTSTR lpParent, LPCTSTR lpFolder);
};
#endif
