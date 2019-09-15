#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "AozoraList.h"

#define TXTMIRU_AOZORA_EXE       _T("TxtMiruDBAozora")
#define TXTMIRU_AOZORA_PROP_NAME _T("TxtMiru2.0-Aozora")

struct TxtMiruData
{
	HWND hWnd;
};
static BOOL CALLBACK EnumWindowsProc(
	HWND hwnd,      // 親ウィンドウのハンドル
	LPARAM lParam   // アプリケーション定義の値
	)
{
	auto handle = GetProp(hwnd, TXTMIRU_AOZORA_PROP_NAME);
	if(!handle){
		return TRUE; // 継続
	}
	auto *lpData = reinterpret_cast<TxtMiruData*>(lParam);
	lpData->hWnd = hwnd;
	return FALSE;
}

void CloseAozoraList(HWND hWnd)
{
	TxtMiruData data = {};
	EnumWindows(EnumWindowsProc, LPARAM(&data));
	if(data.hWnd){
		PostMessage(data.hWnd, WM_CLOSE, 0, 0);
	}
}
void showAozoraList(HWND hWnd, LPCTSTR lpsCmdLine)
{
	TxtMiruData data = {};
	EnumWindows(EnumWindowsProc, LPARAM(&data));
	if(data.hWnd){
		// ForceForegroundWindow
		auto dwForceThread   = GetWindowThreadProcessId(data.hWnd, nullptr);
		auto dwCurrentThread = GetCurrentThreadId();
		if(dwForceThread == dwCurrentThread){
			BringWindowToTop(data.hWnd);
		} else {
			AttachThreadInput(dwForceThread, dwCurrentThread, true);
			BringWindowToTop(data.hWnd);
			AttachThreadInput(dwForceThread, dwCurrentThread, false);
		}
	} else {
		PROCESS_INFORMATION pi = {};
		STARTUPINFO si = {sizeof(STARTUPINFO)};

		auto len = _tcslen(TXTMIRU_AOZORA_EXE) + 1 + _tcslen(lpsCmdLine) + 1;
		auto lpTxtMiruCmdLine = static_cast<LPTSTR>(_malloca((len+1) * sizeof(TCHAR)));
		if (lpTxtMiruCmdLine) {
			_stprintf_s(lpTxtMiruCmdLine, len/* * sizeof(TCHAR)*/, _T("%s %s"), TXTMIRU_AOZORA_EXE, lpsCmdLine);
			CreateProcess(
				nullptr, // LPCTSTR lpApplicationName                , // 実行可能モジュールの名前
				lpTxtMiruCmdLine, // LPTSTR lpCommandLine                     , // コマンドラインの文字列
				nullptr, // LPSECURITY_ATTRIBUTES lpProcessAttributes, // セキュリティ記述子
				nullptr, // LPSECURITY_ATTRIBUTES lpThreadAttributes , // セキュリティ記述子
				FALSE, // BOOL bInheritHandles                     , // ハンドルの継承オプション
				0, // DWORD dwCreationFlags                    , // 作成のフラグ
				nullptr, // LPVOID lpEnvironment                     , // 新しい環境ブロック
				nullptr, // LPCTSTR lpCurrentDirectory               , // カレントディレクトリの名前
				&si, // LPSTARTUPINFO lpStartupInfo              , // スタートアップ情報
				&pi               // LPPROCESS_INFORMATION lpProcessInformation // プロセス情報
			);
			if (pi.hThread) {
				CloseHandle(pi.hThread);
			}
			if (pi.hProcess) {
				CloseHandle(pi.hProcess);
			}
		}
	}
}

