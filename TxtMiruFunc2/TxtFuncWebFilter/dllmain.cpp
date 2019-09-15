// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include "Text.h"

static HINSTANCE g_hInst = NULL;
namespace CGrTxtFunc
{
	HINSTANCE GetDllModuleHandle() { return g_hInst; }
}


void UnInstallAllWebFilter();

BOOL APIENTRY DllMain( HMODULE hModule,
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
