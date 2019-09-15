#include "TxtCanvas.h"
#include "resource.h"
#include "TxtMiru.h"
#include "TxtDocument.h"
#include "CurrentDirectory.h"
#include "shell.h"
#include "TxtMiruTextType.h"
//#define __DBG__
#include "Debug.h"

template <typename T> inline void Swap(T &a, T &b) { T tmp = a; a=b; b=tmp; }

static CGrTxtRendererMgr::FontType l_FTRotateText    [] = { CGrTxtRendererMgr::RotateText    , CGrTxtRendererMgr::BoldRotateText     };
static CGrTxtRendererMgr::FontType l_FTTurnText      [] = { CGrTxtRendererMgr::TurnText      , CGrTxtRendererMgr::BoldTurnText       };
static CGrTxtRendererMgr::FontType l_FTText          [] = { CGrTxtRendererMgr::Text          , CGrTxtRendererMgr::BoldText           };
static CGrTxtRendererMgr::FontType l_FTHalfText      [] = { CGrTxtRendererMgr::HalfText      , CGrTxtRendererMgr::BoldHalfText       };
static CGrTxtRendererMgr::FontType l_FTRotateHalfText[] = { CGrTxtRendererMgr::HalfRotateText, CGrTxtRendererMgr::BoldHalfRotateText };
enum ROTATE_TYPE { ROTP_NONE, ROTP_ROTATE, ROTP_REV };
static ROTATE_TYPE getRotateType(const CGrTxtParam &param, LPCTSTR lpChar)
{
	struct CGrRotateStrCheck : public CGrTxtParam::CGrStringCompare
	{
		LPCTSTR lpChar;
		CGrRotateStrCheck(LPCTSTR c) : lpChar(c) {}
		bool operator == (LPCTSTR lpSrc){return _tcscmp(lpChar, lpSrc) == 0;}
	};
	if(param.FindIF(CGrTxtParam::RotateCharacters, static_cast<CGrTxtParam::CGrStringCompare &&>(CGrRotateStrCheck(lpChar))) >= 0){
		return ROTP_ROTATE;
	} else if(param.FindIF(CGrTxtParam::RRotateCharacters, static_cast<CGrTxtParam::CGrStringCompare &&>(CGrRotateStrCheck(lpChar))) >= 0){
		return ROTP_REV;
	}
	return ROTP_NONE;
}

class CGrCalcSize {
	const CGrTxtLayout &layout;
	SIZE paperSize = {};
	SIZE win_size = {};
	int center_x = 0;
	int center_y = 0;
public:
	CGrCalcSize(const CGrTxtLayout &l) : layout(l){}

	bool initialize(const RECT &rect)
	{
		paperSize = layout.GetPaperSize();
		if(paperSize.cx <= 0 || paperSize.cy <= 0){
			return false;
		}
		win_size.cx = rect.right - rect.left;
		win_size.cy = rect.bottom - rect.top;
		center_x = win_size.cx / 2;
		center_y = win_size.cy / 2;
		int h = win_size.cx * paperSize.cy;
		int w = win_size.cy * paperSize.cx;
		if(h < w){
			win_size.cy = h / paperSize.cx;
		} else {
			win_size.cx = w / paperSize.cy;
		}
		return true;
	}
	void getCanvasRect(RECT &rect)
	{
		rect.top    = center_y - win_size.cy / 2;
		rect.bottom = rect.top + win_size.cy;
		rect.left   = center_x - win_size.cx / 2;
		rect.right  = rect.left + win_size.cx;
	}
	void getPaperRect(RECT &rect)
	{
		rect.left   = 0;
		rect.top    = 0;
		rect.right  = paperSize.cx;
		rect.bottom = paperSize.cy;
	}
	int getFontSize(CGrTxtLayout::LineSizeType lst, int iFont){
		const auto &ls = layout.GetLineSize(lst);
		return calcPos(ls.width - ls.space + ((ls.width - ls.space) * iFont / 8));
	}
	int getFontSize(CGrTxtLayout::LineSizeType lst){
		const auto &ls = layout.GetLineSize(lst);
		return calcPos(ls.width - ls.space);
	}
	int calcPos(int pos){
		return (pos * win_size.cx + paperSize.cx - 1) / paperSize.cx;
	}
};

