#ifndef __MESSAGEBOX_H__
#define __MESSAGEBOX_H__

#include <windows.h>

namespace CGrMessageBox
{
#ifdef _USRDLL
	int Show(HINSTANCE hInst, HWND hwnd, UINT id, UINT title_id=0, UINT nType=MB_OK);
	int Show(HINSTANCE hInst, HWND hwnd, UINT id, LPCTSTR lpszCaption, UINT nType=MB_OK);
	int Show(HINSTANCE hInst, HWND hwnd, LPCTSTR lpszText, LPCTSTR lpszCaption=NULL, UINT nType=MB_OK);
#else
	int Show(HWND hwnd, UINT id, UINT title_id=0, UINT nType=MB_OK);
	int Show(HWND hwnd, UINT id, LPCTSTR lpszCaption, UINT nType=MB_OK);
	int Show(HWND hwnd, LPCTSTR lpszText, LPCTSTR lpszCaption=NULL, UINT nType=MB_OK);
#endif
};

#endif // __MESSAGEBOX_H__
