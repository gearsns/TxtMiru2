#include "CharRenderer.h"
#include <math.h>
#include <vector>
#include "Text.h"
#include "Shell.h"
#include "stlutil.h"
#include "Win32Wrap.h"

#define __DBG__
#include "Debug.h"

#define ODD  0x00FF00FF
#define EVEN 0xFF00FF00
#define RGBQUAD2DWORD(rgb) (*(DWORD*)(&rgb))

class RendererTable {
	static int numofbits(long bits) {
		bits = (bits & 0x55555555) + (bits >> 1 & 0x55555555);
		bits = (bits & 0x33333333) + (bits >> 2 & 0x33333333);
		bits = (bits & 0x0f0f0f0f) + (bits >> 4 & 0x0f0f0f0f);
		bits = (bits & 0x00ff00ff) + (bits >> 8 & 0x00ff00ff);
		return (bits & 0x0000ffff) + (bits >>16 & 0x0000ffff);
	}
public:
	RendererTable(){
		for(long i=0; i!=0x100; ++i){
			byteArray[i] = numofbits(i);
			byteArrayPre[i] = numofbits(i & 0xf0);
			byteArrayAft[i] = numofbits(i & 0x0f);
			byteArrayPre1[i] = numofbits(i & 0x78);
			byteArrayPre2[i] = numofbits(i & 0x3c);
			byteArrayAft1[i] = numofbits(i & 0x07);
			byteArrayAft2[i] = numofbits(i & 0x80);
			byteArrayAft3[i] = numofbits(i & 0x03);
			byteArrayAft4[i] = numofbits(i & 0xc0);
		}
	}
	static UINT byteArray[0x100];
	static UINT byteArrayPre[0x100];
	static UINT byteArrayAft[0x100];
	static UINT byteArrayPre1[0x100];
	static UINT byteArrayPre2[0x100];
	static UINT byteArrayAft1[0x100];
	static UINT byteArrayAft2[0x100];
	static UINT byteArrayAft3[0x100];
	static UINT byteArrayAft4[0x100];
};
UINT RendererTable::byteArray[0x100] = {};
UINT RendererTable::byteArrayPre[0x100] = {};
UINT RendererTable::byteArrayAft[0x100] = {};
UINT RendererTable::byteArrayPre1[0x100] = {};
UINT RendererTable::byteArrayPre2[0x100] = {};
UINT RendererTable::byteArrayAft1[0x100] = {};
UINT RendererTable::byteArrayAft2[0x100] = {};
UINT RendererTable::byteArrayAft3[0x100] = {};
UINT RendererTable::byteArrayAft4[0x100] = {};
static RendererTable l_RenderTable;

class CGrEmfCache {
	using EmfData = std::simple_array<BYTE> ;
	using EmfBuffer = std::map<std::tstring,EmfData>;
	std::tstring m_curDir;
	std::tstring m_dataDir;
	EmfBuffer m_emfBuffer;

	HENHMETAFILE openEmfFile(LPCTSTR lpStr)
	{
		HENHMETAFILE hemf = NULL;
		do {
			TCHAR path[512];
			TCHAR fileName[512];
			GetCurrentDir(path, sizeof(path)/sizeof(TCHAR));
			_stprintf_s(fileName, _T("%s/%s.emf"), path, lpStr);
			hemf = ::GetEnhMetaFile(fileName); if(hemf){ break; }
			//
			GetCurrentDir(path, sizeof(path)/sizeof(TCHAR));
			_stprintf_s(fileName, _T("%s/Font/%s.emf"), path, lpStr);
			hemf = ::GetEnhMetaFile(fileName); if(hemf){ break; }
			//
			_stprintf_s(fileName, _T("%s/Font/%s.emf"), m_dataDir.c_str(), lpStr);
			hemf = ::GetEnhMetaFile(fileName); if(hemf){ break; }
			//
			CGrShell::GetExePath(path);
			_stprintf_s(fileName, _T("%s/Font/%s.emf"), path, lpStr);
			hemf = ::GetEnhMetaFile(fileName); if(hemf){ break; }
		} while(0);
		return hemf;
	}

	bool load(LPCTSTR lpStr){
		auto it=m_emfBuffer.find(lpStr);
		if(it == m_emfBuffer.end()){
			m_emfBuffer[lpStr].resize(0);
			auto hemf = openEmfFile(lpStr);
			if(hemf){
				auto cbBuffer = ::GetEnhMetaFileBits(hemf, 0, nullptr);
				auto &&emfdata = m_emfBuffer[lpStr];
				emfdata.resize(cbBuffer+1);
				::GetEnhMetaFileBits(hemf, cbBuffer, &emfdata[0]);
				::DeleteEnhMetaFile(hemf);
			} else {
				return false;
			}
		}
		return !m_emfBuffer[lpStr].empty();
	}
	HENHMETAFILE getMetaFile(LPCTSTR lpStr){
		if(!load(lpStr)){
			return NULL;
		}
		auto &&emfdata = m_emfBuffer[lpStr];
		return ::SetEnhMetaFileBits(emfdata.size(), &emfdata[0]);
	}
public:
	CGrEmfCache(){};
	void SetDataDir(LPCTSTR data_dir){ m_dataDir = data_dir; }
	void SetCurrentDir(LPCTSTR cur_dir){ m_curDir = cur_dir; }
	void GetCurrentDir(LPTSTR cur_dir, int len){
		if(m_curDir.empty()){
			::GetCurrentDirectory(len, cur_dir);
		} else {
			_stprintf_s(cur_dir, len, _T("%s"), m_curDir.c_str());
		}
	}
	void Clear(){ m_emfBuffer.clear(); }
	bool Draw(HDC hdc, LPCTSTR lpStr, int fontsize){
		auto hemf = getMetaFile(lpStr);
		if(!hemf){ return false; }
		int num = ::GetEnhMetaFilePaletteEntries(hemf, 0, nullptr);
		if(num > 0){
			::DeleteEnhMetaFile(hemf);
			return false;
		}
		RECT rect={0,0,fontsize,fontsize};
		// 一時的に色反転
		RGBQUAD col[2] = {{0x00,0x00,0x00,0x00},{0xff,0xff,0xff,0x00}};
		RGBQUAD revCol[2] = {{0xff,0xff,0xff,0x00},{0x00,0x00,0x00,0x00}};
		::SetDIBColorTable(hdc, 0, 2, revCol);
		::PlayEnhMetaFile(hdc , hemf , &rect);
		::DeleteEnhMetaFile(hemf);
		::SetDIBColorTable(hdc, 0, 2, col);
		return true;
	}
};
static CGrEmfCache l_EmfCache;

static bool DrawFont(HDC hdc, LPCTSTR lpStr, int fontsize)
{
	return l_EmfCache.Draw(hdc, lpStr, fontsize);
}
//
// k-nearest neighbor algorithm、k-NN
CGrCharKNNRenderer::CGrCharKNNRenderer(int k) : m_iHeight(0), m_iWidth(0), m_hWorkDC(NULL), m_hBackDC(NULL), m_k_nearest(k), m_FontSpacing(0.0), m_FontSpacingOffset(0), m_bCentering(false)
{
}
CGrCharKNNRenderer::~CGrCharKNNRenderer()
{
	clearCache();
}
bool CGrCharKNNRenderer::getCache(LPCTSTR str, int len, CGrMonoBitmap **lppWorkBitmap)
{
	*lppWorkBitmap = &m_WorkBitmap;
	// キャッシュチェック
	std::tstring ch;
	if(len > 0){
		ch.assign(str, len);
	} else {
		ch = str;
	}
	auto it = m_FontCache.find(ch);
	if(it != m_FontCache.end()){
		*lppWorkBitmap = it->second;
		if((*lppWorkBitmap)->GetBits()){
			return true;
		}
	}
	return false;
}

enum ParamType {
	PARAMTYPE_CURDIR       ,
	PARAMTYPE_DATADIR      ,
	PARAMTYPE_FONTSPACING  ,
	PARAMTYPE_FONTCENTERING,
};
struct ParamTypeMap {
	LPCTSTR name;
	ParamType type;
} l_paramtypemap[] = { // nameでソートしておくこと
	{_T("CurDir"       ), PARAMTYPE_CURDIR       },
	{_T("DataDir"      ), PARAMTYPE_DATADIR      },
	{_T("FontCentering"), PARAMTYPE_FONTCENTERING},
	{_T("FontSpacing"  ), PARAMTYPE_FONTSPACING  },
};
static int ParamTypeMapMapCompare(const void *key, const void *pdata)
{
	auto name  = static_cast<LPCTSTR>(key);
	auto *pptm = static_cast<const ParamTypeMap*>(pdata);
	return _tcscmp(name, pptm->name);
}
bool CGrCharKNNRenderer::SetParam(LPCTSTR name, LPCTSTR val)
{
	if(name){
		auto *pptm = static_cast<ParamTypeMap*>(bsearch(name, l_paramtypemap, sizeof(l_paramtypemap)/sizeof(ParamTypeMap), sizeof(ParamTypeMap), ParamTypeMapMapCompare));
		if(pptm){
			if(pptm->type == PARAMTYPE_CURDIR){
				l_EmfCache.SetCurrentDir(val);
				return true;
			} else if(pptm->type == PARAMTYPE_DATADIR){
				l_EmfCache.SetDataDir(val);
				return true;
			} else if(pptm->type == PARAMTYPE_FONTSPACING){
				m_FontSpacing = _tstof(val);
				auto font = m_BaseFont;
				SetFont(font);
				return true;
			} else if(pptm->type == PARAMTYPE_FONTCENTERING){
				m_bCentering = (_ttol(val) == 1);
				return true;
			}
		}
	}
	return false;
}

bool CGrCharKNNRenderer::GetParam(LPCTSTR name, LPTSTR val, int len)
{
	if(name){
		auto *pptm = static_cast<ParamTypeMap*>(bsearch(name, l_paramtypemap, sizeof(l_paramtypemap)/sizeof(ParamTypeMap), sizeof(ParamTypeMap), ParamTypeMapMapCompare));
		if(pptm){
			if(pptm->type == PARAMTYPE_CURDIR){
				l_EmfCache.GetCurrentDir(val, len);
				return true;
			} else if(pptm->type == PARAMTYPE_FONTSPACING){
				_stprintf_s(val, len, _T("%g"), m_FontSpacing);
				return true;
			} else if(pptm->type == PARAMTYPE_FONTCENTERING){
				_stprintf_s(val, len, _T("%d"), m_bCentering);
				return true;
			}
		}
	}
	return false;
}

