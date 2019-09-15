#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <math.h>
#include <tchar.h>
#include "PageFlip.h"

CGrPageFlip::CGrPageFlip() : m_rcCanvas()
{
}

CGrPageFlip::~CGrPageFlip()
{
}

double CGrPageFlip::thPat[6*2] = {10,20,45,70,85,89,89,85,70,45,20,10};

void CGrPageFlip::AnimationPattern::createImageMap(double th, int width)
{
	th *= 3.1415926535897932384626433832795 / 180.0 / 2.0; /// Degree -> radian / 2.0;
	imageInfoList.clear();
	imageInfoList.shrink_to_fit();
	shadowMap.clear();
	imageInfoList.resize(width*2+1);
	shadowMap.resize(width*2+1);
	double dsin = sin(th);
	double dcos = cos(th);
	double tmp = (dcos * dcos - dsin * dsin);
	int iLeft  = width;
	int iRight = width;
	double left = 0;
	ImageInfo ii;
	int old_idx = -1;
	double dWidth = width;
	double dWidth8 = dWidth * 0.8;
	for(int x_pos=0; x_pos<width; ++x_pos, --iLeft, ++iRight){
		double right = left + tmp;
		int idx = static_cast<int>(left);
		++ii.len;
		left = right;
		if(idx == old_idx){
			continue;
		}
		auto dz = (static_cast<double>(x_pos)+dWidth8) / dWidth;
		if(dz > 1.0){ dz = 1.0; }
		if(ii.len > 64){
			ii.skip = 8;
			ii.len /= 8;
		} else if(ii.len > 32){
			ii.skip = 4;
			ii.len /= 4;
		} else if(ii.len > 8){
			ii.skip = 2;
			ii.len /= 2;
		} else {
			ii.skip = 1;
		}
		ii.lum = static_cast<int>(dz * 100.0) / ii.len;
		ii.x = iLeft;
		imageInfoList[width-idx] = ii;
		ii.x = iRight;
		ii.skip = -ii.skip;
		imageInfoList[width+idx] = ii;
		ii.len = 0;
		old_idx = idx;
	}
	int max_x = static_cast<int>(left);
	int end_x = (width - max_x) / 10;
	for(int i=0; i<end_x; ++i){
		shadowMap[width-i-max_x] =  shadowMap[width+i+max_x] = static_cast<int>(min(static_cast<double>(i) / static_cast<double>(end_x) + 0.5, 1.0) * 100.0);
	}
}

void CGrPageFlip::AnimationPattern::draw(CGrBitmap &bmpFrom, CGrBitmap &bmpTo, CGrBitmap &bmpOut, const RECT &rc, int x_fix_start, int x_anim_start)
{
	int line_size = bmpOut.Width();
	int width = imageInfoList.size();
	int half_width = width / 2;
	x_fix_start  *= half_width;
	x_anim_start *= half_width;

	int height = rc.bottom - rc.top;
	int offset = rc.left + rc.top * line_size;
	auto lpTo   = bmpTo.GetBits()   + offset;
	auto lpFrom = bmpFrom.GetBits() + offset + x_anim_start;
	auto lpOut  = bmpOut.GetBits()  + offset + x_anim_start;
	auto *ii_start = &imageInfoList[x_anim_start];
	int *s_start = &shadowMap[x_anim_start];
	for(int zy=height-1; zy>=0; --zy, lpOut += line_size, lpTo += line_size, lpFrom += line_size){
		auto lpFromLine = lpFrom;
		auto lpToLine   = lpTo  ;
		auto lpOutLine  = lpOut ;
		int *s = s_start;
		auto *ii = ii_start;
		for(int count=half_width; count>0; --count, ++lpFromLine, ++lpOutLine, ++s, ++ii){
			auto &&rgb = *lpFromLine;
			auto &&color = *lpOutLine;
			if(*s > 0){
				color.rgbRed   = static_cast<BYTE>(rgb.rgbRed   * (*s) / 100);
				color.rgbGreen = static_cast<BYTE>(rgb.rgbGreen * (*s) / 100);
				color.rgbBlue  = static_cast<BYTE>(rgb.rgbBlue  * (*s) / 100);
			} else if(ii->len == 0){
				color = rgb;
			} else {
				int iRed   = 0;
				int iGreen = 0;
				int iBlue  = 0;
				int len = ii->len;
				int skip = ii->skip;
				auto toRgb = lpToLine + ii->x;
				for(; len>0; --len, toRgb += skip){
					iRed   += toRgb->rgbRed  ;
					iGreen += toRgb->rgbGreen;
					iBlue  += toRgb->rgbBlue ;
				}
				color.rgbRed   = static_cast<BYTE>(iRed   * ii->lum / 100);
				color.rgbGreen = static_cast<BYTE>(iGreen * ii->lum / 100);
				color.rgbBlue  = static_cast<BYTE>(iBlue  * ii->lum / 100);
			}
		}
	}
}

void CGrPageFlip::Initialize(const RECT &rc, bool bInc, bool bCountOnly/* = false*/)
{
	if(!bCountOnly){
		int width = rc.right-rc.left;
		if(width != (m_rcCanvas.right-m_rcCanvas.left)){
			for(int idx=0; idx<sizeof(m_animPat)/sizeof(AnimationPattern); ++idx){
				m_animPat[idx].createImageMap(thPat[idx], width/2);
			}
		}
	}
	m_rcCanvas = rc;
	m_iPatternNo = 0;
	m_bInc = bInc;
	m_bCountOnly = bCountOnly;
}

