#pragma warning( disable : 4786 )

#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <process.h>
#include "resource.h"
#include "stltchar.h"
#include "shell.h"
#include "SaveDlg.h"
#include "dlgs.h"
#include "CommDlg.h"
#include "TxtFuncPict.h"
#include "TxtMiruFunc.h"

#define SETSETUP 100
#define SETRANGE 101

CGrSaveDlg::CGrSaveDlg(LPCTSTR filename, int total_page, SIZE paper_size, CGrTxtFuncICanvas *pCanvas)
: m_bCancel(false), m_orgFilename(filename), m_total_page(total_page), m_paperSize(paper_size), m_pCanvas(pCanvas), m_hHandle(NULL)
{
}

CGrSaveDlg::~CGrSaveDlg()
{
}

int CGrSaveDlg::DoModal(HWND hWnd)
{
	return DialogBoxParam(CGrTxtFunc::GetDllModuleHandle(), MAKEINTRESOURCE(IDD_SAVE_PROGRESS), hWnd, reinterpret_cast<DLGPROC>(CGrComonCtrl::WindowMapProc), reinterpret_cast<LPARAM>(this));
}

LRESULT CGrSaveDlg::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		HANDLE_MSG(hWnd, WM_INITDIALOG , OnInitDialog);
		HANDLE_MSG(hWnd, WM_COMMAND    , OnCommand   );
	case WM_DESTROY:
		setCancel();
		break;
	}
	return CGrWinCtrl::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL CGrSaveDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	::SendMessage(GetDlgItem(hwnd, IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, m_total_page));
	::SendMessage(GetDlgItem(hwnd, IDC_PROGRESS), PBM_SETSTEP, 2, 0);
	if(m_hHandle){
		ResumeThread(m_hHandle);
	}
	return TRUE;
}
void CGrSaveDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id){
	case IDOK    :
		setCancel();
		break;
	case IDCANCEL:
		::EndDialog(m_hWnd, id);
		break;
	case SETSETUP:
		::SendMessage(GetDlgItem(hwnd, IDC_PROGRESS), PBM_STEPIT, 0, 0);
		break;
	case SETRANGE:
		::SendMessage(GetDlgItem(hwnd, IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, codeNotify));
		break;
	}
}

bool CGrSaveDlg::isCanceled()
{
	CCriticalSection::CEnterCriticalCtrl ecs(m_cs);
	return m_bCancel;
}
void CGrSaveDlg::setCancel()
{
	CCriticalSection::CEnterCriticalCtrl ecs(m_cs);
	m_bCancel = true;
}

//フックプロシージャ
struct SaveThreadData {
	int w;
	int h;
	int from_page;
	int to_page;
	LPCTSTR baseFileName;
	CGrSaveDlg *pDlg;
	bool bCancel;
};
#define FIX_ASPECT_RATIO   _T("FixAspectRatio"  )
#define SAVE_IMG_FILE_SIZE _T("SaveImgFileSize" )

static UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
	case WM_NOTIFY:
		{
			auto lpof = reinterpret_cast<LPOFNOTIFY>(lParam);
			switch(lpof->hdr.code){
			case CDN_INITDONE:
				{
					auto *lpData = reinterpret_cast<SaveThreadData*>(lpof->lpOFN->lCustData); // lpof->lpOFNが NULLの場合があるので、CDN_INITDONE,CDN_FILEOKの時のみ lpData参照
					::SetWindowLong(hdlg, GWL_USERDATA, reinterpret_cast<LPARAM>(lpData));
					auto hwnd = GetParent(hdlg);
					HWND hchild = NULL;
					if(!hchild){
						hchild = GetDlgItem(hwnd, stc2);
					}
					if(!hchild){
						hchild = GetDlgItem(hwnd, stc3);
					}
					if(hchild){
						int left = 0;
						RECT rect = {0};
						GetWindowRect(hchild, &rect);
						hchild = GetDlgItem(hdlg, IDC_EDIT_PNG_WIDTH);
						MapWindowPoints(HWND_DESKTOP, hdlg, reinterpret_cast<POINT *>(&rect), 2);
						left = rect.left;
						hchild = GetDlgItem(hdlg, 0x45f);
						GetWindowRect(hchild, &rect);
						MapWindowPoints(HWND_DESKTOP, hdlg, reinterpret_cast<POINT *>(&rect), 2);
						for(hchild = GetWindow(hdlg, GW_CHILD); hchild; hchild = GetNextWindow(hchild, GW_HWNDNEXT)){
							GetWindowRect(hchild, &rect);
							MapWindowPoints(HWND_DESKTOP, hdlg, reinterpret_cast<POINT *>(&rect), 2);
							MoveWindow(hchild, rect.left + left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
						}
					}
					int size[2] = {1024,721};
					CGrTxtFunc::GetPointsINI(SAVE_IMG_FILE_SIZE, size, sizeof(size)/sizeof(int));
					SetDlgItemInt(hdlg, IDC_EDIT_PNG_WIDTH , size[0], true);
					SetDlgItemInt(hdlg, IDC_EDIT_PNG_HEIGHT, size[1], true);
					int iFixAspectRatio = 1;
					CGrTxtFunc::GetPointsINI(FIX_ASPECT_RATIO, &iFixAspectRatio, sizeof(iFixAspectRatio)/sizeof(int));
					Button_SetCheck(GetDlgItem(hdlg, IDC_FIX_ASPECT_RATIO), (iFixAspectRatio == 1) ? BST_CHECKED : BST_UNCHECKED);
					SetDlgItemInt(hdlg, IDC_EDIT_BEGIN_PAGE,                         0, true);
					SetDlgItemInt(hdlg, IDC_EDIT_END_PAGE  , lpData->pDlg->TotalPage(), true);
					SetDlgItemInt(hdlg, IDC_MAX_PAGE       , lpData->pDlg->TotalPage(), true);
				}
				break;
			case CDN_FILEOK:
				{
					auto *lpData = reinterpret_cast<SaveThreadData*>(lpof->lpOFN->lCustData);
					if(lpData){
						BOOL bTranslated;
						lpData->w         = GetDlgItemInt(hdlg, IDC_EDIT_PNG_WIDTH , &bTranslated, TRUE);
						lpData->h         = GetDlgItemInt(hdlg, IDC_EDIT_PNG_HEIGHT, &bTranslated, TRUE);
						lpData->from_page = GetDlgItemInt(hdlg, IDC_EDIT_BEGIN_PAGE, &bTranslated, TRUE);
						lpData->to_page   = GetDlgItemInt(hdlg, IDC_EDIT_END_PAGE  , &bTranslated, TRUE);
						if(lpData->w <= 0 || lpData->h <= 0){
							std::tstring mes;
							CGrText::LoadString(IDS_ERROR_IMG_FILE_SIZE, mes);
							std::tstring title;
							CGrText::LoadString(IDS_MES_CONF, title);
							MessageBox(GetParent(hdlg), mes.c_str(), title.c_str(), MB_OK);
							SetWindowLong(hdlg, DWL_MSGRESULT, -1);
							break;
						}
						int size[] = {lpData->w, lpData->h};
						CGrTxtFunc::SetPointsINI(SAVE_IMG_FILE_SIZE, size, sizeof(size)/sizeof(int));
						int iFixAspectRatio = IsDlgButtonChecked(hdlg, IDC_FIX_ASPECT_RATIO);
						CGrTxtFunc::SetPointsINI(FIX_ASPECT_RATIO, &iFixAspectRatio, sizeof(iFixAspectRatio)/sizeof(int));
					}
				}
				break;
			}
		}
		return TRUE;
		break;
	case WM_COMMAND:
		{
			BOOL bTranslated;
			static bool bEditing = false;
			auto *lpData = reinterpret_cast<SaveThreadData*>(::GetWindowLong(hdlg, GWL_USERDATA));
			if(lpData){
				UINT codeNotify = HIWORD(wParam);
				switch(LOWORD(wParam)){
				case IDC_EDIT_PNG_WIDTH:
					if(codeNotify == EN_CHANGE && !bEditing){
						bEditing = true;
						if(IsDlgButtonChecked(hdlg, IDC_FIX_ASPECT_RATIO)){
							const auto &paperSize = lpData->pDlg->PaperSize();
							if(paperSize.cx > 0 && paperSize.cy > 0){
								int w = GetDlgItemInt(hdlg, IDC_EDIT_PNG_WIDTH, &bTranslated, TRUE);
								int h = w * paperSize.cy / paperSize.cx;
								SetDlgItemInt(hdlg, IDC_EDIT_PNG_HEIGHT, h, true);
							}
						}
						bEditing = false;
					}
					break;
				case IDC_EDIT_PNG_HEIGHT:
					if(codeNotify == EN_CHANGE && !bEditing){
						bEditing = true;
						if(IsDlgButtonChecked(hdlg, IDC_FIX_ASPECT_RATIO)){
							const auto &paperSize = lpData->pDlg->PaperSize();
							if(paperSize.cx > 0 && paperSize.cy > 0){
								int h = GetDlgItemInt(hdlg, IDC_EDIT_PNG_HEIGHT, &bTranslated, TRUE);
								int w = h * paperSize.cx / paperSize.cy;
								SetDlgItemInt(hdlg, IDC_EDIT_PNG_WIDTH, w, true);
							}
						}
						bEditing = false;
					}
					break;
				case IDC_FIX_ASPECT_RATIO:
					if(IsDlgButtonChecked(hdlg, IDC_FIX_ASPECT_RATIO) && !bEditing){
						bEditing = true;
						const auto &paperSize = lpData->pDlg->PaperSize();
						if(paperSize.cx > 0 && paperSize.cy > 0){
							int w = GetDlgItemInt(hdlg, IDC_EDIT_PNG_WIDTH, &bTranslated, TRUE);
							int h = w * paperSize.cy / paperSize.cx;
							SetDlgItemInt(hdlg, IDC_EDIT_PNG_HEIGHT, h, true);
						}
						bEditing = false;
					}
					break;
				}
			}
		}
		break;
	}
	return FALSE;
}

