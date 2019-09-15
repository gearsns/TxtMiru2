#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "Image.h"
#include "stltchar.h"

//#define __DBG__
#include "Debug.h"

CGrImage::CGrImage()
{
}

CGrImage::~CGrImage()
{
}

#include "png.h"

#define WIDTHBYTES(i) (((i) + 31) / 32 * 4)

struct PngMemory
{
	LPBYTE m_pb;
	INT m_i;
	INT m_cb;
	PngMemory(LPVOID pv, INT i, INT cb)
	: m_pb(static_cast<LPBYTE>(pv)), m_i(i), m_cb(cb)
	{
	}
};

static void PngReadProc(png_structp png, png_bytep data, png_size_t length)
{
	auto *pngmem = static_cast<PngMemory *>(png_get_io_ptr(png));
	CopyMemory(data, pngmem->m_pb + pngmem->m_i, length);
	pngmem->m_i += length;
}

bool CGrImage::LoadPngFromMemory(CGrBitmap &bmp, LPVOID pv, INT cb)
{
	HBITMAP         hbm = NULL;
	png_structp     png_ptr = nullptr;
	png_infop       info = nullptr;
	PngMemory       pngmem(pv, 0, cb);
	png_bytepp      row_pointers = nullptr;
	bool bRet = true;
	png_uint_32 width  = 0;
	png_uint_32 height = 0;
	png_uint_32 y;

	HANDLE hHeap = NULL;
	do {
		hHeap = ::HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0, 0);
		if(hHeap == NULL){
			bRet = false;
			break;
		}
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(png_ptr == nullptr){
			bRet = false;
			break;
		}
		info = png_create_info_struct(png_ptr);
		if(info == nullptr || setjmp(png_jmpbuf(png_ptr))){
			bRet = false;
			break;
		}
		png_set_read_fn(png_ptr, &pngmem, PngReadProc);
		png_read_info(png_ptr, info);

		int color_type;
		int depth;
		png_get_IHDR(png_ptr, info, &width, &height, &depth, &color_type, NULL, NULL, NULL);
		png_set_expand(png_ptr);

		if(png_get_valid(png_ptr, info, PNG_INFO_tRNS)){
			png_set_tRNS_to_alpha(png_ptr);
		}
		if(depth == 16){
			png_set_strip_16(png_ptr);
		}
		png_set_gray_to_rgb(png_ptr);
		png_set_palette_to_rgb(png_ptr);
		png_set_bgr(png_ptr);
		png_set_packing(png_ptr);
		{
			double gamma;
			if(png_get_gAMA(png_ptr, info, &gamma)){
				png_set_gamma(png_ptr, 2.2, gamma);
			} else {
				png_set_gamma(png_ptr, 2.2, 0.45455);
			}
		}
		png_read_update_info(png_ptr, info);
		png_get_IHDR(png_ptr, info, &width, &height, &depth, &color_type, NULL, NULL, NULL);
		if (width <= 0 || height <= 0) {
			bRet = false;
			break;
		}
		auto rowbytes = png_get_rowbytes(png_ptr, info);
		row_pointers = static_cast<png_bytepp>(::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, height * sizeof(png_bytep)));
		if(row_pointers == nullptr){
			bRet = false;
			break;
		}
		for(y=0; y < height; ++y){
			row_pointers[y] = static_cast<png_bytep>(::HeapAlloc(hHeap, 0, rowbytes));
			if(row_pointers[y] == nullptr){
				bRet = false;
				break;
			}
		}
		if(!bRet){
			break;
		}
		png_read_image(png_ptr, row_pointers);
		png_read_end(png_ptr, NULL);

		BITMAPINFO bi = {sizeof(BITMAPINFOHEADER)};
		bi.bmiHeader.biWidth    = width;
		bi.bmiHeader.biHeight   = height;
		bi.bmiHeader.biPlanes   = 1;
		bi.bmiHeader.biBitCount = static_cast<WORD>(depth * png_get_channels(png_ptr, info));

		int widthbytes = WIDTHBYTES(width * bi.bmiHeader.biBitCount);
		bi.bmiHeader.biSizeImage = widthbytes * height;
		if(!bmp.Create(width, height)){
			bRet = false;
			break;
		}
		if(bi.bmiHeader.biBitCount == 32){
			auto lpByte = reinterpret_cast<LPBYTE>(bmp.GetBits());
			for(y=0; y < height; ++y, lpByte += widthbytes){
				CopyMemory(lpByte, row_pointers[y], rowbytes);
			}
		} else {
			LPBYTE pbBits;
			hbm = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, reinterpret_cast<VOID **>(&pbBits), NULL, 0);
			if(!hbm){
				bRet = false;
				break;
			}
			for(y=0; y < height; ++y){
				CopyMemory(pbBits + y * widthbytes, row_pointers[height - 1 - y], rowbytes);
			}
			auto hdcFrom = CreateCompatibleDC(NULL);
			auto hdcTo = CreateCompatibleDC(NULL);
			auto bmpFromOld = static_cast<HBITMAP>(SelectObject(hdcFrom, hbm));
			auto bmpToOld = static_cast<HBITMAP>(SelectObject(hdcTo, bmp));
			BitBlt(hdcTo, 0, 0, width, height, hdcFrom, 0, 0, SRCCOPY);
			SelectObject(hdcFrom, bmpToOld);
			SelectObject(hdcFrom, bmpFromOld);
			DeleteDC(hdcTo);
			DeleteDC(hdcFrom);
			SetAlphaBits(NULL, &bmp);
		}
	} while(0);

	if(hbm){
		DeleteObject(hbm);
	}
	if(png_ptr){
		if(info){
			png_destroy_read_struct(&png_ptr, &info, NULL);
		} else {
			png_destroy_read_struct(&png_ptr, NULL, NULL);
		}
	}
	if(hHeap){
		::HeapDestroy(hHeap);
	}
	return bRet;
}

