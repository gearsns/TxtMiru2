#ifndef __PAGEFLIP_H__
#define __PAGEFLIP_H__

#include <vector>
#include "Bitmap.h"

class CGrPageFlip
{
private:
	struct ImageInfo {
		int x;
		int skip;
		int len;
		int lum;
		ImageInfo() : x(0), skip(1), len(0), lum(0){}
	};
	static double thPat[6*2];
	struct AnimationPattern {
		std::vector<ImageInfo> imageInfoList;
		std::vector<int> shadowMap;
		//
		AnimationPattern(){};
		void createImageMap(double th, int width);
		void draw(CGrBitmap &bmpFrom, CGrBitmap &bmpTo, CGrBitmap &bmpOut, const RECT &rc, int x_fix_start, int x_anim_start);
	} m_animPat[sizeof(thPat)/sizeof(double)];
	int m_iPatternNo = -1;
	RECT m_rcCanvas;
	bool m_bInc = false;
	bool m_bCountOnly = false;
	CGrBitmap m_bmpFrom;
	CGrBitmap m_bmpTo;
	CGrBitmap m_bmpCanvas;
public:
	CGrPageFlip();
	virtual ~CGrPageFlip();
	void Initialize(const RECT &rc, bool bInc, bool bCountOnly = false);
	void SetFlipEnd(){ m_iPatternNo = -1; }
	void SetFromBmp(CGrBitmap &bmp);
	void SetToBmp(CGrBitmap &bmp);
	bool Next();
	bool IsFilpEnd();
	bool IsDraw();
	void Clear();
	void Draw(HDC hdc, const RECT &paintRect, int offsetX, int offsetY);
	const int GetOffset(int iWidth);
};

#endif // __PAGEFLIP_H__