#include "png.h"

BOOL SaveBitmapAsPngFile(LPCTSTR pszFileName, CGrBitmap &bmp)
{
	BOOL bRet = TRUE;
	HANDLE hFile = NULL;
	png_structp png = nullptr;
	png_infop info = nullptr;
	png_bytepp lines = nullptr;
	do {
		hFile = ::CreateFile(pszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE){
			bRet = FALSE;
			break;
		}
		png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if(!png){
			bRet = FALSE;
			break;
		}
		info = png_create_info_struct(png);
		if(!info){
			bRet = FALSE;
			break;
		}
		if(setjmp(png_jmpbuf(png))){
			bRet = FALSE;
			break;
		}
		png_set_write_fn(png, reinterpret_cast<png_voidp>(hFile),
						 //PNGファイル書き込み関数
						 [](png_structp png_ptr, png_bytep buf, png_size_t size) {
							 auto hFile = reinterpret_cast<HANDLE>(png_get_io_ptr(png_ptr));
							 DWORD dw;
							 ::WriteFile(hFile, buf, size, &dw, nullptr);
						 },
						 //PNGファイルフラッシュ関数
						 [](png_structp png_ptr) {}
						 );
		png_set_IHDR(png, info, bmp.Width(), bmp.Height(), 8,
					 PNG_COLOR_TYPE_RGB_ALPHA,
					 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		png_write_info(png, info);
		png_set_bgr(png);
		png_set_invert_alpha(png);

		auto pbBits = bmp.GetBits();
		lines = new png_bytep[bmp.Height()];
		auto lpSrc = lines;
		int dwWidthBytes = bmp.Width();
		for(auto i=bmp.Height(); i>0; --i, ++lpSrc, pbBits+=dwWidthBytes){
			*lpSrc = reinterpret_cast<png_bytep>(pbBits);
		}
		png_write_image(png, lines);
		png_write_end(png, info);
	} while(0);

	if(lines){
		delete [] lines;
	}
	if(png && info){
		png_destroy_write_struct(&png, &info);
	}
	if(hFile){
		::CloseHandle(hFile);
	}

	return bRet;
}

unsigned int __stdcall CGrSaveDlg::save_png_file(void *arg)
{
	auto *lpData = reinterpret_cast<SaveThreadData *>(arg);

	int total_page = lpData->pDlg->m_total_page;
	auto lpBaseFileName = lpData->baseFileName;
	int count = 0;
	for(int page = total_page/2; page>0; page/=10){
		++count;
	}
	auto *pCanvas = lpData->pDlg->m_pCanvas;
	int width     = lpData->w        ;
	int height    = lpData->h        ;
	int from_page = lpData->from_page / 2 * 2;
	int to_page   = lpData->to_page   / 2 * 2 + 1;
	if(to_page < 0){
		to_page = total_page;
	}
	if(to_page < from_page){
		int page  = to_page  ;
		to_page   = from_page;
		from_page = page     ;
	}
	if(from_page < 0){
		from_page = 0;
	}
	if(to_page > total_page){
		to_page = total_page;
	}
	if(lpData->pDlg && lpData->pDlg->GetWnd()){
		FORWARD_WM_COMMAND(lpData->pDlg->GetWnd(), SETRANGE/*id*/, NULL/*hwndCtl*/, to_page-from_page+1/*codeNotify*/, SendMessage);
	}
	for(int page=from_page; page < to_page; page += 2){
		TCHAR buf[_MAX_PATH] = {};
		_stprintf_s(buf, _T("%s_%0*d.png"), lpBaseFileName, count, page/2 + 1);
		if(lpData->pDlg->isCanceled()){
			break;
		}
		CGrBitmap *pBmp;
		pCanvas->GetBitmap(page, width, height, &pBmp);
		if(lpData->pDlg->isCanceled()){
			break;
		}
		if(!SaveBitmapAsPngFile(buf, *pBmp)){
			break;
		}
		if(lpData->pDlg && lpData->pDlg->GetWnd()){
			FORWARD_WM_COMMAND(lpData->pDlg->GetWnd(), SETSETUP/*id*/, NULL/*hwndCtl*/, 0/*codeNotify*/, SendMessage);
		}
	}
	if(lpData->pDlg && lpData->pDlg->GetWnd()){
		FORWARD_WM_COMMAND(lpData->pDlg->GetWnd(), IDCANCEL/*id*/, NULL/*hwndCtl*/, 0/*codeNotify*/, SendMessage);
	}
	return 0;
}

bool CGrSaveDlg::Save(HWND hWnd)
{
	TCHAR curdir[_MAX_PATH] = {};
	auto pos = m_orgFilename.find(_T("|"));
	if(pos != std::tstring::npos){
		m_orgFilename.resize(pos);
	}
	TCHAR filename[_MAX_PATH] = {};
	if(!CGrShell::IsURI(m_orgFilename.c_str())){
		_tcscpy_s(filename, m_orgFilename.c_str());
		auto lpchr = filename;
		for(; *lpchr; ++lpchr){
			if(*lpchr == _T('/')){
				*lpchr = _T('\\');
			}
		}
		auto *lpExt = CGrShell::GetFileExt(filename);
		if(lpExt){
			*(lpExt-1) = '\0';
		}
	}

	TCHAR filter[2048] = {};
	CGrText::LoadString(IDS_SAVEIMGFILE, filter, sizeof(filter)/sizeof(TCHAR));
	std::tstring str;
	CGrText::LoadString(IDS_SAVE_BASEFILENAME, str);

	SaveThreadData stdata = {};
	stdata.pDlg = this;
	OPENFILENAME of = {sizeof(OPENFILENAME)};
	of.hwndOwner       = hWnd;
	of.hInstance       = CGrTxtFunc::GetDllModuleHandle();
	of.lpstrFilter     = filter;
	of.nMaxCustFilter  = 40;
	of.lpstrTitle      = str.c_str();
	of.lpstrFile       = filename;
	of.nMaxFile        = sizeof(filename)/sizeof(TCHAR) - 1;
	of.lpstrInitialDir = _T(".\\");
	of.lCustData       = reinterpret_cast<LPARAM>(&stdata);
	of.Flags           = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
	of.lpfnHook        = OFNHookProc;
	of.lpTemplateName  = MAKEINTRESOURCE(IDD_PNG_PARAM);
	if(::GetSaveFileName(&of)){
		auto *lpExt = CGrShell::GetFileExt(filename);
		if(lpExt){
			*(lpExt-1) = '\0';
		}
		m_baseFilename = filename;
		stdata.baseFileName = m_baseFilename.c_str();
		auto hHandle = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, save_png_file, reinterpret_cast<void*>(&stdata), CREATE_SUSPENDED, NULL));
		if (hHandle) {
			m_hHandle = hHandle;
			DoModal(hWnd);
			::WaitForSingleObject(hHandle, INFINITE);
			::CloseHandle(hHandle);
			return true;
		}
	}
	return false;
}