// rgbReservedの反転：アルファ値が無い場合、デフォルト 0が入り 表示されないので
void CGrImage::SetAlphaBits(CGrBitmap *lpSrcBmp, CGrBitmap *lpDstBmp)
{
	int wh = lpDstBmp->Height() * lpDstBmp->Width();
	auto lpDst = lpDstBmp->GetBits();
	if(lpSrcBmp){
		// 透過処理への対応：背景色 白と背景色 黒の２つの画像に対して同様の描画を行い 差分で透過設定
		auto lpSrc = lpSrcBmp->GetBits();
		for(; wh>0; --wh, ++lpDst, ++lpSrc){
			if(/**/lpDst->rgbRed   == lpSrc->rgbRed
			   &&  lpDst->rgbGreen == lpSrc->rgbGreen
			   &&  lpDst->rgbBlue  == lpSrc->rgbBlue){
				lpDst->rgbReserved = 255;
			} else {
				lpDst->rgbReserved = 0;
			}
		}
	} else {
		for(; wh>0; --wh, ++lpDst){
			// 0(透明) -> 255(不透明)
			lpDst->rgbReserved = 255;
		}
	}
}

bool CGrImage::LoadCustomImage(CGrBitmap &bmp, LPCTSTR name)
{
	bool bRet = true;
	HANDLE hMem = NULL;
	LPBYTE lpByte = nullptr;
	do {
		auto hRs = ::FindResource(NULL, name, _T("IMAGE"));
		if(!hRs){
			bRet = false;
			break;
		}
		hMem = LoadResource(NULL, hRs);
		if(!hMem){
			bRet = false;
			break;
		}
		lpByte = static_cast<LPBYTE>(LockResource(hMem));
		if(!lpByte){
			bRet = false;
			break;
		}
		auto size = SizeofResource(NULL, hRs);
		if(size <= 0){
			bRet = false;
			break;
		}
		if(!LoadPngFromMemory(bmp, lpByte, size)){
			bRet = false;
			break;
		}
	} while(0);
	if(hMem){
		UnlockResource(hMem);
	}
	if(lpByte){
		FreeResource(lpByte);
	}
	return bRet;
}