void CGrCharKNNRenderer::createWorkDC()
{
	if(m_hWorkDC){ return; }
	// 作業用のビットマップ作成
	m_hWorkDC = ::CreateCompatibleDC(NULL);
	::SetTextColor(m_hWorkDC, RGB(0xff,0xff,0xff));
	::SetBkColor  (m_hWorkDC, RGB(0x00,0x00,0x00));
	if(m_BaseFont.Angle() == 2700){
		::SetTextAlign(m_hWorkDC, VTA_LEFT | VTA_TOP/* | VTA_CENTER*/); // 回転しても原点を固定
	} else if(m_BaseFont.Angle() == 1800){
		::SetTextAlign(m_hWorkDC, VTA_LEFT | VTA_BOTTOM/* | VTA_CENTER*/); // 回転しても原点を固定
	}
}
bool CGrCharKNNRenderer::drawText(LPCTSTR str, int len, CGrMonoBitmap *lpWorkBitmap)
{
	bool bret = true;
	auto lpTmpBits = lpWorkBitmap->GetBits();
	int iTmpHeight = lpWorkBitmap->Height();
	int iTmpWidth  = lpWorkBitmap->Width ();
	memset(lpTmpBits, 0x00, iTmpHeight*iTmpWidth/8);
	auto oldhBitmap = static_cast<HBITMAP>(::SelectObject(m_hWorkDC, *lpWorkBitmap));
	auto hOldFont   = static_cast<HFONT>(::SelectObject(m_hWorkDC, m_DrawFont));
	RECT rc = {0,0,iTmpWidth,iTmpHeight};    // 表示範囲の設定
	if(len > 0){
		rc.left -= m_FontSpacingOffset;
		if(m_bCentering){
			SIZE size = {};
			if(GetTextExtentPoint32(m_hWorkDC, str, len, &size)){
				if(m_DrawFont.Angle() == 0){
					if(size.cx < m_DrawFont.Height()/2){
						// 半角文字 →
						rc.left = (m_DrawFont.Height() / 2 - size.cx) / 2;
					}
				} else if(m_DrawFont.Angle() == 1800){
				} else {
					if(size.cx < m_DrawFont.Height()/2){
						// 半角文字 ↓
						rc.top = (m_DrawFont.Height() / 2 - size.cx) / 2;
					} else {
						rc.left += (m_DrawFont.Height() - size.cy) / 2;
					}
				}
			}
		}
		::ExtTextOut(m_hWorkDC, rc.left, rc.top, ETO_CLIPPED, &rc, str, len, nullptr);
	} else {
		if(!::DrawFont(m_hWorkDC, str, m_DrawFont.Height() - (m_FontSpacingOffset*2))){
			rc.left -= m_FontSpacingOffset;
			// パーサーで変換してしまいたいが、フォントファイル名がいるのでどうするか検討中
			// とりあえず、ベタもちで
			TCHAR ch;
			//
			if(_tcscmp(str, _T("／")) == 0){
				ch = 0x03033;
				OUTLINETEXTMETRIC otm = {}; // 下付きへ
				if(GetOutlineTextMetrics(m_hWorkDC, sizeof(otm), &otm)){
					rc.top = iTmpHeight / 2 - (-otm.otmDescent + otm.otmAscent);
				}
				::ExtTextOut(m_hWorkDC, rc.left, rc.top, ETO_CLIPPED, &rc, &ch, 1, nullptr);
			} else if(_tcscmp(str, _T("／″")) == 0){
				ch = 0x03034;
				OUTLINETEXTMETRIC otm = {}; // 下付きへ
				if(GetOutlineTextMetrics(m_hWorkDC, sizeof(otm), &otm)){
					rc.top = iTmpHeight / 2 - (-otm.otmDescent + otm.otmAscent);
				}
				::ExtTextOut(m_hWorkDC, rc.left, rc.top, ETO_CLIPPED, &rc, &ch, 1, nullptr);
			} else if(_tcscmp(str, _T("＼")) == 0){
				ch = 0x03035;
				::ExtTextOut(m_hWorkDC, rc.left, rc.top, ETO_CLIPPED, &rc, &ch, 1, nullptr);
			} else if(_tcscmp(str, _T("―")) == 0){
				ch = 0x02015;
				::ExtTextOut(m_hWorkDC, rc.left, rc.top, ETO_CLIPPED, &rc, &ch, 1, nullptr);
			} else {
				bret = false;
			}
		}
	}
	::SelectObject(m_hWorkDC, hOldFont  );
	::SelectObject(m_hWorkDC, oldhBitmap);
	return bret;
}

void CGrCharKNNRenderer::PatternFillVert(CGrBitmap& canvas, int x, int y, int width, int height, LPCTSTR str, int len) {}
void CGrCharKNNRenderer::PatternFillHorz(CGrBitmap& canvas, int x, int y, int width, int height, LPCTSTR str, int len) {}
void CGrCharKNNRenderer::clearCache()
{
	for(auto &&item : m_FontCache){
		auto *p = item.second;
		if(p){
			delete p;
		}
	}
	m_FontCache.clear();
}
bool CGrCharKNNRenderer::SetFont(CGrFont &font)
{
	clearCache();
	int h = font.Height();
	m_BaseFont = font;
	m_DrawFont = m_BaseFont;
	if(m_FontSpacing != 0.0){
		m_DrawFont.CreateFont(static_cast<int>(static_cast<double>(h * m_k_nearest) * m_FontSpacing) + 1);
		m_FontSpacingOffset = (static_cast<int>(static_cast<double>(m_BaseFont.Height() * m_k_nearest) * ((m_FontSpacing-1.0)/2.0)) + 1);
	} else {
		m_DrawFont.CreateFont(h * m_k_nearest + 1);
		m_FontSpacingOffset = 0;
	}
	m_iWidth  = (h+1) & (-2);
	m_iHeight = m_iWidth * 2;
	// サイズ毎にガンマ値を切替？
	makeColorMap();
	return true;
}
CGrFont &CGrCharKNNRenderer::GetFont()
{
	return m_BaseFont;
}
void CGrCharKNNRenderer::SetTextColor(COLORREF color)
{
	// テキストカラーマップ作成
	auto textColorR = GetRValue(color);
	auto textColorG = GetGValue(color);
	auto textColorB = GetBValue(color);
	RGBQUAD textColor = {};
	textColor.rgbBlue  = textColorB;
	textColor.rgbGreen = textColorG;
	textColor.rgbRed   = textColorR;
	m_TextColor = textColor;
	auto col = *reinterpret_cast<LPDWORD>(&textColor);
	auto col_odd  = col & ODD ;
	auto col_even = col & EVEN;
	int alpha_r = 256;
	for(int alpha=0; alpha<sizeof(m_TextColorMapODD)/sizeof(DWORD); ++alpha, --alpha_r){
		m_TextColorMapODD [alpha] = col_odd  * alpha_r;
		m_TextColorMapEVEN[alpha] = col_even * alpha_r;
	}
}
COLORREF CGrCharKNNRenderer::GetTextColor()
{
	return RGB(m_TextColor.rgbRed, m_TextColor.rgbGreen, m_TextColor.rgbBlue);
}

void CGrCharKNNRenderer::SetCacheCharList(LPCTSTR str)
{
	auto lpNextStr = str;
	std::map<std::tstring,bool> chk;
	while(*lpNextStr){
		auto lpSrc = lpNextStr;
		lpNextStr = CGrText::CharNext(lpSrc);
		std::tstring ch(lpSrc, lpNextStr);
		if(m_FontCache.find(ch) == m_FontCache.end()){
			m_FontCache[ch] = new CGrMonoBitmap();
		}
		chk[ch] = true;
	}
	auto it=m_FontCache.begin(), e=m_FontCache.end();
	for(; it != e; ++it){
		if(chk.find(it->first) == chk.end()){
			auto *p = it->second;
			if(p){
				delete p;
			}
			m_FontCache.erase(it);
		}
	}
}
void CGrCharKNNRenderer::makeColorMap(){}
///////////////////////
CGrChar4NNRenderer::CGrChar4NNRenderer() : CGrCharKNNRenderer(4)
{
}
CGrChar4NNRenderer::~CGrChar4NNRenderer()
{
}

