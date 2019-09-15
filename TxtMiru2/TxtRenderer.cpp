#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include "TxtRenderer.h"
#include "stltchar.h"
#include "TxtDocument.h"
#include "TxtMiru.h"
#include "Shell.h"
#include "Image.h"

//#define __DBG__
#include "Debug.h"

CGrTxtRendererMgr::CGrTxtRendererMgr()
{
}

CGrTxtRendererMgr::~CGrTxtRendererMgr()
{
	Clear();
}

void CGrTxtRendererMgr::ClearCache()
{
	for(auto &&item : m_bitmapMapCache){
		auto p = item.second;
		if(p){
			delete p;
		}
	}
	m_bitmapMapCache.clear();
}

void CGrTxtRendererMgr::ClearPict()
{
	for(auto &&item : m_pictRendererMap){
		if(item){ delete item; }
		item = nullptr;
	}
	m_pictFileRendererMap.clear();
	ClearCache();
}

void CGrTxtRendererMgr::ClearChar()
{
	for(auto &&item : m_charRendererMap){
		if(item){ delete item; }
		item = nullptr;
	}
}

void CGrTxtRendererMgr::ClearFlexChar()
{
	m_iFontSize = 0;
	for(auto &&item : m_flexCharRendererMap){
		if(item){ delete item; }
		item = nullptr;
	}
}

void CGrTxtRendererMgr::Clear()
{
	ClearPict();
	ClearChar();
	ClearFlexChar();
}

#define QRENDER_FONT_SIZE8  17
// フォントサイズに応じた最適な CharRenderer を選択
CGrCharRenderer &CGrTxtRendererMgr::selectCharRenderer(int font_size, CGrTxtRendererMgr::LPCGrCharRenderer *pcr)
{
	if(*pcr){
		if(m_iAntialias == 2){
			if(_tcscmp((*pcr)->Name(), CharRendererName_LCDR) == 0){
				return **pcr;
			}
		} else if(m_iAntialias == 3){
			if(_tcscmp((*pcr)->Name(), CharRendererName_Ext) == 0){
				return **pcr;
			}
		} else if(font_size > QRENDER_FONT_SIZE8){
			if(_tcscmp((*pcr)->Name(), CharRendererName_4NNR) == 0){
				return **pcr;
			}
		} else {
			if(_tcscmp((*pcr)->Name(), CharRendererName_8NNR) == 0){
				return **pcr;
			}
		}
		delete *pcr;
	}
	if(m_iAntialias == 2){
		*pcr = new CGrCharLCDRenderer();
	} else if(m_iAntialias == 3){
		*pcr = new CGrCharExtRenderer();
	} else if(font_size > QRENDER_FONT_SIZE8){
		*pcr = new CGrChar4NNRenderer();
	} else {
		*pcr = new CGrChar8NNRenderer();
	}
	return **pcr;
}

void CGrTxtRendererMgr::createCharRenderer(FontType ft, CGrTxtParam &param, CGrTxtParam::CharType ct, int size, int iRotate)
{
	try {
		if(size <= 0){
			if(m_currentCharRendererMap[ft]){
				delete m_currentCharRendererMap[ft];
				m_currentCharRendererMap[ft] = nullptr;
			}
			return;
		}
		auto &&cr = selectCharRenderer(size, &m_currentCharRendererMap[ft]);
		LOGFONT logFont = {
			-12, 0,
			FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
			SHIFTJIS_CHARSET, OUT_STRING_PRECIS, CLIP_STROKE_PRECIS,
			DRAFT_QUALITY, FIXED_PITCH, _T("@ＭＳ 明朝")
			};
		_tcscpy_s(logFont.lfFaceName, param.GetFontName(ct));
		logFont.lfHeight = size;
		logFont.lfEscapement  = iRotate;
		logFont.lfOrientation = iRotate;
		logFont.lfWeight      = param.GetFontWeight(ct);
		logFont.lfClipPrecision |= CLIP_LH_ANGLES;
		CGrFont font;
		font.CreateFontIndirect(logFont);
		cr.SetTextColor(param.GetColor(ct));
		cr.SetFont(font);
		TCHAR buf[512] = {0};
		if(iRotate == 2700){
			_stprintf_s(buf, _T("%g"), param.GetFontSpacing(ct));
			cr.SetParam(_T("FontSpacing"), buf);
		}
		_stprintf_s(buf, _T("%d"), param.GetFontCentering(ct));
		cr.SetParam(_T("FontCentering"), buf);
		cr.SetParam(_T("CurDir"), m_curDir.c_str());
		cr.SetParam(_T("DataDir"), CGrTxtMiru::GetDataPath());
	} catch(error_code er){
		throw er;
	}
}
void CGrTxtRendererMgr::createFlexCharRenderer(FontType ft, CGrTxtParam &param, CGrTxtParam::CharType ct, int size, int iRotate)
{
}

