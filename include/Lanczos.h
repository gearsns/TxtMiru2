#ifndef __LANCZOS_H__
#define __LANCZOS_H__

#include "Bitmap.h"
namespace CGrLanczos
{
	void StretchBlt(CGrBitmap &dst_bmp, CGrBitmap &src_bmp, int n, double scale);
	void StretchBlt(CGrBitmap &dst_bmp, CGrBitmap &src_bmp, int n, double scale_w, double scale_h);
};

#endif // __LANCZOS_H__