class CGrTxtCanvasDraw {
	CGrTxtMapper &m_txtMap;
	CGrTxtRendererMgr &m_txtRenderer;;
	RECT &m_rectWindow;
	CGrToolTips::TipsInfoList &m_tipsList;
	CGrTxtCanvas::LinkBoxList &m_linkBox;
	int m_startLine = 0;
	int m_endLine = 0;
	int m_fontTextSize = 0;
	int m_iHoverLink = 0;
	CGrTxtDocument *m_pDoc = nullptr;
	RECT &rcCanvas;
	TxtMiru::TextInfoList &til_layout;
	TxtMiru::TextPoint m_tp;
	CGrTxtRendererMgr::FontType *FTRotateText = l_FTRotateText;
	CGrTxtRendererMgr::FontType *FTTurnText   = l_FTTurnText;
	CGrTxtRendererMgr::FontType *FTText       = l_FTText;
	CGrTxtRendererMgr::FontType *FTHalfText   = l_FTHalfText;
	CGrTxtRendererMgr::FontType FTRubyText    = CGrTxtRendererMgr::RubyText;
public:
	CGrTxtCanvasDraw(CGrTxtCanvas *pCanvas, RECT &rcCanvas, TxtMiru::TextInfoList &til_layout) :
	m_txtMap(pCanvas->m_txtMap),
	m_txtRenderer(pCanvas->m_txtRenderer),
	m_rectWindow(pCanvas->m_rectWindow),
	m_tipsList(pCanvas->m_tipsList),
	m_linkBox(pCanvas->m_linkBox),
	m_pDoc(pCanvas->m_pDoc),
	m_startLine(pCanvas->m_startLine),
	m_fontTextSize(pCanvas->m_fontTextSize),
	m_iHoverLink(pCanvas->m_iHoverLink),
	rcCanvas(rcCanvas),
	til_layout(til_layout)
	{
	}
	void SetHorz()
	{
		FTRotateText = l_FTText;
		FTTurnText = l_FTTurnText;
		FTText = l_FTTurnText;
		FTHalfText = l_FTRotateHalfText;
		FTRubyText = CGrTxtRendererMgr::TurnRubyText;
	}
	void UnSetHorz()
	{
		FTRotateText = l_FTRotateText;
		FTTurnText = l_FTTurnText;
		FTText = l_FTText;
		FTHalfText = l_FTHalfText;
		FTRubyText = CGrTxtRendererMgr::RubyText;
	}
	void Draw(int lines)
	{
		const auto &lineList = m_pDoc->GetLineList();
		int ll_len = lineList.size();
		if(ll_len <= 0){
			return;
		}
		const auto *it_li = &lineList[0];
		// skip
		for(; ll_len>0 && it_li->iEndCol < m_startLine; --ll_len, ++it_li){
			++m_tp.iLine;
		}
		//
		const auto &param = CGrTxtMiru::theApp().Param();
		int note_num = 0;
		m_endLine = m_startLine + lines;
		m_txtMap.CreateCharPointMap(*m_pDoc, m_startLine, rcCanvas, til_layout);
		bool bHorz = false;
		for(; ll_len>0; --ll_len, ++it_li, ++m_tp.iLine){
			const auto &li = (*it_li);
			if(li.iStartCol > m_endLine){ // 終了
				break;
			}
			m_tp.iIndex = 0;
			const auto &text_list = li.text_list;
			if(text_list.empty()){ continue; }
			const auto *it=&text_list[0];
			int it_len = text_list.size();
			// skip
			if(li.iStartCol <= m_startLine && m_startLine <= li.iEndCol){
				const auto &tlpTurnList = li.tlpTurnList;
				if(tlpTurnList.empty()){
					continue;
				}
				const auto *it_turn = &tlpTurnList[0];
				int iIndex = 0;
				for(int it_turn_len = tlpTurnList.size(); it_turn_len>0 && it_turn->iLineNoInAll < m_startLine; --it_turn_len, ++it_turn){
					iIndex = it_turn->iIndex;
				}
				it += iIndex;
				it_len -= iIndex;
				m_tp.iIndex = iIndex;
				// フォントサイズの指定 (折り返し時にフォントサイズが入る)
				const auto &textStyleMap = m_pDoc->GetTextStyleMap();
				auto tsm_it = textStyleMap.find(m_tp);
				if(tsm_it != textStyleMap.end()){
					auto style = tsm_it->second;
					setFontSize(style.fontSize);
					if ((style.uStatus & TxtMiru::TTPStatusHorz) == TxtMiru::TTPStatusHorz) {
						bHorz = true;
					}
				}
			}
			for(; it_len>0; --it_len, ++it, ++m_tp.iIndex){
				const auto &ti = (*it);
				m_tp.iPos = 0;

				const auto &text = ti.str;
				switch (ti.textType) {
				case TxtMiru::TT_TEXT_SIZE:
					setFontSize(static_cast<signed short>(ti.chrType));
					break;
				case TxtMiru::TT_HORZ_START:
					bHorz = true;
					break;
				case TxtMiru::TT_HORZ_END:
					bHorz = false;
					break;
				}
				if(text.empty()){ continue; }
				auto lpNextSrc = text.c_str();
				switch(ti.textType){
				case TxtMiru::TT_OTHER             :
					if(drawOthr(lpNextSrc)){
						break;
					}
					++m_tp.iPos;
					// through
				case TxtMiru::TT_NOTE              : // through
				case TxtMiru::TT_ERROR             : // through
				case TxtMiru::TT_OTHER_NOTE        : // through
				case TxtMiru::TT_NO_READ           :
					{
						++note_num;
						std::tstring fmt;
						param.GetText(CGrTxtParam::NoteFormat, fmt);
						TCHAR buf[512];
						_stprintf_s(buf, fmt.c_str(), note_num);
						drawRuby(buf);

						CGrToolTips::TipsInfo tips;
						{
							auto tmp_tp = m_tp;
							tmp_tp.iPos = 0;
							const auto &cp1st = m_txtMap.GetCharPoint(tmp_tp);
							tmp_tp.iPos = m_tp.iPos-1;
							const auto &cp2nd = m_txtMap.GetCharPoint(tmp_tp);
							tips.rect.left   = cp1st.zx;
							tips.rect.top    = cp1st.zy;
							tips.rect.right  = max(cp2nd.zx + cp2nd.w, cp1st.zx + cp1st.w);
							tips.rect.bottom = cp2nd.y + cp2nd.h;
						}
						tips.str = lpNextSrc;
						m_tipsList.push_back(std::move(tips));
						//
						drawNote(buf      );
						drawNote(lpNextSrc);
					}
					break;
				case TxtMiru::TT_LINK              :
					if(ti.tpEnd.iIndex == -1 && ti.tpEnd.iPos == -1){
						COLORREF link_color = {0}; /* paper, shadow, back */
						param.GetPoints(CGrTxtParam::LinkTextColor, (int*)&link_color, 1);
						m_txtRenderer.SetTextColor(link_color);
					} else {
						drawLink(_T("傍線") , ti, text_list);
						m_txtRenderer.ResetTextColor();
					}
					break;
				case TxtMiru::TT_ROTATE_NUM        : // through // 縦中横
				case TxtMiru::TT_ROTATE_NUM_AUTO   : drawRNum(lpNextSrc); break;
				case TxtMiru::TT_TEXT              : // through
				case TxtMiru::TT_OVERLAP_CHAR      :
					if (bHorz && !IgnoreHorzChar(ti.chrType)) {
						SetHorz();
						drawText(lpNextSrc);
						UnSetHorz();
					}
					else {
						drawText(lpNextSrc);
					}
					break;
				case TxtMiru::TT_SMALL_NOTE        : // through
				case TxtMiru::TT_SMALL_NOTE_R      : // through
				case TxtMiru::TT_SMALL_NOTE_L      : // through
				case TxtMiru::TT_GUID_MARK         : // through
				case TxtMiru::TT_SUP_NOTE          : // through
				case TxtMiru::TT_SUB_NOTE          : drawSTxt(lpNextSrc); break;
				case TxtMiru::TT_KU1_CHAR          : drawKuCh(_T("／"  )); break;
				case TxtMiru::TT_KU2_CHAR          : drawKuCh(_T("／″")); break;
				case TxtMiru::TT_LINE_CHAR         :
					{ // 先読みして、線が途切れないようにする
						int to_iIndex = m_tp.iIndex;
						auto prefetch_tp = m_tp;
						const auto &cp = m_txtMap.GetCharPoint(m_tp);
						const auto *pref_it = it;
						for(int prefetch = it_len; prefetch>0; --prefetch, ++pref_it, ++prefetch_tp.iIndex){
							const auto &pref_ti = (*pref_it);
							if(pref_ti.textType != TxtMiru::TT_LINE_CHAR){ break; }
							if(pref_ti.str != lpNextSrc){ break; }
							if(cp.zx != m_txtMap.GetCharPoint(prefetch_tp).zx){ break; }
							to_iIndex = prefetch_tp.iIndex;
						}
						int x=-1, w=0, y=0, h=0;
						if(to_iIndex == m_tp.iIndex){
							const auto &cp = m_txtMap.GetCharPoint(m_tp);
							x = cp.zx;
							y = cp.zy;
							w = cp.w;
							h = cp.h;
							if(cp.w <= 0 || !isDrawArea(cp.lcol)){
								break;
							}
						} else {
							auto ref_tp = m_tp;
							auto end_tp = m_tp;
							end_tp.iIndex = to_iIndex;
							//
							++it_len, --it, --m_tp.iIndex;
							for(; ref_tp <= end_tp; ++ref_tp.iIndex, --it_len, ++it, ++m_tp.iIndex){
								const auto &cp = m_txtMap.GetCharPoint(ref_tp);
								if(isDrawArea(cp.lcol)){
									if(x < 0){
										y = cp.zy;
									} else if(x != cp.zx){
										m_txtRenderer.PatternFill(CGrTxtRendererMgr::Text, x, y, w, h, lpNextSrc, -1);
										y = cp.zy;
									}
									x = cp.zx;
									w = cp.w;
									h = cp.zy + cp.w - y; /* cp.wより、cp.h が正しいが余白分も付加されているので余白分が無い (縦横の幅が同一とみなして) cp.wの方を使用する */
								}
							}
						}
						h -= (m_fontTextSize / 4); // padding
						m_txtRenderer.PatternFill(CGrTxtRendererMgr::Text, x, y, w, h, lpNextSrc, -1);
					}
					break;
				case TxtMiru::TT_MOVING_BORDER       :
					// 文字囲み   ※文文章の囲みとは別
					drawBorder(_T("傍線") , ti, text_list);
					break;
					// 文字囲み   ※文章の囲みとは別
				case TxtMiru::TT_LINE_BOX_START: // through
				case TxtMiru::TT_LINE_BOX_END  :
					// 文字囲み   ※文章の囲みとは別
					for(; m_txtMap.GetCharPoint(m_tp).h != 0; ++m_tp.iPos){
						CGrTxtRendererMgr::BorderType borderType = {true,false,true,false};
						const auto &cp = m_txtMap.GetCharPoint(m_tp);
						if(m_tp.iPos == 0 && !isDrawArea(cp.lcol)){
							// 開始行が範囲外の場合は、処理しない
							break;
						}
						if(cp.ch_w & TxtMiru::CPBorderRight){
							borderType.right = true;
						}
						if(cp.ch_w & TxtMiru::CPBorderLeft){
							borderType.left = true;
						}
						m_txtRenderer.DrawBorder(cp.zx, cp.zy, cp.w, cp.h, borderType);
					}
					// 文字囲み   ※文章の囲みとは別
					break;
				case TxtMiru::TT_LINE                : // through
				case TxtMiru::TT_WAVE_LINE           : // through
				case TxtMiru::TT_DOT_LINE            : // through
				case TxtMiru::TT_SHORT_DASHED_LINE   : // through
				case TxtMiru::TT_DOUBLE_LINE         : // through
				case TxtMiru::TT_DEL_LINE            : // through
				case TxtMiru::TT_LINE_L              : // through
				case TxtMiru::TT_WAVE_LINE_L         : // through
				case TxtMiru::TT_SHORT_DASHED_LINE_L : // through
				case TxtMiru::TT_DOT_LINE_L          : // through
				case TxtMiru::TT_DOUBLE_LINE_L       : // through
				case TxtMiru::TT_UNDER_LINE          : drawLine(lpNextSrc , ti, text_list); break;
				case TxtMiru::TT_WHITE_DOT_L         : // through
				case TxtMiru::TT_ROUND_DOT_L         : // through
				case TxtMiru::TT_WHITE_ROUND_DOT_L   : // through
				case TxtMiru::TT_BLACK_TRIANGLE_DOT_L: // through
				case TxtMiru::TT_WHITE_TRIANGLE_DOT_L: // through
				case TxtMiru::TT_DOUBLE_ROUND_DOT_L  : // through
				case TxtMiru::TT_BULLS_EYE_DOT_L     : // through
				case TxtMiru::TT_SALTIRE_DOT_L       : // through
				case TxtMiru::TT_DOT_L               : // through
				case TxtMiru::TT_WHITE_DOT           : // through
				case TxtMiru::TT_ROUND_DOT           : // through
				case TxtMiru::TT_WHITE_ROUND_DOT     : // through
				case TxtMiru::TT_BLACK_TRIANGLE_DOT  : // through
				case TxtMiru::TT_WHITE_TRIANGLE_DOT  : // through
				case TxtMiru::TT_DOUBLE_ROUND_DOT    : // through
				case TxtMiru::TT_BULLS_EYE_DOT       : // through
				case TxtMiru::TT_SALTIRE_DOT         : // through
				case TxtMiru::TT_DOT                 : drawMark(lpNextSrc , ti, text_list); break;
				case TxtMiru::TT_RUBY                : // through
				case TxtMiru::TT_RUBY_L              :
					if (bHorz && !IgnoreHorzChar(ti.chrType)) {
						SetHorz();
						drawRuby(lpNextSrc);
						UnSetHorz();
					}
					else {
						drawRuby(lpNextSrc);
					}
					break;
				case TxtMiru::TT_NOTE_L              : // through
				case TxtMiru::TT_UNKOWN_ERROR        : drawRuby(lpNextSrc); break;
				}
			}
		}
	}
private:
	void setFontSize(int iFont)
	{
		CGrCalcSize calcSize(m_pDoc->GetConstLayout());
		if(calcSize.initialize(m_rectWindow)){
			m_txtRenderer.SetFontSize(iFont,
									  calcSize.getFontSize(CGrTxtLayout::LST_Text        , iFont),
									  calcSize.getFontSize(CGrTxtLayout::LST_Ruby               ),
									  calcSize.getFontSize(CGrTxtLayout::LST_Note               ),
									  calcSize.getFontSize(CGrTxtLayout::LST_Nombre             ),
									  calcSize.getFontSize(CGrTxtLayout::LST_RunningHeads       )
									  );
		}
	}
	static bool IgnoreHorzChar(DWORD type)
	{
		return (0x0040 == type || 0x0087 == type || 0x008B == type || 0x0048 == type || 0x0440 == type || 0x8040 == type || 0x0448 == type || 0xF048 == type);
	}
	bool isDrawArea(int col)
	{
		return m_startLine <= col && col < m_endLine;
	}
	bool isDrawArea(const TxtMiru::CharPoint &cp)
	{
		return cp.w > 0 && m_startLine <= cp.lcol && cp.lcol < m_endLine;
	}
	void drawRNum(LPCTSTR lpNextSrc)
	{
		for(;*lpNextSrc; ++m_tp.iPos){
			auto lpSrc = lpNextSrc;
			lpNextSrc = CGrText::CharNext(lpSrc);
			const auto &cp = m_txtMap.GetCharPoint(m_tp);
			if(isDrawArea(cp)){
				if(cp.ch_w == cp.ch_h){
					m_txtRenderer.DrawText(FTText[m_pDoc->IsBold(m_tp)], cp.zx, cp.zy, lpSrc, lpNextSrc);
				} else {
					m_txtRenderer.DrawText(FTRotateText[m_pDoc->IsBold(m_tp)], cp.zx, cp.zy, lpSrc, lpNextSrc);
				}
			}
		}
	}

