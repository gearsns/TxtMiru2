#pragma warning( disable : 4786 )
#include "VersionDlg.h"
#include "LoadDllFunc.h"
#include "stdwin.h"
#include "Win32Wrap.h"

namespace CGrVersionDlg
{
	int DoModal(HWND hWnd)
	{
		auto &&param = CGrTxtMiru::theApp().Param();
		CGrLoadDllFunc func(_T("TxtFuncConfig.dll"));
		bool (cdecl *VersionInfoDlg)(HWND hWnd, HINSTANCE hInst, CGrTxtFuncIParam *pParam);
		SetProcPtr(VersionInfoDlg, func.GetProcAddress("TxtFuncVersionInfoDlg", hWnd));
		if(VersionInfoDlg && VersionInfoDlg(hWnd, CGrStdWin::GetInst(), &param)){
			return IDOK;
		}
		return IDCANCEL;
	}
};
