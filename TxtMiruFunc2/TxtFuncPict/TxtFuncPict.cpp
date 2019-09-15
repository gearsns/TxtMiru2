// TxtFuncPict.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "TxtFuncPict.h"

#include "tchar.h"
#include "SaveDlg.h"

static LPCTSTR g_ini_filename = nullptr;
namespace CGrTxtFunc
{
	void GetPointsINI(LPCTSTR key, int *pnum, int num)
	{
		TCHAR str[1024];
		::GetPrivateProfileString(_T("points"), key, _T(""), str, sizeof(str)/sizeof(TCHAR), g_ini_filename);
		if(str[0] != _T('\0')){
			auto *pbegin = str;
			for(; num>0; --num, ++pnum){
				auto *p = _tcschr(pbegin, _T(','));
				if(p){
					*p = '\0';
				}
				*pnum = _tstoi(pbegin);
				if(!p){
					break;
				}
				pbegin = p + 1;
			}
		}
	}
	void SetPointsINI(LPCTSTR key, int *pnum, int num)
	{
		TCHAR buf[1024];
		_stprintf_s(buf, _T("%d"), pnum[0]);
		std::tstring str(buf);
		for(int col=1; col<num; ++col){
			_stprintf_s(buf, _T(",%d"), pnum[col]);
			str += buf;
		}
		::WritePrivateProfileString(_T("points"), key, str.c_str(), g_ini_filename);
	}
};

extern "C" TXTFUNCPICT_API bool cdecl TxtFuncSavePicture(HWND hWnd, LPCTSTR ini_filename, LPCTSTR filename, int total_page, SIZE paper_size, CGrTxtFuncICanvas *pCanvas)
{
	g_ini_filename = ini_filename;
	CGrSaveDlg dlg(filename, total_page, paper_size, pCanvas);
	dlg.Save(hWnd);
	return true;
}