void CGrTxtRendererMgr::SetTextColor(FontType ft, COLORREF color)
{
	auto pcr = m_currentCharRendererMap[ft];
	if(pcr){
		pcr->SetTextColor(color);
	}
}
COLORREF CGrTxtRendererMgr::GetTextColor(FontType ft)
{
	auto pcr = m_currentCharRendererMap[ft];
	if(pcr){
		return pcr->GetTextColor();
	}
	return {};
}

void CGrTxtRendererMgr::SetTextColor(COLORREF color)
{
	if(GetTextColor(Text              ) != color){ SetTextColor(Text              , color); }
	if(GetTextColor(HalfText          ) != color){ SetTextColor(HalfText          , color); }
	if(GetTextColor(RotateText        ) != color){ SetTextColor(RotateText        , color); }
	if(GetTextColor(HalfRotateText    ) != color){ SetTextColor(HalfRotateText    , color); }
	if(GetTextColor(TurnText          ) != color){ SetTextColor(TurnText          , color); }
	if(GetTextColor(HalfTurnText      ) != color){ SetTextColor(HalfTurnText      , color); }
	if(GetTextColor(BoldText          ) != color){ SetTextColor(BoldText          , color); }
	if(GetTextColor(BoldHalfText      ) != color){ SetTextColor(BoldHalfText      , color); }
	if(GetTextColor(BoldRotateText    ) != color){ SetTextColor(BoldRotateText    , color); }
	if(GetTextColor(BoldHalfRotateText) != color){ SetTextColor(BoldHalfRotateText, color); }
	if(GetTextColor(BoldTurnText      ) != color){ SetTextColor(BoldTurnText      , color); }
	if(GetTextColor(BoldHalfTurnText  ) != color){ SetTextColor(BoldHalfTurnText  , color); }
}
void CGrTxtRendererMgr::ResetTextColor()
{
	auto &&param = CGrTxtMiru::theApp().Param();
	SetTextColor(param.GetColor(CGrTxtParam::Text));
}

void CGrTxtRendererMgr::SetFontSize(int size, int text_size, int ruby_size, int note_size, int nombre_size, int running_heads_size)
{
	if(size == 0){
		if(m_charRendererMap != m_currentCharRendererMap){
			m_currentCharRendererMap = m_charRendererMap;
			auto &&param = CGrTxtMiru::theApp().Param();
			SetTextColor(param.GetColor(CGrTxtParam::Text));
		}
	} else {
		if (m_flexCharRendererMap != m_currentCharRendererMap) {
			m_currentCharRendererMap = m_flexCharRendererMap;
		}
		if(m_iFontSize != size){
			auto &&param = CGrTxtMiru::theApp().Param();
			ClearFlexChar();
			m_iFontSize = size;
			createCharRenderer(text_size, ruby_size, note_size, nombre_size, running_heads_size);
			SetTextColor(param.GetColor(CGrTxtParam::Text));
		}
	}
}

