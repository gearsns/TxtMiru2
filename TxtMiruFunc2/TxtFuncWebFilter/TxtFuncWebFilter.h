// 以下の ifdef ブロックは DLL からのエクスポートを容易にするマクロを作成するための 
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された TXTFUNCWEBFILTER_EXPORTS
// シンボルでコンパイルされます。このシンボルは、この DLL を使うプロジェクトで定義することはできません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、 
// TXTFUNCWEBFILTER_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
#ifdef TXTFUNCWEBFILTER_EXPORTS
#define TXTFUNCWEBFILTER_API __declspec(dllexport)
#else
#define TXTFUNCWEBFILTER_API __declspec(dllimport)
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "stltchar.h"
#include "TxtFuncIParam.h"
extern "C" {
TXTFUNCWEBFILTER_API void cdecl TxtFuncWebFilterRegister(LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam);
TXTFUNCWEBFILTER_API void cdecl TxtFuncWebFilterUnregister(HWND hWnd, TIMERPROC lpTimerFunc);
}