bool CGrChar4NNRenderer::Draw(CGrBitmap &canvas, int x, int y, LPCTSTR str, int len)
{
	// 裏画面用ビットマップ サイズ取得
	int iBgWidth  = (m_iWidth *2  + 31) & (-32); // (m_iWidth *2  + 31) / 32 * 32;
	int iBgHeight = (m_iHeight    + 31) & (-32); // (m_iHeight    + 31) / 32 * 32;
	// 作業用のビットマップサイズを取得
	int iTmpWidth   = (iBgWidth  * 4 + 31) & (-32); // (iBgWidth  * 4 + 31) / 32 * 32;
	int iTmpHeight  = (iBgHeight * 4 + 31) & (-32); // (iBgHeight * 4 + 31) / 32 * 32;
	int iTextWidth  = m_iWidth ;
	int iTextHeight = m_iHeight;
	//
	int iNextLineSrc = iTmpWidth / 2; // (iTmpWidth / 8) Bytes * 4 Lines  <- (iTmpWidth / 8) Bytes <- iTmpWidth bits
	//
	int nextSrcNum = iTmpWidth / 8; // (iTmpWidth / 8) Bytes <- iTmpWidth bits
	int nextSrcNum1 = nextSrcNum * 1;
	int nextSrcNum2 = nextSrcNum * 2;
	int nextSrcNum3 = nextSrcNum * 3;
	//
	int iCanvasWidth  = canvas.Width ();
	int iCanvasHeight = canvas.Height();
	if(x > iCanvasWidth || y > iCanvasHeight || x + iTextWidth < 0 || y + iTmpWidth < 0){ return false; }
	//
	int iNextLineDst  = iCanvasWidth;
	int iLastDstNum   = min(iTextWidth, iCanvasWidth - x);
	if(iLastDstNum <= 0){
		return false; // 描画範囲外
	}
	auto iBaseFontHeight = m_BaseFont.Height();
	if (iBaseFontHeight + y < 0) {
		return false;
	}
	int iBaseHeight = min(iBaseFontHeight, iCanvasHeight - y);
	if(iBaseHeight <= 0){
		return false;
	}
	CGrMonoBitmap *lpWorkBitmap;
	// キャッシュチェック
	bool bCache = getCache(str, len, &lpWorkBitmap);
	// 作業用のビットマップ作成
	createWorkDC();
	// 作業用画面に描画
	if(!lpWorkBitmap->Create(iTmpWidth, iTmpHeight)){ return false; }
	if(!bCache){ if(!drawText(str, len, lpWorkBitmap)){ return false; } }
	//
	auto lpCanvasBits = canvas.GetBits();
	//
	auto lpTmpBits = lpWorkBitmap->GetBits();
	auto lpLastSrc = lpTmpBits + iNextLineSrc * iBaseHeight;
	auto lpNextSrc = lpTmpBits;
	if(y < 0){
		lpNextSrc -= (y % iTmpHeight) * iNextLineSrc;
		y = 0;
	}
	auto textColor = RGBQUAD2DWORD(m_TextColor);
	if(x < 0){
		iLastDstNum += x;
		x = -(x % iTextWidth);
		lpNextSrc += x / 2;
		if(x & 0x01){
			// 左端数有り
			auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + y * iCanvasWidth;
			auto lpSrc = lpNextSrc;
			for(; lpSrc < lpLastSrc; lpSrc += iNextLineSrc, lpDst += iNextLineDst){
				auto alpha = m_ColorMap[
					RendererTable::byteArrayAft[*(lpSrc            )] + RendererTable::byteArrayAft[*(lpSrc+nextSrcNum1)] +
					RendererTable::byteArrayAft[*(lpSrc+nextSrcNum2)] + RendererTable::byteArrayAft[*(lpSrc+nextSrcNum3)] ];
				if(alpha == 0){
					*lpDst = textColor;
				} else if(alpha != 255){
					*lpDst = (
						(( (*lpDst) & ODD ) * alpha + m_TextColorMapODD [alpha]) & EVEN
						|
						(( (*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
						) >> 8;
				}
			}
			x = 1;
			++lpNextSrc;
			--iLastDstNum;
		} else {
			x = 0;
		}
	}
	if(iLastDstNum & 0x01){
		// 右端数有り
		auto iLastSrc = (iLastDstNum % iTextWidth) / 2;
		auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + iLastDstNum - 1 + y * iCanvasWidth;
		auto lpSrc = lpNextSrc + iLastSrc;
		for(; lpSrc < lpLastSrc; lpSrc += iNextLineSrc, lpDst += iNextLineDst){
			auto alpha = m_ColorMap[
				RendererTable::byteArrayPre[*(lpSrc            )] + RendererTable::byteArrayPre[*(lpSrc+nextSrcNum1)] +
				RendererTable::byteArrayPre[*(lpSrc+nextSrcNum2)] + RendererTable::byteArrayPre[*(lpSrc+nextSrcNum3)] ];
			if(alpha == 0){
				*lpDst = textColor;
			} else if(alpha != 255){
				*lpDst = (
					(( (*lpDst) & ODD ) * alpha + m_TextColorMapODD [alpha]) & EVEN
					|
					(( (*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
					) >> 8;
			}
		}
		--iLastDstNum;
	}
	auto lpNextDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + y * iCanvasWidth;
	for(;lpNextSrc < lpLastSrc; lpNextSrc += iNextLineSrc, lpNextDst += iNextLineDst){
		auto lpSrc  = lpNextSrc;
		auto lpDst  = lpNextDst;
		auto lpEnd = lpDst + iLastDstNum;
		for (; lpDst < lpEnd; ++lpDst, ++lpSrc) {
			auto lpSrc1 = lpSrc + nextSrcNum1;
			auto lpSrc2 = lpSrc + nextSrcNum2;
			auto lpSrc3 = lpSrc + nextSrcNum3;
			auto alpha = m_ColorMap[
				RendererTable::byteArrayPre[*(lpSrc )] + RendererTable::byteArrayPre[*(lpSrc1)] +
				RendererTable::byteArrayPre[*(lpSrc2)] + RendererTable::byteArrayPre[*(lpSrc3)] ];
			if(alpha == 0){
				*lpDst = textColor;
			} else if(alpha != 255){
				*lpDst = (
					(( (*lpDst) & ODD ) * alpha + m_TextColorMapODD [alpha]) & EVEN
					|
					(( (*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
					) >> 8;
			}
			++lpDst;
			alpha = m_ColorMap[
				RendererTable::byteArrayAft[*(lpSrc )] + RendererTable::byteArrayAft[*(lpSrc1)] +
				RendererTable::byteArrayAft[*(lpSrc2)] + RendererTable::byteArrayAft[*(lpSrc3)] ];
			if(alpha == 0){
				*lpDst = textColor;
			} else if(alpha != 255){
				*lpDst = (
					(( (*lpDst) & ODD ) * alpha + m_TextColorMapODD [alpha]) & EVEN
					|
					(( (*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
					) >> 8;
			}
		}
	}
	return true;
}

void CGrChar4NNRenderer::PatternFillVert(CGrBitmap& canvas, int x, int y, int width, int height, LPCTSTR str, int len)
{
	// 裏画面用ビットマップ サイズ取得
	int iBgWidth = (m_iWidth * 2 + 31) & (-32); // (m_iWidth *2  + 31) / 32 * 32;
	int iBgHeight = (m_iHeight + 31) & (-32); // (m_iHeight    + 31) / 32 * 32;
	// 作業用のビットマップサイズを取得
	int iTmpWidth = (iBgWidth * 4 + 31) & (-32); // (iBgWidth  * 4 + 31) / 32 * 32;
	int iTmpHeight = (iBgHeight * 4 + 31) & (-32); // (iBgHeight * 4 + 31) / 32 * 32;
	int iTextWidth = m_iWidth;
	int iTextHeight = m_iHeight;
	//
	int iNextLineSrc = iTmpWidth / 2;  // (iTmpWidth / 8) Bytes * 8 Lines  <- (iTmpWidth / 8) Bytes <- iTmpWidth bits
	//
	int nextSrcNum = iTmpWidth / 8; // (iTmpWidth / 8) Bytes <- iTmpWidth bits
	int nextSrcNum1 = nextSrcNum * 1;
	int nextSrcNum2 = nextSrcNum * 2;
	int nextSrcNum3 = nextSrcNum * 3;
	//
	int iCanvasWidth = canvas.Width();
	int iCanvasHeight = canvas.Height();
	if (y + height > iCanvasHeight) {
		height = iCanvasHeight - y;
	}
	if (x + width > iCanvasWidth) {
		width = iCanvasWidth - x;
	}
	if (height <= 0 || width <= 0) { return; }
	if (x > iCanvasWidth || y > iCanvasHeight || x + iTextWidth < 0 || y + iTmpHeight < 0) { return; }
	//
	int iNextLineDst = iCanvasWidth;
	int iLastDstNum = min(iTextWidth, iCanvasWidth - x);
	if (iLastDstNum <= 0) {
		return; // 描画範囲外
	}
	int iBaseHeight = min(m_BaseFont.Height(), iCanvasHeight - y);
	if (iBaseHeight <= 0) {
		return;
	}
	CGrMonoBitmap* lpWorkBitmap;
	// キャッシュチェック
	bool bCache = getCache(str, len, &lpWorkBitmap);
	// 作業用のビットマップ作成
	createWorkDC();
	// 作業用画面に描画
	if (!lpWorkBitmap->Create(iTmpWidth, iTmpHeight)) { return; }
	if (!bCache) { drawText(str, len, lpWorkBitmap); }
	//
	auto lpCanvasBits = canvas.GetBits();
	//
	auto lpTmpBits = lpWorkBitmap->GetBits();
	auto lpLastSrc = lpTmpBits + iNextLineSrc * iBaseHeight;
	auto lpNextSrc = lpTmpBits;
	if (y < 0) {
		lpNextSrc -= (y % iBaseHeight) * iNextLineSrc;
		height += y;
		y = 0;
	}
	auto textColor = RGBQUAD2DWORD(m_TextColor);
	auto lpNextTopSrc = lpTmpBits + iNextLineSrc; // 一ビットかぶせる
	if (x < 0) {
		iLastDstNum += x;
		x = -(x % iTextWidth);
		lpNextSrc += x / 2;
		lpNextTopSrc += x / 2;
		if (x & 0x01) {
			// 左端数有り
			auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + y * iCanvasWidth;
			auto lpLastDst = lpDst + height * iCanvasWidth;
			auto lpSrc = lpNextSrc;
			do {
				for (; lpSrc < lpLastSrc && lpDst < lpLastDst; lpSrc += iNextLineSrc, lpDst += iNextLineDst) {
					auto alpha = m_ColorMap[
						RendererTable::byteArrayAft[*(lpSrc)] + RendererTable::byteArrayAft[*(lpSrc + nextSrcNum1)] +
						RendererTable::byteArrayAft[*(lpSrc + nextSrcNum2)] + RendererTable::byteArrayAft[*(lpSrc + nextSrcNum3)]];
					if (alpha == 0) {
						*lpDst = textColor;
					}
					else if (alpha != 255) {
						*lpDst = (
							(((*lpDst) & ODD) * alpha + m_TextColorMapODD[alpha]) & EVEN
							|
							(((*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
							) >> 8;
					}
				}
				lpSrc = lpNextTopSrc;
			} while (lpDst < lpLastDst);
			++lpNextSrc;
			++lpNextTopSrc;
			--iLastDstNum;
			x = 1;
		}
		else {
			x = 0;
		}
	}
	if (iLastDstNum & 0x01) {
		// 右端数有り
		auto iLastSrc = (iLastDstNum % iTextWidth) / 2;
		auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + iLastDstNum - 1 + y * iCanvasWidth;
		auto lpLastDst = lpDst + height * iCanvasWidth;
		auto lpSrc = lpNextSrc + iLastSrc;
		do {
			for (; lpSrc < lpLastSrc && lpDst < lpLastDst; lpSrc += iNextLineSrc, lpDst += iNextLineDst) {
				auto alpha = m_ColorMap[
					RendererTable::byteArrayPre[*(lpSrc)] + RendererTable::byteArrayPre[*(lpSrc + nextSrcNum1)] +
					RendererTable::byteArrayPre[*(lpSrc + nextSrcNum2)] + RendererTable::byteArrayPre[*(lpSrc + nextSrcNum3)]];
				if (alpha == 0) {
					*lpDst = textColor;
				}
				else if (alpha != 255) {
					*lpDst = (
						(((*lpDst) & ODD) * alpha + m_TextColorMapODD[alpha]) & EVEN
						|
						(((*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
						) >> 8;
				}
			}
			lpSrc = lpNextTopSrc + iLastSrc;
		} while (lpDst < lpLastDst);
		--iLastDstNum;
	}
	auto lpNextDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + y * iCanvasWidth;
	auto lpLastDst = lpNextDst + height * iCanvasWidth;
	do {
		for (; lpNextSrc < lpLastSrc && lpNextDst < lpLastDst; lpNextSrc += iNextLineSrc, lpNextDst += iNextLineDst) {
			auto lpSrc = lpNextSrc;
			auto lpDst = lpNextDst;
			auto lpEnd = lpDst + iLastDstNum;
			for (; lpDst < lpEnd; ++lpDst, ++lpSrc) {
				auto lpSrc1 = lpSrc + nextSrcNum1;
				auto lpSrc2 = lpSrc + nextSrcNum2;
				auto lpSrc3 = lpSrc + nextSrcNum3;
				auto alpha = m_ColorMap[
					RendererTable::byteArrayPre[*(lpSrc)] + RendererTable::byteArrayPre[*(lpSrc1)] +
					RendererTable::byteArrayPre[*(lpSrc2)] + RendererTable::byteArrayPre[*(lpSrc3)]];
				if (alpha == 0) {
					*lpDst = textColor;
				}
				else if (alpha != 255) {
					*lpDst = (
						(((*lpDst) & ODD) * alpha + m_TextColorMapODD[alpha]) & EVEN
						|
						(((*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
						) >> 8;
				}
				++lpDst;
				alpha = m_ColorMap[
					RendererTable::byteArrayAft[*(lpSrc)] + RendererTable::byteArrayAft[*(lpSrc1)] +
					RendererTable::byteArrayAft[*(lpSrc2)] + RendererTable::byteArrayAft[*(lpSrc3)]];
				if (alpha == 0) {
					*lpDst = textColor;
				}
				else if (alpha != 255) {
					*lpDst = (
						(((*lpDst) & ODD) * alpha + m_TextColorMapODD[alpha]) & EVEN
						|
						(((*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
						) >> 8;
				}
			}
		}
		lpNextSrc = lpNextTopSrc;
	} while (lpNextDst < lpLastDst);
}

void CGrChar4NNRenderer::PatternFillHorz(CGrBitmap& canvas, int x, int y, int width, int height, LPCTSTR str, int len)
{
	// 裏画面用ビットマップ サイズ取得
	int iBgWidth = (m_iWidth * 2 + 31) & (-32); // (m_iWidth *2  + 31) / 32 * 32;
	int iBgHeight = (m_iHeight + 31) & (-32); // (m_iHeight    + 31) / 32 * 32;
	// 作業用のビットマップサイズを取得
	int iTmpWidth = (iBgWidth * 4 + 31) & (-32); // (iBgWidth  * 4 + 31) / 32 * 32;
	int iTmpHeight = (iBgHeight * 4 + 31) & (-32); // (iBgHeight * 4 + 31) / 32 * 32;
	int iTextWidth = m_iWidth;
	int iTextHeight = m_iHeight;
	//
	int iNextLineSrc = iTmpWidth / 2;  // (iTmpWidth / 8) Bytes * 8 Lines  <- (iTmpWidth / 8) Bytes <- iTmpWidth bits
	//
	int nextSrcNum = iTmpWidth / 8; // (iTmpWidth / 8) Bytes <- iTmpWidth bits
	int nextSrcNum1 = nextSrcNum * 1;
	int nextSrcNum2 = nextSrcNum * 2;
	int nextSrcNum3 = nextSrcNum * 3;
	//
	int iCanvasWidth = canvas.Width();
	int iCanvasHeight = canvas.Height();
	if (y + height > iCanvasHeight) {
		height = iCanvasHeight - y;
	}
	if (x + width > iCanvasWidth) {
		width = iCanvasWidth - x;
	}
	if (height <= 0 || width <= 0) { return; }
	if (x > iCanvasWidth || y > iCanvasHeight || x + width < 0 || y + iTmpHeight < 0) { return; }
	//
	int iNextLineDst = iCanvasWidth;
	int iLastDstNum = min(width , iCanvasWidth - x);
	if (iLastDstNum <= 0) {
		return; // 描画範囲外
	}
	auto iBaseFontHeight = m_BaseFont.Height();
	if (iBaseFontHeight + y < 0) {
		return;
	}
	int iBaseHeight = min(iBaseFontHeight, iCanvasHeight - y);
	if (iBaseHeight <= 0) {
		return;
	}
	CGrMonoBitmap* lpWorkBitmap;
	// キャッシュチェック
	bool bCache = getCache(str, len, &lpWorkBitmap);
	// 作業用のビットマップ作成
	createWorkDC();
	// 作業用画面に描画
	if (!lpWorkBitmap->Create(iTmpWidth, iTmpHeight)) { return; }
	if (!bCache) { drawText(str, len, lpWorkBitmap); }
	//
	auto lpCanvasBits = canvas.GetBits();
	//
	auto lpTmpBits = lpWorkBitmap->GetBits();
	auto lpLastSrc = lpTmpBits + iNextLineSrc * iBaseHeight;
	auto lpNextSrc = lpTmpBits;
	//
	if (y < 0) {
		lpNextSrc -= (y % iBaseHeight) * iNextLineSrc;
		height += y;
		y = 0;
	}
	iTextWidth -= 2; // 一ビットかぶせる
	auto textColor = RGBQUAD2DWORD(m_TextColor);
	auto lpNextTopSrc = lpNextSrc;
	auto iStartWidthDst = iTextWidth;
	if (x < 0) {
		iLastDstNum += x;
		x = -(x % iTextWidth);
		lpNextSrc += x / 2;
		iStartWidthDst = x;
		if (x & 0x01) {
			// 左端数有り
			auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + y * iCanvasWidth;
			auto lpLastDst = lpDst + height * iCanvasWidth;
			auto lpSrc = lpNextSrc;
			for (; lpSrc < lpLastSrc && lpDst < lpLastDst; lpSrc += iNextLineSrc, lpDst += iNextLineDst) {
				auto alpha = m_ColorMap[
					RendererTable::byteArrayAft[*(lpSrc)] + RendererTable::byteArrayAft[*(lpSrc + nextSrcNum1)] +
						RendererTable::byteArrayAft[*(lpSrc + nextSrcNum2)] + RendererTable::byteArrayAft[*(lpSrc + nextSrcNum3)]];
				if (alpha == 0) {
					*lpDst = textColor;
				}
				else if (alpha != 255) {
					*lpDst = (
						(((*lpDst) & ODD) * alpha + m_TextColorMapODD[alpha]) & EVEN
						|
						(((*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
						) >> 8;
				}
			}
			++lpNextSrc;
			--iLastDstNum;
			x = 1;
			++iStartWidthDst;
		}
		else {
			x = 0;
		}
	}
	if (x + width % iTextWidth) {
		// 右端数有り
		auto iLastSrc = ((iStartWidthDst + iLastDstNum) % iTextWidth) / 2;
		auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + iLastDstNum - 1 + y * iCanvasWidth;
		auto lpLastDst = lpDst + height * iCanvasWidth;
		auto lpSrc = lpNextTopSrc + iLastSrc;
		for (; lpSrc < lpLastSrc && lpDst < lpLastDst; lpSrc += iNextLineSrc, lpDst += iNextLineDst) {
			auto alpha = m_ColorMap[
				RendererTable::byteArrayPre[*(lpSrc)] + RendererTable::byteArrayPre[*(lpSrc + nextSrcNum1)] +
					RendererTable::byteArrayPre[*(lpSrc + nextSrcNum2)] + RendererTable::byteArrayPre[*(lpSrc + nextSrcNum3)]];
			if (alpha == 0) {
				*lpDst = textColor;
			}
			else if (alpha != 255) {
				*lpDst = (
					(((*lpDst) & ODD) * alpha + m_TextColorMapODD[alpha]) & EVEN
					|
					(((*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
					) >> 8;
			}
		}
		--iLastDstNum;
	}
	auto lpNextDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + y * iCanvasWidth;
	auto lpLastDst = lpNextDst + height * iCanvasWidth;
	auto lpNextTopDst = lpNextDst;
	auto lpLastTopDst = lpNextTopDst + iLastDstNum;
	auto lpLineLastDst = lpLastTopDst;
	auto iWidthDst = iTextWidth - iStartWidthDst;
	do {
		auto lpLineLastDst = lpLastTopDst;
		for (; lpNextSrc < lpLastSrc && lpNextDst < lpLastDst; lpNextSrc += iNextLineSrc, lpNextDst += iNextLineDst, lpLineLastDst += iNextLineDst) {
			auto lpSrc = lpNextSrc;
			auto lpDst = lpNextDst;
			auto lpEnd = lpDst + iWidthDst;
			for (; lpDst < lpEnd && lpDst < lpLineLastDst; ++lpDst, ++lpSrc) {
				auto lpSrc1 = lpSrc + nextSrcNum1;
				auto lpSrc2 = lpSrc + nextSrcNum2;
				auto lpSrc3 = lpSrc + nextSrcNum3;
				auto alpha = m_ColorMap[
					RendererTable::byteArrayPre[*(lpSrc)] + RendererTable::byteArrayPre[*(lpSrc1)] +
						RendererTable::byteArrayPre[*(lpSrc2)] + RendererTable::byteArrayPre[*(lpSrc3)]];
				if (alpha == 0) {
					*lpDst = textColor;
				}
				else if (alpha != 255) {
					*lpDst = (
						(((*lpDst) & ODD) * alpha + m_TextColorMapODD[alpha]) & EVEN
						|
						(((*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
						) >> 8;
				}
				++lpDst;
				alpha = m_ColorMap[
					RendererTable::byteArrayAft[*(lpSrc)] + RendererTable::byteArrayAft[*(lpSrc1)] +
						RendererTable::byteArrayAft[*(lpSrc2)] + RendererTable::byteArrayAft[*(lpSrc3)]];
				if (alpha == 0) {
					*lpDst = textColor;
				}
				else if (alpha != 255) {
					*lpDst = (
						(((*lpDst) & ODD) * alpha + m_TextColorMapODD[alpha]) & EVEN
						|
						(((*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
						) >> 8;
				}
			}
		}
		lpNextSrc = lpNextTopSrc;
		lpNextTopDst += iWidthDst;
		lpNextDst = lpNextTopDst;
		iWidthDst = iTextWidth;
	} while (lpNextDst < lpLastTopDst);
}

void CGrChar4NNRenderer::makeColorMap()
{
	int maxNum = sizeof(m_ColorMap)/sizeof(BYTE);
	double gamma = 1.1f;
	double dMax = pow(255.0, gamma) / 255.0;

	for(int idx=0; idx<maxNum; ++idx){
		int color = 255 - idx * 255 / maxNum;
		m_ColorMap[maxNum-idx-1] = static_cast<BYTE>(max(0, min(255, static_cast<int>(pow(static_cast<double>(255)*(idx)/16, gamma) / dMax + 0.5))));
	}
}
//
// k-nearest neighbor algorithm、k-NN
CGrChar8NNRenderer::CGrChar8NNRenderer() : CGrCharKNNRenderer(8)
{
}
CGrChar8NNRenderer::~CGrChar8NNRenderer()
{
}

bool CGrChar8NNRenderer::Draw(CGrBitmap &canvas, int x, int y, LPCTSTR str, int len)
{
	// 裏画面用ビットマップ サイズ取得
	int iBgWidth  = (m_iWidth *2  + 31) & (-32); // (m_iWidth *2  + 31) / 32 * 32;
	int iBgHeight = (m_iHeight    + 31) & (-32); // (m_iHeight    + 31) / 32 * 32;
	// 作業用のビットマップサイズを取得
	int iTmpWidth   = (iBgWidth  * 8 + 31) & (-32); // (iBgWidth  * 8 + 31) / 32 * 32;
	int iTmpHeight  = (iBgHeight * 8 + 31) & (-32); // (iBgHeight * 8 + 31) / 32 * 32;
	int iTextWidth  = m_iWidth ;
	int iTextHeight = m_iHeight;
	//
	int iNextLineSrc = iTmpWidth;  // (iTmpWidth / 8) Bytes * 8 Lines  <- (iTmpWidth / 8) Bytes <- iTmpWidth bits
	//
	int nextSrcNum = iTmpWidth / 8; // (iTmpWidth / 8) Bytes <- iTmpWidth bits
	int nextSrcNum1 = nextSrcNum * 1;
	int nextSrcNum2 = nextSrcNum * 2;
	int nextSrcNum3 = nextSrcNum * 3;
	int nextSrcNum4 = nextSrcNum * 4;
	int nextSrcNum5 = nextSrcNum * 5;
	int nextSrcNum6 = nextSrcNum * 6;
	int nextSrcNum7 = nextSrcNum * 7;
	//
	int iCanvasWidth  = canvas.Width ();
	int iCanvasHeight = canvas.Height();
	if(x > iCanvasWidth || y > iCanvasHeight || x + iTextWidth < 0 || y + iTmpWidth < 0){ return false; }
	//
	int iNextLineDst  = iCanvasWidth;
	int iLastDstNum   = min(iTextWidth, iCanvasWidth - x);
	if(iLastDstNum <= 0){
		return false; // 描画範囲外
	}
	int iBaseHeight = min(m_BaseFont.Height(), iCanvasHeight - y);
	if(iBaseHeight <= 0){
		return false;
	}
	CGrMonoBitmap *lpWorkBitmap;
	// キャッシュチェック
	bool bCache = getCache(str, len, &lpWorkBitmap);
	// 作業用のビットマップ作成
	createWorkDC();
	// 作業用画面に描画
	if(!lpWorkBitmap->Create(iTmpWidth, iTmpHeight)){ return false; }
	if(!bCache){ if(!drawText(str, len, lpWorkBitmap)){ return false; } }
	//
	auto lpCanvasBits = canvas.GetBits();
	//
	auto lpTmpBits = lpWorkBitmap->GetBits();
	auto lpLastSrc = lpTmpBits + iNextLineSrc * iBaseHeight;
	auto lpNextSrc = lpTmpBits;
	if (y < 0) {
		lpNextSrc -= (y % iTmpHeight) * iNextLineSrc;
		y = 0;
	}
	if(x < 0){
		lpNextSrc -= x;
		iLastDstNum += x;
		x = 0;
	}
	auto textColor = RGBQUAD2DWORD(m_TextColor);
	for(auto lpNextDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + y * iCanvasWidth;
		lpNextSrc < lpLastSrc; lpNextSrc += iNextLineSrc, lpNextDst += iNextLineDst){
		auto lpSrc = lpNextSrc;
		auto lpDst = lpNextDst;
		auto lpEnd = lpDst+iLastDstNum;
		for(; lpDst < lpEnd; ++lpSrc, ++lpDst){
			auto alpha = m_ColorMap[
				RendererTable::byteArray[*(lpSrc            )] + RendererTable::byteArray[*(lpSrc+nextSrcNum1)] +
				RendererTable::byteArray[*(lpSrc+nextSrcNum2)] + RendererTable::byteArray[*(lpSrc+nextSrcNum3)] +
				RendererTable::byteArray[*(lpSrc+nextSrcNum4)] + RendererTable::byteArray[*(lpSrc+nextSrcNum5)] +
				RendererTable::byteArray[*(lpSrc+nextSrcNum6)] + RendererTable::byteArray[*(lpSrc+nextSrcNum7)] ];
			if(alpha == 0){
				*lpDst = textColor;
			} else if(alpha != 255){
				*lpDst = (
					(( (*lpDst) & ODD ) * alpha + m_TextColorMapODD [alpha]) & EVEN
					|
					(( (*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
					) >> 8;
			}
		}
	}
	return true;
}

void CGrChar8NNRenderer::PatternFillVert(CGrBitmap &canvas, int x, int y, int width, int height, LPCTSTR str, int len)
{
	// 裏画面用ビットマップ サイズ取得
	int iBgWidth  = (m_iWidth *2  + 31) & (-32); // (m_iWidth *2  + 31) / 32 * 32;
	int iBgHeight = (m_iHeight    + 31) & (-32); // (m_iHeight    + 31) / 32 * 32;
	// 作業用のビットマップサイズを取得
	int iTmpWidth   = (iBgWidth  * 8 + 31) & (-32); // (iBgWidth  * 8 + 31) / 32 * 32;
	int iTmpHeight  = (iBgHeight * 8 + 31) & (-32); // (iBgHeight * 8 + 31) / 32 * 32;
	int iTextWidth  = m_iWidth ;
	int iTextHeight = m_iHeight;
	//
	int iNextLineSrc = iTmpWidth;  // (iTmpWidth / 8) Bytes * 8 Lines  <- (iTmpWidth / 8) Bytes <- iTmpWidth bits
	//
	int nextSrcNum = iTmpWidth / 8; // (iTmpWidth / 8) Bytes <- iTmpWidth bits
	int nextSrcNum1 = nextSrcNum * 1;
	int nextSrcNum2 = nextSrcNum * 2;
	int nextSrcNum3 = nextSrcNum * 3;
	int nextSrcNum4 = nextSrcNum * 4;
	int nextSrcNum5 = nextSrcNum * 5;
	int nextSrcNum6 = nextSrcNum * 6;
	int nextSrcNum7 = nextSrcNum * 7;
	//
	int iCanvasWidth  = canvas.Width ();
	int iCanvasHeight = canvas.Height();
	if(y + height > iCanvasHeight){
		height = iCanvasHeight - y;
	}
	if(x + width > iCanvasWidth){
		width = iCanvasWidth - x;
	}
	if(height <= 0 || width <= 0){ return; }
	if(x > iCanvasWidth || y > iCanvasHeight || x + iTextWidth < 0 || y + iTmpWidth < 0){ return; }
	//
	int iNextLineDst  = iCanvasWidth;
	int iLastDstNum   = min(iTextWidth, iCanvasWidth - x);
	if(iLastDstNum <= 0){
		return; // 描画範囲外
	}
	int iBaseHeight = min(m_BaseFont.Height(), iCanvasHeight - y);
	if(iBaseHeight <= 0){
		return;
	}
	CGrMonoBitmap *lpWorkBitmap;
	// キャッシュチェック
	bool bCache = getCache(str, len, &lpWorkBitmap);
	// 作業用のビットマップ作成
	createWorkDC();
	// 作業用画面に描画
	if(!lpWorkBitmap->Create(iTmpWidth, iTmpHeight)){ return; }
	if(!bCache){ drawText(str, len, lpWorkBitmap); }
	//
	auto lpCanvasBits = canvas.GetBits();
	//
	auto lpTmpBits = lpWorkBitmap->GetBits();
	auto lpLastSrc = lpTmpBits + iNextLineSrc * (iBaseHeight-1); // (iNextLineSrc = nextSrcNum * 8)
	auto lpNextSrc = lpTmpBits;
	if (y < 0) {
		lpNextSrc -= (y % iBaseHeight) * iNextLineSrc;
		height += y;
		y = 0;
	}
	auto lpNextTopSrc = lpTmpBits + iNextLineSrc;
	if(x < 0){
		iLastDstNum += x;
		x = -(x % iTextWidth);
		lpNextSrc += x;
		lpNextTopSrc += x;
		x = 0;
	}
	auto textColor = RGBQUAD2DWORD(m_TextColor);
	auto lpNextDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + y * iCanvasWidth;
	auto lpLastDst = lpNextDst + height * iCanvasWidth;
	do {
		for(; lpNextSrc < lpLastSrc && lpNextDst < lpLastDst; lpNextSrc += iNextLineSrc, lpNextDst += iNextLineDst){
			auto  lpSrc = lpNextSrc;
			auto lpDst = lpNextDst;
			auto lpEnd = lpDst+iLastDstNum;
			for(; lpDst < lpEnd; ++lpSrc, ++lpDst){
				auto alpha = m_ColorMap[
					RendererTable::byteArray[*(lpSrc            )] + RendererTable::byteArray[*(lpSrc+nextSrcNum1)] +
					RendererTable::byteArray[*(lpSrc+nextSrcNum2)] + RendererTable::byteArray[*(lpSrc+nextSrcNum3)] +
					RendererTable::byteArray[*(lpSrc+nextSrcNum4)] + RendererTable::byteArray[*(lpSrc+nextSrcNum5)] +
					RendererTable::byteArray[*(lpSrc+nextSrcNum6)] + RendererTable::byteArray[*(lpSrc+nextSrcNum7)] ];
				if(alpha == 0){
					*lpDst = textColor;
				} else if(alpha != 255){
					*lpDst = (
						(( (*lpDst) & ODD ) * alpha + m_TextColorMapODD [alpha]) & EVEN
						|
						(( (*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
						) >> 8;
				}
			}
		}
		lpNextSrc = lpNextTopSrc;
	} while(lpNextDst < lpLastDst);
}
void CGrChar8NNRenderer::PatternFillHorz(CGrBitmap& canvas, int x, int y, int width, int height, LPCTSTR str, int len)
{
	// 裏画面用ビットマップ サイズ取得
	int iBgWidth = (m_iWidth * 2 + 31) & (-32); // (m_iWidth *2  + 31) / 32 * 32;
	int iBgHeight = (m_iHeight + 31) & (-32); // (m_iHeight    + 31) / 32 * 32;
	// 作業用のビットマップサイズを取得
	int iTmpWidth = (iBgWidth * 8 + 31) & (-32); // (iBgWidth  * 8 + 31) / 32 * 32;
	int iTmpHeight = (iBgHeight * 8 + 31) & (-32); // (iBgHeight * 8 + 31) / 32 * 32;
	int iTextWidth = m_iWidth;
	int iTextHeight = m_iHeight;
	//
	int iNextLineSrc = iTmpWidth;  // (iTmpWidth / 8) Bytes * 8 Lines  <- (iTmpWidth / 8) Bytes <- iTmpWidth bits
	//
	int nextSrcNum = iTmpWidth / 8; // (iTmpWidth / 8) Bytes <- iTmpWidth bits
	int nextSrcNum1 = nextSrcNum * 1;
	int nextSrcNum2 = nextSrcNum * 2;
	int nextSrcNum3 = nextSrcNum * 3;
	int nextSrcNum4 = nextSrcNum * 4;
	int nextSrcNum5 = nextSrcNum * 5;
	int nextSrcNum6 = nextSrcNum * 6;
	int nextSrcNum7 = nextSrcNum * 7;
	//
	int iCanvasWidth = canvas.Width();
	int iCanvasHeight = canvas.Height();
	if (y + height > iCanvasHeight) {
		height = iCanvasHeight - y;
	}
	if (x + width > iCanvasWidth) {
		width = iCanvasWidth - x;
	}
	if (height <= 0 || width <= 0) { return; }
	if (x > iCanvasWidth || y > iCanvasHeight || x + width < 0 || y + iTmpWidth < 0) { return; }
	//
	int iNextLineDst = iCanvasWidth;
	int iLastDstNum = min(width, iCanvasWidth - x);
	if (iLastDstNum <= 0) {
		return; // 描画範囲外
	}
	int iBaseHeight = min(m_BaseFont.Height(), iCanvasHeight - y);
	if (iBaseHeight <= 0) {
		return;
	}
	CGrMonoBitmap* lpWorkBitmap;
	// キャッシュチェック
	bool bCache = getCache(str, len, &lpWorkBitmap);
	// 作業用のビットマップ作成
	createWorkDC();
	// 作業用画面に描画
	if (!lpWorkBitmap->Create(iTmpWidth, iTmpHeight)) { return; }
	if (!bCache) { drawText(str, len, lpWorkBitmap); }
	//
	auto lpCanvasBits = canvas.GetBits();
	//
	auto lpTmpBits = lpWorkBitmap->GetBits();
	auto lpLastSrc = lpTmpBits + iNextLineSrc * iBaseHeight;
	auto lpNextSrc = lpTmpBits;
	if (y < 0) {
		lpNextSrc -= (y % iTmpHeight) * iNextLineSrc;
		height += y;
		y = 0;
	}
	--iTextWidth; // 一ビットかぶせる
	auto textColor = RGBQUAD2DWORD(m_TextColor);
	auto lpNextTopSrc = lpNextSrc;
	auto iStartWidthDst = iTextWidth;
	if (x < 0) {
		iLastDstNum += x;
		x = -(x % iTextWidth);
		lpNextSrc += x;
		iStartWidthDst = x;
		x = 0;
	}
	auto lpNextDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + y * iCanvasWidth;
	auto lpLastDst = lpNextDst + height * iCanvasWidth;
	auto lpNextTopDst = lpNextDst;
	auto lpLastTopDst = lpNextTopDst + iLastDstNum;
	auto lpLineLastDst = lpLastTopDst;
	auto iWidthDst = iTextWidth - iStartWidthDst;
	do {
		auto lpLineLastDst = lpLastTopDst;
		for (; lpNextSrc < lpLastSrc && lpNextDst < lpLastDst; lpNextSrc += iNextLineSrc, lpNextDst += iNextLineDst, lpLineLastDst += iNextLineDst) {
			auto lpSrc = lpNextSrc;
			auto lpDst = lpNextDst;
			auto lpEnd = lpDst + iWidthDst;
			for (; lpDst < lpEnd && lpDst < lpLineLastDst; ++lpSrc, ++lpDst) {
				auto alpha = m_ColorMap[
					RendererTable::byteArray[*(lpSrc)] + RendererTable::byteArray[*(lpSrc + nextSrcNum1)] +
					RendererTable::byteArray[*(lpSrc + nextSrcNum2)] + RendererTable::byteArray[*(lpSrc + nextSrcNum3)] +
					RendererTable::byteArray[*(lpSrc + nextSrcNum4)] + RendererTable::byteArray[*(lpSrc + nextSrcNum5)] +
					RendererTable::byteArray[*(lpSrc + nextSrcNum6)] + RendererTable::byteArray[*(lpSrc + nextSrcNum7)]];
				if (alpha == 0) {
					*lpDst = textColor;
				}
				else if (alpha != 255) {
					*lpDst = (
						(((*lpDst) & ODD) * alpha + m_TextColorMapODD[alpha]) & EVEN
						|
						(((*lpDst) & EVEN) * alpha + m_TextColorMapEVEN[alpha]) & ODD
						) >> 8;
				}
			}
		}
		lpNextSrc = lpNextTopSrc;
		lpNextTopDst += iWidthDst;
		lpNextDst = lpNextTopDst;
		iWidthDst = iTextWidth;
	} while (lpNextDst < lpLastTopDst);
}

void CGrChar8NNRenderer::makeColorMap()
{
	int maxNum = sizeof(m_ColorMap)/sizeof(BYTE);
	double gamma = 1.1f;
	double dMax = pow(255.0, gamma) / 255.0;

	for(int idx=0; idx<maxNum; ++idx){
		int color = 255 - idx * 255 / maxNum;
		m_ColorMap[maxNum-idx-1] = static_cast<BYTE>(max(0, min(255, static_cast<int>(pow(static_cast<double>(255)*(idx)/64, gamma) / dMax + 0.5))));
	}
}

//
// CGrCharLCDRenderer
CGrCharLCDRenderer::CGrCharLCDRenderer() : CGrCharKNNRenderer(4)
{
}
CGrCharLCDRenderer::~CGrCharLCDRenderer()
{
}

//      1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8
//   R  ■■■■□□□□
//   G    ■■■■□□□□
//   B      ■■■■□□□□
//LCD_EVEN_R = 1111000000000000; // 0xf0 = byteArrayPre
//LCD_EVEN_G = 0111100000000000; // 0x78 = byteArrayPre1
//LCD_EVEN_B = 0011110000000000; // 0x3c = byteArrayPre2
//LCD_ODD_R  = 0000111100000000; // 0x0f = byteArrayAft
//LCD_ODD_G  = 0000011100000000; // 0x07 = byteArrayAft1
//         + = 0000000010000000; // 0x80 = byteArrayAft2
//LCD_ODD_B  = 0000001100000000; // 0x03 = byteArrayAft3
//         + = 0000000011000000; // 0xc0 = byteArrayAft4
#define LCD_INIT_ODD \
	LPBYTE lpSrc1 = lpSrc+nextSrcNum1; \
	LPBYTE lpSrc2 = lpSrc+nextSrcNum2; \
	LPBYTE lpSrc3 = lpSrc+nextSrcNum3; \
	LPBYTE lpSrc4 = lpSrc+nextSrcNum4; \
	LPBYTE lpSrc1p = lpSrc1 + 1; \
	LPBYTE lpSrc2p = lpSrc2 + 1; \
	LPBYTE lpSrc3p = lpSrc3 + 1; \
	LPBYTE lpSrc4p = lpSrc4 + 1;
#define LCD_INIT_EVEN \
	LPBYTE lpSrc1 = lpSrc+nextSrcNum1; \
	LPBYTE lpSrc2 = lpSrc+nextSrcNum2; \
	LPBYTE lpSrc3 = lpSrc+nextSrcNum3; \
	LPBYTE lpSrc4 = lpSrc+nextSrcNum4;
#define LCD_ODD_R (m_ColorMap[RendererTable::byteArrayAft[*(lpSrc1)] + RendererTable::byteArrayAft[*(lpSrc2)] + RendererTable::byteArrayAft[*(lpSrc3)] + RendererTable::byteArrayAft[*(lpSrc4)]])
#define LCD_ODD_G (m_ColorMap[ \
							   RendererTable::byteArrayAft1[*(lpSrc1 )] + RendererTable::byteArrayAft1[*(lpSrc2 )] + RendererTable::byteArrayAft1[*(lpSrc3 )] + RendererTable::byteArrayAft1[*(lpSrc4 )] + \
							   RendererTable::byteArrayAft2[*(lpSrc1p)] + RendererTable::byteArrayAft2[*(lpSrc2p)] + RendererTable::byteArrayAft2[*(lpSrc3p)] + RendererTable::byteArrayAft2[*(lpSrc4p)]   \
							   ])
#define LCD_ODD_B (m_ColorMap[ \
							   RendererTable::byteArrayAft3[*(lpSrc1 )] + RendererTable::byteArrayAft3[*(lpSrc2 )] + RendererTable::byteArrayAft3[*(lpSrc3 )] + RendererTable::byteArrayAft3[*(lpSrc4 )] + \
							   RendererTable::byteArrayAft4[*(lpSrc1p)] + RendererTable::byteArrayAft4[*(lpSrc2p)] + RendererTable::byteArrayAft4[*(lpSrc3p)] + RendererTable::byteArrayAft4[*(lpSrc4p)]   \
							   ])
#define LCD_EVEN_R (m_ColorMap[RendererTable::byteArrayPre [*(lpSrc1)] + RendererTable::byteArrayPre [*(lpSrc2)] + RendererTable::byteArrayPre [*(lpSrc3)] + RendererTable::byteArrayPre [*(lpSrc4)]])
#define LCD_EVEN_G (m_ColorMap[RendererTable::byteArrayPre1[*(lpSrc1)] + RendererTable::byteArrayPre1[*(lpSrc2)] + RendererTable::byteArrayPre1[*(lpSrc3)] + RendererTable::byteArrayPre1[*(lpSrc4)]])
#define LCD_EVEN_B (m_ColorMap[RendererTable::byteArrayPre2[*(lpSrc1)] + RendererTable::byteArrayPre2[*(lpSrc2)] + RendererTable::byteArrayPre2[*(lpSrc3)] + RendererTable::byteArrayPre2[*(lpSrc4)]])
#define LCD_SET_COLOR(__v__, __color__) { \
	int v = __v__; \
	if(v == 0){ \
		((RGBQUAD*)lpDst)->rgb##__color__ = m_TextColor.rgb##__color__; \
	} else if(v != 255){ \
		((RGBQUAD*)lpDst)->rgb##__color__ = (m_i##__color__[v] + ((int)((RGBQUAD*)lpDst)->rgb##__color__ * v)) >> 8; \
	} \
}

bool CGrCharLCDRenderer::Draw(CGrBitmap &canvas, int x, int y, LPCTSTR str, int len)
{
	// 裏画面用ビットマップ サイズ取得
	int iBgWidth  = ((m_iWidth+1) *2  + 31) & (-32); // (m_iWidth *2  + 31) / 32 * 32; // lpSrc + 1で値を参照するので 一つ多めにとっておく
	int iBgHeight = (m_iHeight    + 31) & (-32); // (m_iHeight    + 31) / 32 * 32;
	// 作業用のビットマップサイズを取得
	int iTmpWidth   = (iBgWidth  * 4 + 31) & (-32); // (iBgWidth  * 4 + 31) / 32 * 32;
	int iTmpHeight  = (iBgHeight * 4 + 31) & (-32); // (iBgHeight * 4 + 31) / 32 * 32;
	int iTextWidth  = m_iWidth ;
	int iTextHeight = m_iHeight;
	//
	int iNextLineSrc = iTmpWidth / 2; // (iTmpWidth / 8) Bytes * 4 Lines  <- (iTmpWidth / 8) Bytes <- iTmpWidth bits
	//
	int nextSrcNum  = iTmpWidth/8 ; // (iTmpWidth / 8) Bytes <- iTmpWidth bits
	int nextSrcNum1 = nextSrcNum*1;
	int nextSrcNum2 = nextSrcNum*2;
	int nextSrcNum3 = nextSrcNum*3;
	int nextSrcNum4 = nextSrcNum*4;
	//
	int iCanvasWidth  = canvas.Width ();
	int iCanvasHeight = canvas.Height();
	if(x > iCanvasWidth || y > iCanvasHeight || x + iTextWidth < 0 || y + iTmpWidth < 0){ return false; }
	//
	int iNextLineDst  = iCanvasWidth;
	int iLastDstNum   = min(iTextWidth, iCanvasWidth - x);
	if(iLastDstNum <= 0){
		return false; // 描画範囲外
	}
	auto iBaseFontHeight = m_BaseFont.Height();
	if (iBaseFontHeight + y < 0) {
		return false;
	}
	int iBaseHeight = min(iBaseFontHeight, iCanvasHeight - y);
	if(iBaseHeight <= 0){
		return false;
	}
	CGrMonoBitmap *lpWorkBitmap;
	// キャッシュチェック
	bool bCache = getCache(str, len, &lpWorkBitmap);
	// 作業用のビットマップ作成
	createWorkDC();
	// 作業用画面に描画
	if(!lpWorkBitmap->Create(iTmpWidth, iTmpHeight)){ return false; }
	if(!bCache){ if(!drawText(str, len, lpWorkBitmap)){ return false; } }
	//
	auto lpCanvasBits = canvas.GetBits();
	//
	auto lpTmpBits = lpWorkBitmap->GetBits();
	auto lpLastSrc = lpTmpBits + iNextLineSrc * iBaseHeight;
	auto lpNextSrc = lpTmpBits;
	if(y < 0){
		lpNextSrc -= (y % iTmpHeight) * iNextLineSrc;
		y = 0;
	}
	if(x < 0){
		iLastDstNum += x;
		x = -(x % iTextWidth);
		lpNextSrc += x / 2;
		if(x & 0x01){
			// 左端数有り
			auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + y * iCanvasWidth;
			auto lpSrc = lpNextSrc;
			for(; lpSrc < lpLastSrc; lpSrc += iNextLineSrc, lpDst += iNextLineDst){
				LCD_INIT_ODD;
				LCD_SET_COLOR(LCD_ODD_R, Red  );
				LCD_SET_COLOR(LCD_ODD_G, Green);
				LCD_SET_COLOR(LCD_ODD_B, Blue );
			}
			x = 1;
			++lpNextSrc;
			--iLastDstNum;
		} else {
			x = 0;
		}
	}
	if(iLastDstNum & 0x01){
		// 右端数有り
		auto iLastSrc = (iLastDstNum % iTextWidth) / 2;
		auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + iLastDstNum - 1 + y * iCanvasWidth;
		auto lpSrc = lpNextSrc + iLastSrc;
		for(; lpSrc < lpLastSrc; lpSrc += iNextLineSrc, lpDst += iNextLineDst){
			LCD_INIT_EVEN;
			LCD_SET_COLOR(LCD_EVEN_R, Red  );
			LCD_SET_COLOR(LCD_EVEN_G, Green);
			LCD_SET_COLOR(LCD_EVEN_B, Blue );
		}
		--iLastDstNum;
	}
	auto lpNextDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + y * iCanvasWidth;
	for(;lpNextSrc < lpLastSrc; lpNextSrc += iNextLineSrc, lpNextDst += iNextLineDst){
		auto lpSrc  = lpNextSrc;
		auto lpDst  = lpNextDst;
		auto lpEnd  = lpDst+iLastDstNum;
		for(; lpDst < lpEnd; ++lpDst, ++lpSrc){
			LCD_INIT_ODD;
			LCD_SET_COLOR(LCD_EVEN_R, Red  );
			LCD_SET_COLOR(LCD_EVEN_G, Green);
			LCD_SET_COLOR(LCD_EVEN_B, Blue );
			++lpDst;
			LCD_SET_COLOR(LCD_ODD_R, Red  );
			LCD_SET_COLOR(LCD_ODD_G, Green);
			LCD_SET_COLOR(LCD_ODD_B, Blue );
		}
	}
	return true;
}

void CGrCharLCDRenderer::PatternFillVert(CGrBitmap &canvas, int x, int y, int width, int height, LPCTSTR str, int len)
{
	// 裏画面用ビットマップ サイズ取得
	int iBgWidth  = (m_iWidth *2  + 31) & (-32); // (m_iWidth *2  + 31) / 32 * 32;
	int iBgHeight = (m_iHeight    + 31) & (-32); // (m_iHeight    + 31) / 32 * 32;
	// 作業用のビットマップサイズを取得
	int iTmpWidth   = (iBgWidth  * 4 + 31) & (-32); // (iBgWidth  * 4 + 31) / 32 * 32;
	int iTmpHeight  = (iBgHeight * 4 + 31) & (-32); // (iBgHeight * 4 + 31) / 32 * 32;
	int iTextWidth  = m_iWidth ;
	int iTextHeight = m_iHeight;
	//
	int iNextLineSrc = iTmpWidth/2;  // (iTmpWidth / 8) Bytes * 8 Lines  <- (iTmpWidth / 8) Bytes <- iTmpWidth bits
	//
	int nextSrcNum  = iTmpWidth/8 ; // (iTmpWidth / 8) Bytes <- iTmpWidth bits
	int nextSrcNum1 = nextSrcNum*1;
	int nextSrcNum2 = nextSrcNum*2;
	int nextSrcNum3 = nextSrcNum*3;
	int nextSrcNum4 = nextSrcNum*4;
	//
	int iCanvasWidth  = canvas.Width ();
	int iCanvasHeight = canvas.Height();
	if(y + height > iCanvasHeight){
		height = iCanvasHeight - y;
	}
	if(x + width > iCanvasWidth){
		width = iCanvasWidth - x;
	}
	if(height <= 0 || width <= 0){ return; }
	if(x > iCanvasWidth || y > iCanvasHeight || x + iTextWidth < 0 || y + iTmpWidth < 0){ return; }
	//
	int iNextLineDst  = iCanvasWidth;
	int iLastDstNum   = min(iTextWidth, iCanvasWidth - x);
	if(iLastDstNum <= 0){
		return; // 描画範囲外
	}
	int iBaseHeight = min(m_BaseFont.Height(), iCanvasHeight - y);
	if(iBaseHeight <= 0){
		return;
	}
	CGrMonoBitmap *lpWorkBitmap;
	// キャッシュチェック
	bool bCache = getCache(str, len, &lpWorkBitmap);
	// 作業用のビットマップ作成
	createWorkDC();
	// 作業用画面に描画
	if(!lpWorkBitmap->Create(iTmpWidth, iTmpHeight)){ return; }
	if(!bCache){ drawText(str, len, lpWorkBitmap); }
	//
	auto lpCanvasBits = canvas.GetBits();
	//
	auto lpTmpBits = lpWorkBitmap->GetBits();
	auto lpLastSrc = lpTmpBits + iNextLineSrc * (iBaseHeight-1);
	auto lpNextSrc = lpTmpBits;
	if(y < 0){
		lpNextSrc -= (y % iBaseHeight) * iNextLineSrc;
		height += y;
		y = 0;
	}
	auto lpNextTopSrc = lpTmpBits + iNextLineSrc; // 一ビットかぶせる
	if(x < 0){
		iLastDstNum += x;
		x = -(x % iTextWidth);
		lpNextSrc += x / 2;
		lpNextTopSrc += x / 2;
		if(x & 0x01){
			// 左端数有り
			auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + y * iCanvasWidth;
			auto lpLastDst = lpDst + height * iCanvasWidth;
			auto lpSrc = lpNextSrc;
			do {
				for(; lpSrc < lpLastSrc && lpDst < lpLastDst; lpSrc += iNextLineSrc, lpDst += iNextLineDst){
					LCD_INIT_ODD;
					LCD_SET_COLOR(LCD_ODD_R, Red  );
					LCD_SET_COLOR(LCD_ODD_G, Green);
					LCD_SET_COLOR(LCD_ODD_B, Blue );
				}
				lpSrc = lpNextTopSrc;
			} while(lpDst < lpLastDst);
			++lpNextSrc;
			++lpNextTopSrc;
			--iLastDstNum;
			x = 1;
		} else {
			x = 0;
		}
	}
	if(iLastDstNum & 0x01){
		// 右端数有り
		auto iLastSrc = (iLastDstNum % iTextWidth) / 2;
		auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + iLastDstNum - 1 + y * iCanvasWidth;
		auto lpLastDst = lpDst + height * iCanvasWidth;
		auto lpSrc = lpNextSrc + iLastSrc;
		do {
			for(; lpSrc < lpLastSrc && lpDst < lpLastDst; lpSrc += iNextLineSrc, lpDst += iNextLineDst){
				LCD_INIT_EVEN;
				LCD_SET_COLOR(LCD_EVEN_R, Red  );
				LCD_SET_COLOR(LCD_EVEN_G, Green);
				LCD_SET_COLOR(LCD_EVEN_B, Blue );
			}
			lpSrc = lpNextTopSrc + iLastSrc;
		} while(lpDst < lpLastDst);
		--iLastDstNum;
	}
	auto lpNextDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + y * iCanvasWidth;
	auto lpLastDst = lpNextDst + height * iCanvasWidth;
	do {
		for(; lpNextSrc < lpLastSrc && lpNextDst < lpLastDst; lpNextSrc += iNextLineSrc, lpNextDst += iNextLineDst){
			auto lpSrc = lpNextSrc;
			auto lpDst = lpNextDst;
			auto lpEnd = lpDst+iLastDstNum;
			for(; lpDst < lpEnd; ++lpDst, ++lpSrc){
				LCD_INIT_ODD;
				LCD_SET_COLOR(LCD_EVEN_R, Red  );
				LCD_SET_COLOR(LCD_EVEN_G, Green);
				LCD_SET_COLOR(LCD_EVEN_B, Blue );
				++lpDst;
				LCD_SET_COLOR(LCD_ODD_R, Red  );
				LCD_SET_COLOR(LCD_ODD_G, Green);
				LCD_SET_COLOR(LCD_ODD_B, Blue );
			}
		}
		lpNextSrc = lpNextTopSrc;
	} while(lpNextDst < lpLastDst);
}
void CGrCharLCDRenderer::PatternFillHorz(CGrBitmap& canvas, int x, int y, int width, int height, LPCTSTR str, int len)
{
	// 裏画面用ビットマップ サイズ取得
	int iBgWidth = (m_iWidth * 2 + 31) & (-32); // (m_iWidth *2  + 31) / 32 * 32;
	int iBgHeight = (m_iHeight + 31) & (-32); // (m_iHeight    + 31) / 32 * 32;
	// 作業用のビットマップサイズを取得
	int iTmpWidth = (iBgWidth * 4 + 31) & (-32); // (iBgWidth  * 4 + 31) / 32 * 32;
	int iTmpHeight = (iBgHeight * 4 + 31) & (-32); // (iBgHeight * 4 + 31) / 32 * 32;
	int iTextWidth = m_iWidth;
	int iTextHeight = m_iHeight;
	//
	int iNextLineSrc = iTmpWidth / 2;  // (iTmpWidth / 8) Bytes * 8 Lines  <- (iTmpWidth / 8) Bytes <- iTmpWidth bits
	//
	int nextSrcNum = iTmpWidth / 8; // (iTmpWidth / 8) Bytes <- iTmpWidth bits
	int nextSrcNum1 = nextSrcNum * 1;
	int nextSrcNum2 = nextSrcNum * 2;
	int nextSrcNum3 = nextSrcNum * 3;
	int nextSrcNum4 = nextSrcNum * 4;
	//
	int iCanvasWidth = canvas.Width();
	int iCanvasHeight = canvas.Height();
	if (y + height > iCanvasHeight) {
		height = iCanvasHeight - y;
	}
	if (x + width > iCanvasWidth) {
		width = iCanvasWidth - x;
	}
	if (height <= 0 || width <= 0) { return; }
	if (x > iCanvasWidth || y > iCanvasHeight || x + width < 0 || y + iTmpHeight < 0) { return; }
	//
	int iNextLineDst = iCanvasWidth;
	int iLastDstNum = min(width, iCanvasWidth - x);
	if (iLastDstNum <= 0) {
		return; // 描画範囲外
	}
	auto iBaseFontHeight = m_BaseFont.Height();
	if (iBaseFontHeight + y < 0) {
		return;
	}
	int iBaseHeight = min(iBaseFontHeight, iCanvasHeight - y);
	if (iBaseHeight <= 0) {
		return;
	}
	CGrMonoBitmap* lpWorkBitmap;
	// キャッシュチェック
	bool bCache = getCache(str, len, &lpWorkBitmap);
	// 作業用のビットマップ作成
	createWorkDC();
	// 作業用画面に描画
	if (!lpWorkBitmap->Create(iTmpWidth, iTmpHeight)) { return; }
	if (!bCache) { drawText(str, len, lpWorkBitmap); }
	//
	auto lpCanvasBits = canvas.GetBits();
	//
	auto lpTmpBits = lpWorkBitmap->GetBits();
	auto lpLastSrc = lpTmpBits + iNextLineSrc * iBaseHeight;
	auto lpNextSrc = lpTmpBits;
	if (y < 0) {
		lpNextSrc -= (y % iBaseHeight) * iNextLineSrc;
		height += y;
		y = 0;
	}
	iTextWidth -= 2; // 一ビットかぶせる
	auto lpNextTopSrc = lpNextSrc;
	auto iStartWidthDst = iTextWidth;
	if (x < 0) {
		iLastDstNum += x;
		x = -(x % iTextWidth);
		lpNextSrc += x / 2;
		iStartWidthDst = x;
		if (x & 0x01) {
			// 左端数有り
			auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + y * iCanvasWidth;
			auto lpLastDst = lpDst + height * iCanvasWidth;
			auto lpSrc = lpNextSrc;
			for (; lpSrc < lpLastSrc && lpDst < lpLastDst; lpSrc += iNextLineSrc, lpDst += iNextLineDst) {
				LCD_INIT_ODD;
				LCD_SET_COLOR(LCD_ODD_R, Red);
				LCD_SET_COLOR(LCD_ODD_G, Green);
				LCD_SET_COLOR(LCD_ODD_B, Blue);
			}
			++lpNextSrc;
			--iLastDstNum;
			x = 1;
			++iStartWidthDst;
		}
		else {
			x = 0;
		}
	}
	if (x + width % iTextWidth) {
		// 右端数有り
		auto iLastSrc = ((iStartWidthDst + iLastDstNum) % iTextWidth) / 2;
		auto lpDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + iLastDstNum - 1 + y * iCanvasWidth;
		auto lpLastDst = lpDst + height * iCanvasWidth;
		auto lpSrc = lpNextTopSrc + iLastSrc;
		for (; lpSrc < lpLastSrc && lpDst < lpLastDst; lpSrc += iNextLineSrc, lpDst += iNextLineDst) {
			LCD_INIT_EVEN;
			LCD_SET_COLOR(LCD_EVEN_R, Red);
			LCD_SET_COLOR(LCD_EVEN_G, Green);
			LCD_SET_COLOR(LCD_EVEN_B, Blue);
		}
		--iLastDstNum;
	}
	auto lpNextDst = reinterpret_cast<LPDWORD>(lpCanvasBits) + x + y * iCanvasWidth;
	auto lpLastDst = lpNextDst + height * iCanvasWidth;
	auto lpNextTopDst = lpNextDst;
	auto lpLastTopDst = lpNextTopDst + iLastDstNum;
	auto lpLineLastDst = lpLastTopDst;
	auto iWidthDst = iTextWidth - iStartWidthDst;
	do {
		auto lpLineLastDst = lpLastTopDst;
		for (; lpNextSrc < lpLastSrc && lpNextDst < lpLastDst; lpNextSrc += iNextLineSrc, lpNextDst += iNextLineDst, lpLineLastDst += iNextLineDst) {
			auto lpSrc = lpNextSrc;
			auto lpDst = lpNextDst;
			auto lpEnd = lpDst + iWidthDst;
			for (; lpDst < lpEnd && lpDst < lpLineLastDst; ++lpDst, ++lpSrc) {
				LCD_INIT_ODD;
				LCD_SET_COLOR(LCD_EVEN_R, Red);
				LCD_SET_COLOR(LCD_EVEN_G, Green);
				LCD_SET_COLOR(LCD_EVEN_B, Blue);
				++lpDst;
				LCD_SET_COLOR(LCD_ODD_R, Red);
				LCD_SET_COLOR(LCD_ODD_G, Green);
				LCD_SET_COLOR(LCD_ODD_B, Blue);
			}
		}
		lpNextSrc = lpNextTopSrc;
		lpNextTopDst += iWidthDst;
		lpNextDst = lpNextTopDst;
		iWidthDst = iTextWidth;
	} while (lpNextDst < lpLastTopDst);
}

void CGrCharLCDRenderer::makeColorMap()
{
	int maxNum = sizeof(m_ColorMap)/sizeof(BYTE);
	double gamma = 1.1f;
	double dMax = pow(255.0, gamma) / 255.0;

	for(int idx=0; idx<maxNum; ++idx){
		m_ColorMap[maxNum-idx-1] = static_cast<BYTE>(max(0, min(255, static_cast<int>(pow(static_cast<double>(255)*(idx)/16, gamma) / dMax + 0.5))));
	}
}

void CGrCharLCDRenderer::SetTextColor(COLORREF color)
{
	// テキストカラーマップ作成
	auto textColorR = GetRValue(color);
	auto textColorG = GetGValue(color);
	auto textColorB = GetBValue(color);
	RGBQUAD textColor = {0};
	textColor.rgbBlue  = textColorB;
	textColor.rgbGreen = textColorG;
	textColor.rgbRed   = textColorR;
	m_TextColor = textColor;
	auto col = *reinterpret_cast<LPDWORD>(&textColor);
	auto col_odd  = col & ODD ;
	auto col_even = col & EVEN;
	int alpha_r = 256;
	for(int alpha=0; alpha<sizeof(m_TextColorMapODD)/sizeof(DWORD); ++alpha, --alpha_r){
		m_TextColorMapODD [alpha] = col_odd  * alpha_r;
		m_TextColorMapEVEN[alpha] = col_even * alpha_r;
		m_iRed  [alpha] = textColorR * alpha_r;
		m_iGreen[alpha] = textColorG * alpha_r;
		m_iBlue [alpha] = textColorB * alpha_r;
	}
}

//////////////////////////////////////////

CGrCharExtRenderer::CGrCharExtRenderer(){
	//CGrCharRenderer
	m_fileName = _T("TxtFuncCharFTRenderer.dll");
	void *(*pCreate)();
	SetProcPtr(pCreate, GetProcAddress("TxtFuncCreateCharRenderer"));
	if(pCreate){
		m_pCharRenderer = static_cast<CGrCharRenderer*>(pCreate());
	}
}
CGrCharExtRenderer::~CGrCharExtRenderer(){
	Unload();
}
void CGrCharExtRenderer::Unload()
{
	if(m_hDLL){
		if(m_pCharRenderer){
			void (*pDelete)(void*);
			SetProcPtr(pDelete, GetProcAddress("TxtFuncDeleteCharRenderer"));
			if(pDelete){
				pDelete(m_pCharRenderer);
			}
		}
		FreeLibrary(m_hDLL);
	}
	m_hDLL = NULL;
}

bool CGrCharExtRenderer::Draw(CGrBitmap &canvas, int x, int y, LPCTSTR str, int len)
{
	if(m_pCharRenderer){
		return m_pCharRenderer->Draw(canvas, x, y, str, len);
	}
	return false;
}

void CGrCharExtRenderer::PatternFillVert(CGrBitmap& canvas, int x, int y, int width, int height, LPCTSTR str, int len)
{
	if (m_pCharRenderer) {
		return m_pCharRenderer->PatternFillVert(canvas, x, y, width, height, str, len);
	}
}
void CGrCharExtRenderer::PatternFillHorz(CGrBitmap& canvas, int x, int y, int width, int height, LPCTSTR str, int len)
{
	if (m_pCharRenderer) {
		return m_pCharRenderer->PatternFillHorz(canvas, x, y, width, height, str, len);
	}
}
bool CGrCharExtRenderer::SetFont(CGrFont &font)
{
	if(m_pCharRenderer){
		return m_pCharRenderer->SetFont(font);
	}
	return true;
}
CGrFont &CGrCharExtRenderer::GetFont()
{
	if(m_pCharRenderer){
		return m_pCharRenderer->GetFont();
	}
	return m_defFont;
}

void CGrCharExtRenderer::SetTextColor(COLORREF color)
{
	if(m_pCharRenderer){
		m_pCharRenderer->SetTextColor(color);
	}
}

COLORREF CGrCharExtRenderer::GetTextColor()
{
	if(m_pCharRenderer){
		return m_pCharRenderer->GetTextColor();
	}
	return RGB(0,0,0);
}