void CGrTxtRendererMgr::createCharRenderer(int text_size, int ruby_size, int note_size, int nombre_size, int running_heads_size)
{
	auto &&param = CGrTxtMiru::theApp().Param();
	int half_text_size = text_size / 2;
	createCharRenderer(Text              , param, CGrTxtParam::Text        , text_size         , 2700/*, false*/);
	createCharRenderer(HalfText          , param, CGrTxtParam::Text        , half_text_size    , 2700/*, false*/);
	createCharRenderer(RotateText        , param, CGrTxtParam::Text        , text_size         ,    0/*, false*/);
	createCharRenderer(HalfRotateText    , param, CGrTxtParam::Text        , half_text_size    ,    0/*, false*/);
	createCharRenderer(TurnText          , param, CGrTxtParam::Text        , text_size         , 1800/*, false*/);
	createCharRenderer(HalfTurnText      , param, CGrTxtParam::Text        , half_text_size    , 1800/*, false*/);
	createCharRenderer(BoldText          , param, CGrTxtParam::Bold        , text_size         , 2700/*, true */);
	createCharRenderer(BoldHalfText      , param, CGrTxtParam::Bold        , half_text_size    , 2700/*, true */);
	createCharRenderer(BoldRotateText    , param, CGrTxtParam::Bold        , text_size         ,    0/*, true */);
	createCharRenderer(BoldHalfRotateText, param, CGrTxtParam::Bold        , half_text_size    ,    0/*, true */);
	createCharRenderer(BoldTurnText      , param, CGrTxtParam::Bold        , text_size         , 1800/*, true */);
	createCharRenderer(BoldHalfTurnText  , param, CGrTxtParam::Bold        , half_text_size    , 1800/*, true */);
	createCharRenderer(RubyText          , param, CGrTxtParam::Ruby        , ruby_size         , 2700/*, false*/);
	createCharRenderer(TurnRubyText      , param, CGrTxtParam::Ruby        , ruby_size         , 1800/*, false*/);
	createCharRenderer(Note              , param, CGrTxtParam::Note        , note_size         , 2700/*, false*/);
	createCharRenderer(Nombre            , param, CGrTxtParam::Nombre      , nombre_size       ,    0/*, false*/);
	createCharRenderer(RunningHeads      , param, CGrTxtParam::RunningHeads, running_heads_size,    0/*, false*/);
}

//
void CGrTxtRendererMgr::Create(CGrTxtDocument &doc, int text_size, int ruby_size, int note_size, int nombre_size, int running_heads_size)
{
	// カレントディレクトリ設定(テキストファイルのパスに設定)
	doc.GetCurDir(m_curDir);
	//
	createCharRenderer(text_size, ruby_size, note_size, nombre_size, running_heads_size);
	// キャッシュ候補設定
	auto pcr_text = m_currentCharRendererMap[Text];
	if(pcr_text){
		// ★サイズが大きいと時間がかかる
		std::tstring str;
		doc.GetUsedTopN(str, 30);
		pcr_text->SetCacheCharList(str.c_str());
	}
}

void CGrTxtRendererMgr::SetBitmap(CGrBitmap &bmp)
{
	m_pBmpCanvas = &bmp;
}

bool CGrTxtRendererMgr::DrawText(FontType ft, int x, int y, LPCTSTR lpSrc, LPCTSTR lpSrcEnd)
{
	return DrawText(ft, x, y, lpSrc, lpSrcEnd-lpSrc);
}

bool CGrTxtRendererMgr::DrawText(FontType ft, int x, int y, LPCTSTR lpSrc, int len)
{
	auto pcr = m_currentCharRendererMap[ft];
	if(pcr){
		return pcr->Draw(*m_pBmpCanvas, x, y, lpSrc, len);
	}
	return false;
}

void CGrTxtRendererMgr::PatternFill(FontType ft, int x, int y, int w, int h, LPCTSTR lpSrc, int len)
{
	auto pcr = m_currentCharRendererMap[ft];
	if(pcr){
		pcr->PatternFill(*m_pBmpCanvas, x, y, w, h, lpSrc, len);
	}
}

int CGrTxtRendererMgr::GetFontHeight(FontType ft)
{
	auto pcr = m_currentCharRendererMap[ft];
	if(pcr){
		return pcr->GetFont().Height();
	}
	return 1;
}

