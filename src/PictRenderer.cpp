#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "PictRenderer.h"
#include <olectl.h>
#include "Lanczos.h"
#include "Shell.h"
#include "Image.h"
#include "Win32Wrap.h"

//#define __DBG__
#include "Debug.h"

CGrPictOleRenderer::CGrPictOleRenderer()
{
}

CGrPictOleRenderer::~CGrPictOleRenderer()
{
}

bool CGrPictOleRenderer::IsSupported(LPCTSTR fileName)
{
	return Open(fileName);
}

void CGrPictOleRenderer::Draw(HDC hdc, int x, int y, int w, int h, LPCTSTR fileName)
{
	CGrBitmap bmp;
	if(!Open(fileName, &bmp)){
		return;
	}
	int iWidth  = bmp.Width();
	int iHeight = bmp.Height();
	x += w / 2 - iWidth  / 2;
	y += h / 2 - iHeight / 2;
	auto bitmapDC = ::CreateCompatibleDC(hdc);
	auto hOldBitmap = (HBITMAP)::SelectObject(bitmapDC, bmp);
	::BitBlt(hdc, x, y, iWidth, iHeight, bitmapDC, 0, 0, SRCCOPY);
	::SelectObject(bitmapDC, hOldBitmap);
	::DeleteDC(bitmapDC);
}


static void stretchBlt(CGrBitmap &canvas, CGrBitmap &bmp, int x, int y, int w, int h, double scale_x, double scale_y)
{
	CGrBitmap dst_bmp;
	CGrLanczos::StretchBlt(dst_bmp, bmp, 3, scale_x, scale_y);
	CGrImage::CopyBitmapRect(canvas, x, y, w, h, dst_bmp, 0, 0);
}

void CGrPictOleRenderer::Draw(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName)
{
	CGrBitmap bmp;
	if(!Open(fileName, &bmp)){
		return;
	}
	int iWidth  = bmp.Width();
	int iHeight = bmp.Height();

	double scale_x = static_cast<double>(w) / static_cast<double>(iWidth );
	double scale_y = static_cast<double>(h) / static_cast<double>(iHeight);
	double scale = min(scale_y, scale_x);

	int iToWidth  = static_cast<int>(scale * iWidth );
	int iToHeight = static_cast<int>(scale * iHeight);

	HBITMAP hbmp = canvas;
	if(!hbmp){
		canvas.Create(iToWidth, iToHeight);
	}
	::stretchBlt(canvas, bmp, x, y, iToWidth, iToHeight, scale, scale);
}

void CGrPictOleRenderer::StretchBlt(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName)
{
	CGrBitmap bmp;
	if(!Open(fileName, &bmp)){
		return;
	}
	int iWidth  = bmp.Width();
	int iHeight = bmp.Height();

	double scale_x = static_cast<double>(w) / static_cast<double>(iWidth );
	double scale_y = static_cast<double>(h) / static_cast<double>(iHeight);

	::stretchBlt(canvas, bmp, x, y, w, h, scale_x, scale_y);
}

void CGrPictOleRenderer::GetFileTypeList(CGrPictRenderer::PictFileTypeList &pftl)
{
	PictFileType pft;
	pft.ext  = _T("*.jpg;*.jpeg");
	pft.name = _T("Jpeg File");
	pftl.push_back(pft);
	pft.ext  = _T("*.png");
	pft.name = _T("PNG File");
	pftl.push_back(pft);
}

#include "Image.h"

