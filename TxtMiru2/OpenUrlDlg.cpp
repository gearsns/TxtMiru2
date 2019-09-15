#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "OpenUrlDlg.h"
#include "Image.h"
#include "Text.h"
#include "ImageFile.h"
#include "TxtMiru.h"
#include "TxtMiruTheme.h"

#define IsKeyDown(vk) (::GetAsyncKeyState(vk) & 0x8000)

CGrURLEditBox::CGrURLEditBox() : CGrComonCtrl()
{
}
CGrURLEditBox::~CGrURLEditBox()
{
	if(m_hImage){
		::ImageList_Destroy(m_hImage);
	}
}

#include "stdwin.h"
#define ID_TIMER_LOADING 100
#define ANIMATION_TIME   100
void CGrURLEditBox::SetImage(UINT id)
{
	if(m_hImage){
		::ImageList_Destroy(m_hImage);
	}
	auto hInst = CGrStdWin::GetInst();
	CGrBitmap bmp;
	{
		LOGFONT logfont = {};
		GetObject(GetWindowFont(m_hWnd), sizeof(logfont), &logfont);
		m_iconSize = abs(logfont.lfHeight);
		bmp.Create(m_iconSize, m_iconSize);
		int icon_num = 0;
		bool bNext = true;
		CGrImageFile imgFile(_T("toolbar_icon\\loading_"));
		while(true){
			++icon_num;
			if(!imgFile.Draw(bmp, icon_num, m_iconSize, m_iconSize)){
				break;
			}
			if(icon_num == 1){
				m_hImage = ImageList_Create(m_iconSize, m_iconSize, ILC_COLOR32, 1, 1);
			}
			ImageList_Add(m_hImage, bmp, NULL);
		}
		if(icon_num <= 1){
			m_iconSize = 16;
			m_hImage = ImageList_Create(m_iconSize, m_iconSize, ILC_COLOR32, 1, 1);
			CGrImage::LoadCustomImage(bmp, MAKEINTRESOURCE(id));
			ImageList_Add(m_hImage, bmp, NULL);
		}
	}
	m_imageCount = ImageList_GetImageCount(m_hImage);
	m_imageNo = 0;

	::SetTimer(m_hWnd, ID_TIMER_LOADING, ANIMATION_TIME, NULL);
}

BOOL CGrURLEditBox::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	return TRUE;
}

LRESULT CGrURLEditBox::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_TIMER:
		KillTimer(hWnd, ID_TIMER_LOADING);
		++m_imageNo;
		if(m_imageNo >= m_imageCount){
			m_imageNo = 0;
		}
		::InvalidateRect(m_hWnd, NULL, FALSE);
		::SetTimer(m_hWnd, ID_TIMER_LOADING, ANIMATION_TIME, NULL);
		break;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = wParam == 0 ? BeginPaint(hWnd, &ps) : reinterpret_cast<HDC>(wParam);
			int nOldMode = SetBkMode(hdc, OPAQUE);
			SetTextColor(hdc, TxtMiruTheme_GetSysColor(COLOR_WINDOWTEXT));	// テキストの色
			auto ret = CGrComonCtrl::WndProc(hWnd, uMsg, reinterpret_cast<WPARAM>(hdc), lParam);
			if (m_imageCount > 0 && m_iconSize > 0) {
				RECT rect;
				GetClientRect(hWnd, &rect);
				ImageList_Draw(m_hImage, m_imageNo, hdc, rect.right - m_iconSize, ((rect.bottom - rect.top) / 2) - (m_iconSize / 2), ILD_NORMAL);
			}
			SetBkMode(hdc, nOldMode); /*  reset painting mode */
			if (!wParam) {
				EndPaint(hWnd, &ps);
			}
			return FALSE;
		}
		break;
	case WM_SETCURSOR:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			ClientToScreenRect(hWnd, &rect);
			POINT pos;
			GetCursorPos(&pos);
			if(pos.x > rect.right-m_iconSize){
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				return 0;
			}
		}
		SetCursor(LoadCursor(NULL, IDC_IBEAM));
		break;
	case WM_LBUTTONDOWN:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			ClientToScreenRect(hWnd, &rect);
			POINT pos;
			GetCursorPos(&pos);
			if(pos.x > rect.right-m_iconSize){
				FORWARD_WM_COMMAND(GetParent(hWnd), IDCANCEL/*id*/, hWnd/*hwndCtl*/, 0/*codeNotify*/, SendMessage);
			}
		}
		break;
	}
	return CGrComonCtrl::WndProc(hWnd, uMsg, wParam, lParam);
};
//////////////////

static bool l_bWaitting = false;

bool CGrOpenURLDlg::IsWaitting()
{
	return l_bWaitting;
}
CGrOpenURLDlg::CGrOpenURLDlg() : CGrComonCtrl()
{
}
CGrOpenURLDlg::~CGrOpenURLDlg()
{
	Close();
}