bool CGrPageFlip::Next()
{
	if(m_iPatternNo >= sizeof(thPat)/sizeof(double) || m_iPatternNo < 0){
		Clear();
		return false;
	}
	int iPat = m_iPatternNo;
	if(!m_bCountOnly){
		if(!m_bmpCanvas.Create(m_bmpTo.Width(), m_bmpTo.Height())){
			return false;
		}
		if(m_bInc){
			//        | f : f | f : t | t : t
			// 2 <- 1 | 2 : 1 | 2 : 3 | 4 : 3
			if(m_iPatternNo == 0){
				// ‰E‚Ì‚Ý | [2-4] : f
				//        |   2   : 1
				CopyMemory(m_bmpCanvas.GetBits(), m_bmpFrom.GetBits(), m_bmpFrom.Width() * m_bmpFrom.Height() * sizeof(RGBQUAD));
			} else if(m_iPatternNo == sizeof(thPat)/sizeof(double)/2){
				// ¶‚Ì‚Ý | t : [1-3]
				//        | 4 :   3
				CopyMemory(m_bmpCanvas.GetBits(), m_bmpTo.GetBits(), m_bmpTo.Width() * m_bmpTo.Height() * sizeof(RGBQUAD));
			}
			iPat = sizeof(thPat)/sizeof(double) - m_iPatternNo - 1;
			if(iPat >= sizeof(thPat)/sizeof(double)/2){
				iPat = sizeof(thPat)/sizeof(double) - iPat - 1;
				m_animPat[iPat].draw(m_bmpTo, m_bmpFrom, m_bmpCanvas, m_rcCanvas, 1, 0);
			} else {
				m_animPat[iPat].draw(m_bmpFrom, m_bmpTo, m_bmpCanvas, m_rcCanvas, 0, 1);
			}
		} else {
			//        | f : f | t : f | t : t
			// 2 -> 1 | 4 : 3 | 2 : 3 | 2 : 1
			if(m_iPatternNo == 0){
				CopyMemory(m_bmpCanvas.GetBits(), m_bmpFrom.GetBits(), m_bmpFrom.Width() * m_bmpFrom.Height() * sizeof(RGBQUAD));
			} else if(m_iPatternNo == sizeof(thPat)/sizeof(double)/2){
				CopyMemory(m_bmpCanvas.GetBits(), m_bmpTo.GetBits(), m_bmpTo.Width() * m_bmpTo.Height() * sizeof(RGBQUAD));
			}
			if(iPat >= sizeof(thPat)/sizeof(double)/2){
				iPat = sizeof(thPat)/sizeof(double) - iPat - 1;
				m_animPat[iPat].draw(m_bmpFrom, m_bmpTo, m_bmpCanvas, m_rcCanvas, 1, 0);
			} else {
				m_animPat[iPat].draw(m_bmpTo, m_bmpFrom, m_bmpCanvas, m_rcCanvas, 0, 1);
			}
		}
	}
	++m_iPatternNo;
	if(m_iPatternNo >= sizeof(thPat)/sizeof(double)){
		Clear();
		return false;
	}
	return true;
}

void CGrPageFlip::Draw(HDC hdc, const RECT &paintRect, int offsetX, int offsetY)
{
	auto hbkHDC = ::CreateCompatibleDC(hdc);
	if(!hbkHDC){ return; }
	auto holdBitmap = static_cast<HBITMAP>(::SelectObject(hbkHDC, m_bmpCanvas));

	::BitBlt(hdc, paintRect.left, paintRect.top, paintRect.right, paintRect.bottom, hbkHDC, paintRect.left-offsetX, paintRect.top-offsetY, SRCCOPY);
	::SelectObject(hbkHDC, holdBitmap);
	::DeleteDC(hbkHDC);
}

void CGrPageFlip::SetFromBmp(CGrBitmap &bmp)
{
	if(m_bmpFrom.Create(bmp.Width(), bmp.Height())){
		CopyMemory(m_bmpFrom.GetBits(), bmp.GetBits(), bmp.Width() * bmp.Height() * sizeof(RGBQUAD));
	}
}

void CGrPageFlip::SetToBmp(CGrBitmap &bmp)
{
	if(m_bmpTo.Create(bmp.Width(), bmp.Height())){
		CopyMemory(m_bmpTo.GetBits(), bmp.GetBits(), bmp.Width() * bmp.Height() * sizeof(RGBQUAD));
	}
}

bool CGrPageFlip::IsFilpEnd()
{
	return m_iPatternNo < 0;
}

void CGrPageFlip::Clear()
{
	m_iPatternNo = -1;
	m_bmpFrom.Destroy();
	m_bmpTo.Destroy();
	m_bmpCanvas.Destroy();
}

const int CGrPageFlip::GetOffset(int iWidth)
{
	if(m_iPatternNo == -1){
		return 0;
	}
	int ptn[] = {0,0,0,0,0,0,9,19,44,69,100,100,100,100,100,100,100,100,};
	if(m_bInc){
		return iWidth * ptn[m_iPatternNo] / 100 - iWidth;
	} else {
		return iWidth * (100-ptn[m_iPatternNo]) / 100;
	}
}

bool CGrPageFlip::IsDraw()
{
	if(IsFilpEnd() || m_bCountOnly){
		return false;
	}
	return true;
}