	void drawText(LPCTSTR lpNextSrc)
	{
		const auto &param = CGrTxtMiru::theApp().Param();
		for(;*lpNextSrc; ++m_tp.iPos){
			auto lpSrc = lpNextSrc;
			lpNextSrc = CGrText::CharNext(lpSrc);
			const auto &cp = m_txtMap.GetCharPoint(m_tp);
			if(isDrawArea(cp)){
				std::tstring ofch(lpSrc, lpNextSrc-lpSrc);
				auto offset_point = param.GetCharOffset(ofch);
				offset_point.y = cp.h * offset_point.y / 100;
				offset_point.x = cp.w * offset_point.x / 100;
				switch(getRotateType(param, ofch.c_str())){
				case ROTP_ROTATE: m_txtRenderer.DrawText(FTTurnText[m_pDoc->IsBold(m_tp)], cp.zx + offset_point.x, cp.zy + offset_point.y, lpSrc, lpNextSrc); break;
				case ROTP_REV   : m_txtRenderer.DrawText(FTRotateText[m_pDoc->IsBold(m_tp)], cp.zx + offset_point.x, cp.zy + offset_point.y, lpSrc, lpNextSrc); break;
				default         : m_txtRenderer.DrawText(FTText[m_pDoc->IsBold(m_tp)], cp.zx + offset_point.x, cp.zy + offset_point.y, lpSrc, lpNextSrc); break;
				}
			}
		}
	}
	void drawSTxt(LPCTSTR lpNextSrc)
	{
		for(;*lpNextSrc; ++m_tp.iPos){
			auto lpSrc = lpNextSrc;
			lpNextSrc = CGrText::CharNext(lpSrc);
			const auto &cp = m_txtMap.GetCharPoint(m_tp);
			if(isDrawArea(cp)){
				m_txtRenderer.DrawText(FTHalfText[m_pDoc->IsBold(m_tp)], cp.zx, cp.zy, lpSrc, lpNextSrc);
			}
		}
	}
	void drawPict(LPCTSTR lpNextSrc, TxtMiru::TextPoint &tp)
	{
		const auto &cp = m_txtMap.GetCharPoint(tp);
		if(isDrawArea(cp)){
			m_txtRenderer.DrawPict(cp.zx, cp.zy, cp.w, cp.h, lpNextSrc);
		}
	}
	void drawKuCh(LPCTSTR lpNextSrc)
	{
		const auto &cp = m_txtMap.GetCharPoint(m_tp);
		if(isDrawArea(cp.lcol)){
			auto ft = FTText[m_pDoc->IsBold(m_tp)];
			m_txtRenderer.DrawText(ft, cp.zx, cp.zy, lpNextSrc, -1);
			m_txtRenderer.DrawText(ft, cp.zx, cp.zy+m_fontTextSize, _T("＼"), -1);
		}
	}
	void drawLChr(LPCTSTR lpNextSrc, TxtMiru::TextPoint &tp)
	{
		const auto &cp = m_txtMap.GetCharPoint(tp);
		if(isDrawArea(cp)){
			m_txtRenderer.PatternFill(CGrTxtRendererMgr::Text, cp.zx, cp.zy, cp.w, cp.h, lpNextSrc, -1);
		}
	}
	void drawLink(LPCTSTR lpNextSrc, const TxtMiru::TextInfo &ti, const TxtMiru::TextInfoList &text_list)
	{
		TxtMiru::TextPoint ref_tp = TxtMiru::TextParserPoint(m_tp.iLine, ti.tpBegin);
		TxtMiru::TextPoint end_tp = TxtMiru::TextParserPoint(m_tp.iLine, ti.tpEnd  );
		int x=-1, w=0, y=0, b=0;
		CGrTxtCanvas::LinkBox lb;
		lb.id = ti.chrType;
		lb.url = ti.str;
		int prelcol = -1;
		for(;ref_tp <= end_tp; ++m_tp.iPos){
			const auto &cp = m_txtMap.GetCharPoint(m_tp);
			ref_tp = CGrTxtMapper::NextTextPoint(ref_tp, text_list);
			if(isDrawArea(cp)){
				// 線の描画は、文字間隔にかかわらず連続して描画されるようにする
				if(x < 0){
					y = cp.zy;
					x = cp.zx;
					lb.rect.left = cp.zx - cp.w;
					lb.rect.right = cp.zx;
					if(cp.ch_w < cp.ch_h){
						int w4 = cp.w / 4;
						x -= w4;
						lb.rect.left  -= w4;
						lb.rect.right += w4;
					}
				} else if(y > cp.zy || (prelcol >= 0 && prelcol != cp.lcol)){
					// データは、上から順に配置されることを前提にしているので y座標が小さくなったら 改行したとみなし そこまでの線を描画する
					// ※半角文字の場合、x座標の一致でチェックすると線ががたがたになるので(以下の max(cp.zx,x)も同様
					if(m_iHoverLink == lb.id){
						m_txtRenderer.PatternFill(CGrTxtRendererMgr::Text, x, y, w, b - y, lpNextSrc, -1);
					}
					lb.rect.top = y;
					lb.rect.bottom = b;
					m_linkBox.push_back(lb);
					y = cp.zy;
					x = cp.zx;
					b = cp.zy + cp.h;// - y;
					lb.rect.left = cp.zx - cp.w;
					lb.rect.right = cp.zx;
					if(cp.ch_w < cp.ch_h){
						int w4 = cp.w / 4;
						x -= w4;
						lb.rect.left  -= w4;
						lb.rect.right += w4;
					}
				}
				if(cp.ch_w < cp.ch_h){
					x = max(cp.zx - cp.w / 2, x);
				} else {
					x = max(cp.zx, x);
				}
				lb.rect.left = min(lb.rect.left, x);
				lb.rect.right = max(lb.rect.right, x);
				w = cp.w;
				b = max(cp.zy + cp.h, b);
				prelcol = cp.lcol;
			}
		}
		if(m_iHoverLink == lb.id){
			m_txtRenderer.PatternFill(CGrTxtRendererMgr::Text, x, y, w, b - y, lpNextSrc, -1);
		}
		lb.rect.top = y;
		lb.rect.bottom = b;
		m_linkBox.push_back(lb);
	}
	void drawBorder(LPCTSTR lpNextSrc, const TxtMiru::TextInfo &ti, const TxtMiru::TextInfoList &text_list)
	{
		TxtMiru::TextPoint ref_tp = TxtMiru::TextParserPoint(m_tp.iLine, ti.tpBegin);
		TxtMiru::TextPoint end_tp = TxtMiru::TextParserPoint(m_tp.iLine, ti.tpEnd  );
		int x=-1, w=0, y=0, b=0;
		int lx = 0;
		int prelcol = -1;
		int lcol_begin = -1;
		int lcol_end   = 0;
		CGrTxtRendererMgr::BorderType borderType = {true,true,false,true};
		for(;ref_tp <= end_tp; ++m_tp.iPos){
			const auto &cp = m_txtMap.GetCharPoint(m_tp);
			ref_tp = CGrTxtMapper::NextTextPoint(ref_tp, text_list);
			if(lcol_begin == -1){
				lcol_begin = cp.lcol;
			}
			if(isDrawArea(cp)){
				// 線の描画は、文字間隔にかかわらず連続して描画されるようにする
				if(x < 0){
					y = cp.zy;
					x = cp.zx;
					lx = x - cp.w;
				} else if(y > cp.zy || (prelcol >= 0 && prelcol != cp.lcol)){
					// データは、上から順に配置されることを前提にしているので y座標が小さくなったら 改行したとみなし そこまでの線を描画する
					// ※半角文字の場合、x座標の一致でチェックすると線ががたがたになるので(以下の max(cp.zx,x)も同様
					m_txtRenderer.DrawBorder(lx-1, y-1, x-lx+1, b-y-1, borderType);
					y = cp.zy;
					b = cp.zy + cp.h;// - y;
					borderType.top = false;
				}
				x = min(cp.zx, x);
				lx = min(cp.zx - cp.w, lx);
				w = cp.w;
				b = max(cp.zy + cp.h, b);
				prelcol = cp.lcol;
			}
			lcol_end = max(cp.lcol, lcol_end);
		}
		borderType.top    = (lcol_begin == prelcol);
		borderType.bottom = (lcol_end   == prelcol);
		m_txtRenderer.DrawBorder(lx-1, y-1, x-lx+1, b-y-1, borderType);
	}
	void drawLine(LPCTSTR lpNextSrc, const TxtMiru::TextInfo &ti, const TxtMiru::TextInfoList &text_list)
	{
		TxtMiru::TextPoint ref_tp = TxtMiru::TextParserPoint(m_tp.iLine, ti.tpBegin);
		TxtMiru::TextPoint end_tp = TxtMiru::TextParserPoint(m_tp.iLine, ti.tpEnd  );
		int x=-1, w=0, y=0, b=0;
		int prelcol = -1;
		for(;ref_tp <= end_tp; ++m_tp.iPos){
			const auto &cp = m_txtMap.GetCharPoint(m_tp);
			ref_tp = CGrTxtMapper::NextTextPoint(ref_tp, text_list);
			if(isDrawArea(cp)){
				// 線の描画は、文字間隔にかかわらず連続して描画されるようにする
				if(x < 0){
					y = cp.zy;
					x = cp.zx;
				} else if(y > cp.zy || (prelcol >= 0 && prelcol != cp.lcol)){
					// データは、上から順に配置されることを前提にしているので y座標が小さくなったら 改行したとみなし そこまでの線を描画する
					// ※半角文字の場合、x座標の一致でチェックすると線ががたがたになるので(以下の max(cp.zx,x)も同様
					m_txtRenderer.PatternFill(CGrTxtRendererMgr::Text, x, y, w, b - y, lpNextSrc, -1);
					y = cp.zy;
					b = cp.zy + cp.h;// - y;
				}
				x = min(cp.zx, x);
				w = cp.w;
				b = max(cp.zy + cp.h, b);
				prelcol = cp.lcol;
			}
		}
		m_txtRenderer.PatternFill(CGrTxtRendererMgr::Text, x, y, w, b - y, lpNextSrc, -1);
	}
	void drawMark(LPCTSTR lpNextSrc, const TxtMiru::TextInfo &ti, const TxtMiru::TextInfoList &text_list)
	{
		TxtMiru::TextPoint ref_tp = TxtMiru::TextParserPoint(m_tp.iLine, ti.tpBegin);
		TxtMiru::TextPoint end_tp = TxtMiru::TextParserPoint(m_tp.iLine, ti.tpEnd  );
		int h = m_txtRenderer.GetFontHeight(CGrTxtRendererMgr::Text);
		for(;ref_tp <= end_tp; ++m_tp.iPos){
			const auto &cp = m_txtMap.GetCharPoint(m_tp);
			ref_tp = CGrTxtMapper::NextTextPoint(ref_tp, text_list);
			if(isDrawArea(cp)){
				m_txtRenderer.DrawText(CGrTxtRendererMgr::Text, cp.zx, (cp.zy * 2 + cp.h - h) / 2, lpNextSrc, -1);
			}
		}
	}
	void drawRuby(LPCTSTR lpNextSrc)
	{
		for(;*lpNextSrc; ++m_tp.iPos){
			auto lpSrc = lpNextSrc;
			lpNextSrc = CGrText::CharNext(lpSrc);
			const auto &cp = m_txtMap.GetCharPoint(m_tp);
			if(isDrawArea(cp)){
				m_txtRenderer.DrawText(FTRubyText, cp.zx, cp.zy, lpSrc, lpNextSrc);
			}
		}
	}
	void drawNote(LPCTSTR lpNextSrc)
	{
		for(;*lpNextSrc; ++m_tp.iPos){
			auto lpSrc = lpNextSrc;
			lpNextSrc = CGrText::CharNext(lpSrc);
			const auto &cp = m_txtMap.GetCharPoint(m_tp);
			if(isDrawArea(cp)){
				m_txtRenderer.DrawText(CGrTxtRendererMgr::Note, cp.zx, cp.zy, lpSrc, lpNextSrc);
			}
		}
	}
	bool drawOthr(LPCTSTR lpNextSrc)
	{
		const auto &cp = m_txtMap.GetCharPoint(m_tp);
		return (isDrawArea(cp) && m_txtRenderer.DrawText(FTText[m_pDoc->IsBold(m_tp)], cp.zx, cp.zy, lpNextSrc, -1));
	}
};

