#ifndef __MEMDC_H__
#define __MEMDC_H__
#include "Bitmap.h"
#include <windows.h>
class CGrMemDC
{
public:
	CGrMemDC(HDC hdc, RECT &rect);
	virtual ~CGrMemDC();
	operator HDC(){ return m_hMemDC; }
private:
	CGrBitmap m_bitmap;
	HBITMAP m_oldBitmap = NULL;
	HDC m_hMemDC = NULL;
	HDC m_hDC    = NULL;
	RECT m_rect;
};

#endif // __MEMDC_H__
