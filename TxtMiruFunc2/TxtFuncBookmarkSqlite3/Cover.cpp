#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "Cover.h"
#include "Shell.h"
#include <wininet.h>
#include <shlwapi.h>
#pragma warning (push)
#pragma warning(disable:4768)
#pragma warning(disable:4091)
#include <shlobj.h>
#pragma warning (pop)
#include "TxtFunc.h"
#include "TxtFuncIParam.h"
#include "TxtMiruTheme.h"

static bool GetUrlCacheFileName(LPCWSTR lpwUrl, LPWSTR lpszwFileName, DWORD dwFileNameInBytes)
{
	if(CGrText::isMatchChar(lpwUrl, _T("file:///"))){
		if (lstrcpynW(lpszwFileName, &(lpwUrl[sizeof(_T("file:///")) / sizeof(TCHAR) - 1]), dwFileNameInBytes)) {

		}
		return true;
	}
	DWORD dwCbCacheInfo = 1024;
	auto lpCacheInfo = reinterpret_cast<LPINTERNET_CACHE_ENTRY_INFO>(new char[dwCbCacheInfo]);
	if (!lpCacheInfo) {
		return false;
	}
	bool bret = true;

	do {
		ZeroMemory(lpCacheInfo, dwCbCacheInfo);
		if(!GetUrlCacheEntryInfo(lpwUrl, lpCacheInfo, &dwCbCacheInfo)){
			switch(GetLastError()){
			case ERROR_FILE_NOT_FOUND:
				if(S_OK == URLDownloadToCacheFile(NULL, lpwUrl, lpszwFileName, dwFileNameInBytes, 0, NULL)){
					delete [] lpCacheInfo;
					return true;
				}
				bret = false;
				break;
			case ERROR_INSUFFICIENT_BUFFER:
				delete [] lpCacheInfo;
				lpCacheInfo = reinterpret_cast<LPINTERNET_CACHE_ENTRY_INFO>(new char[dwCbCacheInfo]);
				continue;
			default:
				bret = false;
				break;
			}
		}
		break;
	} while(true);
	if(bret && FAILED(_tcscpy_s(lpszwFileName, dwFileNameInBytes, lpCacheInfo->lpszLocalFileName))){
		bret = false;
	}
	delete [] lpCacheInfo;

	return bret;
}

CGrCover::CGrCover()
{
}

CGrCover::~CGrCover()
{
}
HWND CGrCover::Create(HWND hWnd)
{
	m_hWnd = CreateWindow(_T("static"), _T("cover"), WS_CHILD | WS_VISIBLE,
						  0, 0, 8, 8, hWnd, NULL, CGrTxtFunc::GetDllModuleHandle(), NULL);
	Attach(m_hWnd);
	DragAcceptFiles(m_hWnd, TRUE);
	return m_hWnd;
}

void CGrCover::Attach(HWND hWnd)
{
	m_pictMgr.Initialize();
	CGrWinCtrl::Attach(hWnd);
}

LRESULT CGrCover::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORSTATIC:
		{
			auto hDC = reinterpret_cast<HDC>(wParam);
			auto hCtrl = reinterpret_cast<HWND>(lParam);
			SetBkMode(hDC, OPAQUE);
			SetTextColor(hDC, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor(hDC, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
			return reinterpret_cast<LRESULT>(TxtMiruTheme_GetSysColorBrush(COLOR_WINDOW));
		}
	case WM_ERASEBKGND:
		return TRUE;
	case WM_SIZE:
		{
			auto state = static_cast<UINT>(wParam);
			int cx = static_cast<int>(static_cast<short>(LOWORD(lParam)));
			int cy = static_cast<int>(static_cast<short>(HIWORD(lParam)));
			if(!m_filename.empty() && m_image.Width() != cx && m_image.Height() != cy){
				m_image.Destroy();
				m_pictMgr.Draw(m_image, 0, 0, cx, cy, m_filename.c_str());
				InvalidateRect(m_hWnd, nullptr, FALSE);
			}
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			auto hdc = ::BeginPaint(hWnd, &ps);
			RECT rect;
			::GetClientRect(hWnd, &rect);
			int height = rect.bottom - rect.top ;
			int width  = rect.right  - rect.left;
			auto oldclr = ::SetBkColor(hdc, TxtMiruTheme_GetSysColor(COLOR_WINDOW));
			::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			if(height > 0 && width > 0){
				{
					auto image_rect = rect;
					if(m_image.Width() > 0){
						int x = width  / 2 - m_image.Width () / 2;
						int y = height / 2 - m_image.Height() / 2;
						auto hImageDC = CreateCompatibleDC(NULL);
						auto hOldBitmap = SelectBitmap(hImageDC, m_image);
						BitBlt(hdc, x, y, m_image.Width(), m_image.Height(), hImageDC, 0, 0, SRCCOPY);
						SelectBitmap(hImageDC, hOldBitmap);
						DeleteDC(hImageDC);
					}
				}
				//
			}
			::SetBkColor(hdc, oldclr);
			::EndPaint(hWnd, &ps);
		}
		break;
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

bool CGrCover::load(LPCTSTR lpFileName)
{
	bool bret = false;
	if(m_pictMgr.GetSupportedFile(m_filename, lpFileName)){
		m_image.Destroy();
		if(m_hWnd){
			RECT rect;
			::GetClientRect(m_hWnd, &rect);
			m_pictMgr.Draw(m_image, 0, 0, rect.right-rect.left, rect.bottom-rect.top, m_filename.c_str());
		}
		bret = true;
	}
	if(m_hWnd){
		InvalidateRect(m_hWnd, NULL, FALSE);
	}
	return bret;
}

bool CGrCover::Load(int id)
{
	m_bLoad = false;
	bool bret = false;
	auto &param = CGrTxtFunc::Param();
	std::tstring str;
	CGrTxtFunc::GetBookmarkFolder(&param, str);

	m_filename = _T("");
	m_image.Destroy();
	TCHAR path[MAX_PATH];
	_stprintf_s(path, _T("%s/cover/%d"), str.c_str(), id);
	bret = load(path);
	if(bret){
		m_bLoad = true;
	} else {
		TCHAR curPath[MAX_PATH];
		CGrShell::GetExePath(curPath);
		_stprintf_s(path, _T("%s/toolbar_icon/no_image"),curPath);
		bret = load(path);
	}
	return bret;
}

bool CGrCover::Load(LPCTSTR lpFileName)
{
	bool bret = false;
	TCHAR tmp_fileName[MAX_PATH] = {};
	if(CGrShell::IsURI(lpFileName)){
		if(GetUrlCacheFileName(lpFileName, tmp_fileName, sizeof(tmp_fileName)/sizeof(TCHAR))){
			lpFileName = tmp_fileName;
		}
	}
	if(m_pictMgr.GetSupportedFile(m_filename, lpFileName)){
		m_image.Destroy();
		RECT rect;
		::GetClientRect(m_hWnd, &rect);
		m_pictMgr.Draw(m_image, 0, 0, rect.right-rect.left, rect.bottom-rect.top, m_filename.c_str());
		bret = true;
		m_bLoad = true;
	}
	InvalidateRect(m_hWnd, nullptr, FALSE);
	return bret;
}

const LPCTSTR CGrCover::GetFilename()
{
	if(m_bLoad){
		return m_filename.c_str();
	} else {
		return nullptr;
	}
}
