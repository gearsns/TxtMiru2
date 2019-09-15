// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

#include "stltchar.h"
#include "stlutil.h"
// 2.0.23.0
#include "TxtMiruFunc.h"
#include "FunctionKeyMap.h"
#include "KeyState.h"
#include "TxtFunc.h"
#include "Text.h"
#include "TxtMiruTheme.h"

static HINSTANCE g_hInst = NULL;
namespace CGrTxtFunc
{
	HINSTANCE GetDllModuleHandle() { return g_hInst; }
};

#include "TxtFuncBookmarkDef.h"
#include <map>
struct ModelessInfo {
	CGrModelessWnd *pWnd;
	HANDLE hThread;
	unsigned int threadid;
};
using WindCtrlMap = std::map<TxtFuncBookmark::ModelWindowID,ModelessInfo>;
WindCtrlMap g_ModelessDlgMap;

#include <process.h>
#pragma warning (push)
#pragma warning(disable:4768)
#pragma warning(disable:4091)
#include <shlobj.h>
#pragma warning (pop)

std::map<CGrKeyboardState,UINT> g_key_id_map;
#define KEYBIND_FILE  _T("keybind.ini")
static void LoadKeyMap(bool bForce = false)
{
	static bool bFirst = true;
	if(!bForce && !bFirst){
		return;
	}
	bFirst = false;
	TCHAR keyBindFileName[200];
	_stprintf_s(keyBindFileName, _T("%s/%s"), CGrTxtFunc::GetDataPath(), KEYBIND_FILE);
	CGrFunctionKeyMap::FunctionMap fmap;
	if(!CGrFunctionKeyMap::Load(keyBindFileName, fmap)){
	}
	g_key_id_map.clear();
	for(const auto &item : fmap){
		const auto &ks = item.first;
		const auto &fn = item.second;
		UINT id = 0;
		for(const auto name : TxtMiru::l_TxtMiruFuncNameList){
			if(fn == name){
				g_key_id_map[ks] = id;
			}
			++id;
		}
	}
}
//
static unsigned int __stdcall ModelessProc(void *lpParam)
{
	auto id = static_cast<TxtFuncBookmark::ModelWindowID>(reinterpret_cast<UINT>(lpParam));
	auto* pWnd = static_cast<CGrModelessWnd*>(GetInstallWnd(id));
	if(!pWnd){
		return 0;
	}
	LoadKeyMap();
	if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) == S_OK) {
		;
	}
	if (OleInitialize(NULL) == S_OK) {
		;
	}
	pWnd->Show(NULL);
	auto hwnd = pWnd->GetWnd();
	MSG msg = {};
	while(::GetMessage(&msg, NULL, 0, 0)){
		if(msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN){
			auto vk = static_cast<UINT>(msg.wParam);
			auto it = g_key_id_map.find(CGrKeyboardState(vk));
			if(it != g_key_id_map.end()){
				if(/**/it->second == TxtMiru::FnN_ShowBookList
				   ||  it->second == TxtMiru::FnN_ShowRubyList
				   ||  it->second == TxtMiru::FnN_ShowSubtitleBookmark
				   ){
					::PostMessage(pWnd->GetParentWnd(), msg.message, msg.wParam, msg.lParam);
					continue;
				}
			}
		}
		if(msg.message == WM_MOUSEWHEEL){
			POINT pos = {};
			GetCursorPos(&pos);
			auto hChild = WindowFromPoint(pos);
			msg.hwnd = hChild;
		}
		if(pWnd->IsDialogMessage(hwnd, msg)){
			continue;
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	CoUninitialize();
	OleUninitialize();
	auto it = g_ModelessDlgMap.find(id);
	if(it != g_ModelessDlgMap.end()){
		auto &&info = it->second;
		delete info.pWnd;
		info.pWnd = nullptr;
	}
	return 0;
}

CGrModelessWnd* GetInstallWnd(TxtFuncBookmark::ModelWindowID id)
{
	auto it = g_ModelessDlgMap.find(id);
	if(it != g_ModelessDlgMap.end()){
		return (it->second).pWnd;
	}
	return nullptr;
}

bool InstallWnd(TxtFuncBookmark::ModelWindowID id, CGrModelessWnd *pWnd)
{
	ModelessInfo info = {pWnd, 0};
	g_ModelessDlgMap[id] = info;
	g_ModelessDlgMap[id].hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0 , ModelessProc, reinterpret_cast<void*>(id)/*param*/, 0, &(g_ModelessDlgMap[id].threadid)));
	return true;
}

bool UnInstallWnd(TxtFuncBookmark::ModelWindowID id)
{
	auto it = g_ModelessDlgMap.find(id);
	if(it != g_ModelessDlgMap.end()){
		auto &&info = it->second;
		if(info.hThread){
			PostThreadMessage(info.threadid, WM_QUIT, 0, 0);
			::WaitForSingleObject(info.hThread, INFINITE);
			CloseHandle(info.hThread);
			info.hThread = 0;
			info.threadid = 0;
		}
	}
	return true;
}
#include "TxtFuncBookmark.h"
extern "C" TXTFUNCBOOKMARK_API void cdecl TxtFuncUnInstall()
{
	for(auto &&item : g_ModelessDlgMap){
		auto &&info = item.second;
		if(info.hThread){
			if(info.pWnd){
				auto hWnd = info.pWnd->GetWnd();
				if(!IsWindowEnabled(hWnd)){
					// Modal画面起動中だと終了しないのでここで強制的にIDCANCELを送る
					auto hPopup = GetWindow(hWnd, GW_ENABLEDPOPUP);
					if(hPopup != hWnd){
						FORWARD_WM_COMMAND(hPopup, IDCANCEL, 0, 0, SendMessage);
					}
				}
				PostMessage(hWnd, WM_DESTROY, 0, 0);
			} else {
				PostThreadMessage(info.threadid, WM_QUIT, 0, 0);
			}
		}
	}
	for(auto &&item : g_ModelessDlgMap){
		auto &&info = item.second;
		if(info.hThread){
			::WaitForSingleObject(info.hThread, INFINITE);
			CloseHandle(info.hThread);
			info.hThread = 0;
			info.threadid = 0;
		}
	}
	g_ModelessDlgMap.clear();
}

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = static_cast<HINSTANCE>(hModule);
		CGrText::SetDllModuleHandle(g_hInst);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