void CGrTxtRendererMgr::Initialize(CGrTxtDocument &doc)
{
	ClearPict();
	doc.GetCurDir(m_curDir);
	if(m_curDir.empty()){
		m_curDir = CGrTxtMiru::GetDataPath();
	}
	const auto &param = CGrTxtMiru::theApp().Param();
	m_pictRendererMap[PRT_Spi] = new CGrPictSPIRenderer();
	m_pictRendererMap[PRT_Ole] = new CGrPictOleRenderer();
	m_pictRendererMap[PRT_Emf] = new CGrPictEmfRenderer();
	//
	std::tstring dir;
	param.GetText(CGrTxtParam::SpiPluginFolder, dir);
	m_pictRendererMap[PRT_Spi]->SetParam(_T("PluginDir"), dir.c_str());
	//
	for(auto &&item : m_pictRendererMap){
		if(item){
			item->SetParam(_T("CurDir"), m_curDir.c_str());
			item->SetParam(_T("DataDir"), CGrTxtMiru::GetDataPath());
		}
	}
}

CGrPictRenderer *CGrTxtRendererMgr::getPictRenderer(LPCTSTR lpFileName)
{
	for(auto &&item : m_pictRendererMap){
		if(!item){ continue; }
		item->SetParam(_T("CurDir"), m_curDir.c_str());
		if(item->IsSupported(lpFileName)){
			m_pictFileRendererMap[lpFileName] = item;
			return item;
		}
	}
	return nullptr;
}

bool CGrTxtRendererMgr::drawCacheBlt(int x, int y, int w, int h, LPCTSTR fileName)
{
	auto it = m_bitmapMapCache.find(fileName);
	if(it != m_bitmapMapCache.end()){
		auto pcb = it->second;
		if(pcb && pcb->size.cx == w && pcb->size.cy == h){
			if(pcb->bmp){
				x += w / 2 - pcb->bmp.Width()  / 2;
				y += h / 2 - pcb->bmp.Height() / 2;
				CGrImage::AlphaBitBlt(*m_pBmpCanvas, x, y, pcb->bmp.Width(), pcb->bmp.Height(), pcb->bmp, 0, 0);
				++pcb->iRefCount;
			}
			return true;
		}
	}
	return false;
}

#include <vector>
#define MAX_CACHE_SIZE 4

static bool compare(const CGrTxtRendererMgr::CacheBitmapMap::const_iterator& l, const CGrTxtRendererMgr::CacheBitmapMap::const_iterator& r){
	auto lbmp = l->second;
	auto rbmp = r->second;
	int iRefCountl = -1;
	if(lbmp){
		iRefCountl = lbmp->iRefCount;
	}
	int iRefCountr = -1;
	if(rbmp){
		iRefCountr = rbmp->iRefCount;
	}
	return (iRefCountl < iRefCountr);
}
static void removeOldCache(CGrTxtRendererMgr::CacheBitmapMap &cbm, int n)
{
	int remove_size = cbm.size() - n;
	if(remove_size <= 0){
		return;
	}
	std::vector<CGrTxtRendererMgr::CacheBitmapMap::const_iterator> f;
	auto hitit=cbm.begin(), hite=cbm.end();
	for(;hitit != hite; ++hitit){
		f.push_back(hitit);
	}
	std::partial_sort(f.begin(), f.begin()+remove_size, f.end(), compare); // 使用頻度でソート
	auto *fit = &f[0];
	for(;remove_size>0; --remove_size, ++fit){
		const auto &str = (*fit)->first;
		auto it = cbm.find(str);
		if(it != cbm.end()){
			auto p = it->second;
			if(p){
				delete p;
			}
			cbm.erase(str);
		}
	}
}
void CGrTxtRendererMgr::clearOldCache()
{
	if(m_bitmapMapCache.size() > MAX_CACHE_SIZE){
		removeOldCache(m_bitmapMapCache, MAX_CACHE_SIZE);
	}
}

SIZE CGrTxtRendererMgr::GetCachePictSize(LPCTSTR lpFileName)
{
	SIZE size = {-1,-1};
	auto it = m_bitmapMapCache.find(lpFileName);
	if(it != m_bitmapMapCache.end()){
		auto pcb = it->second;
		if(pcb &&pcb->bmp){
			size.cx = pcb->bmp.Width();
			size.cy = pcb->bmp.Height();
		}
	}
	return size;
}

