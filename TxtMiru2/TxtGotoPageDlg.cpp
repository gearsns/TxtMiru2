#pragma warning( disable : 4786 )
#include "TxtGotoPageDlg.h"
#include "LoadDllFunc.h"
#include "Win32Wrap.h"

namespace CGrTxtGotoPageDlg
{
	int DoModal(HWND hWnd, int maxnum, int cur_page, int *out_page)
	{
		CGrLoadDllFunc func(_T("TxtFuncConfig.dll"));
		bool (cdecl *GotoPageDlg)(HWND hWnd, int maxnum, int cur_page, int *out_page);
		SetProcPtr(GotoPageDlg, func.GetProcAddress("TxtFuncGotoPageDlg", hWnd));
		if(GotoPageDlg && GotoPageDlg(hWnd, maxnum, cur_page, out_page)){
			return IDOK;
		}
		return IDCANCEL;
	}
};