int min3(int a, int b, int c)
{
	if(b < a){
		a = b;
	}
	if(c < a){
		a = c;
	}
	return a;
}
void CGrImage::CopyBitmapRect(CGrBitmap &dst, int x, int y, int w, int h, CGrBitmap &src, int sx, int sy)
{
	int iDstLine = dst.Width();
	int iSrcLine = src.Width();
	h = min3(h, dst.Height()-y, src.Height()-sy);
	if(h <= 0){
		return;
	}
	w = min3(w, iDstLine-x, iSrcLine-sx);
	if(w <= 0){
		return;
	}
	auto lpDst = reinterpret_cast<LPDWORD>(dst.GetBits()) + x  + y  * iDstLine;
	auto lpSrc = reinterpret_cast<LPDWORD>(src.GetBits()) + sx + sy * iSrcLine;
	for(; h>0; --h){
		auto lpDstLine = lpDst;
		auto lpSrcLine = lpSrc;
		for(auto dw = w; dw>0; --dw, ++lpDstLine, ++lpSrcLine){
			*lpDstLine = *lpSrcLine;
		}
		lpDst += iDstLine;
		lpSrc += iSrcLine;
	}
}

static double dWhiteTransRate = 1.2;
static bool bWhiteTrans = false;
static BYTE alphaMap[256*3] = {};

void CGrImage::SetWhiteTrans(bool bwt, double bwtr)
{
	bWhiteTrans     = bwt ;
	dWhiteTransRate = 1.0 + bwtr;
	for(int ic=0; ic<sizeof(alphaMap)/sizeof(BYTE); ++ic){
		auto alpha = (255.0 - static_cast<double>(ic)/sizeof(alphaMap) * 255) * dWhiteTransRate;
		if(alpha > 255.0){
			alpha = 255.0;
		} else if(alpha < 0){
			alpha = 0.0;
		}
		alphaMap[ic] = static_cast<BYTE>(alpha);
	}
}

#define ODD  0x00FF00FF
#define EVEN 0xFF00FF00
void CGrImage::AlphaBitBlt(CGrBitmap &dst, int x, int y, int w, int h, CGrBitmap &src, int sx, int sy)
{
	// アルファブレンドで BitBlt
	int iDstLine = dst.Width();
	int iSrcLine = src.Width();
	h = min3(h, dst.Height()-y, src.Height()-sy);
	if(h <= 0){
		return;
	}
	w = min3(w, iDstLine-x, iSrcLine-sx);
	if(w <= 0){
		return;
	}
	auto lpDstLine = reinterpret_cast<LPDWORD>(dst.GetBits()) + x  + y  * iDstLine;
	auto lpSrcLine = reinterpret_cast<LPDWORD>(src.GetBits()) + sx + sy * iSrcLine;
	if(bWhiteTrans){
		for(; h>0; --h){
			auto lpDst = lpDstLine;
			auto lpSrc = lpSrcLine;
			for(int dw = w; dw>0; --dw, ++lpDst, ++lpSrc){
				auto lpSrcBit = reinterpret_cast<LPRGBQUAD>(lpSrc);
				int iAlpha = lpSrcBit->rgbRed + lpSrcBit->rgbGreen + lpSrcBit->rgbBlue;
				auto alpha = min(lpSrcBit->rgbReserved, alphaMap[iAlpha]);
				auto alpha_r = 255 - alpha;
				*lpDst = (
					(((*lpDst) & ODD ) * alpha_r + ((*lpSrc) & ODD ) * alpha) & EVEN
					|
					(((*lpDst) & EVEN) * alpha_r + ((*lpSrc) & EVEN) * alpha) & ODD
					) >> 8;
			}
			lpDstLine += iDstLine;
			lpSrcLine += iSrcLine;
		}
	} else {
		for(; h>0; --h){
			auto lpDst = lpDstLine;
			auto lpSrc = lpSrcLine;
			for(int dw = w; dw>0; --dw, ++lpDst, ++lpSrc){
				auto alpha = reinterpret_cast<LPRGBQUAD>(lpSrc)->rgbReserved;
				auto alpha_r = 255 - alpha;
				*lpDst = (
					(((*lpDst) & ODD ) * alpha_r + ((*lpSrc) & ODD ) * alpha) & EVEN
					|
					(((*lpDst) & EVEN) * alpha_r + ((*lpSrc) & EVEN) * alpha) & ODD
					) >> 8;
			}
			lpDstLine += iDstLine;
			lpSrcLine += iSrcLine;
		}
	}
}