#define  HIMETRIC_INCH  2540// = 1[inch]/0.01[mm]
bool CGrPictOleRenderer::Open(LPCTSTR lpFileName, CGrBitmap *lpBmp)
{
	bool bret = false;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HGLOBAL hGlobal = nullptr;
	do {
		// png, Jpegファイルをメモリにロード
		TCHAR cur_dir[512];
		TCHAR tmp_filename[512];
		TCHAR *lpfilepart;
		::GetCurrentDirectory(sizeof(cur_dir)/sizeof(TCHAR), cur_dir);
		::SetCurrentDirectory(m_curDir.c_str());
		::GetFullPathName(lpFileName, sizeof(tmp_filename)/sizeof(TCHAR), tmp_filename, &lpfilepart);
		::SetCurrentDirectory(cur_dir);

		hFile = ::CreateFile(tmp_filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if(hFile == INVALID_HANDLE_VALUE){
			break;
		}
		auto dwFileSize = ::GetFileSize(hFile, nullptr);
		if(dwFileSize <= 0){
			break;
		}
		hGlobal = ::GlobalAlloc(GPTR, dwFileSize);
		if(!hGlobal){
			break;
		}
		bool bPng = false;
		DWORD dwReadByte = 0;
		{
			auto lpByte = static_cast<LPBYTE>(GlobalLock(hGlobal));
			if (::ReadFile(hFile, lpByte, dwFileSize, &dwReadByte, nullptr)) {
				if (dwFileSize > 8
					&& lpByte[0] == 0x89
					&& lpByte[1] == 0x50
					&& lpByte[2] == 0x4E
					&& lpByte[3] == 0x47
					&& lpByte[4] == 0x0D
					&& lpByte[5] == 0x0A
					&& lpByte[6] == 0x1A
					&& lpByte[7] == 0x0A) {
					bPng = true;
				}
			}
			::GlobalUnlock(hGlobal);
		}
		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
		if(bPng){
			bret = true;
			if(lpBmp){
				auto lpByte = static_cast<LPBYTE>(GlobalLock(hGlobal));
				bret = CGrImage::LoadPngFromMemory(*lpBmp, lpByte ,dwFileSize);
				::GlobalUnlock(hGlobal);
			}
		} else {
			// データストリームの作成
			IStream *iStream;
			IPicture *iPicture;
			if (::CreateStreamOnHGlobal(hGlobal, FALSE, &iStream) != S_OK) {
				break;
			}
			if(::OleLoadPicture(iStream, dwFileSize, TRUE, IID_IPicture, reinterpret_cast<LPVOID*>(&iPicture)) != S_OK || !iPicture){
				iStream->Release();
				break;
			}
			iStream->Release();
			if(lpBmp){
				// Bitmapへの描画
				OLE_XSIZE_HIMETRIC lWidth;
				OLE_YSIZE_HIMETRIC lHeight;
				iPicture->get_Width (&lWidth );
				iPicture->get_Height(&lHeight);
				auto hdc = ::CreateCompatibleDC(nullptr);
				long iWidth  = MulDiv(lWidth , GetDeviceCaps(hdc, LOGPIXELSX), HIMETRIC_INCH);
				long iHeight = MulDiv(lHeight, GetDeviceCaps(hdc, LOGPIXELSX), HIMETRIC_INCH);
				lpBmp->Create(iWidth, iHeight);

				bool bCompare = false;
				{
					DWORD dwAttr = 0;
					iPicture->get_Attributes(&dwAttr);
					if(dwAttr & PICTURE_TRANSPARENT){
						bCompare = true;
					}
					if(!bCompare){
						HBITMAP hBitmap;
						if(S_OK == iPicture->get_Handle(reinterpret_cast<OLE_HANDLE FAR *>(&hBitmap))){
							BITMAP bmBitmap;
							GetObject(hBitmap, sizeof (BITMAP), &bmBitmap);
							if(bmBitmap.bmBitsPixel < 32){
								bCompare = true;
							}
						}
					}
				}
				RECT rc = {0,0,lWidth,lHeight};
				auto hOldBitmap = static_cast<HBITMAP>(::SelectObject(hdc, *lpBmp));
				if(bCompare){
					auto oldclr = SetBkColor(hdc, RGB(0xff,0xff,0xff));
					::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr);
					SetBkColor(hdc, oldclr);
				}
				RECT rect_tmp = {};
				iPicture->Render(hdc, 0, 0, iWidth, iHeight, 0, lHeight, lWidth, -lHeight, &rect_tmp);
				CGrBitmap tmp_bmp;
				if(bCompare){
					tmp_bmp.Create(iWidth, iHeight);
					::SelectObject(hdc, tmp_bmp);
					auto oldclr = SetBkColor(hdc, RGB(0x00,0x00,0x00));
					::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr);
					SetBkColor(hdc, oldclr);
					iPicture->Render(hdc, 0, 0, iWidth, iHeight, 0, lHeight, lWidth, -lHeight, &rect_tmp);
				}
				::SelectObject(hdc, hOldBitmap);
				::DeleteDC(hdc);
				if(bCompare){
					// アルファ値の設定
					CGrImage::SetAlphaBits(&tmp_bmp, lpBmp);
				}
				bret = true;
			} else {
				OLE_XSIZE_HIMETRIC lWidth;
				OLE_YSIZE_HIMETRIC lHeight;
				iPicture->get_Width (&lWidth );
				iPicture->get_Height(&lHeight);

				if(lWidth > 0 && lHeight > 0){
					bret = true;
				}
			}
			iPicture->Release();
		}
	} while(0);

	if(hGlobal){
		::GlobalFree(hGlobal);
	}
	if(hFile != INVALID_HANDLE_VALUE){
		::CloseHandle(hFile);
	}
	return bret;
}