void CGrTxtRendererMgr::DrawPict(int x, int y, int w, int h, LPCTSTR lpFileName)
{
	if(drawCacheBlt(x, y, w, h, lpFileName)){
		return;
	}
	clearOldCache();
	auto ppr = getPictRenderer(lpFileName);
	if(ppr){
		LPCacheBitmap pcb = nullptr;
		auto it = m_bitmapMapCache.find(lpFileName);
		if(it != m_bitmapMapCache.end()){
			pcb = it->second;
		} else {
			pcb = new CacheBitmap();
			m_bitmapMapCache[lpFileName] = pcb;
		}
		pcb->size.cx = w;
		pcb->size.cy = h;
		pcb->bmp.Destroy();
		ppr->Draw(pcb->bmp, 0, 0, w, h, lpFileName);
		drawCacheBlt(x, y, w, h, lpFileName);
	}
}

void CGrTxtRendererMgr::StretchBlt(int x, int y, int w, int h, LPCTSTR lpFileName)
{
	if(drawCacheBlt(x, y, w, h, lpFileName)){
		return;
	}
	clearOldCache();
	auto ppr = getPictRenderer(lpFileName);
	if(ppr){
		LPCacheBitmap pcb = nullptr;
		auto it = m_bitmapMapCache.find(lpFileName);
		if(it != m_bitmapMapCache.end()){
			pcb = it->second;
		} else {
			pcb = new CacheBitmap();
			m_bitmapMapCache[lpFileName] = pcb;
		}
		pcb->size.cx = w;
		pcb->size.cy = h;
		if(pcb->bmp.Create(w, h)){
			ppr->StretchBlt(pcb->bmp, 0, 0, w, h, lpFileName);
		}
		drawCacheBlt(x, y, w, h, lpFileName);
	}
}

void CGrTxtRendererMgr::SetAntialias(int iAntialias)
{
	m_iAntialias = iAntialias;
}
int CGrTxtRendererMgr::GetAntialias()
{
	return m_iAntialias;
}

bool CGrTxtRendererMgr::DrawBorder(int x, int y, int w, int h, BorderType borderType)
{
	if(!m_pBmpCanvas || w <= 0 || h <= 0){
		return false;
	}
	COLORREF color = 0;
	auto pcr = m_currentCharRendererMap[Text];
	if(pcr){
		color = pcr->GetTextColor();
	} else {
		return false;
	}
	RGBQUAD textColor = {0};
	textColor.rgbBlue  = GetBValue(color);
	textColor.rgbGreen = GetGValue(color);
	textColor.rgbRed   = GetRValue(color);
	//
	auto lpCanvasBits = m_pBmpCanvas->GetBits();
	int iCanvasWidth  = m_pBmpCanvas->Width ();
	int iCanvasHeight = m_pBmpCanvas->Height();
	w = min(w, iCanvasWidth - x);
	//
	int left   = x;
	int right  = x + w + 1;
	int top    = y;
	int bottom = y + h;
	if(left >= 0 && bottom < iCanvasHeight){
		if(borderType.top){
			auto lpSrc = lpCanvasBits + left + top * iCanvasWidth;
			for(int dx=left; dx<=right; ++dx){
				*lpSrc = textColor;
				++lpSrc;
			}
		}
		if(borderType.left){
			auto lpSrc = lpCanvasBits + left + top * iCanvasWidth;
			for(int dy=top; dy<bottom; ++dy){
				*lpSrc = textColor;
				lpSrc += iCanvasWidth;
			}
		}
		if(borderType.right){
			auto lpSrc = lpCanvasBits + right + top * iCanvasWidth;
			for(int dy=top; dy<bottom; ++dy){
				*lpSrc = textColor;
				lpSrc += iCanvasWidth;
			}
		}
		if(borderType.bottom){
			auto lpSrc = lpCanvasBits + left + bottom * iCanvasWidth;
			for(int dx=left; dx<=right; ++dx){
				*lpSrc = textColor;
				++lpSrc;
			}
		}
	}
	return true;
}
