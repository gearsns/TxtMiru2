#pragma once

#ifdef TXTMIRUFUNCLIB_EXPORTS
#define TXTMIRUFUNCLIB_API __declspec(dllexport)
#else
#define TXTMIRUFUNCLIB_API __declspec(dllimport)
#endif

#ifdef  TXTMIRUFUNCLIB_DLL
#include <Windows.h>
TXTMIRUFUNCLIB_API HINSTANCE GetDllModuleHandle();
TXTMIRUFUNCLIB_API void SetDllModuleHandle(HINSTANCE hInst);
#endif