CGrTxtCanvas::CGrTxtCanvas()
{
}

CGrTxtCanvas::~CGrTxtCanvas()
{
	ClearCache();
	Clear();
}

void CGrTxtCanvas::Attach(CGrTxtDocument *pDoc)
{
	m_pDoc = pDoc;
}

void CGrTxtCanvas::Initialize()
{
	if(m_pDoc){
		m_txtRenderer.Initialize(*m_pDoc);
	}
	m_linkBox.clear();
}

void CGrTxtCanvas::SetOffset(int x, int y)
{
	m_offsetX = x;
	m_offsetY = y;
}

void CGrTxtCanvas::setFontSize(int iFont)
{
	CGrCalcSize calcSize(m_pDoc->GetConstLayout());
	if(calcSize.initialize(m_rectWindow)){
		m_txtRenderer.SetFontSize(iFont,
								  calcSize.getFontSize(CGrTxtLayout::LST_Text        , iFont),
								  calcSize.getFontSize(CGrTxtLayout::LST_Ruby               ),
								  calcSize.getFontSize(CGrTxtLayout::LST_Note               ),
								  calcSize.getFontSize(CGrTxtLayout::LST_Nombre             ),
								  calcSize.getFontSize(CGrTxtLayout::LST_RunningHeads       )
								  );
	}
}

