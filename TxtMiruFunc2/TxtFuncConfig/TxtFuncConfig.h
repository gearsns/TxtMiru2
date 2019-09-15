// 以下の ifdef ブロックは DLL からのエクスポートを容易にするマクロを作成するための
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された TXTFUNCCONFIG_EXPORTS
// シンボルでコンパイルされます。このシンボルは、この DLL を使うプロジェクトで定義することはできません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、
// TXTFUNCCONFIG_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
#ifdef TXTFUNCCONFIG_EXPORTS
#define TXTFUNCCONFIG_API __declspec(dllexport)
#else
#define TXTFUNCCONFIG_API __declspec(dllimport)
#endif
#include "TxtFuncIParam.h"
extern "C" {
TXTFUNCCONFIG_API bool cdecl TxtFuncConfig(HWND hWnd, LPCTSTR lpkeyBindFileName, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCCONFIG_API bool cdecl TxtFuncLayoutConfig(HWND hWnd, LPCTSTR lpDataDir, LPCTSTR type, LPCTSTR name, LPCTSTR lpFileName, CGrTxtFuncIParam *pParam);
TXTFUNCCONFIG_API bool cdecl TxtFuncGetBrowserURL(LPTSTR *ppURL, CGrTxtFuncIParam *pParam);
TXTFUNCCONFIG_API bool cdecl TxtFuncGotoPageDlg(HWND hWnd, int maxnum, int cur_page, int *out_page);
TXTFUNCCONFIG_API bool cdecl TxtFuncDocInfoDlg(HWND hWnd, LPCTSTR title, LPCTSTR author, LPCTSTR filename, LPCTSTR text, LPCTSTR writetime, int page, int maxpage);
TXTFUNCCONFIG_API bool cdecl TxtFuncVersionInfoDlg(HWND hWnd, HINSTANCE hInst, CGrTxtFuncIParam *pParam);
};