bool CGrPictOleRenderer::SetParam(LPCTSTR name, LPCTSTR val)
{
	if(name && _tcscmp(name, _T("CurDir")) == 0){
		m_curDir = val;
		return true;
	} else {
		return false;
	}
}

bool CGrPictOleRenderer::GetParam(LPCTSTR name, LPTSTR val, int len)
{
	if(name && _tcscmp(name, _T("CurDir")) == 0){
		if(m_curDir.empty()){
			::GetCurrentDirectory(len, val);
		} else {
			_stprintf_s(val, len, _T("%s"), m_curDir.c_str());
		}
		return true;
	} else {
		return false;
	}
}

void CGrPictOleRenderer::Clear()
{
}

////////////////////
class CGrSPIFile {

	int (PASCAL *funcGetPluginInfo)(int, LPSTR, int) = nullptr;
	int (PASCAL *funcIsSupported)(LPSTR, DWORD) = nullptr;
	int (PASCAL *funcGetPicture)(LPSTR, long, unsigned int, HLOCAL*, HLOCAL*, FARPROC, long) = nullptr;
	HMODULE m_hDLL = nullptr;
private:
	CGrSPIFile()
	{

	}
	void Free()
	{
		if(m_hDLL){
			::FreeLibrary(m_hDLL);
			m_hDLL = nullptr;
		}
		funcGetPluginInfo = nullptr;
		funcIsSupported = nullptr;
		funcGetPicture = nullptr;
	}
	bool load(LPCTSTR filename)
	{
		do {
			m_hDLL = ::LoadLibrary(filename);
			if(!m_hDLL){
				break;
			}
			SetProcAddressPtr(funcGetPluginInfo, m_hDLL, "GetPluginInfo");
			if(!funcGetPluginInfo){ break; }
			SetProcAddressPtr(funcIsSupported, m_hDLL, "IsSupported");
			if(!funcIsSupported){ break; }
			SetProcAddressPtr(funcGetPicture, m_hDLL, "GetPicture");
			if(!funcGetPicture){ break; }
			return true;
		} while(0);
		Free();
		return false;
	}
	void w2c(LPCTSTR w, std::string &c)
	{
		size_t wLen = 0;
		c.resize(_tcslen(w) * 3);
		if(!c.empty()){
			auto err = wcstombs_s(&wLen, &c[0], c.size()-1, w, _TRUNCATE);
		}
	}
	void c2w(LPCSTR c, std::tstring &w)
	{
		size_t cLen = 0;
		w.resize(strlen(c) * 3);
		if(!w.empty()){
			auto err =  mbstowcs_s(&cLen, &w[0], w.size()-1, c, _TRUNCATE);
		}
	}
public:
	virtual ~CGrSPIFile()
	{
		Free();
	}

