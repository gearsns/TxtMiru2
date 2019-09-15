#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include "tchar.h"
#include "Gesture.h"
#include "Win32Wrap.h"

CGrGesture::CGrGesture()
{
}

CGrGesture::~CGrGesture()
{
	if(m_hDll){
		::FreeLibrary(m_hDll);
	}
}

CGrGesture &CGrGesture::Gesture()
{
	static CGrGesture g_gesture;
	return g_gesture;
}

bool CGrGesture::Initialize()
{
	if(m_hDll){
		return (m_pGetGestureInfo && m_pCloseGestureInfoHandle && m_pSetGestureConfig);
	}
	m_hDll = ::LoadLibrary(_T("user32.dll"));
	if(!m_hDll){
		return false;
	}
	SetProcAddressPtr(m_pGetGestureInfo, m_hDll, "GetGestureInfo");
	SetProcAddressPtr(m_pCloseGestureInfoHandle, m_hDll, "CloseGestureInfoHandle");
	SetProcAddressPtr(m_pSetGestureConfig, m_hDll, "SetGestureConfig");
	if(m_pGetGestureInfo && m_pCloseGestureInfoHandle && m_pSetGestureConfig){
		return true;
	}
	return false;
}

bool CGrGesture::IsSupported()
{
	return Initialize();
}

BOOL WINAPI CGrGesture::CloseGestureInfoHandle(__in HGESTUREINFO hGestureInfo)
{
	if(m_hDll && m_pCloseGestureInfoHandle){
		return m_pCloseGestureInfoHandle(hGestureInfo);
	}
	return FALSE;
}

_Success_(return == TRUE)
BOOL WINAPI CGrGesture::GetGestureInfo(__in HGESTUREINFO hGestureInfo, __out PGESTUREINFO pGestureInfo)
{
	if(m_hDll && m_pGetGestureInfo){
		return m_pGetGestureInfo(hGestureInfo, pGestureInfo);
	}
	return FALSE;
}

BOOL WINAPI CGrGesture::SetGestureConfig(__in HWND hwnd, __in DWORD dwReserved, __in UINT cIDs, __in_ecount(cIDs) PGESTURECONFIG pGestureConfig, __in UINT cbSize)
{
	if(m_hDll && m_pSetGestureConfig){
		return m_pSetGestureConfig(hwnd, dwReserved, cIDs, pGestureConfig, cbSize);
	}
	return FALSE;
}
