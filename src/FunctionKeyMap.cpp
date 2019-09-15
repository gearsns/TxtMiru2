#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "Text.h"
#include "Shell.h"
#include "FunctionKeyMap.h"

namespace CGrFunctionKeyMap
{
	void add(LPCTSTR line, int len, FunctionMap &fmap)
	{
		UINT vk    = 0;
		bool shift = false;
		bool ctrl  = false;
		bool alt   = false;
		bool delay = false;
		auto lpNextSrc = line;
		while(*lpNextSrc){
			auto lpSrc = lpNextSrc;
			lpNextSrc = CGrText::CharNext(lpSrc);
			if(CGrText::isMatchChar(lpSrc, _T("C-"))){
				ctrl = true;
				lpNextSrc = CGrText::CharNext(lpNextSrc);
			} else if(CGrText::isMatchChar(lpSrc, _T("S-"))){
				shift = true;
				lpNextSrc = CGrText::CharNext(lpNextSrc);
			} else if(CGrText::isMatchChar(lpSrc, _T("M-"))){
				alt = true;
				lpNextSrc = CGrText::CharNext(lpNextSrc);
			} else if(CGrText::isMatchChar(lpSrc, _T("D-"))){
				delay = true;
				lpNextSrc = CGrText::CharNext(lpNextSrc);
			} else if(CGrText::isMatchChar(lpSrc, _T("="))){
				fmap[CGrKeyboardState(vk, shift, ctrl, alt, delay)] = std::tstring(lpNextSrc, line+len-1);
				std::tstring str;
				CGrKeyboardState(vk, shift, ctrl, alt, delay).to_s(str);
				break;
			} else {
				TCHAR *lpStop;
				vk = _tcstol(lpSrc, &lpStop, 16);
				if(lpStop > lpNextSrc){
					lpNextSrc = lpStop;
				}
			}
		};
	}
	bool Load(LPCTSTR lpFileName, FunctionMap &fmap)
	{
		auto hFile = ::CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
		if(hFile == INVALID_HANDLE_VALUE){
			return false;
		}
		bool ret = true;
		DWORD sizeHigh;
		// サイズを取得して領域を確保します。
		auto fsize = ::GetFileSize(hFile, &sizeHigh);
		if(fsize > 0){
			auto hHeap = ::HeapCreate(0, 0, 0);
			if (!hHeap) {
				return false;
			}
			auto *data = static_cast<BYTE*>(::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(BYTE)*(fsize+1)));
			DWORD dw;
			do {
				if(!data){
					ret = false;
					break;
				}
				if(0 == ::ReadFile(hFile, data, fsize, &dw, nullptr)){
					ret = false;
					break;
				}
				auto *beginData = data;
#ifdef UNICODE
				if(fsize >= 2 &&
				   data[0] == 0xff && data[1] == 0xfe){
					beginData = &data[2];
				}
#endif
				data[dw] = _T('\0');
				auto lpStr = reinterpret_cast<LPTSTR>(beginData), lpLine = reinterpret_cast<LPTSTR>(beginData);
				for(;*lpStr; ++lpStr){
					if(*lpStr == _T('\n')){
						*lpStr = _T('\0');
						add(lpLine, lpStr-lpLine, fmap);
						lpLine = lpStr + 1;
					}
				}
				if(lpLine){
					add(lpLine, lpStr-lpLine, fmap);
				}
			} while(0);
			::HeapFree(hHeap, 0, data);
			::HeapDestroy(hHeap);
		}
		::CloseHandle(hFile);

		return ret;
	}
	bool Save(LPCTSTR lpFileName, const FunctionMap &fmap)
	{
		auto hFile = ::CreateFile(lpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
		if(hFile == INVALID_HANDLE_VALUE){
			TCHAR folder[MAX_PATH];
			_tcscpy_s(folder, lpFileName);
			CGrShell::ModifyBackSlash(folder);
			if(CGrShell::RemoveFileName(folder)){
				CGrShell::CreateFolder(folder);
				hFile = ::CreateFile(lpFileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
			}
			if(hFile == INVALID_HANDLE_VALUE){
				return FALSE;
			}
		}
		DWORD dw;
#ifdef UNICODE
		BYTE header[] = {0xff, 0xfe};
		::WriteFile(hFile, header, sizeof(header), &dw, nullptr);
#endif
		auto it=fmap.begin(), e=fmap.end();
		for(;it != e; ++it){
			const auto &ks = it->first;
			const auto &fn = it->second;
			std::tstring key_name;
			std::tstring line;
			ks.to_s(key_name);
			CGrText::FormatMessage(line, _T("%1!s!=%2!s!\n"), key_name.c_str(), fn.c_str());
			::WriteFile(hFile, line.c_str(), _tcslen(line.c_str())*sizeof(TCHAR), &dw, nullptr);
		}
		::CloseHandle(hFile);
		return true;
	}
};