	void GetFilters(CGrPictRenderer::PictFileTypeList &pftl)
	{
		if(!funcGetPluginInfo){ return; }
		char ext[512];
		for(int no = 0;
			0 != funcGetPluginInfo(2*no+2, ext, sizeof(ext)/sizeof(char)); ++no){
			char name[512];
			if(0 == funcGetPluginInfo(2*no+3, name, sizeof(name)/sizeof(char))){
				break;
			}
			CGrPictRenderer::PictFileType pft;
			c2w(ext , pft.ext );
			c2w(name, pft.name);
			pftl.push_back(pft);
		}
	}
	bool IsSupported(LPCTSTR filename, DWORD dw)
	{
		if(!funcIsSupported){ return false; }
		std::string f;
		w2c(filename, f);
		return funcIsSupported(const_cast<LPSTR>(f.c_str()), dw) != 0;
	}
	// Case9: fnProgressCallback は NULL ではなく、ダミー関数を渡した方がより安全
	// http://home.netyou.jp/cc/susumu/progSusie.html
	static int PASCAL __progressCallback(int nNum, int nDenom, long lData)
	{
		return 0; // always continue
	}
	bool GetPicture(LPCTSTR filename, CGrBitmap &bmp)
	{
		if(!funcGetPicture){ return false; }
		std::string f;
		w2c(filename, f);
		HANDLE hBitmapInfo;                   // LPBITMAPINFOのためのハンドル
		HANDLE hBitmapBitData;                // LPSTR:BitmapのBitデータのためのハンドル
		if(0 != funcGetPicture(const_cast<LPSTR>(f.c_str()),0,0,&hBitmapInfo,&hBitmapBitData,reinterpret_cast<FARPROC>(__progressCallback),NULL)){
			return false;
		}
		auto lpBmpInfo = static_cast<LPBITMAPINFO>(::LocalLock(hBitmapInfo));
		if (!lpBmpInfo) {
			return false;
		}
		auto lpData    = static_cast<LPBYTE>(::LocalLock(hBitmapBitData));
		if (!lpData) {
			::LocalUnlock(lpBmpInfo);
			return false;
		}
		bmp.Create(lpBmpInfo->bmiHeader.biWidth, lpBmpInfo->bmiHeader.biHeight);
		::SetDIBits(nullptr, bmp, 0, lpBmpInfo->bmiHeader.biHeight, lpData, lpBmpInfo, DIB_RGB_COLORS);
		if (lpBmpInfo->bmiHeader.biBitCount < 32) {
			// アルファ値の設定
			CGrImage::SetAlphaBits(nullptr, &bmp);
		}
		::LocalUnlock(lpBmpInfo);
		::LocalUnlock(lpData   );
		::LocalFree(lpBmpInfo);
		::LocalFree(lpData   );
		return true;
	}
public:
	static CGrSPIFile* Load(LPCTSTR fileName){
		auto *pspi = new CGrSPIFile();
		if(!pspi->load(fileName)){
			delete pspi;
			return nullptr;
		}
		return pspi;
	}
};


CGrPictSPIRenderer::CGrPictSPIRenderer() : m_bLoadPlugin(false)
{
}
CGrPictSPIRenderer::~CGrPictSPIRenderer()
{
	Clear();
}

//
bool CGrPictSPIRenderer::loadPlugin()
{
	TCHAR curpath[512];
	::GetCurrentDirectory(sizeof(curpath)/sizeof(TCHAR), curpath);

	TCHAR filepath[512];
	GetParam(_T("PluginDir"), filepath, sizeof(filepath)/sizeof(TCHAR));
	::SetCurrentDirectory(m_pluginDir.c_str());

	WIN32_FIND_DATA wfd;
	auto hFind= ::FindFirstFile(_T("*.SPI"), &wfd);
	if(hFind == INVALID_HANDLE_VALUE){
		::SetCurrentDirectory(curpath);
		return false;
	}
	while(1){
		if((wfd.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_TEMPORARY)) != 0){
			continue;
		}
		m_plugins.push_back(CGrSPIFile::Load(wfd.cFileName));
		if(!::FindNextFile(hFind, &wfd)){ break; }
	};
	::FindClose(hFind);
	m_bLoadPlugin = true;
	::SetCurrentDirectory(curpath);
	return true;
}