void CGrTxtCanvas::Resize(int cx, int cy)
{
	if(!m_pDoc){ return; }
	cx -= m_offsetX;
	cy -= m_offsetY;
	RECT rect = {0,0,cx,cy};
	if(cx <= 0 || cy <= 0){
		return;
	}
	m_rectCanvas = rect;
	m_rectWindow = rect;

	const auto &layout = m_pDoc->GetConstLayout();
	CGrCalcSize calcSize(layout);
	if(!calcSize.initialize(rect)){
		return;
	}
	calcSize.getCanvasRect(m_rectCanvas);

	if(!m_bmpCanvas.Create(cx, cy)){ return; }

	try {
		//
		setFontSize(0); // m_fontTextSizeが変わっている場合があるので、カレントを一旦通常フォントに戻して フォント再作成
		int fontTextSize         = calcSize.getFontSize(CGrTxtLayout::LST_Text        );
		int fontRubySize         = calcSize.getFontSize(CGrTxtLayout::LST_Ruby        );
		int fontNoteSize         = calcSize.getFontSize(CGrTxtLayout::LST_Note        );
		int fontNombreSize       = calcSize.getFontSize(CGrTxtLayout::LST_Nombre      );
		int fontRunningHeadsSize = calcSize.getFontSize(CGrTxtLayout::LST_RunningHeads);
		m_txtRenderer.Create(*m_pDoc, fontTextSize, fontRubySize, fontNoteSize, fontNombreSize, fontRunningHeadsSize);
		m_txtRenderer.ClearFlexChar();
		m_fontTextSize = fontTextSize;

		const auto &txt_layout = layout.GetConstLayoutList(CGrTxtLayout::LLT_Text);
		int txt_layout_len = txt_layout.size();
		if(m_layoutBox.size() != txt_layout_len){
			m_layoutBox.resize(txt_layout_len);
		}
		if(txt_layout_len <= 1){ return; }
		int idx = txt_layout_len - 1;
		const auto *b = &txt_layout[0];
		const auto *ptl = b + idx;
		auto *plb = &m_layoutBox[idx];
		int old_line = layout.GetMaxLine();
		for(; idx>=0; --idx, --ptl, --plb){
			const auto &tl = *ptl;
			auto &&lb = *plb;
			auto &&rc     = lb.rc;
			auto &&org_rc = lb.org_rc;
			calcSize.getPaperRect(rc);
			const auto *tmp_it=b;
			for(int len=txt_layout_len; len>0; --len, ++tmp_it){
				const auto &tmp_tl = *tmp_it;
				if(tmp_tl.right < tl.left && rc.left < tmp_tl.right){
					rc.left = tmp_tl.right;
				}
				if(tmp_tl.bottom < tl.top && rc.top < tmp_tl.bottom){
					rc.top = tmp_tl.bottom;
				}
				if(tmp_tl.left > tl.right && rc.right > tmp_tl.left){
					rc.right = tmp_tl.left;
				}
				if(tmp_tl.top > tl.bottom && rc.bottom > tmp_tl.top){
					rc.bottom = tmp_tl.top;
				}
			}
			lb.max_line = old_line - 1;
			lb.min_line = old_line - tl.lines;
			old_line = lb.min_line;
			rc.left      = calcSize.calcPos(rc.left  ) + m_rectCanvas.left;
			rc.top       = calcSize.calcPos(rc.top   ) + m_rectCanvas.top ;
			rc.right     = calcSize.calcPos(rc.right ) + m_rectCanvas.left;
			rc.bottom    = calcSize.calcPos(rc.bottom) + m_rectCanvas.top ;
			org_rc.left  = calcSize.calcPos(tl.left  ) + m_rectCanvas.left;
			org_rc.top   = calcSize.calcPos(tl.top   ) + m_rectCanvas.top ;
			org_rc.right = calcSize.calcPos(tl.right ) + m_rectCanvas.left;
			org_rc.bottom= calcSize.calcPos(tl.bottom) + m_rectCanvas.top ;
		}
	} catch(...){}
}

bool CGrTxtCanvas::isDrawArea(int col, int lines)
{
	return m_startLine <= col && col < m_startLine + lines;
}

void CGrTxtCanvas::UpdateParam()
{
	if(!m_pDoc){ return; }
	m_totalPage = m_pDoc->GetTotalPage();
	// 背景画像フルパス変換
	CGrCurrentDirectory curDir;
	const auto &param = CGrTxtMiru::theApp().Param();
	param.GetText(CGrTxtParam::BackgroundImage, m_bkImageFilename);

	TCHAR fullpath[MAX_PATH] = {0};
	TCHAR *filename;
	if(0 != ::GetFullPathName(m_bkImageFilename.c_str(), sizeof(fullpath)/sizeof(TCHAR), fullpath, &filename)){
		CGrShell::ToPrettyFileName(fullpath, m_bkImageFilename);
	}
	m_txtRenderer.ClearFlexChar();
}
static void setNombre(TxtMiru::TextInfoList &til_layout, int page, int total_page, const CGrTxtDocument &doc)
{
	if(page <= 0){ return; }
	TxtMiru::TextInfo ti;
	auto ft = CGrTxtLayout::FT_Nombre1;
	if(page & 0x01){
		ti.textType = TxtMiru::TT_NOMBRE1;
	} else {
		ti.textType = TxtMiru::TT_NOMBRE2;
		ft = CGrTxtLayout::FT_Nombre2;
	}
	const auto &txtlayout = doc.GetConstLayout();
	CGrText::FormatMessage(ti.str, txtlayout.GetFormat(ft).c_str(), page, total_page);
	til_layout.push_back(std::move(ti));
}

#define MUTEX TEXT("CANVAS_MUTEX")
class CWaitMutex
{
private:
	HANDLE m_hMutex;
public:
	CWaitMutex(){
		m_hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE , MUTEX);
		if (m_hMutex) {
			WaitForSingleObject(m_hMutex, INFINITE);
		}
	}
	~CWaitMutex(){
		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);
	}
};

int CGrTxtCanvas::SetPageFlip(int page, bool bScroll)
{
	CWaitMutex w;
	m_pageFlip.SetFlipEnd();
	if(page < 0){
		page = m_currentPage;
	}
	int scrolltype = 0;
	if(bScroll){
		int page_d = m_currentPage-page;
		bScroll = false;
		if(page % 2 == 0){
			if(page_d == 0){
				scrolltype = 1;
			}
		} else {
			if(page_d == -1){
				scrolltype = 2;
			}
		}
	}
	page &= (-2); // 最下位ビット切り捨て // page = page / 2 * 2;
	if(scrolltype == 0){
		switch(m_currentPage-page){
		case -2:
			m_pageFlip.Initialize(m_rectCanvas, true);
			break;
		case 2:
			m_pageFlip.Initialize(m_rectCanvas, false);
			break;
		default:
			return Update(page);
		}
		if(m_bmpCanvas.Width() > 0){
			m_pageFlip.SetFromBmp(m_bmpCanvas);
			page = Update(page);
			m_pageFlip.SetToBmp(m_bmpCanvas);
			m_pageFlip.Next();
		} else {
			return Update(page);
		}
	} else {
		m_pageFlip.Initialize(m_rectCanvas, (scrolltype == 1), true);
		m_pageFlip.Next();
	}
	return page;
}

bool CGrTxtCanvas::PageFilpNext()
{
	CWaitMutex w;
	if(m_pageFlip.IsFilpEnd()){
		return false;
	}
	m_pageFlip.Next();
	return true;
}

