#pragma warning( disable : 4786 )

#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "TxtMiru.h"

#define TXTMIRUAPP

CGrTxtMiru &CGrTxtMiru::theApp()
{
	static CGrTxtMiru app; // <- (各Include先で実体が作成されるので)実体をヘッダに記述しないこと! ※確認するのであれば、コンストラクタが何回呼ばれているかをチェック
	return app;
}

BOOL CGrTxtMiru::IsDialogMessage(HWND hwnd, MSG &msg)
{
	if(m_bUseAcess && ::TranslateAccelerator(hwnd, m_haccel, &msg)){
		return TRUE;
	}
	if(m_hModelessDlg && ::IsDialogMessage(m_hModelessDlg, &msg)){
		return TRUE;
	}
	if(IsUseDialogMessage() && ::IsDialogMessage(hwnd, &msg)){
		return TRUE;
	}
	return FALSE;
}

#include "shell.h"
static TCHAR l_data_path[MAX_PATH] = {};

LPCTSTR CGrTxtMiru::GetDataPath()
{
	if(l_data_path[0] == '\0'){
		TCHAR curPath[MAX_PATH];
		CGrShell::GetExePath(curPath);
		TCHAR iniFileName[MAX_PATH];
		_stprintf_s(iniFileName, _T("%s/%s.ini"), curPath, CGrTxtMiru::AppName());
		int iUserApplicatinData = ::GetPrivateProfileInt(_T("TxtMiru"), _T("UseApplicationData"), 0, iniFileName);
		if(iUserApplicatinData == 1){
			CGrShell::GetDataPath(l_data_path, _countof(l_data_path), AppName());
			CGrShell::CreateFolder(l_data_path);
		} else {
			_tcscpy_s(l_data_path, curPath);
		}
	}
	return l_data_path;
}

void CGrTxtMiru::MoveDataDir()
{
	::SetCurrentDirectory(GetDataPath());
}

TCHAR l_work_path[_MAX_PATH+1] = {};
LPCTSTR CGrTxtMiru::GetWorkPath()
{
	if(l_work_path[0] == '\0'){
		TCHAR buf[_MAX_PATH] = {};
		if(::GetTempPath(sizeof(buf)/sizeof(TCHAR), buf) == 0){
			return nullptr;
		}
		CGrShell::AddBackSlash(buf);
		std::tstring filename;
		CGrShell::ToPrettyFileName(buf, filename);
		_stprintf_s(l_work_path, _T("%sTxtMiru.%d/"), filename.c_str(), ::GetCurrentProcessId());
		UINT len = _tcslen(l_work_path);
		l_work_path[_tcslen(l_work_path)+1] = '\0'; // 末尾が \0\0 になるように
	}
	return l_work_path;
}

//
void CGrTxtMiru::SetMouseCapture(HWND hwnd)
{
	m_hMouseCaptureWnd = hwnd;
}
void CGrTxtMiru::ReleaseMouseCapture()
{
	m_hMouseCaptureWnd = NULL;
}
void CGrTxtMiru::PostMouseCaptureMessage(MSG &msg)
{
	if(m_hMouseCaptureWnd && m_hMouseCaptureWnd != msg.hwnd && (msg.message == WM_NCMOUSEMOVE || msg.message == WM_MOUSEMOVE)){
		POINT pos = {};
		if(GetCursorPos(&pos)){
			ScreenToClient(m_hMouseCaptureWnd, &pos);
			::PostMessage(m_hMouseCaptureWnd, msg.message, msg.lParam, MAKELPARAM((pos.x), (pos.y)));
		}
	}
}