void CGrOpenURLDlg::Show(HWND hWnd, LPCTSTR url)
{
	m_hParentWnd = hWnd;
	if(url){
		m_url = url;
	}
	l_bWaitting = true;
	if(!m_hWnd){
		::CreateDialogParam(GetWindowInstance(CGrStdWin::GetWnd()), MAKEINTRESOURCE(IDD_OPENURL), hWnd, reinterpret_cast<DLGPROC>(CGrWinCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
	}
	if(m_hWnd){
		::ShowWindow(m_hWnd, SW_SHOW);
		EnableWindow(m_hParentWnd, FALSE);
	}
}

void CGrOpenURLDlg::Close()
{
	if(m_hWnd){
		EnableWindow(m_hParentWnd, TRUE);
		::DestroyWindow(m_hWnd);
	}
	l_bWaitting = false;
}

int CGrOpenURLDlg::DoModal(HWND hWnd, LPCTSTR url)
{
	m_hParentWnd = hWnd;
	m_doModal = true;
	if(url){
		m_url = url;
	}
	return DialogBoxParam(GetWindowInstance(hWnd), MAKEINTRESOURCE(IDD_OPENURL), hWnd, reinterpret_cast<DLGPROC>(CGrComonCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
}
void CGrOpenURLDlg::SetWindowSize()
{
	const int PADDING = 4;
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	auto hcbtn = GetDlgItem(m_hWnd, IDCANCEL);
	if(hcbtn){
		RECT btn_rect;
		GetWindowRect(hcbtn, &btn_rect);
		int w = (btn_rect.right - btn_rect.left);
		rect.right -= (w+PADDING);
		::SetWindowPos(hcbtn, 0, rect.right, PADDING, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		rect.right -= PADDING;
	}
	hcbtn = GetDlgItem(m_hWnd, IDOK);
	if(hcbtn){
		if(m_doModal){
			RECT btn_rect;
			GetWindowRect(hcbtn, &btn_rect);
			int w = (btn_rect.right - btn_rect.left);
			rect.right -= (w+PADDING);
			::SetWindowPos(hcbtn, 0, rect.right, PADDING, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			rect.right -= PADDING;
		} else {
			::SetWindowPos(hcbtn, 0, 0, PADDING, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	auto hcwnd = GetDlgItem(m_hWnd, IDC_EDIT_URL);
	if(hcwnd){
		RECT edt_rect;
		GetWindowRect(hcwnd, &edt_rect);
		ScreenToClientRect(&edt_rect);
		int w = (rect.right - edt_rect.left);
		int h = (edt_rect.bottom - edt_rect.top);
		::SetWindowPos(hcwnd, 0, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}
LRESULT CGrOpenURLDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG , OnInitDialog);
		HANDLE_MSG(hWnd, WM_COMMAND    , OnCommand   );
	case WM_NCHITTEST:
		{
			auto result = DefWindowProc(hWnd, uMsg, wParam, lParam);
			switch(result){
			case HTTOPLEFT    : result = HTLEFT   ; break;
			case HTBOTTOMLEFT : result = HTCAPTION; break;
			case HTTOPRIGHT   : result = HTRIGHT  ; break;
			case HTBOTTOMRIGHT: result = HTCAPTION; break;
			case HTLEFT       : /*               */ break;
			case HTRIGHT      : /*               */ break;
			case HTTOP        : result = HTCAPTION; break;
			case HTBOTTOM     : result = HTCAPTION; break;
			case HTCLIENT     : result = HTCAPTION; break;
			}
			SetWindowLong(hWnd, DWL_MSGRESULT, result);
			return TRUE;
		}
		break;
	case WM_SIZE:
		{
			int cx = static_cast<int>(static_cast<short>(LOWORD(lParam)));
			int cy = static_cast<int>(static_cast<short>(HIWORD(lParam)));
			if(cx > 0 && cy > 0){
				SetWindowSize();
			}
		}
		break;
	case WM_DESTROY:
		EnableWindow(m_hParentWnd, TRUE);
		l_bWaitting = false;
		m_hWnd = NULL;
		break;
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}
void CGrOpenURLDlg::GetURL(std::tstring &url)
{
	url = m_url;
}
BOOL CGrOpenURLDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	TxtMiruTheme_SetWindowSubclass(m_hWnd);
	TxtMiruTheme_UpdateDarkMode(m_hWnd);
	auto hcwnd = GetDlgItem(hwnd, IDC_EDIT_URL);
	if(hcwnd){
		SetWindowText(hcwnd, m_url.c_str());
		m_editBox.Attach(hcwnd);
		if(!m_doModal){
			m_editBox.SetImage(IDR_PNGLOADING);
			::EnableWindow(m_editBox, FALSE);
		}
	}
	if(!m_doModal){
		auto hcbtn = GetDlgItem(m_hWnd, IDOK);
		if(hcbtn){
			ShowWindow(hcbtn, SW_HIDE);
		}
	}
	if(m_hParentWnd){
		RECT rect;
		GetClientRect(m_hParentWnd, &rect);
		ClientToScreenRect(m_hParentWnd, &rect);
		RECT winRect;
		GetWindowRect(hwnd, &winRect);
		int x = rect.left + (rect.right - rect.left)/2 - (winRect.right - winRect.left) / 2;
		int y = rect.top  + (rect.bottom - rect.top)/2 - (winRect.bottom - winRect.top) / 2;
		SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING|SWP_NOSIZE|SWP_NOZORDER);
	}
	SetWindowSize();
	return TRUE;
}
void CGrOpenURLDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDOK    :
		if(m_doModal){
			auto hEditWnd = GetDlgItem(m_hWnd, IDC_EDIT_URL);
			int len = GetWindowTextLength(hEditWnd);
			m_url.resize(len + 1);
			if(IsKeyDown(VK_CONTROL)){
				m_refresh = true;
			} else {
				m_refresh = false;
			}
			GetWindowText(hEditWnd, &m_url[0], len+1);
			::EndDialog(m_hWnd, id);
		} else {
			l_bWaitting = false;
			Close();
		}
		break;
	case IDCANCEL:
		if(m_doModal){
			::EndDialog(m_hWnd, id);
		} else {
			l_bWaitting = false;
			Close();
		}
		break;
	}
}

bool CGrOpenURLDlg::IsRefresh()
{
	return m_refresh;
}
