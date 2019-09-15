#ifndef __TXTBOOKMARK_H__
#define __TXTBOOKMARK_H__
// 以下の ifdef ブロックは DLL からのエクスポートを容易にするマクロを作成するための
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された TXTFUNCBOOKMARK_EXPORTS
// シンボルでコンパイルされます。このシンボルは、この DLL を使うプロジェクトで定義することはできません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、
// TXTFUNCBOOKMARK_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
#ifdef TXTFUNCBOOKMARK_EXPORTS
#define TXTFUNCBOOKMARK_API __declspec(dllexport)
#else
#define TXTFUNCBOOKMARK_API __declspec(dllimport)
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "stltchar.h"
#include "TxtFuncIParam.h"
#include "TxtFuncIBookmark.h"

extern "C" {
TXTFUNCBOOKMARK_API bool cdecl TxtFuncBookmarkSaveAs(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncBookmarkOpen(LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncShowBookList(HWND hWnd, LPCTSTR lpFileName, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncShowLinkList(HWND hWnd, int mode, CGrTxtFuncIBookmark *pBookmark, CGrTxtFuncISubTitle *pSubTitle, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncRefreshBookList();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsShowBookList();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsBookListMember(HWND hWnd);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncRefreshLinkList();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsShowLinkList();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsLinkListMember(HWND hWnd);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncUpdatePage();
TXTFUNCBOOKMARK_API HWND cdecl TxtFuncGetWnd(int id);
TXTFUNCBOOKMARK_API void cdecl TxtFuncUnInstall();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncSearchFiles(HWND hWnd, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncShowRubyList(HWND hWnd, void *pDoc, CGrTxtFuncIBookmark *pBookmark, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCBOOKMARK_API bool cdecl TxtFuncIsShowRubyList();
TXTFUNCBOOKMARK_API bool cdecl TxtFuncRefreshRubyList();
TXTFUNCBOOKMARK_API int cdecl TxtFuncAddFavorite(HWND hWnd, LPCTSTR lpURL);
};
#endif // __TXTBOOKMARK_H__