CGrSPIFile *CGrPictSPIRenderer::getSPI(LPCTSTR fileName)
{
	if(!m_bLoadPlugin){
		if(!loadPlugin()){
			return nullptr;
		}
	}
	{
		auto it = m_files.find(fileName);
		if(m_files.end() != it){
			CGrSPIFile *pspi = it->second;
			return pspi;
		}
	}
	m_files[fileName] = nullptr;
	char header[2000];
	auto hFile = ::CreateFile(fileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
	if(hFile == INVALID_HANDLE_VALUE){
		return nullptr;
	}
	DWORD dwCanReadSize;
	//実際のファイルサイズが2000Byte以下の場合はそのファイルサイズのみをReadするように設定
	if (::ReadFile(hFile, header, 2000, &dwCanReadSize, 0)/*2.0.39.0*/) {
		for (auto& pspi : m_plugins) {
			if (pspi) {
				::SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
				auto dw = reinterpret_cast<DWORD>(hFile);
				if (pspi->IsSupported(fileName, dw)) {
					m_files[fileName] = pspi;
					break;
				}
				else if (pspi->IsSupported(fileName, reinterpret_cast<DWORD>(header))) {
					m_files[fileName] = pspi;
					break;
				}
			}
		}
	}
	::CloseHandle(hFile);
	return m_files[fileName];
}

void CGrPictSPIRenderer::GetFileTypeList(CGrPictRenderer::PictFileTypeList &pftl)
{
	if(!m_bLoadPlugin){
		if(!loadPlugin()){
			return;
		}
	}
	for(auto &pspi : m_plugins){
		if(pspi){
			pspi->GetFilters(pftl);
		}
	}
}

bool CGrPictSPIRenderer::IsSupported(LPCTSTR fileName)
{
	return getSPI(fileName) != nullptr;
}

void CGrPictSPIRenderer::Draw(HDC hdc, int x, int y, int w, int h, LPCTSTR fileName)
{
	auto *pspi = getSPI(fileName);
	if(!pspi){ return; }
	CGrBitmap bmp;
	if(!pspi->GetPicture(fileName, bmp)){ return; }
}

void CGrPictSPIRenderer::Draw(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName)
{
	auto *pspi = getSPI(fileName);
	if(!pspi){ return; }
	CGrBitmap bmp;
	if(!pspi->GetPicture(fileName, bmp)){ return; }
	int iWidth  = bmp.Width();
	int iHeight = bmp.Height();

	double scale_x = static_cast<double>(w) / static_cast<double>(iWidth );
	double scale_y = static_cast<double>(h) / static_cast<double>(iHeight);
	double scale = min(scale_y, scale_x);

	int iToWidth  = (int)(scale * iWidth );
	int iToHeight = (int)(scale * iHeight);

	HBITMAP hbmp = canvas;
	if(!hbmp){
		canvas.Create(iToWidth, iToHeight);
	}
	::stretchBlt(canvas, bmp, x, y, iToWidth, iToHeight, scale, scale);
}

void CGrPictSPIRenderer::StretchBlt(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName)
{
	auto *pspi = getSPI(fileName);
	if(!pspi){ return; }
	CGrBitmap bmp;
	if(!pspi->GetPicture(fileName, bmp)){ return; }
	int iWidth  = bmp.Width();
	int iHeight = bmp.Height();

	double scale_x = static_cast<double>(w) / static_cast<double>(iWidth );
	double scale_y = static_cast<double>(h) / static_cast<double>(iHeight);

	::stretchBlt(canvas, bmp, x, y, w, h, scale_x, scale_y);
}

void CGrPictSPIRenderer::Clear()
{
	for(auto &pspi : m_plugins){
		if(pspi){
			delete pspi;
		}
	}
	m_plugins.clear();
	m_plugins.shrink_to_fit();
	m_files.clear();
	m_bLoadPlugin = false;
}

bool CGrPictSPIRenderer::SetParam(LPCTSTR name, LPCTSTR val)
{
	if(!name){
		return false;
	} else if(_tcscmp(name, _T("CurDir")) == 0){
		m_curDir = val;
		return true;
	} else if(_tcscmp(name, _T("PluginDir")) == 0){
		m_pluginDir = val;
		return true;
	} else {
		return false;
	}
}

bool CGrPictSPIRenderer::GetParam(LPCTSTR name, LPTSTR val, int len)
{
	if(!name){
		return false;
	} else if(_tcscmp(name, _T("CurDir")) == 0){
		if(m_curDir.empty()){
			::GetCurrentDirectory(len, val);
		} else {
			_stprintf_s(val, len, _T("%s"), m_curDir.c_str());
		}
		return true;
	} else if(_tcscmp(name, _T("PluginDir")) == 0){
		if(m_pluginDir.empty()){
			TCHAR filepath[MAX_PATH];
			CGrShell::GetExePath(filepath);
			m_pluginDir = filepath;
		}
		_stprintf_s(val, len, _T("%s"), m_pluginDir.c_str());
		return true;
	} else {
		return false;
	}
}
//
CGrPictEmfRenderer::CGrPictEmfRenderer()
{
}
CGrPictEmfRenderer::~CGrPictEmfRenderer()
{
	Clear();
}
bool CGrPictEmfRenderer::IsSupported(LPCTSTR fileName)
{
	auto lpExt = CGrShell::GetFileExt((TCHAR*)fileName);
	return (lpExt && _tcsicmp(lpExt, _T("emf")) == 0);
}
void CGrPictEmfRenderer::Draw(HDC hdc, int x, int y, int w, int h, LPCTSTR fileName)
{
	if(!Open(fileName)){
		return;
	}
	int size = m_emfData.size();
	if(size > 0){
		auto hemf = ::SetEnhMetaFileBits(size, &m_emfData[0]);
		HPALETTE hOldPal = NULL;
		if(m_hPal){
			hOldPal = ::SelectPalette(hdc, m_hPal, FALSE);
			::RealizePalette(hdc);
		}
		double scale_x = static_cast<double>(w) / static_cast<double>(m_iWidth );
		double scale_y = static_cast<double>(h) / static_cast<double>(m_iHeight);
		double scale = min(scale_y, scale_x);
		int iWidth  = static_cast<int>(scale * m_iWidth );
		int iHeight = static_cast<int>(scale * m_iHeight);
		x += w / 2 - iWidth  / 2;
		y += h / 2 - iHeight / 2;
		RECT rect = {x,y,x+w,y+h};
		::PlayEnhMetaFile(hdc, hemf , &rect);
		::DeleteEnhMetaFile(hemf);
		if(m_hPal){
			::SelectPalette(hdc, hOldPal, FALSE);
			::RealizePalette(hdc);
		}
	}
}
void CGrPictEmfRenderer::Draw(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName)
{
	CGrBitmap bmp;
	int iWidth  = w*4;//bmp.Width();
	int iHeight = h*4;//bmp.Height();

	bmp.Create(iWidth, iHeight);
	int l = iWidth * iHeight;
	auto lprgbquad = bmp.GetBits();
	for(l = iWidth * iHeight; l > 0; --l, ++lprgbquad){
		lprgbquad->rgbRed      = 255;
		lprgbquad->rgbGreen    = 255;
		lprgbquad->rgbBlue     = 255;
		lprgbquad->rgbReserved = 255;
	}

	auto bitmapDC = ::CreateCompatibleDC(NULL);
	auto hOldBitmap = static_cast<HBITMAP>(::SelectObject(bitmapDC, bmp));
	Draw(bitmapDC, 0, 0, iWidth, iHeight, fileName);
	::SelectObject(bitmapDC, hOldBitmap);
	::DeleteDC(bitmapDC);
	lprgbquad = bmp.GetBits();
	for(l = iWidth * iHeight; l > 0; --l, ++lprgbquad){
		lprgbquad->rgbReserved = 255 - lprgbquad->rgbReserved;
	}
	::stretchBlt(canvas, bmp, x, y, w, h, 0.5/2, 0.5/2);
}
void CGrPictEmfRenderer::StretchBlt(CGrBitmap &canvas, int x, int y, int w, int h, LPCTSTR fileName)
{
	CGrBitmap bmp;
	int iWidth  = w*2;//bmp.Width();
	int iHeight = h*2;//bmp.Height();

	bmp.Create(iWidth, iHeight);

	auto bitmapDC = ::CreateCompatibleDC(NULL);
	auto hOldBitmap = static_cast<HBITMAP>(::SelectObject(bitmapDC, bmp));
	Draw(bitmapDC, 0, 0, iWidth, iHeight, fileName);
	::SelectObject(bitmapDC, hOldBitmap);
	::DeleteDC(bitmapDC);
	::stretchBlt(canvas, bmp, x, y, w, h, 0.5, 0.5);
}
void CGrPictEmfRenderer::Clear()
{
	m_emfData.resize(0);
	if(m_hPal){
		::DeleteObject(m_hPal);
		m_hPal = NULL;
	}
}
void CGrPictEmfRenderer::GetFileTypeList(PictFileTypeList &pftl)
{
	PictFileType pft;
	pft.ext  = _T("*.emf");
	pft.name = _T("EMF File");
	pftl.push_back(pft);
}

bool CGrPictEmfRenderer::SetParam(LPCTSTR name, LPCTSTR val)
{
	if(name && _tcscmp(name, _T("CurDir")) == 0){
		m_curDir = val;
		return true;
	} else {
		return false;
	}
}
bool CGrPictEmfRenderer::GetParam(LPCTSTR name, LPTSTR val, int len)
{
	if(name && _tcscmp(name, _T("CurDir")) == 0){
		if(m_curDir.empty()){
			::GetCurrentDirectory(len, val);
		} else {
			_stprintf_s(val, len, _T("%s"), m_curDir.c_str());
		}
		return true;
	} else {
		return false;
	}
}
bool CGrPictEmfRenderer::Open(LPCTSTR fileName)
{
	Clear();
	auto hemf = GetEnhMetaFile(fileName);
	if(hemf){
		ENHMETAHEADER hdr = {0};
		::GetEnhMetaFileHeader(hemf, sizeof(ENHMETAHEADER), &hdr);
		m_iWidth = hdr.rclBounds.right-hdr.rclBounds.left;
		m_iHeight= hdr.rclBounds.bottom-hdr.rclBounds.top;

		UINT cbBuffer = ::GetEnhMetaFileBits(hemf, 0, NULL);
		m_emfData.resize(cbBuffer+1);
		::GetEnhMetaFileBits(hemf, cbBuffer, &m_emfData[0]);
		int num = ::GetEnhMetaFilePaletteEntries(hemf, 0, NULL);
		if(num > 0){
			std::simple_array<BYTE> pal_data;
			pal_data.resize(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*num);
			LPLOGPALETTE   lplogpal = reinterpret_cast<LPLOGPALETTE>(&pal_data[0]);
			LPPALETTEENTRY lppalent = nullptr;
			lplogpal->palVersion    = 0x300;
			lplogpal->palNumEntries = static_cast<WORD>(num);
			lppalent = lplogpal->palPalEntry;
			::GetEnhMetaFilePaletteEntries(hemf, num, lppalent);
			m_hPal = CreatePalette(lplogpal);
		} else {
			num = 2;
			std::simple_array<BYTE> pal_data;
			pal_data.resize(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*num);
			LPLOGPALETTE   lplogpal = reinterpret_cast<LPLOGPALETTE>(&pal_data[0]);
			LPPALETTEENTRY lppalent = nullptr;
			lplogpal->palVersion    = 0x300;
			lplogpal->palNumEntries = static_cast<WORD>(num);
			lppalent = lplogpal->palPalEntry;
			lppalent[0].peRed   = 0xff;
			lppalent[0].peGreen = 0xff;
			lppalent[0].peBlue  = 0xff;
			lppalent[0].peFlags = 0x00;
			lppalent[1].peRed   = 0x00;
			lppalent[1].peGreen = 0x00;
			lppalent[1].peBlue  = 0x00;
			lppalent[1].peFlags = 0x00;
			m_hPal = CreatePalette(lplogpal);
		}
		::DeleteEnhMetaFile(hemf);
	} else {
		return false;
	}
	return true;
}
