#pragma warning( disable : 4786 )
#include "TxtDocInfoDlg.h"
#include "LoadDllFunc.h"
#include "Win32Wrap.h"

namespace CGrTxtDocInfoDlg
{
	int DoModal(HWND hWnd, LPCTSTR title, LPCTSTR author, LPCTSTR filename, LPCTSTR text, LPCTSTR writetime, int page, int maxpage)
	{
		CGrLoadDllFunc func(_T("TxtFuncConfig.dll"));
		bool (cdecl *DocInfoDlg)(HWND hWnd, LPCTSTR title, LPCTSTR author, LPCTSTR filename, LPCTSTR text, LPCTSTR writetime, int page, int maxpage);
		SetProcPtr(DocInfoDlg, func.GetProcAddress("TxtFuncDocInfoDlg", hWnd));
		if(DocInfoDlg && DocInfoDlg(hWnd, title, author, filename, text, writetime, page, maxpage)){
			return IDOK;
		}
		return IDCANCEL;
	}
};
