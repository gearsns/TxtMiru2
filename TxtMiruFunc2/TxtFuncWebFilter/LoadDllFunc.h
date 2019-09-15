#ifndef __LOADDLLFUNC_H__
#define __LOADDLLFUNC_H__

#include <windows.h>
#include <windowsx.h>
#include "Text.h"

class CGrLoadDllFunc
{
protected:
	HMODULE m_hDLL;
	std::tstring m_fileName;
public:
	CGrLoadDllFunc(LPCTSTR lpfilename) : m_hDLL(NULL), m_fileName(lpfilename)
	{
		
	}
	virtual ~CGrLoadDllFunc()
	{
		Unload();
	}
	virtual void Unload()
	{
		if(m_hDLL){
			FreeLibrary(m_hDLL);
		}
		m_hDLL = NULL;
	}
	bool isLoaded(){ return m_hDLL != NULL; }
	FARPROC GetProcAddress(LPCSTR funcname, HWND hWnd)
	{
		if(!m_hDLL){
			m_hDLL = ::LoadLibrary(m_fileName.c_str());
		}
		if(m_hDLL){
			return ::GetProcAddress(m_hDLL, funcname);
		} else if(hWnd){
		}
		return nullptr;
	}
};

#endif // __LOADDLLFUNC_H__