int CGrTxtCanvas::Update(int page)
{
	if(m_fontTextSize <= 0){
		return 0; // 初期化できていない
	}
	//
	const auto &param = CGrTxtMiru::theApp().Param();
	auto rcCanvas = m_rectCanvas;
	auto rcWindow = m_rectWindow;
	// 背景描画
	m_txtRenderer.SetBitmap(m_bmpCanvas);
	{
		COLORREF page_color[3] = {}; /* paper, shadow, back */
		param.GetPoints(CGrTxtParam::PageColor, reinterpret_cast<int*>(page_color), sizeof(page_color)/sizeof(COLORREF));
		auto hbkHDC = ::CreateCompatibleDC(NULL);
		auto holdBitmap = static_cast<HBITMAP>(::SelectObject(hbkHDC, m_bmpCanvas));
		auto oldclr = ::SetBkColor(hbkHDC, page_color[2]);
		::ExtTextOut(hbkHDC, 0, 0, ETO_OPAQUE, &rcWindow, NULL, 0, NULL);

		::SetBkColor(hbkHDC, page_color[0]);
		::ExtTextOut(hbkHDC, 0, 0, ETO_OPAQUE, &rcCanvas, NULL, 0, NULL);
		::SetBkColor(hbkHDC, oldclr);
		::SelectObject(hbkHDC, holdBitmap);
		::DeleteDC(hbkHDC);

		if(!m_bkImageFilename.empty()){
			m_txtRenderer.StretchBlt(rcCanvas.left, rcCanvas.top, rcCanvas.right-rcCanvas.left, rcCanvas.bottom-rcCanvas.top, m_bkImageFilename.c_str());
		}
	}

	const auto &lineList = m_pDoc->GetLineList();
	int ll_len = lineList.size();
	if(ll_len <= 0){
		// データ無
		m_currentPage = 0;
		return 0;
	}
	//
	setFontSize(0);
	//
	m_linkBox.clear();
	m_linkBox.shrink_to_fit();
	m_tipsList.clear();
	m_tipsList.shrink_to_fit();
	m_txtRenderer.ResetTextColor();
	if(!m_pDoc){ return m_currentPage; }
	if(page < 0){
		page = m_currentPage;
	}
	if(page > m_totalPage || page < 0){
		return m_currentPage;
	}
	page &= (-2); // 最下位ビット切り捨て // page = page / 2 * 2;

	//
	int lines = m_pDoc->GetConstLayout().GetMaxLine(); // lines = size.cx
	m_startLine = lines * page / 2;
	m_currentPage = page;
	m_selectBeginPoint.bEnable = false;
	//
	TxtMiru::TextInfoList til_layout;
	setNombre(til_layout, page+0, m_totalPage, *m_pDoc);
	setNombre(til_layout, page+1, m_totalPage, *m_pDoc);
	// 見出し
	{
		TxtMiru::TextInfo ti;
		ti.textType = TxtMiru::TT_RUNNINGHEADS;

		const auto &subtitle = m_pDoc->GetSubtitle();
		int len=subtitle.Count();
		if(len > 0){
			auto lpName = subtitle.GetNameByPage(page+1);
			if(lpName){
				ti.str = lpName;
			}
		}
		if(ti.str.empty()){
			ti.str = m_pDoc->GetTitle();
		}
		til_layout.push_back(std::move(ti));
	}
	{
		CGrTxtCanvasDraw m_draw(this, rcCanvas, til_layout);
		m_draw.Draw(lines);
	}
	RECT *prcNoDispArea = nullptr;
	if(param.GetBoolean(CGrTxtFuncIParam::PictPaddingNone)){ // 画像をページいっぱいに
		prcNoDispArea = static_cast<RECT*>(calloc(m_layoutBox.size(), sizeof(RECT)));
	}
	// 挿し絵
	{
		int iCenterX = (rcCanvas.right - rcCanvas.left) / 2 + rcCanvas.left;
		int iCenterY = (rcCanvas.bottom - rcCanvas.top) / 2 + rcCanvas.top;
		// iLayout は、1からカウント
		int iLayoutFrom=m_currentPage * m_layoutBox.size() / 2 +1;
		int iLayoutTo  =iLayoutFrom+m_layoutBox.size()-1;
		for(const auto &pictpos : m_pDoc->GetConstPictPosMap()){
			const auto &tp = pictpos.first;
			const auto &pp = pictpos.second;
			if(iLayoutFrom <= pp.iStartLayout && pp.iStartLayout <= iLayoutTo){
				const auto &li = lineList[tp.iLine];
				const auto &text_list = li.text_list;
				const auto &ti=text_list[tp.iIndex];
				int iLayoutBegin = pp.iStartLayout-iLayoutFrom;
				const auto *plb=&m_layoutBox[iLayoutBegin];
				auto rect = plb->org_rc;
				int len=min(pp.iEndLayout-pp.iStartLayout+1, static_cast<signed int>(m_layoutBox.size())-iLayoutBegin);
				for(int i=iLayoutBegin; len>0; --len, ++plb, ++i){
					auto lb_rc = plb->org_rc;
					if(prcNoDispArea){ // 画像をページいっぱいに
						if(lb_rc.left < iCenterX){
							lb_rc.left = rcCanvas.left;
							if(lb_rc.right < iCenterX){
								lb_rc.right = iCenterX;
							}
						}
						if(lb_rc.right > iCenterX){
							lb_rc.right = rcCanvas.right;
							if(lb_rc.left > iCenterX){
								lb_rc.left = iCenterX;
							}
						}
						if(lb_rc.top < iCenterY){
							lb_rc.top = rcCanvas.top;
							if(lb_rc.bottom < iCenterY){
								lb_rc.bottom = iCenterY;
							}
						}
						if(lb_rc.bottom > iCenterY){
							lb_rc.bottom = rcCanvas.bottom;
							if(lb_rc.top > iCenterY){
								lb_rc.top = iCenterY;
							}
						}
						prcNoDispArea[i] = lb_rc;
					}
					if(rect.left   > lb_rc.left  ){ rect.left  = lb_rc.left  ; }
					if(rect.right  < lb_rc.right ){ rect.right = lb_rc.right ; }
					if(rect.top    > lb_rc.top   ){ rect.top   = lb_rc.top   ; }
					if(rect.bottom < lb_rc.bottom){ rect.bottom= lb_rc.bottom; }
				}
				int caption_height = 0;
				std::vector<std::tstring> string_list;
				// 0:filename,1:note,2:size,3:pos,4:caption
				CGrTxtParser::GetPictInfo(ti.str, string_list);
				const auto &caption = string_list[4];
				if(!caption.empty()){
					auto tmp_tp = tp;
					auto lpNextSrc = caption.c_str();
					while(*lpNextSrc){
						auto lpSrc = lpNextSrc;
						lpNextSrc = CGrText::CharNext(lpSrc);
						const auto &cp = m_txtMap.GetCharPoint(tmp_tp);
						if(caption_height < cp.zy + cp.h){
							caption_height = cp.zy + cp.h;
						}
						++tmp_tp.iPos;
					}
				}
				m_txtRenderer.DrawPict(rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top-caption_height, ti.str.c_str());
				if(caption_height > 0){
					auto size = m_txtRenderer.GetCachePictSize(ti.str.c_str());
					int offset_y = rect.top + (rect.bottom-rect.top-caption_height) / 2 - size.cy / 2 + size.cy;
					auto tmp_tp = tp;
					auto lpNextSrc = caption.c_str();
					while(*lpNextSrc){
						auto lpSrc = lpNextSrc;
						lpNextSrc = CGrText::CharNext(lpSrc);
						const auto &cp = m_txtMap.GetCharPoint(tmp_tp);
						m_txtRenderer.DrawText(CGrTxtRendererMgr::RunningHeads, cp.zx, cp.zy+offset_y, lpSrc, lpNextSrc);
						++tmp_tp.iPos;
					}
				}
				auto note = string_list[1].c_str();
				if(note && _tcslen(note) > 0){
					CGrToolTips::TipsInfo tips;
					tips.rect = rect;
					tips.str = note;
					m_tipsList.push_back(std::move(tips));
				}
				// 画像へのリンク
				if(ti.chrType != 0xffff && text_list.size() > static_cast<unsigned int>(ti.tpEnd.iIndex)){
					LinkBox lb;
					lb.id = ti.chrType;
					const auto &ti_link=text_list[ti.tpEnd.iIndex];
					lb.url = ti_link.str;
					lb.rect = rect;
					m_linkBox.push_back(std::move(lb));
				}
			}
		}
	}
	// Nombre
	TxtMiru::TextPoint tp;
	int it_len = til_layout.size();
	if(it_len > 0){
		const auto *it = &til_layout[0];
		tp.iLine  = 0;
		tp.iIndex = 0;
		for(; it_len>0; --it_len, ++it, ++tp.iIndex){
			const auto &ti = (*it);
			const auto &text = ti.str;
			if(text.empty()){ ++tp.iIndex; continue; }
			auto lpNextSrc = text.c_str();
			auto ft = CGrTxtRendererMgr::Nombre;
			if(ti.textType == TxtMiru::TT_RUNNINGHEADS){
				ft = CGrTxtRendererMgr::RunningHeads;
			}
			RECT rect = {-1,-1,-1,-1};
			for(tp.iPos=0; *lpNextSrc; ++tp.iPos){
				auto lpSrc = lpNextSrc;
				lpNextSrc = CGrText::CharNext(lpSrc);
				const auto &cp = m_txtMap.GetLayoutCharPoint(tp);
				if(cp.w > 0){
					if(rect.left == -1 || rect.left > cp.zx){
						rect.left = cp.zx;
					}
					if(rect.top == -1 || rect.top > cp.zy){
						rect.top = cp.zy;
					}
					if(rect.right == -1 || rect.right < cp.zx + cp.w){
						rect.right = cp.zx + cp.w;
					}
					if(rect.bottom == -1 || rect.bottom < cp.zy + cp.h){
						rect.bottom = cp.zy + cp.h;
					}
					bool bNoDisp = false;
					if(prcNoDispArea){
						POINT pt = {cp.zx, cp.zy};
						for(int i=0; i<m_layoutBox.size(); ++i){
							if(PtInRect(&prcNoDispArea[i], pt)){
								bNoDisp = true;
								break;
							}
						}
					}
					if(bNoDisp){
						break;
					}
					m_txtRenderer.DrawText(ft, cp.zx, cp.zy, lpSrc, lpNextSrc);
				}
			}
			// 総ページ数
			if(ti.textType == TxtMiru::TT_NOMBRE1 || ti.textType == TxtMiru::TT_NOMBRE2){
				CGrToolTips::TipsInfo tips;
				tips.rect = rect;
				CGrText::FormatMessage(tips.str, _T("%1!s!/%2!d!"), text.c_str(), m_totalPage);
				m_tipsList.push_back(std::move(tips));
			}
		}
	}
	if(prcNoDispArea){
		free(prcNoDispArea);
	}
	return m_currentPage;
}

