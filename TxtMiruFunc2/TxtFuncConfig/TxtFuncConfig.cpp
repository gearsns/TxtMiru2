// TxtFuncConfig.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "TxtFuncConfig.h"

#include "resource.h"
#include "TxtConfigDlg.h"
#include "TxtLayoutMgr.h"

#include "stltchar.h"
#include "stlutil.h"
#include "TxtFunc.h"

#define TXTMIRUAPP
#include "TxtMiruFunc.h"
#include "Text.h"
#include "Shell.h"

LPCTSTR g_dataDir = nullptr;
CGrTxtFuncIParam *g_pParam = nullptr;
namespace CGrTxtFunc
{
	void MoveDataDir()
	{
		SetCurrentDirectory(g_dataDir);
	}
	LPCTSTR GetDataPath()
	{
		return g_dataDir;
	}
	CGrTxtFuncIParam &Param()
	{
		return *g_pParam;
	}
	LPCTSTR AppName()
	{
		return _T("TxtMiru");
	}
	void GetBookmarkFolder(CGrTxtFuncIParam *pParam, std::tstring &str)
	{
		TCHAR file_path[512];
		TCHAR buf[1024];
		pParam->GetText(CGrTxtFuncIParam::BookMarkFolder, buf, sizeof(buf)/sizeof(TCHAR));
		str = buf;
		_tcscpy_s(file_path, CGrTxtFunc::GetDataPath());
		switch(str.size()){
		case 0:
			CGrText::FormatMessage(str, _T("%1!s!/Bookmark"), file_path);
			break;
		case 1:
			if(CGrShell::IsBackSlash(str[0])){
				// 「\」ルート 指定
				if(CGrShell::IsBackSlash(file_path[0])){
					// ネットワークパスに対して、「\」 トップ指定は多分できないのでデフォルトパスに変更
					CGrText::FormatMessage(str, _T("%1!s!/Bookmark"), file_path);
				} else {
					file_path[2] = _T('\0');
					CGrText::FormatMessage(str, _T("%1!s!/"), file_path);
				}
			} else {
				// 相対パスから絶対パスに変更
				CGrText::FormatMessage(str, _T("%1!s!/%2!s!"), file_path, str.c_str());
			}
			break;
		default:
			if(CGrShell::IsBackSlash(str[0]) && !CGrShell::IsBackSlash(str[1])){
				// 「\」ルート 指定
				if(CGrShell::IsBackSlash(file_path[0])){
					// ネットワークパスに対して、「\」 トップ指定は多分できないのでデフォルトパスに変更
					CGrText::FormatMessage(str, _T("%1!s!/Bookmark"), file_path);
				} else {
					file_path[2] = _T('\0');
					CGrText::FormatMessage(str, _T("%1!s!/%2!s!"), file_path, str.c_str());
				}
			} else if(str[1] != _T(':')){
				// 相対パスから絶対パスに変更
				CGrText::FormatMessage(str, _T("%1!s!/%2!s!"), file_path, str.c_str());
			}
			break;
		}
	}
}

#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
extern "C" TXTFUNCCONFIG_API bool cdecl TxtFuncConfig(HWND hWnd, LPCTSTR lpkeyBindFileName, LPCTSTR lpDataDir, CGrTxtFuncIParam *pParam)
{
	g_dataDir = lpDataDir;
	g_pParam  = pParam;
	if(!lpDataDir || !pParam){
		return false;
	}
	CGrTxtConfigDlg dlg;
	dlg.DoModal(hWnd, lpkeyBindFileName);
	return true;
}

extern "C" TXTFUNCCONFIG_API bool cdecl TxtFuncLayoutConfig(HWND hWnd, LPCTSTR lpDataDir, LPCTSTR type, LPCTSTR name, LPCTSTR lpFileName, CGrTxtFuncIParam *pParam)
{
	g_dataDir = lpDataDir;
	g_pParam  = pParam;
	if(!lpDataDir || !pParam){
		return false;
	}
	CGrTxtLayoutMgr mgr;
	mgr.ConfigurationDlg(hWnd, type, name, lpFileName);
	return true;
}

#include "TxtFuncBrowser.h"
extern "C" TXTFUNCCONFIG_API bool cdecl TxtFuncGetBrowserURL(LPTSTR *ppURL, CGrTxtFuncIParam *pParam)
{
	TCHAR szLanguageName[100];
	auto idLocal = GetSystemDefaultLCID();
	::GetLocaleInfo(idLocal, LOCALE_SENGLANGUAGE, szLanguageName, _countof(szLanguageName));
	_tsetlocale(LC_ALL,szLanguageName);
	*ppURL = nullptr;
	TCHAR buf[4098];
	pParam->GetText(CGrTxtFuncIParam::BrowserAppName, buf, sizeof(buf)/sizeof(TCHAR));
	CGrCSVText csv(buf);
	auto *pRow = csv.GetRow(0);
	if(pRow){
		for(const auto &item : *pRow){
			auto pURL = getURLinBrowser(item.c_str());
			if(pURL){
				*ppURL = pURL;
				return true;
			}
		}
	}
	return false;
}

#include "TxtGotoPageDlg.h"
extern "C" bool cdecl TxtFuncGotoPageDlg(HWND hWnd, int maxnum, int cur_page, int *out_page)
{
	CGrTxtGotoPageDlg dlg(maxnum, cur_page);
	if(dlg.DoModal(hWnd) == IDOK){
		*out_page = dlg.GetPage();
		return true;
	}
	return false;
}

#include "TxtDocInfoDlg.h"
extern "C" bool cdecl TxtFuncDocInfoDlg(HWND hWnd, LPCTSTR title, LPCTSTR author, LPCTSTR filename, LPCTSTR text, LPCTSTR writetime, int page, int maxpage)
{
	CGrTxtDocInfoDlg dlg(title, author, filename, text, writetime, page, maxpage);
	if(dlg.DoModal(hWnd) == IDOK){
		return true;
	}
	return false;
}

#include "VersionDlg.h"
extern "C" bool cdecl TxtFuncVersionInfoDlg(HWND hWnd, HINSTANCE hInst, CGrTxtFuncIParam *pParam)
{
	g_pParam  = pParam;
	CGrVersionDlg dlg(hInst);
	if(dlg.DoModal(hWnd) == IDOK){
		return true;
	}
	return false;
}
