#ifndef __TXTDOCINFODLG_H__
#define __TXTDOCINFODLG_H__

#include <windows.h>
namespace CGrTxtDocInfoDlg
{
	int DoModal(HWND hWnd, LPCTSTR title, LPCTSTR author, LPCTSTR filename, LPCTSTR text, LPCTSTR writetime, int page, int maxpage);
};

#endif // __TXTDOCINFODLG_H__