void CGrTxtCanvas::Draw(HDC hdc, const RECT &rect, const RECT &paintRect, int offset_x/*=0*/, int offset_y/*=0*/)
{
	offset_x = m_offsetX - offset_x;
	offset_y = m_offsetY - offset_y;
	CWaitMutex w;
	if(m_pageFlip.IsDraw()){
		m_pageFlip.Draw(hdc, paintRect, offset_x, offset_y);
		return;
	}
	if(!m_pDoc){ return; }
	auto hbkHDC = ::CreateCompatibleDC(hdc);
	if(!hbkHDC){ return; }
	auto holdBitmap = static_cast<HBITMAP>(::SelectObject(hbkHDC, m_bmpCanvas));

	if(m_selectBeginPoint.bEnable){
		auto hbkWorkHDC = ::CreateCompatibleDC(hdc);
		if(hbkWorkHDC){
			CGrBitmap bmpWork;
			if(bmpWork.Create(m_bmpCanvas.Width(), m_bmpCanvas.Height())){
				auto holdWorkBitmap = static_cast<HBITMAP>(::SelectObject(hbkWorkHDC, bmpWork));
				::BitBlt(hbkWorkHDC, paintRect.left, paintRect.top, paintRect.right-offset_x, paintRect.bottom-offset_y, hbkHDC, paintRect.left, paintRect.top, SRCCOPY);
				auto offset_rect = rect;
				offset_rect.left   -= offset_x;
				offset_rect.right  -= offset_x;
				offset_rect.top    -= offset_y;
				offset_rect.bottom -= offset_y;
				invertSelectRect(hbkWorkHDC, offset_rect);
				::BitBlt(hdc, paintRect.left, paintRect.top, paintRect.right-offset_x, paintRect.bottom-offset_y, hbkWorkHDC, paintRect.left-offset_x, paintRect.top-offset_y, SRCCOPY);
				::SelectObject(hbkWorkHDC, holdWorkBitmap);
			}
			::DeleteDC(hbkWorkHDC);
		}
	} else {
		::BitBlt(hdc, paintRect.left, paintRect.top, paintRect.right, paintRect.bottom, hbkHDC, paintRect.left-offset_x, paintRect.top-offset_y, SRCCOPY);
	}
	::SelectObject(hbkHDC, holdBitmap);
	::DeleteDC(hbkHDC);
}

static bool isText(TxtMiru::TextType tt)
{
	return TxtMiruType::isTextOrRotateNumOrKuChar(tt)
		|| tt == TxtMiru::TT_SMALL_NOTE
			|| tt == TxtMiru::TT_GUID_MARK
				|| tt == TxtMiru::TT_SUP_NOTE
					|| tt == TxtMiru::TT_SUB_NOTE
						;
}

bool CGrTxtCanvas::IsTextArea(int x, int y)
{
	TextCharPoint tcp;
	return getHitCharPointMap(x, y, tcp);
}
int CGrTxtCanvas::GetLinkIndex(int x, int y)
{
	if(x < 0){
		return -1;
	}
	POINT pos = {x,y};
	for(const auto &lb : m_linkBox){
		if(PtInRect(&(lb.rect), pos)){
			return lb.id;
		}
	}
	return -1;
}
int CGrTxtCanvas::GetCurrentHoverLink()
{
	return m_iHoverLink;
}
void CGrTxtCanvas::SetCurrentHoverLink(int iHoverLink)
{
	m_iHoverLink = iHoverLink;
}
LPCTSTR CGrTxtCanvas::GetLinkURL(int id)
{
	for(const auto &lb : m_linkBox){
		if(lb.id == id){
			return lb.url.c_str();
		}
	}
	return _T("");
}

// 初回クリック用 文字位置判定
bool CGrTxtCanvas::getHitCharPointMap(int x, int y, TextCharPoint &out_tcp)
{
	int lines = m_pDoc->GetConstLayout().GetMaxLine();
	int line_width = m_fontTextSize;
	for(const auto &charpoint : m_txtMap.GetCharPointMap()){
		const auto &tp = charpoint.first;
		const auto &cp = charpoint.second;
		if(cp.ch_h <= 0){ // 注記を除外するために 高さ 0 の文字は選択しないように
			continue;
		}
		if(cp.x < x && (x < cp.x + cp.w || x < cp.x + line_width)
		   && cp.y < y && y < cp.y + cp.h){
			try {
				const auto &li = m_pDoc->GetLineInfo(tp.iLine);
				const auto &ti = li.text_list[tp.iIndex];
				if(::isText(ti.textType)){
					if(!isDrawArea(cp.lcol, lines)){
						return false;
					}
					out_tcp.bEnable = true;
					out_tcp.tp = tp;
					out_tcp.cp = cp;
					return true;
				}
			} catch(...){
				return false;
			}
		}
	}
	out_tcp.bEnable = false;
	return false;
}

