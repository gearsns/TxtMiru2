#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "Bitmap.h"

class CGrImage
{
public:
	CGrImage();
	virtual ~CGrImage();
	static bool LoadCustomImage(CGrBitmap &bmp, LPCTSTR name);
	static bool LoadPngFromMemory(CGrBitmap &bmp, LPVOID pv, INT cb);
	static void CopyBitmapRect(CGrBitmap &dst, int x, int y, int w, int h, CGrBitmap &src, int sx, int sy);
	static void SetAlphaBits(CGrBitmap *lpSrcBmp, CGrBitmap *lpDstBmp);
	static void AlphaBitBlt(CGrBitmap &dst, int x, int y, int w, int h, CGrBitmap &src, int sx, int sy);
	static void SetWhiteTrans(bool bwt, double bwtr);
};

#endif // __IMAGE_H__
