#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <wininet.h>
#include <process.h>
#include "resource.h"
#include "VersionDlg.h"
#include "Shell.h"
#include "stlutil.h"
#include "TxtFunc.h"
#include "TxtFuncIParam.h"
#include "CSVText.h"
#include "TxtMiruTheme.h"

CGrVersionDlg::CGrVersionDlg(HINSTANCE hInst) : m_hInst(hInst)
{
}

CGrVersionDlg::~CGrVersionDlg()
{
}

int CGrVersionDlg::DoModal(HWND hWnd)
{
	return DialogBoxParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_TXTMIRU), hWnd, reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
}

#define WM_VERUPDATE (WM_APP+1)

LRESULT CGrVersionDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG    , OnInitDialog   );
		HANDLE_MSG(hWnd, WM_COMMAND       , OnCommand      );
		HANDLE_MSG(hWnd, WM_SIZE          , OnSize         );
		HANDLE_MSG(hWnd, WM_GETMINMAXINFO , OnGetMinMaxInfo);
	case WM_DESTROY:
		if(m_hThread){
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		break;
	case WM_VERUPDATE:
		{
			if(m_hThread){
				CloseHandle(m_hThread);
				m_hThread = NULL;
			}
			if(m_server_version.size() > 0){
				auto hEdit = GetDlgItem(hWnd, IDC_EDIT_INFO);
				if(hEdit){
					std::tstring str;
					str = m_client_version;
					str +=_T("\r\n\r\n");
					TCHAR buf[512];
					CGrText::LoadString(IDS_NEWVERSION, buf, sizeof(buf)/sizeof(TCHAR));
					str += buf;
					str += m_server_version;
					Edit_SetText(hEdit, str.c_str());
				}
			}
		}
		break;
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

unsigned __stdcall CGrVersionDlg::GetVersionProc(void *lpParam)
{
	auto* pDlg = reinterpret_cast<CGrVersionDlg*>(lpParam);
	std::tstring version_info;
	auto &&param = CGrTxtFunc::Param();
	char *charProxyServer    = nullptr;
	char *charNoProxyAddress = nullptr;
	std::tstring server_version;
	HINTERNET hInet = NULL;
	HINTERNET hFile = NULL;
	auto hWnd = pDlg->GetWnd();
	do {
		hInet = InternetOpen(_T("TxtMiru"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if(!hInet){
			break;
		}
		LPCTSTR url = _T("https://drive.google.com/uc?id=1Hi6pI2tu0l_c9J6vnL2cEz68XRB7x4zY");

		INTERNET_PROXY_INFO proxy = {0};
		if(param.GetBoolean(CGrTxtFuncIParam::PointsType::UseIESetting)){
			proxy.dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG;
		} else if(param.GetBoolean(CGrTxtFuncIParam::PointsType::UseProxy)){
			//プロクシの変更
			TCHAR strProxyServer   [2048];
			TCHAR strNoProxyAddress[2048];
			param.GetText(CGrTxtFuncIParam::TextType::ProxyServer   , strProxyServer   , sizeof(strProxyServer   )/sizeof(TCHAR));
			param.GetText(CGrTxtFuncIParam::TextType::NoProxyAddress, strNoProxyAddress, sizeof(strNoProxyAddress)/sizeof(TCHAR));
			int lenStrProxyServer    = lstrlen(strProxyServer   ) * 3;
			int lenStrNoProxyAddress = lstrlen(strNoProxyAddress) * 3;
			charProxyServer    = new char[lenStrProxyServer    + 1];
			charNoProxyAddress = new char[lenStrNoProxyAddress + 1];
			if(!charProxyServer || !charNoProxyAddress){
				break;
			}
			memset(charProxyServer   , 0x00, lenStrProxyServer   +1);
			memset(charNoProxyAddress, 0x00, lenStrNoProxyAddress+1);
			if(lenStrProxyServer > 0){
				sprintf_s(charProxyServer   , lenStrProxyServer   , "%S", strProxyServer   );
			}
			if(lenStrNoProxyAddress > 0){
				sprintf_s(charNoProxyAddress, lenStrNoProxyAddress, "%S", strNoProxyAddress);
			}
			proxy.dwAccessType    = INTERNET_OPEN_TYPE_PROXY   ;
			proxy.lpszProxy       = reinterpret_cast<LPCTSTR>(charProxyServer)   ;
			proxy.lpszProxyBypass = reinterpret_cast<LPCTSTR>(charNoProxyAddress);
		} else {
			proxy.dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
		}
		InternetSetOption(hInet, INTERNET_OPTION_PROXY, &proxy, sizeof(proxy));
		DeleteUrlCacheEntry(url);
		hFile = InternetOpenUrl(hInet, url, NULL, 0,
								INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE
								| INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID
								| INTERNET_FLAG_IGNORE_CERT_CN_INVALID
								, 0);
		if(!hInet){
			break;
		}
		TCHAR buf[2048] = {};
		DWORD dwSize = 0;
		do {
			if(InternetReadFile(hFile, buf, sizeof(buf)/sizeof(TCHAR), &dwSize)){
				if(dwSize/sizeof(TCHAR) < sizeof(buf)/sizeof(TCHAR)){
					buf[dwSize/sizeof(TCHAR)] = '\0';
				} else {
					buf[sizeof(buf)/sizeof(TCHAR)-1] = '\0';
				}
				server_version += buf;
			}
		} while(dwSize > 0);
	} while(0);
	if(hFile){
		 InternetCloseHandle(hFile);
	}
	if(charProxyServer   ){ delete [] charProxyServer   ; }
	if(charNoProxyAddress){ delete [] charNoProxyAddress; }

	if(server_version.size() > 0 && IsWindowVisible(hWnd)){
		pDlg->m_server_version = server_version;
		PostMessage(hWnd, WM_VERUPDATE, 0, 0);
	}
	return 0;
}

BOOL CGrVersionDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowPosCenter();

	RECT win_rect;
	::GetWindowRect(m_hWnd, &win_rect);
	m_minSize.cx = win_rect.right - win_rect.left;
	m_minSize.cy = win_rect.bottom - win_rect.top;

	TCHAR str[2048] = {};
	//
	TCHAR module_path[MAX_PATH+1];
	CGrShell::GetOwnFileName(module_path, sizeof(module_path)/sizeof(TCHAR));
	LPVOID lpData = nullptr;
	//
	auto hIcon = static_cast<HICON>(LoadImage(m_hInst, MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 32, 32, LR_SHARED));
	SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
	auto hIconSm = reinterpret_cast<HICON>(LoadImage(CGrTxtFunc::GetDllModuleHandle(),MAKEINTRESOURCE(IDI_APP),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR));
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSm));

	do {
		DWORD dw = 0;
		auto size = ::GetFileVersionInfoSize(module_path, &dw);
		if(size <= 0){ break; }
		lpData = new char[size];
		if(!lpData){ break; }
		// ファイルのバージョン情報を取得
		if(!::GetFileVersionInfo(module_path, 0, size, lpData)){
			break;
		}
		UINT ud;
		LPWORD lpTrans;
		if(!::VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), reinterpret_cast<void**>(&lpTrans), &ud)){
			break;
		}
		auto dwLangCharset = MAKELONG(lpTrans[1], lpTrans[0]);
		TCHAR key[512];
		LPCTSTR lpProductName, lpProductVersion, lpFileDescription, lpLegalCopyright, lpComments;
		// 製品名
		_stprintf_s(key, _T("\\StringFileInfo\\%08x\\%s"), dwLangCharset, _T("ProductName"));
		if(!::VerQueryValue(lpData, key, reinterpret_cast<void**>(const_cast<TCHAR**>(&lpProductName)), &ud)){
			break;
		}
		// バージョン
		_stprintf_s(key, _T("\\StringFileInfo\\%08x\\%s"), dwLangCharset, _T("ProductVersion"));
		if(!::VerQueryValue(lpData, key, reinterpret_cast<void**>(const_cast<TCHAR**>(&lpProductVersion)), &ud)){
			break;
		}
		std::tstring version = lpProductVersion;
		std::replace(version, _T(" "), _T(""));
		std::replace(version, _T(","), _T("."));
		lpProductVersion = version.c_str();
		// 説明
		_stprintf_s(key, _T("\\StringFileInfo\\%08x\\%s"), dwLangCharset, _T("FileDescription"));
		if(!::VerQueryValue(lpData, key, reinterpret_cast<void**>(const_cast<TCHAR**>(&lpFileDescription)), &ud)){
			break;
		}
		// コピーライト
		_stprintf_s(key, _T("\\StringFileInfo\\%08x\\%s"), dwLangCharset, _T("LegalCopyright"));
		if(!::VerQueryValue(lpData, key, reinterpret_cast<void**>(const_cast<TCHAR**>(&lpLegalCopyright)), &ud)){
			break;
		}
		// コメント
		_stprintf_s(key, _T("\\StringFileInfo\\%08x\\%s"), dwLangCharset, _T("Comments"));
		if(!::VerQueryValue(lpData, key, reinterpret_cast<void**>(const_cast<TCHAR**>(&lpComments)), &ud)){
			break;
		}
		//
		_stprintf_s(str, _T("%s\r\n%s %s\r\n\r\n%s\r\n\r\n%s"), lpFileDescription, lpProductName, lpProductVersion, lpLegalCopyright, lpComments);
	} while(0);
	if(lpData){ delete [] lpData; }

	//
	m_client_version = str;
	Edit_SetText(GetDlgItem(hwnd, IDC_EDIT_INFO), str);
	//
	TxtMiruTheme_SetWindowSubclass(m_hWnd);

	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0 , GetVersionProc, reinterpret_cast<void*>(this)/*param*/, 0, &m_threadid));
	setWindowSize(-1, -1);

	return TRUE;
}

void CGrVersionDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if(id == IDOK || id == IDCANCEL){
		::EndDialog(m_hWnd, IDOK);
	}
}

void CGrVersionDlg::setWindowSize(int cx, int cy)
{
	if(cx < 0 || cy < 0){
		RECT win_rect;
		::GetClientRect(m_hWnd, &win_rect);
		cx = win_rect.right - win_rect.left;
		cy = win_rect.bottom - win_rect.top;
	}
	auto hdwp = BeginDeferWindowPos(2);
	do {
		RECT rect;
		int bottom = cy;
		{
			auto hCWnd = GetDlgItem(m_hWnd, IDOK);
			if(!hCWnd){
				break;
			}
			GetWindowRect(hCWnd, &rect);
			ScreenToClientRect(&rect);
			int height = rect.bottom - rect.top;
			int width = rect.right - rect.left;
			bottom -= height;
			rect.left = cx /2 - width / 2;
			hdwp = DeferWindowPos(hdwp, hCWnd, NULL, rect.left, bottom, -1, -1, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
		}
		{
			auto hCWnd = GetDlgItem(m_hWnd, IDC_EDIT_INFO);
			if(!hCWnd){
				break;
			}
			GetWindowRect(hCWnd, &rect);
			ScreenToClientRect(&rect);
			int height = bottom - rect.top;
			int width = cx - rect.left;
			hdwp = DeferWindowPos(hdwp, hCWnd, NULL, -1, -1, width, height, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
		}
	} while(0);
	EndDeferWindowPos(hdwp);
}

void CGrVersionDlg::OnSize(HWND hwnd, UINT nType, int cx, int cy)
{
	setWindowSize(cx, cy);
}

void CGrVersionDlg::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	lpMinMaxInfo->ptMinTrackSize.x = m_minSize.cx;
	lpMinMaxInfo->ptMinTrackSize.y = m_minSize.cy;
}