// ドラッグ用 文字位置判定
bool CGrTxtCanvas::getHitLastCharPointMap(int x, int y, TextCharPoint &out_tcp)
{
	if(!m_selectBeginPoint.bEnable){
		return getHitCharPointMap(x, y, out_tcp);
	}
	POINT point = {x, y};
	RECT begin_rect = {m_selectBeginPoint.cp.x, m_selectBeginPoint.cp.y, m_selectBeginPoint.cp.x+m_selectBeginPoint.cp.w, m_selectBeginPoint.cp.y+m_selectBeginPoint.cp.h};
	if(PtInRect(&begin_rect, point)){
		return getHitCharPointMap(x, y, out_tcp);
	}
	int lines = m_pDoc->GetConstLayout().GetMaxLine();
	// 多段レイアウト時の選択制限を行う範囲を取得
	LayoutBox layoutbox;
	layoutbox.max_line = 0;
	layoutbox.min_line = lines;
	{
		auto bSameArea = true;
		LayoutBox same_lb = {};
		if(point.x < 0){ point.x = 0; } else if(point.x > m_rectWindow.right ){ point.x=m_rectWindow.right ; } // ウインドウ範囲外の場合、どのレイアウトボックスの範囲か
		if(point.y < 0){ point.y = 0; } else if(point.y > m_rectWindow.bottom){ point.y=m_rectWindow.bottom; } // わからないのではみ出し部分を強制的にウインドウ内に置き換える
		int len=m_layoutBox.size();
		if(len <= 0){ return false; }
		const auto *l_it = &m_layoutBox[0];
		POINT begin_pos={m_selectBeginPoint.cp.x, m_selectBeginPoint.cp.y};
		for(; len>0; --len, ++l_it){
			const auto &lb = *l_it;
			if(PtInRect(&lb.rc, point)){
				bSameArea = false;
				if(layoutbox.max_line < lb.max_line){
					layoutbox.max_line = lb.max_line;
				}
				if(layoutbox.min_line > lb.min_line){
					layoutbox.min_line = lb.min_line;
				}
				if(PtInRect(&lb.rc, begin_pos)){
					layoutbox = lb;
					break;
				}
			} else if(PtInRect(&lb.rc, begin_pos)){
				same_lb = lb;
			}
		}
		if(bSameArea){ // マウスの位置がどのレイアウトにも属していない場合、最初にクリックしたレイアウトボックスの範囲内とする
			layoutbox = same_lb;
		}
	}
	layoutbox.min_line += m_startLine;
	layoutbox.max_line += m_startLine;
	const auto &cpMap = m_txtMap.GetCharPointMap();
	{ // 縦中横の選択対応
		for (const auto& item : cpMap) {
			const auto& tp = item.first;
			const auto& cp = item.second;
			if (point.y >= cp.y && point.y <= (cp.y + cp.h)
				&& point.x >= cp.x && point.x <= (cp.x + cp.w)) {
				try {
					const auto& li = m_pDoc->GetLineInfo(tp.iLine);
					const auto& ti = li.text_list[tp.iIndex];
					if (::isText(ti.textType)) {
						out_tcp.bEnable = true;
						out_tcp.tp = tp;
						out_tcp.cp = cp;
						return true;
					}
				}
				catch (...) {
					return false;

				}
			}
		}
	}
	bool b_prev = (x > begin_rect.right || (x > begin_rect.left && y < begin_rect.top));
	if(b_prev){
		// 右へドラッグ
		for(const auto &item : cpMap){
			const auto &tp = item.first;
			const auto &cp = item.second;
			if(cp.ch_h <= 0 || // 注記を除外するために 高さ 0 の文字は選択しないように
			   !isDrawArea(cp.lcol, lines) || layoutbox.min_line > cp.lcol || cp.lcol > layoutbox.max_line){
				continue;
			}
			if(point.x > (cp.x + cp.w) || (point.x > cp.x && point.y < (cp.y + cp.h))){
				try {
					const auto &li = m_pDoc->GetLineInfo(tp.iLine);
					const auto &ti = li.text_list[tp.iIndex];
					if(::isText(ti.textType)){
						out_tcp.bEnable = true;
						out_tcp.tp = tp;
						out_tcp.cp = cp;
						return true;
					}
				} catch(...){
					return false;
				}
			}
		}
		return false;
	} else {
		// 左へドラッグ
		for(const auto &item : cpMap){
			const auto &tp = item.first;
			const auto &cp = item.second;
			if(cp.ch_h <= 0 || // 注記を除外するために 高さ 0 の文字は選択しないように
			   !isDrawArea(cp.lcol, lines) || layoutbox.min_line > cp.lcol || cp.lcol > layoutbox.max_line){
				continue;
			}
			if(cp.x > point.x || (cp.x + cp.w) > point.x && point.y > cp.y){
				try {
					const auto &li = m_pDoc->GetLineInfo(tp.iLine);
					const auto &ti = li.text_list[tp.iIndex];
					if(::isText(ti.textType)){
						out_tcp.bEnable = true;
						out_tcp.tp = tp;
						out_tcp.cp = cp;
					}
				} catch(...){
					return false;
				}
			}
		}
		if(out_tcp.bEnable){ return true; }
	}
	out_tcp.bEnable = false;
	return false;
}

void CGrTxtCanvas::SetBeginSelect(const TxtMiru::TextPoint &tp)
{
	m_selectBeginPoint.bEnable = true;
	m_selectBeginPoint.tp = tp;
	m_selectBeginPoint.cp = m_txtMap.GetCharPoint(tp);
}

void CGrTxtCanvas::SetEndSelect(const TxtMiru::TextPoint &tp)
{
	m_selectEndPoint.bEnable = true;
	m_selectEndPoint.tp = tp;
	m_selectEndPoint.cp = m_txtMap.GetCharPoint(tp);
}

bool CGrTxtCanvas::SetBeginSelect(const POINT &pos)
{
	if(!m_pDoc){ return false; }
	bool bSelect = m_selectBeginPoint.bEnable;
	m_selectEndPoint.bEnable = true;
	if(getHitCharPointMap(pos.x, pos.y, m_selectBeginPoint)){
		m_selectEndPoint = m_selectBeginPoint;
		m_selectEndPoint.bEnable = false;
		return true;
	} else if(bSelect){
		return true;
	}
	return false;
}

bool CGrTxtCanvas::SetEndSelect(const POINT &pos)
{
	if(!m_pDoc){ return false; }
	bool bSelect = m_selectBeginPoint.bEnable && !m_selectEndPoint.bEnable;
	if(bSelect && getHitLastCharPointMap(pos.x, pos.y, m_selectEndPoint)){
		m_selectEndPoint.bEnable = false;
		return true;
	}
	return false;
}

void CGrTxtCanvas::ValidateSelect(bool bSelect)
{
	m_selectEndPoint.bEnable = bSelect;
}

bool CGrTxtCanvas::GetSelectTextPoint(TxtMiru::TextPoint &tpb, TxtMiru::TextPoint &tpe)
{
	if(!m_selectBeginPoint.bEnable){
		return false;
	}
	auto selectBeginPoint = m_selectBeginPoint;
	auto selectEndPoint   = m_selectEndPoint  ;
	if(selectEndPoint.tp < selectBeginPoint.tp){
		Swap(selectBeginPoint, selectEndPoint);
	}
	tpb = selectBeginPoint.tp;
	tpe = selectEndPoint.tp;
	return true;
}

// 選択範囲を反転表示に ※ここで、開始位置と終了位置を元に反転する文字を特定する
void CGrTxtCanvas::invertSelectRect(HDC hdc, const RECT &rect)
{
	if(!m_selectBeginPoint.bEnable){
		return;
	}
	CGrBitmap bmp;
	if(!bmp.Create(m_bmpCanvas.Width(), m_bmpCanvas.Height())){
		return;
	}
	auto hbkHDC = ::CreateCompatibleDC(hdc);
	auto holdWorkBitmap = static_cast<HBITMAP>(::SelectObject(hbkHDC, bmp));
	SetBkColor(hbkHDC, RGB(0xff,0xff,0xff));

	auto selectBeginPoint = m_selectBeginPoint;
	auto selectEndPoint   = m_selectEndPoint  ;
	if(selectEndPoint.tp < selectBeginPoint.tp){
		Swap(selectBeginPoint, selectEndPoint);
	}
	int lines = m_pDoc->GetConstLayout().GetMaxLine(); // lines = size.cx
	for(const auto &item : m_txtMap.GetCharPointMap()){
		const auto &tp = item.first;
		const auto &cp = item.second;
		if(cp.ch_h > 0 && isDrawArea(cp.lcol, lines) && selectBeginPoint.tp <= tp && tp <= selectEndPoint.tp){
			try {
				const auto &li = m_pDoc->GetLineInfo(tp.iLine);
				const auto &ti = li.text_list[tp.iIndex];
				if(::isText(ti.textType)){ // 注記を除外するために 高さ 0 の文字は選択しないように
					RECT rc={cp.x,cp.y,cp.x+cp.w,cp.y+cp.h};
					::ExtTextOut(hbkHDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
				}
			} catch(...){
				break;
			}
		}
	}
	::BitBlt(hdc, 0, 0, rect.right, rect.bottom, hbkHDC, 0, 0, SRCINVERT);
	::SelectObject(hbkHDC, holdWorkBitmap);
	::DeleteDC(hbkHDC);
}

void CGrTxtCanvas::Clear()
{
	m_txtRenderer.ClearPict();
}

void CGrTxtCanvas::ClearCache()
{
	m_txtRenderer.ClearCache();
}

void CGrTxtCanvas::SetAntialias(int iAntialias)
{
	if(GetAntialias() != iAntialias){
		m_txtRenderer.SetAntialias(iAntialias);
		Resize(m_rectWindow.right, m_rectWindow.bottom);
		Update();
	}
}

int CGrTxtCanvas::GetAntialias()
{
	return m_txtRenderer.GetAntialias();
}

