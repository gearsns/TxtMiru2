#pragma warning( disable : 4786 )

#include "TxtMiru.h"
#include "TxtMapper.h"

#include <algorithm>
#include "stlutil.h"
#include "TxtMiruTextType.h"

//#define __DBG__
//#define __DBG2__
#include "Debug.h"

const int text_l_pre_size = 360; // 文字サイズの変更を 1.2刻みで5段階まで すべて整数でチェックするには 360 倍にする必要がある
const int text_l_4byte_size      = 8*text_l_pre_size;
const int text_l_2byte_size      = 4*text_l_pre_size;
const int text_l_1byte_size      = 2*text_l_pre_size;
const int text_l_half_2byte_size = 2*text_l_pre_size;
const int text_l_half_1byte_size = 1*text_l_pre_size;
const int l_max_offsetnum        = text_l_2byte_size;
// http://www.nminoru.jp/~nminoru/programming/bitcount.html
class TextOffsetTable {
	static int numofbits(long bits) {
		bits = (bits & 0x55555555) + (bits >> 1 & 0x55555555);
		bits = (bits & 0x33333333) + (bits >> 2 & 0x33333333);
		bits = (bits & 0x0f0f0f0f) + (bits >> 4 & 0x0f0f0f0f);
		bits = (bits & 0x00ff00ff) + (bits >> 8 & 0x00ff00ff);
		return (bits & 0x0000ffff) + (bits >>16 & 0x0000ffff);
	}
	static bool bCalc;
public:
	TextOffsetTable(){
		if(!bCalc){
			bCalc = true;
			for(long i=0; i!=0x100; ++i){
				long x = i & 0xf0;
				long y = i & 0x0f;
				byteArrayW[i] = numofbits(x) * text_l_half_1byte_size;
				byteArrayH[i] = numofbits(y) * text_l_half_1byte_size;
				if(x & 0x80){
					byteArrayX[i] = 0;
				} else if(x & 0x40){
					byteArrayX[i] = text_l_half_1byte_size;
				} else if(x & 0x20){
					byteArrayX[i] = text_l_half_1byte_size + text_l_half_1byte_size;
				} else if(x & 0x10){
					byteArrayX[i] = text_l_half_1byte_size + text_l_half_1byte_size + text_l_half_1byte_size;
				} else {
					byteArrayX[i] = 0;
				}
				if(y & 0x08){
					byteArrayY[i] = 0;
				} else if(y & 0x04){
					byteArrayY[i] = text_l_half_1byte_size;
				} else if(y & 0x02){
					byteArrayY[i] = text_l_half_1byte_size + text_l_half_1byte_size;
				} else if(y & 0x01){
					byteArrayY[i] = text_l_half_1byte_size + text_l_half_1byte_size + text_l_half_1byte_size;
				} else {
					byteArrayY[i] = 0;
				}
			}
			byteArrayX[0] = l_max_offsetnum;
			byteArrayY[0] = l_max_offsetnum;
			byteArrayW[0] = 0;
			byteArrayH[0] = 0;
		}
	}
	static UINT byteArrayW[0x100];
	static UINT byteArrayH[0x100];
	static UINT byteArrayX[0x100];
	static UINT byteArrayY[0x100];
};
bool TextOffsetTable::bCalc = false;
UINT TextOffsetTable::byteArrayW[0x100] = {0};
UINT TextOffsetTable::byteArrayH[0x100] = {0};
UINT TextOffsetTable::byteArrayX[0x100] = {0};
UINT TextOffsetTable::byteArrayY[0x100] = {0};
static TextOffsetTable l_TextOffsetTable;

CGrTxtMapper::~CGrTxtMapper()
{
}

static bool HalfChar(DWORD type)
{
	return (0x0040 == type || 0x0048 == type || 0x0440 == type || 0x8040 == type || 0x0448 == type || 0xF048 == type);
}

// 縦中横 文字のチェック
inline static bool isTateChuYoko(const TxtMiru::TextInfoList &text_list, int iIndex, DWORD chrType, LPCTSTR lpSrc, int tatechunum){
	if(((chrType == 0x0040 || chrType == 0xF048)) && (CGrText::CharLen(lpSrc) <= tatechunum)){
		if(iIndex > 0){
			{
				// 一つ前が半角の空白文字ならもうひとつ前の文字を参照する
				const auto &tt = text_list[iIndex-1];
				if(iIndex > 1 && tt.textType == TxtMiru::TextType::TEXT && tt.str.c_str()[0] == L' '){
					--iIndex;
				}
			}
			{
				// 前の文字が半角文字なら縦中横にしない
				const auto &tt = text_list[iIndex-1];
				if(HalfChar(tt.chrType) && tt.textType == TxtMiru::TextType::TEXT){
					{ // 最低でもアルファベットを含むことを条件にする
						for(const auto ch : tt.str){
							if((ch >= L'A' && ch <= 'Z')
							   || (ch >= L'a' && ch <= 'z')){
								return false;
							}
						}
					}
				}
			}
		}
		return true;
	} else {
		return false;
	}
}

const TxtMiru::CharPoint &CGrTxtMapper::GetCharPoint(const TxtMiru::TextPoint &tp)
{
	return m_cpMap[tp];
}
const TxtMiru::CharPoint &CGrTxtMapper::GetLayoutCharPoint(const TxtMiru::TextPoint &tp)
{
	return m_cpLayoutMap[tp];
}

TxtMiru::TextPoint CGrTxtMapper::NextTextPoint(const TxtMiru::TextPoint &tp, const TxtMiru::TextInfoList &text_list)
{
	TxtMiru::TextPoint nextTp = tp;
	if(static_cast<signed int>(text_list.size()) <= tp.iIndex){
		nextTp.iPos = -1;
		++nextTp.iIndex;
		return nextTp;
	}
	auto it=text_list.begin()+tp.iIndex, e=text_list.end();
	int iPos = tp.iPos;
	auto lpNextSrc = it->str.c_str();
	for(; *lpNextSrc && iPos >= 0; --iPos){
		lpNextSrc = CGrText::CharNext(lpNextSrc);
	}
	while(it != e){
		++it;
		if(*lpNextSrc){
			++nextTp.iPos;
			break;
		}
		nextTp.iPos = -1;
		++nextTp.iIndex;
		if(it == e){ break; }
		lpNextSrc = it->str.c_str();
	}
	return nextTp;
}

struct CGrMapStringCompare : public CGrTxtParam::CGrStringCompare
{
	LPCTSTR lpTarget;
	CGrMapStringCompare() : lpTarget(nullptr){}
	CGrMapStringCompare(LPCTSTR lpSrc) : lpTarget(lpSrc){}
	void operator = (LPCTSTR lpSrc){ lpTarget = lpSrc; }
	virtual bool operator == (LPCTSTR lpSrc){ return CGrText::isMatchChar(lpTarget, lpSrc); }
};

static bool isLineStartNGCharacters(const CGrTxtParam &param, LPCTSTR lpNextSrc)
{
	int matchNum = 0;
	CGrMapStringCompare sc;
	while(*lpNextSrc){
		sc = lpNextSrc;
		lpNextSrc = CGrText::CharNext(lpNextSrc);
		if(param.FindIF(CGrTxtParam::ValueType::LineStartNGCharacters, static_cast<CGrTxtParam::CGrStringCompare &&>(sc)) >= 0){
			++matchNum;
			if(matchNum > 3){ // 3 文字以上は、ぶら下げない
				return false;
			}
		} else {
			break;
		}
	}
	return (matchNum != 0);
}

static bool isLineStartNGCharacters(const CGrTxtParam &param, TxtMiru::TextInfo *it, int len, LPCTSTR lpSrc, TxtMiru::TextType textType=TxtMiru::TextType::TEXT)
{
	std::tstring str1st;
	int matchNum = 0;
	auto lpNextSrc = lpSrc;
	if(!*lpNextSrc){
		++it;
		--len;
		if(len > 0){
			lpNextSrc = it->str.c_str();
		} else {
			return false;
		}
	}
	CGrMapStringCompare sc;
	for(;len > 0; ++it, --len){
		if(lpNextSrc == lpSrc){
			lpSrc = nullptr;
		} else {
			lpNextSrc = it->str.c_str();
		}
		bool bBreak = true;
		if(TxtMiruType::isRotateNum(it->textType)
		   ){
			bBreak = true;
			while(*lpNextSrc){
				sc = lpNextSrc;
				lpNextSrc = CGrText::CharNext(lpNextSrc);
				if(param.FindIF(CGrTxtParam::ValueType::LineStartNGCharacters, static_cast<CGrTxtParam::CGrStringCompare&&>(sc)) >= 0){
					bBreak = false;
				} else {
					bBreak = true;
					break;
				}
				if (!lpNextSrc) {
					bBreak = true;
					break;
				}
			}
			if(bBreak){
				break;
			} else {
				++matchNum;
			}
		} else if(it->textType == textType){
			while(*lpNextSrc){
				if(str1st.empty()){
					str1st = std::tstring(lpNextSrc, CGrText::CharNext(lpNextSrc));
				}
				bBreak = true;
				sc = lpNextSrc;
				lpNextSrc = CGrText::CharNext(lpNextSrc);
				if (!lpNextSrc) {
					bBreak = true;
					break;
				}
				if(param.FindIF(CGrTxtParam::ValueType::LineStartNGCharacters, static_cast<CGrTxtParam::CGrStringCompare&&>(sc)) >= 0){
					++matchNum;
					bBreak = false;
					// 3 文字以上は、ぶら下げないが
					//  2 文字目が 1文字目と違う場合は 取りあえず ぶら下げ文字としておく
					// 「 。・・・」 の時、この条件にひっかるので「。」が行頭にきてしまう。取りあえず、これで様子をみる
					if(matchNum == 2){
						if(sc == str1st.c_str()){
							;
						} else {
							bBreak = true;
							break;
						}
					} else if(matchNum > 3){ // 3 文字以上は、ぶら下げない
						matchNum = 0;
						break;
					}
				}
				if(bBreak){ break; }
			}
		}
		if(bBreak){ break; }
	}
	return (matchNum != 0);
}

static void copyCharPointMap(TxtMiru::CharPoint &cp_to, const TxtMiru::CharPoint &cp_from)
{
	cp_to = cp_from;
	cp_to.zx = cp_from.x;
	cp_to.zy = cp_from.y;
}

class CGrPointMapper
{
	const CGrTxtDocument         &m_doc;
	const CGrTxtBuffer           &m_txtBuffer    ;
	const TxtMiru::TextOffsetMap &m_textOffsetMap;
	const TxtMiru::TxtLayoutList &m_textLayout   ;
	const TxtMiru::TxtLayoutList &m_noteLayout   ;
	const CGrTxtParam            &m_param        ;
	TxtMiru::CharPointMap        &m_cpMap;

	const int m_maxLine; // 画面内に表示できる最大行数
	int m_iLayoutMaxNum = 0;
	int m_iStartLine = 0;
	int m_iCurTxtLayout           = 0;
	int m_iCurTextLayoutMaxLines  = 0;
	int m_iCenterILine            = -1;

	// 用紙サイズに、基づき位置情報を設定(画面への座標変換はここでは行わない)
	struct CurPoint
	{
		CurPoint(const CGrTxtDocument &doc)
		{
			const auto &param = CGrTxtMiru::theApp().Param();
			txtLayout.left = txtLayout.top = txtLayout.right = txtLayout.bottom = -1;
			const auto &layout = doc.GetConstLayout();
			lsText = layout.GetLineSize(CGrTxtLayout::LineSizeType::Text);
			lsRuby = layout.GetLineSize(CGrTxtLayout::LineSizeType::Ruby);
			lsNote = layout.GetLineSize(CGrTxtLayout::LineSizeType::Note);
			izLineWidth = lsText.width + lsText.space + lsRuby.width + lsRuby.space;
			param.GetText(CGrTxtParam::TextType::NoteFormat, noteFmt);
			iCurTextWidth = lsText.width;
			bRubyPosition = param.GetBoolean(CGrTxtParam::PointsType::RubyPosition);
		}
		void SetTxtLayout(const TxtMiru::TxtLayout tl){
			txtLayout = tl;
			cp.x = txtLayout.right + lsText.space + lsRuby.width + lsRuby.space;
		}
		void SetLine(const TxtMiru::LineInfo &li, int line, int iStartLine){
			NextLine(li, iStartLine);
			tpCur.iLine = line;
		}
		int calcCurX(int col){
			return txtLayout.right - (col-m_iStartLayoutLine+1) * izLineWidth + lsText.space + lsRuby.width + lsRuby.space;
		}
		void NextLine(const TxtMiru::LineInfo &li, int iStartLine){
			m_pTTP = &li.tlpTurnList;
			tpCur.iPos   = 0;
			tpCur.iIndex = 0;
			iTurn  = 0;
			ttpCur = li.tlpTurnList[iTurn];
			if(m_pTTP->size() > 1){
				// tlpTurnListが iTurnより少ない場合に備えて tlpTurnListの範囲内の場合だけ TextTurnPos を更新する
				ttpCur.iIndex   = (*m_pTTP)[1].iIndex  ;
				ttpCur.iPos     = (*m_pTTP)[1].iPos    ;
			}
			++tpCur.iLine;
			cp.x = calcCurX(li.iStartCol);
			cp.lcol = li.iStartCol; // 全頁を通しての画面右からの行数
			//
			SetTopIndent(li.iFirstIndent, li.iSecondIndent);
			if(ttpCur.iRIndent >= 0){
				SetBottomIndent(ttpCur.iRIndent); // 行末からのインデント開始
			} else {
				SetBottomIndent(li.iRIndent);
			}
			izTopIndent = izTopIndent1st;
			calcLine();
			bKeepPos = false;
		}
		void NextPos       (){ nextPos(cp.h   ); }
		void NextPosNoSpace(){ nextPos(cp.ch_h); }
		auto Get1stChar(LPCTSTR lpSrc){
			if(tpCur.iPos > 0){
				return CGrText::CharNextN(lpSrc, tpCur.iPos);
			}
			return lpSrc;
		}
		void BeginHorz(int len)
		{
			// h <-> w
			auto t = cp.ch_h;
			cp.ch_h = cp.ch_w;
			cp.ch_w = t;
			cp.h    = cp.w;
			cp.w    = cp.ch_w;
			//
			auto width = cp.ch_w * len;
			cp.x += iCurTextWidth / 2 - width / 2;
		}
		void EndHorz()
		{
			cp.x = old_cp.x;
			if(!isSkipChar()){
				cp.y = old_cp.y + cp.h;
				++iCharacters;
			}
		}
		void NextHorzPos(){
			++tpCur.iPos;
			cp.x += cp.w;
		}
		void BeginOverlap(TxtMiru::CharPointMap &cp_map, const TxtMiru::TextListPos &tlp)
		{
			auto it = cp_map.find(TxtMiru::TextParserPoint(tpCur.iLine, tlp));
			if(it != cp_map.end()){
				cp.y = it->second.y;
				cp.h = it->second.h;
			}
		}
		void EndOverlap()
		{
			cp = old_cp;
		}
		void DrawLine(const TxtMiru::TextInfoList &text_list, TxtMiru::CharPointMap &cp_map, const TxtMiru::TextInfo &ti, int dir){
			auto cp_bk = cp;
			auto iCurRubySpace = lsRuby.space + lsRuby.space*iCharSize / 8;
			auto x = (iCurTextWidth + iCurRubySpace) * dir; // 本来、幅は範囲をチェックして最大の幅で行う必要がある？？
			TxtMiru::TextPoint ref_tp = TxtMiru::TextParserPoint(tpCur.iLine, ti.tpBegin);
			TxtMiru::TextPoint end_tp = TxtMiru::TextParserPoint(tpCur.iLine, ti.tpEnd  );
			int size = text_list.size();
			while(ref_tp <= end_tp){
				if(cp_map.find(ref_tp) == cp_map.end()){
					ref_tp = CGrTxtMapper::NextTextPoint(ref_tp, text_list);
					++tpCur.iPos;
					continue;
				} else if(ref_tp.iIndex < size && ref_tp.iIndex >= 0 && ref_tp.iPos != 0 && text_list[ref_tp.iIndex].textType == TxtMiru::TextType::OTHER){ // 注記の外枠への表示部分は対象外(iPos ≧ 1 以降に注記が入る)
					++ref_tp.iIndex;
					ref_tp.iPos = 0;
					continue;
				} else if(text_list[ref_tp.iIndex].textType == TxtMiru::TextType::RUBY){ // ルビには、線とは引かない
					++ref_tp.iIndex;
					ref_tp.iPos = 0;
					continue;
				}
				const auto &ref_cp = cp_map[ref_tp];
				ref_tp = CGrTxtMapper::NextTextPoint(ref_tp, text_list);
				cp = ref_cp;
				cp.w = iCurTextWidth;
				cp.x += x;
				SetCharPointMap(cp_map);
				++tpCur.iPos;
			}
			cp = cp_bk;
		}
		void NextIndex()
		{
			tpCur.iPos = 0;
			++tpCur.iIndex;
		}
		bool IsTurnup(){
			return (ttpCur.iIndex != 0 || ttpCur.iPos != 0/*一文字目以外で*/) && ((tpCur.iIndex > ttpCur.iIndex) || (tpCur.iIndex == ttpCur.iIndex && tpCur.iPos >= ttpCur.iPos));
		}
		void Turnup(){
			ttpCur.iIndex = 0;
			ttpCur.iPos   = 0;
			if(m_pTTP){
				int len = m_pTTP->size()-1;
				if(iTurn < len){
					++iTurn;
					ttpCur.iNum         = (*m_pTTP)[iTurn].iNum        ;
					ttpCur.iHeight      = (*m_pTTP)[iTurn].iHeight     ;
					ttpCur.iLineNoInAll = (*m_pTTP)[iTurn].iLineNoInAll;
					ttpCur.iRIndent     = (*m_pTTP)[iTurn].iRIndent    ;
					if(len > iTurn){
						// tlpTurnListが iTurnより少ない場合に備えて tlpTurnListの範囲内の場合だけ TextTurnPos を更新する
						ttpCur.iIndex   = (*m_pTTP)[iTurn+1].iIndex;
						ttpCur.iPos     = (*m_pTTP)[iTurn+1].iPos  ;
					}
				}
			}
			resetKeepPos();
			cp.lcol = ttpCur.iLineNoInAll; // 全頁を通しての画面右からの行数
			cp.x = calcCurX(cp.lcol);
			izTopIndent = izTopIndent2nd;
			if(ttpCur.iRIndent >= 0){
				SetBottomIndent(ttpCur.iRIndent);
			}
			calcLine();
		}
		//
		// iStartLine : 現在表示ページの論理開始行
		const TxtMiru::TextInfo *skipFirst(const TxtMiru::TextInfo *it, int &len, int iStartLine, CGrPointMapper *pPM)
		{
			if(!m_pTTP || cp.lcol >= iStartLine){
				return it;
			}
			int iIndex = 0;
			int i = -1;
			for(const auto &ttp : *m_pTTP){
				if(ttp.iLineNoInAll >= iStartLine){
					tpCur.iIndex  = ttp.iIndex;
					tpCur.iPos    = ttp.iPos  ;
					iIndex = ttp.iIndex;
					iTurn  = i;
					// レイアウト番号を再計算
					pPM->setSkipLine(ttp.iLineNoInAll);
					Turnup();
					break;
				}
				++i;
			}
			len -= iIndex;
			return it + iIndex;
		}
		//
		void SetTextType(const TxtMiru::TextInfo &ti){
			ttCur = ti.textType;
			switch(ti.textType){
			case TxtMiru::TextType::SMALL_NOTE    : // through
			case TxtMiru::TextType::SMALL_NOTE_R  : // through
			case TxtMiru::TextType::SUP_NOTE      : setHalfChar(ti.chrType, true ); break;
			case TxtMiru::TextType::SMALL_NOTE_L  : // through
			case TxtMiru::TextType::SUB_NOTE      : // through
			case TxtMiru::TextType::GUID_MARK     : setHalfChar(ti.chrType, false); break;
			case TxtMiru::TextType::SKIP_CHAR     :
				cp.h = cp.w = cp.ch_w = cp.ch_h = 0;
				resetKeepPos();
				break;
			case TxtMiru::TextType::KU1_CHAR      : // through
			case TxtMiru::TextType::KU2_CHAR      :
				cp.ch_h = lsText.width * 2;
				cp.h    = cp.ch_h + izTextSpace;
				cp.ch_w = cp.w = iCurTextWidth;
				resetKeepPos();
				break;
			default:
				if(HalfChar(ti.chrType)){
					cp.ch_h = iCurTextWidth / 2;
				} else {
					cp.ch_h = iCurTextWidth;
				}
				cp.h    = cp.ch_h + izTextSpace;
				cp.ch_w = cp.w = iCurTextWidth;
				resetKeepPos();
				break;
			}
		}
		void SetCharPointMap(TxtMiru::CharPointMap &cp_map)
		{
			copyCharPointMap(cp_map[tpCur], cp);
			auto &&item = cp_map[tpCur];
			if(isSkipChar()){
				item.h = item.w = item.ch_w = item.ch_h = 0;
			}
			// フォントサイズが変更されている場合は、位置をずらす
			if(iCharSize != 0){
				auto sx = (iCurTextWidth - lsText.width) / 2;
				item.zx -= sx;
				item.x -= sx;
			}
		}
		void SetTopIndent(int indent1, int indent2, bool bCalc = false)
		{
			auto num = max(txtLayout.characters, 1);
			auto height = txtLayout.bottom - txtLayout.top;
			// 高さより字下げの文字数ほうが多い場合、最低一文字は表示されるように調整
			if(indent1 >= txtLayout.characters){
				indent1 = txtLayout.characters - 1;
			}
			if(indent2 >= txtLayout.characters){
				indent2 = txtLayout.characters - 1;
			}
			// 文字間隔があると字下げの数がずれて表示されるので、均等割りした場合の高さで計算する
			izTopIndent1st = indent1 * height / num;
			izTopIndent2nd = indent2 * height / num;
			if(bCalc){
				izTopIndent = izTopIndent1st;
				calcLine();
			}
		}
		void SetBottomIndent(int indent, int iheight = -1)
		{
			izBottomIndent = indent * lsText.width;
			if(iheight > 0){
				ttpCur.iHeight -= iheight;
			}
			calcLine(false);
		}
		int GetLine(){ return cp.lcol; }
		void setNote(TxtMiru::CharPointMap &cp_map, LPCTSTR lpNextSrc, TxtMiru::CharPoint &note_cp, int w_half, int w)
		{
			while(*lpNextSrc){
				auto lpSrc = lpNextSrc;
				lpNextSrc = CGrText::CharNext(lpSrc);
				if(HalfChar(CGrText::GetStringTypeEx(lpSrc, lpNextSrc-lpSrc))){
					note_cp.h = w_half;
				} else {
					note_cp.h = w;
				}
				// 注記を除外するために 高さ 0 の文字は選択しないように 対応
				copyCharPointMap(cp_map[tpCur], note_cp);
				++tpCur.iPos;
				note_cp.y += note_cp.h;
			}
		}
		void Note(const TxtMiru::TxtLayout &noteLayout, const TxtMiru::TextInfoList &text_list, TxtMiru::CharPointMap &cp_map, const TxtMiru::TextInfo &ti)
		{
			auto lpNextSrc = ti.str.c_str();
			TxtMiru::TextPoint ref_tp = TxtMiru::TextParserPoint(tpCur.iLine, ti.tpBegin);
			auto note_cp = cp_map[ref_tp];
			auto vspace = ((noteLayout.bottom - noteLayout.top) - ((noteLayout.characters * text_l_2byte_size + text_l_1byte_size) * lsNote.width / text_l_2byte_size)) / (noteLayout.characters/4/*Layoutは文字４分割を最小単位で数える*/-1);
			if(vspace <= 0){
				vspace = 1;
			}
			if(iNoteNum == 0){
				cpNote.y = noteLayout.top;
				cpNote.x = noteLayout.right - lsNote.width - lsNote.space;
			} else {
				cpNote.x -= (lsNote.width + lsNote.space);
			}
			++iNoteNum;
			TCHAR buf[512];
			_stprintf_s(buf, noteFmt.c_str(), iNoteNum);

			note_cp.w = lsRuby.width;
			if(ti.textType != TxtMiru::TextType::OTHER){
				note_cp.x += lsText.width + lsRuby.space;
			}
			setNote(cp_map, buf, note_cp, lsRuby.width / 2, lsRuby.width);
			//
			note_cp.y = cpNote.y;
			note_cp.x = cpNote.x;
			note_cp.h    = lsNote.width;
			note_cp.ch_h = 0; // 注記を選択から除外するために 高さ 0 の文字に
			note_cp.w    = lsNote.width + lsNote.space;
			note_cp.ch_w = lsNote.width;
			setNote(cp_map, buf, note_cp, lsNote.width / 2, lsNote.width);
			//
			note_cp.y += (lsNote.width / 2 + vspace);
			setNote(cp_map, lpNextSrc, note_cp, lsNote.width / 2 + vspace, lsNote.width + vspace);
		}
		void recalcRubyPos(TxtMiru::CharPointMap &cp_map, int left, int top, int bottom, const std::vector<TxtMiru::CharPoint> &ch_map, int lcol, int iPos)
		{
			if(left > (cp.x + cp.ch_w + lsRuby.space)){
				int new_x = -1;
				for(const auto &it : ch_map){
					if(lcol != it.lcol/*同一行のみ対象に*/ || it.y < top){
						continue;
					}
					if(it.y > bottom){
						break;
					}
					if(new_x == -1 || new_x < it.x + it.ch_w + lsRuby.space){
						new_x = it.x + it.ch_w + lsRuby.space;
					}
				}
				if(new_x < left){
					auto tmpTP = tpCur;
					tmpTP.iPos = iPos;
					for(; tmpTP.iPos<tpCur.iPos; ++tmpTP.iPos){
						cp_map[tmpTP].zx = new_x;
					}
				}
			}
		}
		void setRuby(TxtMiru::CharPointMap &cp_map, LPCTSTR lpNextSrc, TxtMiru::CharPoint &ruby_cp, const std::vector<int> &ruby_h_list, int ruby_padding)
		{
			for (const auto ruby_h : ruby_h_list) {
				if ('\0' == *lpNextSrc) {
					break;
				}
				auto lpSrc = lpNextSrc;
				lpNextSrc = CGrText::CharNext(lpSrc);
				copyCharPointMap(cp_map[tpCur], ruby_cp);
				++tpCur.iPos;
				ruby_cp.y += ruby_h + ruby_padding;
			}
		}
		void DrawRuby(const TxtMiru::TextInfoList &text_list, TxtMiru::CharPointMap &cp_map, const TxtMiru::TextInfo &ti, const TxtMiru::TextOffsetMap &textOffsetMap)
		{
			TxtMiru::TextPoint tp_begin = TxtMiru::TextParserPoint(tpCur.iLine, ti.tpBegin);
			TxtMiru::TextPoint tp_end   = TxtMiru::TextParserPoint(tpCur.iLine, ti.tpEnd  );
			auto &&begin_ch = cp_map[tp_begin];
			auto &&end_ch   = cp_map[tp_end  ];
			std::vector<TxtMiru::CharPoint> ch_map;
			getCPMapRuby(ch_map, cp_map.end(), cp_map.find(tp_begin), cp_map.find(tp_end));
			auto iPrevY=getPreRubyPos(cp_map, text_list, ti.textType); // ルビ 重なりチェック用
			//
			const auto &text = ti.str;
			auto lpNextSrc = text.c_str();
			auto ruby_len = max(CGrText::CharLen(lpNextSrc), 1);
			auto nextHeight = HalfChar(ti.chrType) ? lsRuby.width / 2 : lsRuby.width;
			std::vector<int> ruby_h_list;
			ruby_h_list.resize(ruby_len);
			int ruby_all_height = 0;
			{
				// 各文字の高さを事前にチェック
				auto lpWorkNextSrc = lpNextSrc;
				for(int i=0; i<ruby_len && *lpWorkNextSrc; ++i){
					auto lpSrc = lpWorkNextSrc;
					lpWorkNextSrc = CGrText::CharNext(lpSrc);
					auto chrType = CGrText::GetStringTypeEx(lpSrc, lpWorkNextSrc-lpSrc);
					ruby_h_list[i] = HalfChar(chrType) ? lsRuby.width / 2 : lsRuby.width;
					ruby_all_height += ruby_h_list[i];
				}
			}
			//
			RECT range[2] = {
				{ -1, -1, -1, -1 },
				{ -1, -1, -1, -1 },
			};
			if(ti.textType == TxtMiru::TextType::RUBY_L || ti.textType == TxtMiru::TextType::NOTE_L){
				getRubyRangeL(range, begin_ch.lcol, ch_map); // 範囲の取得
			} else {
				getRubyRangeR(range, begin_ch.lcol, ch_map); // 範囲の取得
			}
			// □RUBY長対応
			{
				if(begin_ch.lcol == end_ch.lcol){
					auto it_tom = textOffsetMap.find(TxtMiru::TextPoint{tpCur.iLine, tpCur.iIndex, -1});
					if(it_tom != textOffsetMap.end()){
						const auto &to = it_tom->second;
						int text_height = range[0].bottom - max(range[0].top, iPrevY);
						auto tmp_ruby_all_height = ruby_all_height;
						if(range[0].top + tmp_ruby_all_height > txtLayout.bottom){
							// ルビの長さに合わせた結果、行末を超える場合は 文字間隔は 行末までで調節する
							// ※改行するかどうかは ここでは判断しない（CGrIndexCreatorでチェック済）
							tmp_ruby_all_height = txtLayout.bottom - range[0].top;
						}
						if(tmp_ruby_all_height > text_height){
							// 前の文字が、記号 又は 平仮名のときは ルビ一文字分ははみ出てもＯＫに
							if(to.y < 0){
								tmp_ruby_all_height += lsRuby.width * to.y / 2;
								range[0].top    += lsRuby.width * to.y / 2;
							}
							// 後の文字が、記号 又は 平仮名のときは ルビ一文字分ははみ出てもＯＫに
							if(to.h > 0){
								tmp_ruby_all_height -= lsRuby.width * to.h / 2;
								range[0].bottom += lsRuby.width * to.h / 2;
							}
						}
						if(tmp_ruby_all_height > text_height){
							auto padding = tmp_ruby_all_height - text_height;
							int maxIndex = text_list.size();
							std::map<int,int> text_padding_map;
							{
								for(const auto & [tp, it_cp] : cp_map){
									if(tp < tp_begin || tp_end < tp || tp.iIndex < 0 || tp.iIndex >= maxIndex){
										continue;
									}
									const auto &item = text_list[tp.iIndex];
									if(CGrTxtParser::isText(item.textType)){
										text_padding_map[it_cp.zy] = 0;
									}
								}
							}

							int padding_ch = padding / max(text_padding_map.size(),1);
							int padding_top = padding_ch / 2;
							{
								int padding_pre = padding_top;
								for(auto &&it : text_padding_map){
									int &it_padding = it.second;
									it_padding = padding_pre;
									padding_pre += padding_ch;
								}
							}
							{
								for(auto &&it : cp_map){
									const auto &tp = it.first;
									if(tp < tp_begin || tp_end < tp || tp.iIndex < 0 || tp.iIndex >= maxIndex){
										continue;
									}
									auto &&it_cp = it.second;
									const auto &item = text_list[tp.iIndex];
									if(CGrTxtParser::isText(item.textType)){
										it_cp.y += text_padding_map[it_cp.zy];
										it_cp.zy += text_padding_map[it_cp.zy];
									}
								}
							}
							cp.y += padding;
						}
					}
				}
			}
			//
			int ch_map_size = ch_map.size();
			TxtMiru::CharPoint ruby_cp {begin_ch.lcol, range[0].right, range[0].top, nextHeight, lsRuby.width, nextHeight, lsRuby.width};
			if(ch_map_size >= 2 && ruby_len < ch_map_size && (iPrevY == 0 && 0 <= ch_map[tpCur.iPos].y)/*先頭位置に、既にルビがあった場合*/){
				//  2文字以上の文字に、ルビは均等配置
				int text_height = range[0].bottom - max(range[0].top, iPrevY);
				auto ruby_padding = (text_height - ruby_all_height) / ruby_len;
				ruby_cp.h += ruby_padding;
				ruby_cp.y = max(range[0].top + ruby_padding / 2, iPrevY);
				setRuby(cp_map, lpNextSrc, ruby_cp, ruby_h_list, ruby_padding);
				recalcRubyPos(cp_map, ruby_cp.x, max(range[0].top + ruby_padding / 2, iPrevY), ruby_cp.y-ruby_cp.h, ch_map, begin_ch.lcol, 0);
				return;
			}
			// 「ルビ」の文字数と、「ルビを付ける文字」の数が同じ場合
			if(ruby_len == ch_map_size){
				if((iPrevY == 0 && 0 <= ch_map[tpCur.iPos].y)
				   || (iPrevY > 0 && iPrevY < ch_map[tpCur.iPos].y)
				   ){
					// 文字ごとに均等割
					if(bRubyPosition){
						// ルビ中付き
						while(*lpNextSrc){
							auto lpSrc = lpNextSrc;
							lpNextSrc = CGrText::CharNext(lpSrc);
							auto &&rc = range[(ch_map[tpCur.iPos].lcol == begin_ch.lcol) ? 0 : 1];
							ruby_cp.lcol = ch_map[tpCur.iPos].lcol;
							ruby_cp.y    = ch_map[tpCur.iPos].y + ch_map[tpCur.iPos].h / 2 - ruby_cp.h / 2;
							ruby_cp.x    = rc.right;
							copyCharPointMap(cp_map[tpCur], ruby_cp);
							++tpCur.iPos;
						}
					} else {
						// ルビ肩付き
						while(*lpNextSrc){
							auto lpSrc = lpNextSrc;
							lpNextSrc = CGrText::CharNext(lpSrc);
							auto &&rc = range[(ch_map[tpCur.iPos].lcol == begin_ch.lcol) ? 0 : 1];
							ruby_cp.lcol = ch_map[tpCur.iPos].lcol;
							ruby_cp.y    = ch_map[tpCur.iPos].y;
							ruby_cp.x    = rc.right;
							copyCharPointMap(cp_map[tpCur], ruby_cp);
							++tpCur.iPos;
						}
					}
					return;
				} else if(iPrevY > ch_map[tpCur.iPos].y){
					/*先頭位置に、既にルビがあった場合 重ならないように*/
					ruby_cp.y = range[0].top = iPrevY;
				}
			}
			// 「ルビを付ける文字」が複数行にまたがっているかどうか？
			if(begin_ch.lcol != end_ch.lcol){
				// 「ルビをつける文字」の高さを計算
				// ルビの配置
				int text_1st_height = range[0].bottom - max(range[0].top, iPrevY);
				int text_2nd_height = range[1].bottom - range[1].top;
				int text_height = text_1st_height + text_2nd_height;

				if(ruby_all_height <= text_height && ruby_len > 1){ // ルビの文字のほうが、短い場合は 間隔をあける
					ruby_cp.h += (text_height - ruby_all_height) / ruby_len;
					ruby_cp.y = max(range[0].top, iPrevY);
				} else {
					ruby_cp.y = range[0].bottom - text_1st_height * ruby_all_height / text_height + ruby_cp.h / 2;
					if(iPrevY != 0 && iPrevY > ruby_cp.y){
						// ルビを振る上位置に制限がある場合
						ruby_cp.y = iPrevY;
					}
				}
				int bottom = range[0].bottom-ruby_cp.h/2; // ruby_cp.h/2 = 折り返し位置調整
				const auto &param = CGrTxtMiru::theApp().Param();
				while(*lpNextSrc){
					auto lpSrc = lpNextSrc;
					lpNextSrc = CGrText::CharNext(lpSrc);
					copyCharPointMap(cp_map[tpCur], ruby_cp);
					++tpCur.iPos;
					ruby_cp.y += ruby_cp.h;
					if(bottom > 0 && ruby_cp.y > bottom){
						// ルビ行頭禁則 ぶら下げ? // 画面外にルビが配置される場合がある.......(一旦コメント)
						//if(!isLineStartNGCharacters(param, lpNextSrc)){
							bottom = 0;
							ruby_cp.y = range[1].top;
							ruby_cp.x = range[1].right;
							++ruby_cp.lcol;
						//}
					}
				}
				return;
			}
			int ruby_padding = 0;
			// そのまま均等割
			int text_height = range[0].bottom - max(range[0].top, iPrevY);
			if(ruby_all_height > text_height){
				// 「ルビ」のサイズが「ルビをつける文字」のサイズより大きい場合
				ruby_cp.y = range[0].top;
				if(ruby_cp.y + ruby_all_height > txtLayout.bottom){
					ruby_cp.y = txtLayout.bottom - ruby_all_height;
				}
				if(iPrevY != 0 && iPrevY > ruby_cp.y){
					// ルビを振る上位置に制限がある場合
					ruby_cp.y = iPrevY;
				}
			} else if(ruby_all_height < text_height){
				// 「ルビ」のサイズが「ルビをつける文字」のサイズより小さい場合
				if(ruby_len > 1){ // ルビの文字のほうが、短い場合は 間隔をあける
					ruby_padding = (text_height - ruby_all_height) / ruby_len;
					ruby_cp.y = max(range[0].top + ruby_padding / 2, iPrevY);
				} else if(ruby_len == 1){
					if(bRubyPosition){
						ruby_cp.y += text_height / 2 - ruby_cp.h / 2;
					}
				}
			}
			setRuby(cp_map, lpNextSrc, ruby_cp, ruby_h_list, ruby_padding);
		}
		void SetCharSize(int size)
		{
			iCharSize = size;
			iCurTextWidth = lsText.width + lsText.width*iCharSize / 8;
		}
		//
	private:
		void nextPos(int h){
			++tpCur.iPos;
			if(!isSkipChar()){
				cp.y += h;
				++iCharacters;
			}
		}
		bool isSkipChar(){ return (ttCur == TxtMiru::TextType::SKIP_CHAR_AUTO && iCharacters == 0); }
		// 範囲の取得(2行以上にまたがる場合は 考慮しない)
		void getRubyRangeL(RECT range[2], int lcol, const std::vector<TxtMiru::CharPoint> &ch_map)
		{
			for(const auto &it : ch_map){
				auto &&rc = range[(it.lcol == lcol) ? 0 : 1];
				if(rc.top == -1 || rc.top > it.y){
					rc.top = it.y;
				}
				if(rc.right == -1 || rc.right < it.x - lsRuby.width){
					rc.right = it.x - lsRuby.width;
				}
				if(rc.bottom == -1 || rc.bottom < it.y + it.h){
					rc.bottom = it.y + it.h;
				}
			}
			if(range[0].bottom < range[0].top){ range[0].bottom = range[0].top; }
			if(range[1].bottom < range[1].top){ range[1].bottom = range[1].top; }
		}
		void getRubyRangeR(RECT range[2], int lcol, const std::vector<TxtMiru::CharPoint> &ch_map)
		{
			for(const auto &it : ch_map){
				auto &&rc = range[(it.lcol == lcol) ? 0 : 1];
				if(rc.top == -1 || rc.top > it.y){
					rc.top = it.y;
				}
				if(rc.right == -1 || rc.right < it.x + it.ch_w + lsRuby.space){
					rc.right = it.x + it.ch_w + lsRuby.space;
				}
				if(rc.bottom == -1 || rc.bottom < it.y + it.h){
					rc.bottom = it.y + it.h;
				}
			}
			if(range[0].bottom < range[0].top){ range[0].bottom = range[0].top; }
			if(range[1].bottom < range[1].top){ range[1].bottom = range[1].top; }
		}
		// 同一行上の一つ前(上にある)のルビの位置(なければ、0)
		int getPreRubyPos(TxtMiru::CharPointMap &cp_map, const TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType)
		{
			auto it=cp_map.rbegin(), e=cp_map.rend();
			int maxIndex = text_list.size();
			if(textType == TxtMiru::TextType::RUBY_L || textType == TxtMiru::TextType::NOTE_L){
				for(;it != e; ++it){
					auto iIndex = it->first.iIndex;
					if(tpCur.iLine == it->first.iLine && iIndex >= 0 && iIndex < maxIndex && it->second.lcol == cp.lcol &&
					   (text_list[iIndex].textType == TxtMiru::TextType::RUBY_L || text_list[iIndex].textType == TxtMiru::TextType::NOTE_L
						|| (text_list[iIndex].textType == TxtMiru::TextType::ERROR_STR && !text_list[iIndex].str.empty()))){
						return it->second.y + it->second.h;
					}
				}
			} else {
				for(;it != e; ++it){
					auto iIndex = it->first.iIndex;
					if(tpCur.iLine == it->first.iLine && iIndex >= 0 && iIndex < maxIndex && it->second.lcol == cp.lcol &&
					   (text_list[iIndex].textType == TxtMiru::TextType::RUBY || text_list[iIndex].textType == TxtMiru::TextType::UNKOWN_ERROR
						|| (text_list[iIndex].textType == TxtMiru::TextType::ERROR_STR && !text_list[iIndex].str.empty()))){
						return it->second.y + it->second.h;
					}
				}
			}
			return 0;
		}
		// ルビを振る文字の位置マップの取得
		void getCPMapRuby(std::vector<TxtMiru::CharPoint> &ch_map,
						  TxtMiru::CharPointMap::const_iterator e,
						  TxtMiru::CharPointMap::const_iterator it,
						  TxtMiru::CharPointMap::const_iterator it_end_pos)
		{
			for(;it != e; ++it){
				if(it->second.ch_h > 0){ // 注記を除外するために 高さ 0 の文字は選択しないように
					ch_map.push_back(it->second);
				}
				if(it == it_end_pos){
					break;
				}
			}
		}
		void resetKeepPos()
		{
			if(bKeepPos){
				cp.x = old_cp.x;
				if(old_cp.y > cp.y){
					cp.y = old_cp.y;
				}
				if(old_sup_cp.y > cp.y){
					cp.y = old_sup_cp.y;
				}
				if(old_sub_cp.y > cp.y){
					cp.y = old_sub_cp.y;
				}
				bKeepPos = false;
			}
			old_cp = cp;
		}
		void setHalfChar(WORD chrType, bool bOffset)
		{
			if(!bKeepPos){
				old_cp = cp;
				old_sup_cp = cp;
				old_sub_cp = cp;
			} else {
				if(bOldOffset){
					old_sup_cp.y = cp.y; // 上付き
				} else {
					old_sub_cp.y = cp.y; // 下付き
				}
			}
			bKeepPos = true;
			if(HalfChar(chrType)){
				cp.ch_h = lsText.width / 4;
			} else {
				cp.ch_h = lsText.width / 2;
			}
			cp.w = cp.ch_w = lsText.width / 2;
			cp.h = cp.ch_h + izTextSpace;
			// 上付き、下付きそれぞれで文字位置を進める
			if(bOffset){
				cp.y = old_sup_cp.y;
				cp.x = old_cp.x + cp.w;
			} else {
				cp.y = old_sub_cp.y;
				cp.x = old_cp.x;
			}
			bOldOffset = bOffset;
		}
		// 行ごとの間隔などの計算
		void calcLine(bool bTrun = true)
		{
			iCharacters = 0;
			auto height = txtLayout.bottom - txtLayout.top - izTopIndent; // 画面のサイズ
			if(izBottomIndent > 0){
				height -= izBottomIndent;
			}
			auto need_height = (ttpCur.iHeight * lsText.width / text_l_2byte_size); /* 文字表示最低限必要な高さ */
			if(ttpCur.iHeight <= 0){
				need_height = (txtLayout.characters * lsText.width);
			}
			// 文字間隔の計算
			if(ttpCur.iNum > 1){
				izTextSpace = (height - need_height) / (ttpCur.iNum-1); // 均等割り
			} else if(txtLayout.characters > 1){
				auto iNum = -ttpCur.iNum;
				if(iNum > 0){
					if(bTrun && (need_height + (iNum-1) * izTextSpace) <= height){
						// 折り返していて、折り返しの前の文字間隔で範囲内に収まっていたら 文字間隔はそのまま保持
						// izTextSpace は、前回のものを継続して使用
					} else {
						int iz_layout_space/*レイアウト枠縦余白*/ = (txtLayout.bottom - txtLayout.top)/*レイアウト枠の高さ*/ - height/*表示領域の高さ*/;
						int max_line_characters/*カレント行最大文字数*/ = max(txtLayout.characters - (iz_layout_space / lsText.width), 1)/*余白を文字数に換算*/;
						izTextSpace = ((txtLayout.bottom - txtLayout.top) - txtLayout.characters * lsText.width) / (txtLayout.characters-1);
						if(need_height + izTextSpace * iNum <= height){
							// 標準の文字間隔
						} else if(iNum >= max_line_characters/*カレント行最大文字数*/){
							// 文字数が、指定数より多いとトータルで1行に収まらないので無理やり合わせる
							izTextSpace = (height - need_height) / (iNum - 1);
						}
						else {
							izTextSpace = 0;
						}
					}
				} else {
					izTextSpace = (height - (txtLayout.characters * lsText.width)) / (txtLayout.characters-1);
				}
			} else {
				izTextSpace = 0;
			}
			if(izTextSpace < 0){ izTextSpace = 0; }
			auto old_y = cp.y; /* 途中から、地+Nになる場合もあるので 現在の位置を保持 */
			old_cp.y = cp.y = txtLayout.top + izTopIndent;
			if(izBottomIndent >= 0){
				auto bottom_need_height = need_height; // 残り
				if(bTrun){
					/* 折り返した場合は、開始位置を(天+Nを足した)行頭に */
					old_y = cp.y;
					// need_height が 必要な長さ
				} else if(cp.y == old_y){
					/* 継続：途中から 地付きになるが それが 一文字目の場合は (折り返していない) need_height が 必要な長さ*/
				} else {
					/*
					  aaaa[折り返し]bb
					  need_height = a*4 + b*2 = 6文字分
					  old_y = 4文字分 + txtLayout.top + izTopIndent
					  残り = old_y - need_height - txtLayout.top - izTopIndent が b*2 に必要な長さ
					 */
					bottom_need_height = (ttpCur.iHeight * lsText.width / text_l_2byte_size);
				}
				// izBottomIndent = n字上 * lsText.width
				auto y = txtLayout.bottom - izBottomIndent - bottom_need_height;
				if(y > old_y){
					izTextSpace = 0;
					old_cp.y = cp.y = y;
				}
			}
			if(bTrun){
				cp.h = cp.ch_h + izTextSpace; // izTextSpaceの値を変更しているので 折り返したときは 高さを 再設定
			}
		}
	public:
		int m_iStartLayoutLine = 0;
		std::tstring noteFmt;
		// 上付き、下付きそれぞれ 最終文字位置を保持
		bool bOldOffset = false;
		TxtMiru::CharPoint   old_sup_cp;
		TxtMiru::CharPoint   old_sub_cp;
		//
		TxtMiru::TextType    ttCur = TxtMiru::TextType::MaxNum;
		TxtMiru::TextPoint   tpCur;
		TxtMiru::TextTurnPos ttpCur;
		TxtMiru::CharPoint   cp;
		TxtMiru::CharPoint   old_cp;
		TxtMiru::TxtLayout   txtLayout;
		TxtMiru::CharPoint   cpNote;
		bool bKeepPos  = false;
		int iNoteNum   =  0;
		int iTurn      = -1;
		const std::vector<TxtMiru::TextTurnPos> *m_pTTP = nullptr;

		TxtMiru::LineSize lsText; // 文字サイズ, 行間隔(設定ファイルより)
		TxtMiru::LineSize lsRuby; // ルビサイズ, ルビと文字の間隔((設定ファイルより)
		TxtMiru::LineSize lsNote; // 注記サイズ, 注記と文字の間隔((設定ファイルより)
		int iCharacters    =  0;
		int izLineWidth    =  0; // izTextWidth + izTextLineSpace + izRubyWidth + izRubyLineSpace
		int izTopIndent1st =  0; // 行頭からのインデント
		int izTopIndent2nd =  0; //           折り返し後のインデント
		int izTopIndent    =  0;
		int izBottomIndent = -1; // 行末からのインデント
		int izTextSpace    =  0; // 行ごとに計算された値
		int iCharSize      =  0;
		int iCurTextWidth  =  0;
		bool bRubyPosition = false;
	} m_cur;
public:
	CGrPointMapper(const CGrTxtDocument &doc, TxtMiru::CharPointMap &cp_map)
	: m_doc(doc), m_txtBuffer(doc.GetTxtBuffer()), m_textOffsetMap(doc.GetConstTextOffsetMap()),
	  m_textLayout(doc.GetConstLayout().GetConstLayoutList(CGrTxtLayout::LayoutListType::Text)),
	  m_noteLayout(doc.GetConstLayout().GetConstLayoutList(CGrTxtLayout::LayoutListType::Note)),
	  m_param(CGrTxtMiru::theApp().Param()), m_cpMap(cp_map),
	  m_maxLine(doc.GetConstLayout().GetMaxLine()),
	  m_cur(doc)
	{
		m_iLayoutMaxNum = m_textLayout.size();
	}
	/*
	  offset : TxtLayoutListに CGrTxtBufferのどの部分から流し込むかの指定
	 */
	void Create(int iStartLine)
	{
		m_cpMap.clear();
		m_iStartLine = iStartLine;
		const auto &lineList = m_doc.GetConstLineList();
		if(!lineList.empty()){
			for_eachLineList(&lineList[0], lineList.size());
		}
	}
private:
	//
	void setSkipLine(int line)
	{
		setCurrentTxtLayout((GetLayoutNo(line)-1) % m_iLayoutMaxNum, line);
		nextLine(line);// setcurrent
	}
	void setCenterPos(int start_index, int end_index)
	{
		auto &&ll = m_doc.GetConstLineList();
		auto &&li = ll[m_cur.tpCur.iLine-1];
		if(li.iEndCol >= m_iCurTextLayoutMaxLines){
			return;
		}
		auto diff = (m_cur.calcCurX(ll[end_index].iEndCol) - m_cur.txtLayout.left) / 2;
		for(auto &&it : m_cpMap){
			const auto &tp = it.first;
			auto &&cp = it.second;
			if(tp.iLine <= start_index || tp.iLine > end_index || cp.x < m_cur.txtLayout.left || cp.x > m_cur.txtLayout.right){
				continue;
			}
			cp.x -= diff;
			cp.zx -= diff;
		}
	}
	//
	void setCurrentTxtLayout(int iCurTxtLayout, int line)
	{
		if(m_iLayoutMaxNum <= iCurTxtLayout){
			m_iCurTxtLayout = 0;
		} else {
			m_iCurTxtLayout = iCurTxtLayout;
		}
		if(m_iCenterILine > 0 && m_cur.tpCur.iLine > 0){
			setCenterPos(m_iCenterILine, m_cur.tpCur.iLine-1);
		}
		m_iCenterILine = -1;
		//
		auto iLayout = GetLayoutNo(line);
		m_iCurTxtLayout = (iLayout-1) % m_iLayoutMaxNum;
		const auto &txtLayout = getCurrentTxtLayout();
		m_cur.m_iStartLayoutLine = (iLayout-1) * txtLayout.lines; // 全ページでのレイアウト開始行
		m_cur.SetTxtLayout(txtLayout);
		if(line < 0 || line >= m_iStartLine){
			if(m_iCurTxtLayout != 0){
				// 折り返し後の初期位置修正
				if(line >= m_iStartLine && line > m_iCurTextLayoutMaxLines + txtLayout.lines-1){
					m_iCurTextLayoutMaxLines += txtLayout.lines - 1;
					return setCurrentTxtLayout(iCurTxtLayout+1, line);
				}
				m_iStartLine = line;
			}
		} else {
			// 表示開始位置が、画面外から指定されている場合(折り返し)
		}
		m_iCurTextLayoutMaxLines = m_iStartLine + txtLayout.lines - 1;
		//
		if(line > 0){
			auto page = (iLayout-1) / m_iLayoutMaxNum;
			m_iCurTextLayoutMaxLines = page * m_maxLine;
			auto iTmpLayout = page * m_iLayoutMaxNum + 1;
			int iIndex=0;
			while(iLayout >= iTmpLayout){
				m_iCurTextLayoutMaxLines += m_textLayout[iIndex].lines;
				++iTmpLayout;
				++iIndex;
				if(iIndex >= m_iLayoutMaxNum){
					iIndex = 0;
				}
			}
			--m_iCurTextLayoutMaxLines;
			bool bNextLayout = false;
			for(const auto &item : m_doc.GetConstPictPosMap()){
				const auto &pp = item.second;
				if(pp.iStartLayout <= iLayout && iLayout <= pp.iEndLayout){
					bNextLayout = true;
					break;
				}
			}
			if(bNextLayout){
				if(nextLayout()){ // 行を進めたときのみ、再帰呼び出し(無限ループするので)
					if(iCurTxtLayout == m_iCurTxtLayout && line == m_cur.GetLine()){
						// 無限ループしないように
					} else {
						setCurrentTxtLayout(m_iCurTxtLayout, m_cur.GetLine());
					}
				}
			}
		}
	}
	bool nextLayout()
	{
		auto iTurn = m_cur.iTurn;
		auto len = m_iCurTextLayoutMaxLines-m_cur.GetLine();
		if(len <= 0){ return false; }
		for(;len>0; --len){
			m_cur.Turnup();
			m_cur.iTurn = iTurn;
		}
		return true;
	}
	void nextLine(){ nextLine(m_cur.GetLine()+1); }
	void nextLine(int line)
	{
		if(line > m_iCurTextLayoutMaxLines){
			setCurrentTxtLayout(m_iCurTxtLayout+1, line);
		}
	}
	const TxtMiru::TxtLayout &getCurrentTxtLayout()
	{
		return m_textLayout[m_iCurTxtLayout];
	}
	struct LineBoxPoint {
		TxtMiru::TextPoint tp;
		int iTxtLayout = 0;
		int top = 0;
		int bottom = 0;
	};
	std::map<int,LineBoxPoint> m_lineBoxTextPointMap;
	// 罫囲み 上、高さ設定
	void setLineBoxCpMapY(const TxtMiru::TextPoint &tp, const TxtMiru::TxtLayout &layout, LineBoxPoint &lbp)
	{
		auto num = max(layout.characters, 1);
		auto height = layout.bottom - layout.top;
		auto izTopIndent    = lbp.top * height / num;
		auto izBottomIndent = lbp.bottom * m_cur.lsText.width;
		m_cpMap[tp].zy = layout.top + izTopIndent - (m_cur.lsText.width / 2);
		m_cpMap[tp].h  = layout.bottom - layout.top - izTopIndent - izBottomIndent + m_cur.lsText.width;
	}
	void setLineBoxContinueCPLeft(const TxtMiru::TextPoint &tp, int left)
	{
		auto &&cp = m_cpMap[tp];
		auto right = cp.zx + cp.w;
		cp.zx = left;
		cp.w = right - left;
	}
	// 罫囲み 継続で終了
	void setLineBoxContinue(LineBoxPoint &lbp, int left)
	{
		auto &&tp = lbp.tp;
		if(lbp.iTxtLayout == m_iCurTxtLayout){
			setLineBoxContinueCPLeft(tp, left);
		} else if(m_iCurTxtLayout < lbp.iTxtLayout){
			// lbp.iTxtLayoutの左端までを範囲に変更
			if (lbp.iTxtLayout < m_iLayoutMaxNum) {
				const auto& layout = m_textLayout[lbp.iTxtLayout];
				setLineBoxContinueCPLeft(tp, layout.left);
			}
		} else {
			for(; lbp.iTxtLayout<m_iCurTxtLayout; ++lbp.iTxtLayout){
				{ // Pre
					const auto &layout = m_textLayout[lbp.iTxtLayout];
					setLineBoxContinueCPLeft(tp, layout.left);
				}
				++tp.iPos;
				if ((lbp.iTxtLayout + 1) < m_iLayoutMaxNum) {
					const auto& layout = m_textLayout[lbp.iTxtLayout + 1];
					setLineBoxCpMapY(tp, layout, lbp);
					TxtMiru::CharPoint& cp = m_cpMap[tp];
					cp.zx = layout.right;
					cp.w = 0;
					cp.ch_w = TxtMiru::CPBorderNoSide;
				}
			}
		}
	}
	///
	void for_eachLineList(const TxtMiru::LineInfo *pli, int li_len)
	{
		int iStart = m_iStartLine;
		int iEnd   = m_maxLine + m_iStartLine;
		int iLine  = 0;
		for(;li_len>0; --li_len, ++pli, ++iLine){
			if((iStart <= pli->iStartCol || iStart < pli->iEndCol) && pli->iStartCol < pli->iEndCol){
				break;
			}
		}
		if(li_len <= 0){ return; }
		if(pli->iStartCol > iEnd){ return; }
		{ // 初期設定( + 1行目の処理)
			const auto &li = (*pli);
			setCurrentTxtLayout((GetLayoutNo(li.iStartCol)-1) % m_iLayoutMaxNum, li.iStartCol);
			m_cur.SetLine(li, iLine/*LineInfoのインデックス*/, m_iStartLine);
			if(li.iStartCol > iEnd){
				return;
			}
			const auto &text_list = li.text_list;
			int len = text_list.size();
			if(len > 0){
				// ルビが折り返して改ページすると表示されないのでひとつ前の行からは処理する
				const auto* pti = m_cur.skipFirst(&text_list[0], len, m_iStartLine - 1, this);
				for_eachTextInfoList(pti, len, text_list);
			}
			++pli;
			++iLine;
			--li_len;
		}
		for(; li_len>0; --li_len, ++pli, --iLine){
			const auto &li = (*pli);
			nextLine(li.iStartCol);
			m_cur.NextLine(li, m_iStartLine);

			if(li.iStartCol > iEnd){ break; } // 範囲外の為、終了
			const auto &text_list = li.text_list;
			int len = text_list.size();
			if(len > 0){
				for_eachTextInfoList(&text_list[0], len, text_list);
			}
		}
		// 罫囲み 表示領域を超えても閉じていないものを対象に 罫囲みを設定
		const auto &layout = m_textLayout[m_textLayout.size()-1];
		int left = layout.left;
		for(auto &&item : m_lineBoxTextPointMap){
			setLineBoxContinue(item.second, left);
		}
		m_lineBoxTextPointMap.clear();
	}

	void for_eachTextInfoList(const TxtMiru::TextInfo *it, int len, const TxtMiru::TextInfoList &text_list)
	{
		// フォントサイズの指定
		const auto &textStyleMap = m_txtBuffer.GetConstTextStyleMap();
		auto tsm_it = textStyleMap.find(m_cur.tpCur);
		if(tsm_it != textStyleMap.end()){
			m_cur.SetCharSize(tsm_it->second.fontSize);
		}
		//
		for(;len>0; ++it, --len){
			doLine(*it, text_list);
		}
	}
	void doLine(const TxtMiru::TextInfo &ti, const TxtMiru::TextInfoList &text_list)
	{
		const auto &text = ti.str;
		auto lpNextSrc = m_cur.Get1stChar(text.c_str());
		m_cur.SetTextType(ti);
		switch(ti.textType){
		case TxtMiru::TextType::CENTER    :
			m_iCenterILine = m_cur.tpCur.iLine;
			break;
		case TxtMiru::TextType::TEXT_SIZE :
			m_cur.SetCharSize(static_cast<signed short>(ti.chrType));
			break;
		case TxtMiru::TextType::LINE_BOX_START  :
			{
				// 罫囲み 番号(n番目)
				int line_no = ti.chrType;
				auto it = m_lineBoxTextPointMap.find(line_no);
				if(it == m_lineBoxTextPointMap.end()){
					// 罫囲み 開始
					LineBoxPoint lbp;
					lbp.tp = m_cur.tpCur;
					lbp.iTxtLayout = m_iCurTxtLayout;
					lbp.top    = ti.tpBegin.iIndex;
					lbp.bottom = ti.tpBegin.iPos  ;
					//
					m_lineBoxTextPointMap[line_no] = lbp;
					m_cur.SetCharPointMap(m_cpMap);
					auto &&tp = lbp.tp;
					setLineBoxCpMapY(tp, m_cur.txtLayout, lbp);
					if(ti.tpEnd.iPos == 0){ /* TYPE:開始=0、途中=1, 終了=2 */
						m_cpMap[tp].ch_w = TxtMiru::CPBorderRight;
					} else {
						m_cpMap[tp].ch_w = TxtMiru::CPBorderNoSide;
					}
					{
						// 折り返していると 開始位置がずれるので カレント TextList の最初の位置で設定しなおす
						// 例)
						// aaaaaaaaaaaaa[折り返し]bbbbbb[TT_LINE_BOX_START] となっている場合
						// m_cur は TT_LINE_BOX_START の位置 になるので aaaaaaaaaaaaa に罫線がかかれない
						//    aaaaaaaaaaaaa
						//    bbbbbb[TT_LINE_BOX_START]
						// ので 最初の a の位置を取得しなおす
						int iLine = m_cur.tpCur.iLine;
						TxtMiru::TextPoint ref_tp{iLine, 0, 0};
						TxtMiru::TextPoint end_tp{m_cur.tpCur};
						int size = text_list.size();
						while(ref_tp <= end_tp){
							if(m_cpMap.find(ref_tp) == m_cpMap.end()){
								ref_tp = CGrTxtMapper::NextTextPoint(ref_tp, text_list);
								continue;
							} else if(ref_tp.iIndex < size && ref_tp.iIndex >= 0 && text_list[ref_tp.iIndex].textType == TxtMiru::TextType::OTHER){ // 注記は対象外
								++ref_tp.iIndex;
								ref_tp.iPos = 0;
								continue;
							}
							const TxtMiru::CharPoint &ref_cp = m_cpMap[ref_tp];
							m_cpMap[tp].zx = ref_cp.zx;
							m_cpMap[tp].w  = ref_cp.w ;
							if (ref_cp.lcol >= m_iStartLine) {
								break;
							}
							++ref_tp.iIndex;
						}
					}
				} else {
					// 罫囲み カレント行まで継続
					setLineBoxContinue(it->second, m_cur.cp.x);
				}
			}
			break;
		case TxtMiru::TextType::LINE_BOX_END    :
			{
				// 罫囲み 番号(n番目)
				int line_no = ti.chrType;
				auto it = m_lineBoxTextPointMap.find(line_no);
				if(it == m_lineBoxTextPointMap.end()){
					LineBoxPoint lbp;
					lbp.tp = m_cur.tpCur;
					lbp.iTxtLayout = m_iCurTxtLayout;
					lbp.top    = ti.tpBegin.iIndex;
					lbp.bottom = ti.tpBegin.iPos  ;
					m_cur.SetCharPointMap(m_cpMap);
					const auto &tp = lbp.tp;
					setLineBoxCpMapY(tp, m_cur.txtLayout, lbp);
					m_cpMap[tp].ch_w = TxtMiru::CPBorderLeft;
				} else {
					auto &&lbp = (it->second);
					const auto &tp = lbp.tp;
					setLineBoxContinue(lbp, m_cur.cp.x);
					m_cpMap[tp].ch_w |= TxtMiru::CPBorderLeft;
					m_lineBoxTextPointMap.erase(it);
				}
			}
			break;
		case TxtMiru::TextType::RINDENT         : // through
		case TxtMiru::TextType::RINDENT_START   : // through
		case TxtMiru::TextType::RINDENT_END     : m_cur.SetBottomIndent(ti.tpBegin.iIndex, ti.tpEnd.iPos); break; // 行末からのインデント開始
		case TxtMiru::TextType::INDENT          : // through
		case TxtMiru::TextType::INDENT_START    : // through
		case TxtMiru::TextType::INDENT_END      : m_cur.SetTopIndent(ti.tpBegin.iIndex, ti.tpEnd.iIndex, true); break;  // 行頭からのインデント開始
		case TxtMiru::TextType::ROTATE_NUM      : // through
		case TxtMiru::TextType::ROTATE_NUM_AUTO:
			if(m_cur.IsTurnup()){
				nextLine();
				m_cur.Turnup();
			}
			m_cur.BeginHorz(CGrText::CharLen(lpNextSrc));
			{
				int offset = 0;
				while(*lpNextSrc){
					auto lpNextNextSrc=CGrText::CharNext(lpNextSrc);
					m_cur.SetCharPointMap(m_cpMap);
					if(!HalfChar(CGrText::GetStringTypeEx(lpNextSrc, lpNextNextSrc-lpNextSrc))){
						offset += m_cur.cp.w;
						m_cur.cp.x += m_cur.cp.w;
						auto &&cpItem = m_cpMap[m_cur.tpCur];
						cpItem.w    = cpItem.h   ;
						cpItem.ch_w = cpItem.ch_h;
					}
					m_cur.NextHorzPos();
					lpNextSrc = lpNextNextSrc;
				}
				if(offset != 0){
					int offset_half = offset / 2;
					TxtMiru::TextPoint tpCur(m_cur.tpCur);
					for(tpCur.iPos=0; tpCur.iPos<m_cur.tpCur.iPos; ++tpCur.iPos){
						auto &&cpItem = m_cpMap[tpCur];
						cpItem.zx -= offset_half;
						cpItem.x -= offset_half;
					}
				}
			}
			m_cur.EndHorz();
			break;
		case TxtMiru::TextType::SMALL_NOTE      : // through
		case TxtMiru::TextType::SMALL_NOTE_R    : // through
		case TxtMiru::TextType::SMALL_NOTE_L    : // through
		case TxtMiru::TextType::SUP_NOTE        : // through
		case TxtMiru::TextType::SUB_NOTE        : // through
		case TxtMiru::TextType::GUID_MARK       : // through
		case TxtMiru::TextType::SKIP_CHAR       : // through
		case TxtMiru::TextType::SKIP_CHAR_AUTO  : // through
		case TxtMiru::TextType::TEXT            : // through
		case TxtMiru::TextType::LINE_CHAR       :
			for(;*lpNextSrc; lpNextSrc=CGrText::CharNext(lpNextSrc)){
				if(m_cur.IsTurnup()){
					nextLine();
					m_cur.Turnup();
				}
				auto it_tom = m_textOffsetMap.find(m_cur.tpCur);
				if(it_tom != m_textOffsetMap.end()){
					const auto &of = it_tom->second;
					int h = m_cur.cp.h;
					m_cur.cp.h = l_TextOffsetTable.byteArrayH[of.offsetType] * m_cur.cp.h / l_max_offsetnum;
					m_cur.SetCharPointMap(m_cpMap);
					m_cpMap[m_cur.tpCur].zy -= l_TextOffsetTable.byteArrayY[of.offsetType] * h / l_max_offsetnum;
					m_cur.NextPos();
					m_cur.cp.h = h;
				} else {
					m_cur.SetCharPointMap(m_cpMap);
					m_cur.NextPos();
				}
			}
			break;
		case TxtMiru::TextType::KU1_CHAR       : // through
		case TxtMiru::TextType::KU2_CHAR       :
			for(;*lpNextSrc; lpNextSrc=CGrText::CharNext(lpNextSrc)){
				if(m_cur.IsTurnup()){
					nextLine();
					m_cur.Turnup();
				}
				m_cur.SetCharPointMap(m_cpMap);
			}
			// TT_KU1_CHAR, TT_KU2_CHAR は、4バイトとして処理
			m_cur.NextPosNoSpace();
			break;
		case TxtMiru::TextType::OTHER          :
			m_cur.SetCharPointMap(m_cpMap);
			m_cur.NextPos();
			m_cur.Note(m_noteLayout[0], text_list, m_cpMap, ti);
			break;
		case TxtMiru::TextType::OVERLAP_CHAR   :
			m_cur.BeginOverlap(m_cpMap, ti.tpBegin);
			for(;*lpNextSrc; lpNextSrc=CGrText::CharNext(lpNextSrc)){
				if(m_cur.IsTurnup()){
					nextLine();
					m_cur.Turnup();
				}
				m_cur.SetCharPointMap(m_cpMap);
				m_cur.NextPos();
			}
			m_cur.EndOverlap();
			break;
		case TxtMiru::TextType::LINK              :
			if(ti.tpEnd.iIndex == -1 && ti.tpEnd.iPos == -1){
				;
			} else {
				m_cur.DrawLine(text_list, m_cpMap, ti, 1/*-1*/);
			}
			break;
		case TxtMiru::TextType::NO_READ             : // through
		case TxtMiru::TextType::OTHER_NOTE          : // through
		case TxtMiru::TextType::ERROR_STR           : // through
		case TxtMiru::TextType::NOTE                : m_cur.Note(m_noteLayout[0], text_list, m_cpMap, ti); break;
		case TxtMiru::TextType::MOVING_BORDER       : // through
		case TxtMiru::TextType::DOT                 : // through
		case TxtMiru::TextType::SALTIRE_DOT         : // through
		case TxtMiru::TextType::WHITE_DOT           : // through
		case TxtMiru::TextType::ROUND_DOT           : // through
		case TxtMiru::TextType::WHITE_ROUND_DOT     : // through
		case TxtMiru::TextType::BLACK_TRIANGLE_DOT  : // through
		case TxtMiru::TextType::WHITE_TRIANGLE_DOT  : // through
		case TxtMiru::TextType::DOUBLE_ROUND_DOT    : // through
		case TxtMiru::TextType::BULLS_EYE_DOT       : // through
		case TxtMiru::TextType::LINE                : // through
		case TxtMiru::TextType::WAVE_LINE           : // through
		case TxtMiru::TextType::SHORT_DASHED_LINE   : // through
		case TxtMiru::TextType::DOT_LINE            : // through
		case TxtMiru::TextType::DOUBLE_LINE         : m_cur.DrawLine(text_list, m_cpMap, ti, 1 ); break;
		case TxtMiru::TextType::DEL_LINE            : m_cur.DrawLine(text_list, m_cpMap, ti, 0 ); break;
		case TxtMiru::TextType::WHITE_DOT_L         : // through
		case TxtMiru::TextType::ROUND_DOT_L         : // through
		case TxtMiru::TextType::WHITE_ROUND_DOT_L   : // through
		case TxtMiru::TextType::BLACK_TRIANGLE_DOT_L: // through
		case TxtMiru::TextType::WHITE_TRIANGLE_DOT_L: // through
		case TxtMiru::TextType::DOUBLE_ROUND_DOT_L  : // through
		case TxtMiru::TextType::BULLS_EYE_DOT_L     : // through
		case TxtMiru::TextType::SALTIRE_DOT_L       : // through
		case TxtMiru::TextType::DOT_L               : // through
		case TxtMiru::TextType::LINE_L              : // through
		case TxtMiru::TextType::WAVE_LINE_L         : // through
		case TxtMiru::TextType::SHORT_DASHED_LINE_L : // through
		case TxtMiru::TextType::DOT_LINE_L          : // through
		case TxtMiru::TextType::DOUBLE_LINE_L       : // through
		case TxtMiru::TextType::UNDER_LINE          : m_cur.DrawLine(text_list, m_cpMap, ti, -1); break;
		case TxtMiru::TextType::RUBY                : // through
		case TxtMiru::TextType::RUBY_L              : // through
		case TxtMiru::TextType::NOTE_L              : // through
		case TxtMiru::TextType::UNKOWN_ERROR        : m_cur.DrawRuby(text_list, m_cpMap, ti, m_textOffsetMap); break;
		case TxtMiru::TextType::PICTURE_LAYOUT      : // through
		case TxtMiru::TextType::PICTURE_HALF_PAGE   : // through
		case TxtMiru::TextType::PICTURE_FULL_PAGE   :
			setCaption(ti);
			break;
		}
		m_cur.NextIndex();
		if(m_cur.IsTurnup()){
			nextLine();
			m_cur.Turnup();
		}
	}
	void setCaption(const TxtMiru::TextInfo &ti)
	{
		std::vector<std::tstring> string_list;
		CGrTxtParser::GetPictInfo(ti.str, string_list);
		const auto &caption = string_list[4];
		if(caption.empty()){
			return;
		}
		int line = 1;
		const auto &layout = m_doc.GetConstLayout();
		const auto &lsRunningHeads = layout.GetLineSize(CGrTxtLayout::LineSizeType::RunningHeads);
		TxtMiru::TextPoint tp;
		tp.iLine  = m_cur.tpCur.iLine;
		tp.iIndex = ti.tpBegin.iIndex;
		//
		const auto &ppm = m_doc.GetConstPictPosMap();
		const auto &pp = ppm.find(tp);
		if(pp == ppm.end()){
			return;
		}
		//
		int left = 0;
		int right = 0;
		for(int iLayout=pp->second.iStartLayout; iLayout<=pp->second.iEndLayout; ++iLayout){
			const auto &layout = m_textLayout[(iLayout-1) % m_iLayoutMaxNum];
			if(left == 0 || left > layout.left){
				left = layout.left;
			}
			if(right == 0 || right < layout.right){
				right = layout.right;
			}
		}
		TxtMiru::CharPoint cp;
		cp.x = left;
		cp.y = 0;
		cp.h    = cp.w    = lsRunningHeads.width + lsRunningHeads.space;
		cp.ch_h = cp.ch_w = lsRunningHeads.width;
		auto lpNextSrc = caption.c_str();
		for(;*lpNextSrc; ++tp.iPos){
			auto lpSrc = lpNextSrc;
			lpNextSrc = CGrText::CharNext(lpSrc);
			if(HalfChar(CGrText::GetStringTypeEx(lpSrc, lpNextSrc-lpSrc))){
				cp.ch_w = cp.w = lsRunningHeads.width / 2;
			} else {
				cp.ch_w = cp.w = lsRunningHeads.width;
			}
			if(*lpSrc == _T('\n') || cp.x > right){
				cp.x = left;
				cp.y += lsRunningHeads.width;
				++line;
			}
			copyCharPointMap(m_cpMap[tp], cp);
			cp.x += cp.w + lsRunningHeads.space;
		}
	}

	// 全頁を通してのレイアウト番号
	int GetLayoutNo(int lcol)
	{
		int page = lcol / m_maxLine;
		int iLayout = page * m_iLayoutMaxNum;
		int line = lcol % m_maxLine;
		int iIndex=m_iCurTxtLayout;
		while(line >= m_textLayout[iIndex].lines){
			line -= m_textLayout[iIndex].lines;
			++iIndex;
			if (iIndex >= m_iLayoutMaxNum) {
				iIndex = 0;
			}
			++iLayout;
		}
		return iLayout+1;
	}
};

static int calcPos(int pos, double ws, double ps)
{
	return static_cast<int>((static_cast<double>(pos) * ws + ps - 1.0) / ps + 0.5);
}
static int calcPosUp(int pos, double ws, double ps)
{
	return static_cast<int>((static_cast<double>(pos) * ws + ps - 1.0) / ps + 0.9);
}

void CGrTxtMapper::setNombre(const CGrTxtDocument &doc, const TxtMiru::TextInfo &ti, int iIndex)
{
	auto lpNextSrc = ti.str.c_str();
	int nombre = 0;
	const auto &layout = doc.GetConstLayout();
	enum class PositionFormat { LEFT, CENTER, RIGHT } pf = PositionFormat::LEFT;
	switch(ti.textType){
	case TxtMiru::TextType::NOMBRE1:
		// デフォルト値を使用 nombre = 0;
		switch(layout.GetNombreFormatType()){
		case CGrTxtLayout::NombreFormatType::center : pf = PositionFormat::CENTER; break;
		case CGrTxtLayout::NombreFormatType::inside : pf = PositionFormat::RIGHT ; break;
			// デフォルト値を使用
		default: break;
		}
		break;
	case TxtMiru::TextType::NOMBRE2:
		nombre = 1;
		switch(layout.GetNombreFormatType()){
		case CGrTxtLayout::NombreFormatType::center : pf = PositionFormat::CENTER; break;
			// デフォルト値を使用
		case CGrTxtLayout::NombreFormatType::outside: pf = PositionFormat::RIGHT ; break;
		default: break;
		}
		break;
	default: return;
	}
	const auto &layoutNombre=layout.GetConstLayoutList(CGrTxtLayout::LayoutListType::Nombre);
	if(nombre >= static_cast<signed int>(layoutNombre.size())){
		return;
	}
	if(layoutNombre[nombre].lines <= 0){ // 非表示の場合は、高さ0に
		return;
	}
	const auto &lsNombre = layout.GetLineSize(CGrTxtLayout::LineSizeType::Nombre);
	TxtMiru::TextPoint tp;
	tp.iIndex = iIndex;
	TxtMiru::CharPoint cp;
	cp.x    = layoutNombre[nombre].left;
	cp.y    = layoutNombre[nombre].top ;
	cp.h    = cp.w    = lsNombre.width + lsNombre.space;
	cp.ch_h = cp.ch_w = lsNombre.width;

	for(;*lpNextSrc; ++tp.iPos){
		auto lpSrc = lpNextSrc;
		lpNextSrc = CGrText::CharNext(lpSrc);
		if(HalfChar(CGrText::GetStringTypeEx(lpSrc, lpNextSrc-lpSrc))){
			cp.ch_w = cp.w = lsNombre.width / 2;
		} else {
			cp.ch_w = cp.w = lsNombre.width;
		}
		copyCharPointMap(m_cpLayoutMap[tp], cp);
		cp.x += cp.w + lsNombre.space;
	}
	int x = 0;
	switch(pf){
	case PositionFormat::CENTER:
		x = (layoutNombre[nombre].right - cp.x - cp.w) / 2;
		break;
	case PositionFormat::RIGHT :
		x = layoutNombre[nombre].right - cp.x - cp.w;
		break;
	}
	if(x != 0){
		while(tp.iPos > 0){
			--tp.iPos;
			auto &&it_cp = m_cpLayoutMap[tp];
			it_cp.x += x;
			it_cp.zx = it_cp.x;
		}
	}
}
void CGrTxtMapper::setRunningHeads(const CGrTxtDocument &doc, const TxtMiru::TextInfo &ti, int iIndex)
{
	if(ti.textType != TxtMiru::TextType::RUNNINGHEADS){
		return;
	}
	auto lpNextSrc = ti.str.c_str();
	const auto &layout = doc.GetConstLayout();
	const auto &layoutRunningHeads=layout.GetConstLayoutList(CGrTxtLayout::LayoutListType::RunningHeads);
	if(0 >= static_cast<signed int>(layoutRunningHeads.size())){
		return;
	}
	const auto &lsRunningHeads = layout.GetLineSize(CGrTxtLayout::LineSizeType::RunningHeads);
	int half_rh_width = lsRunningHeads.width / 2 + lsRunningHeads.space;
	int rh_width      = lsRunningHeads.width     + lsRunningHeads.space;
	TxtMiru::TextPoint tp;
	tp.iIndex = iIndex;
	TxtMiru::CharPoint cp;
	cp.x    = layoutRunningHeads[0].left;
	cp.y    = layoutRunningHeads[0].top ;
	cp.h    = cp.w    = rh_width            ;
	cp.ch_h = cp.ch_w = lsRunningHeads.width;
	for(; *lpNextSrc; ++tp.iPos){
		auto lpSrc = lpNextSrc;
		lpNextSrc = CGrText::CharNext(lpSrc);
		copyCharPointMap(m_cpLayoutMap[tp], cp);
		if(HalfChar(CGrText::GetStringTypeEx(lpSrc, lpNextSrc-lpSrc))){
			cp.x += half_rh_width;
		} else {
			cp.x += rh_width     ;
		}
	}
}
static void calcCharPointMap(TxtMiru::CharPointMap &cpm, int iWindowSize, int iPaperSize, const RECT &rc_canvas)
{
	double dWindowSize = static_cast<double>(iWindowSize);
	double dPaperSize  = static_cast<double>(iPaperSize );
	for(auto &&it : cpm){
		auto &&cp = (it.second);
		cp.y  = calcPos(cp.y , dWindowSize, dPaperSize) + rc_canvas.top ;
		cp.x  = calcPos(cp.x , dWindowSize, dPaperSize) + rc_canvas.left;
		cp.h  = calcPos(cp.h , dWindowSize, dPaperSize);
		cp.w  = calcPos(cp.w , dWindowSize, dPaperSize);
		if(cp.ch_h > cp.ch_w){
			cp.zy = calcPosUp(cp.zy, dWindowSize, dPaperSize) + rc_canvas.top ;
			cp.zx = calcPosUp(cp.zx, dWindowSize, dPaperSize) + rc_canvas.left;
		} else {
			cp.zy = calcPos(cp.zy, dWindowSize, dPaperSize) + rc_canvas.top ;
			cp.zx = calcPos(cp.zx, dWindowSize, dPaperSize) + rc_canvas.left;
		}
	}
}
void CGrTxtMapper::CreateCharPointMap(const CGrTxtDocument &doc, int offset, const RECT &rc_canvas, const TxtMiru::TextInfoList &til_layout)
{
	m_cpMap.clear();
	m_cpLayoutMap.clear();
	const auto &layout = doc.GetConstLayout();
	int iPaperSize = layout.GetPaperSize().cx;
	if(iPaperSize <= 0){ return; }
	CGrPointMapper pm(doc, m_cpMap);
	pm.Create(offset);

	int iWindowSize = rc_canvas.right - rc_canvas.left;
	calcCharPointMap(m_cpMap, iWindowSize, iPaperSize, rc_canvas);
	//
	int iIndex = 0;
	for(const auto &ti : til_layout){
		setNombre(doc, ti, iIndex);
		setRunningHeads(doc, ti, iIndex);
		++iIndex;
	}
	calcCharPointMap(m_cpLayoutMap, iWindowSize, iPaperSize, rc_canvas);
}

// 漢数字の位取りを示す読点の処理
// 「一・二」 とかの場合、「・」の前後の空白を詰める
static LPCTSTR l_numPaddingStr[] = { _T("九八七六五四三二一〇"), _T("９８７６５４３２１０"), };

#include <set>
class CGrIndexCreator
{
private:
	std::set<int/*iLayout*/> m_imageList;

	CGrTxtDocument &m_doc;
	TxtMiru::TextOffsetMap &m_textOffsetMap;
	const TxtMiru::TxtLayoutList &m_textLayout;
	const CGrTxtParam            &m_param     ;

	int m_iLayout                = 0    ; // カレントのレイアウト番号
	int m_iLayoutMaxNum          = 0    ; // 2(表示)ページあたりのレイアウト設定数
	int m_iPageLayoutNum         = 1    ; // 1ページあたりのレイアウト設定数
	int m_iTopIndent1st          = 0    ; // 行頭からのインデント 1行目
	int m_iTopIndent2nd          = 0    ; //                      2行目以降
	int m_iBottomIndent          = 0    ; // 行末からのインデント
	int m_iLimitChar             = 0    ; // 字詰
	int m_iTateChuNum            = 2    ; // 縦中横にする文字数
	bool m_bBottomIndent         = false; // 行末からのインデント有無
	bool m_bInComment            = false; // コメント
	bool m_bForceNewLine         = false; // 改行
	bool m_bNextLayout           = false; // 次のレイアウトへ移動したどうか

	int m_iLLIndex               = -1   ; // iLine // TxtMiru::LineListのインデックス
	bool m_bNextLine             = false;
	bool m_bSkipNextLine         = false; // 字下げとかのブロック指定時、改行を抑制
	int m_iCurTxtLayout          = 0    ;
	int m_iCurTextLayoutLines    = -1   ;
	int m_iCurTextLayoutMaxLines = -1   ;
	//
	int m_iCurChar               = 0    ; // row
	int m_iCurSubChar            = 0    ; // row_sub
	int m_iCurLine               = 0    ; // col
	int m_iLineTop               = 0    ; // top
	int m_iLineBottom            = 0    ; // bottom
	int m_iLineCharNum           = 0    ; // line_ch_num
	//
	int m_iTILIndex              = 0    ; // iIndex // TxtMiru::TextInfoList
	int m_iNewTopIndent1st       = 0    ;
	int m_iNewTopIndent2nd       = 0    ;
	int m_iNewBottomIndent       = 0    ;
	bool m_bNewBottomIndent      = false;
	bool m_bNoPrevLine           = false; // 追い込み禁止
	bool m_bImageNextLayout      = false; // 挿絵が現れた時点で次のレイアウトに

	bool m_bHorz                 = false; // 横組み
	int m_iCharSize              = 0    ;
	int m_iCurTextL1byteSize     = 0    ;
	int m_iCurTextL2byteSize     = 0    ;
	inline int HalfCharLHalf(WORD charType){ return HalfChar(charType) ? text_l_half_1byte_size : text_l_half_2byte_size; }
	inline int HalfCharCur(WORD charType){ return HalfChar(charType) ? m_iCurTextL1byteSize : m_iCurTextL2byteSize; }

	TxtMiru::TextTurnPos m_ttpNewLine;
	TxtMiru::LineInfo    *m_pCurLi = nullptr;
public:
	CGrIndexCreator(CGrTxtDocument &doc, const TxtMiru::TxtLayoutList &txtLayout)
	: m_doc(doc), m_textOffsetMap(doc.GetTextOffsetMap()), m_textLayout(txtLayout), m_param(CGrTxtMiru::theApp().Param())
	{
		m_textOffsetMap.clear();
		m_param.GetPoints(CGrTxtParam::PointsType::TateChuNum, (int*)&m_iTateChuNum, 1);
		m_iLayoutMaxNum = m_textLayout.size();
		if(m_iPageLayoutNum < m_iLayoutMaxNum / 2){
			m_iPageLayoutNum = m_iLayoutMaxNum / 2;
		}
		// 追い込みができるだけのスペースがあるかどうかをチェック
		const auto &layout = doc.GetConstLayout();
		auto lsText = layout.GetLineSize(CGrTxtLayout::LineSizeType::Text);
		const auto &tl = getCurrentTxtLayout();
		int i_line_width = (tl.bottom - tl.top); /* 行幅 */
		int i_need_line_width = lsText.width * (tl.characters + 1/*追い込み用*/); /* 必要な行幅 */
		if(i_line_width < i_need_line_width){
			m_bNoPrevLine = true;
		}
		m_iCurTextL1byteSize = text_l_1byte_size;
		m_iCurTextL2byteSize = text_l_2byte_size;
		m_bImageNextLayout   = m_param.GetBoolean(CGrTxtParam::PointsType::ImageNextLayout);
	}
	void Create()
	{
		for(auto &&li : m_doc.GetLineList()){
			doLineList(li);
		}
		m_doc.CalcTotalPage();
	}
private:
	void setCharSize(int size)
	{
		m_iCharSize = size;
		m_iCurTextL1byteSize = text_l_1byte_size + text_l_1byte_size*m_iCharSize / 8;
		m_iCurTextL2byteSize = text_l_2byte_size + text_l_2byte_size*m_iCharSize / 8;
	}
	// 始め括弧類(cl-01)、終わり括弧類(cl-02)、読点類(cl- 07)、句点類(cl-06)が連続する場合の配置
	int setArrangeSize(LPCTSTR lpCurSStr, LPCTSTR lpCurEStr, LPCTSTR lpPreSStr, LPCTSTR lpPreEStr, int iPos, int iPrevIndex, int iPrevPos)
	{
		auto pre_type = m_param.GetCharOffsetType(std::tstring(lpPreSStr, lpPreEStr));
		auto cur_type = m_param.GetCharOffsetType(std::tstring(lpCurSStr, lpCurEStr));
		if(pre_type && cur_type){
			TxtMiru::TextPoint curtp{m_iLLIndex, m_iTILIndex, iPos    };
			TxtMiru::TextPoint pretp{m_iLLIndex, iPrevIndex , iPrevPos};
			TxtMiru::TextOffset of = {cur_type};
			m_textOffsetMap[curtp] = of;
			if(m_iLineCharNum > 1){
				if(m_textOffsetMap.find(pretp) == m_textOffsetMap.end()){
					of.offsetType = pre_type;
					m_textOffsetMap[pretp] = of;
					m_iCurChar -= l_max_offsetnum-l_TextOffsetTable.byteArrayH[pre_type];
				}
			}
			return l_TextOffsetTable.byteArrayH[cur_type];
		} else {
			return -1;
		}
	}
	// 句読点や括弧類などの基本的な配置
	int arrangePos(int nextHeight, LPCTSTR lpSrc, LPCTSTR lpNextSrc, int iPos, const TxtMiru::TextInfoList &text_list)
	{
		LPCTSTR lpPrevSrc = nullptr;
		if(iPos > 0 && m_iCurChar > 0){
			auto lpBeginSrc = text_list[m_iTILIndex].str.c_str();
			lpPrevSrc = CGrText::CharPrev(lpBeginSrc, lpSrc);
			// 始め括弧類(cl-01)、終わり括弧類(cl-02)、読点類(cl- 07)、句点類(cl-06)が連続する場合の配置
			int pos = setArrangeSize(lpSrc, lpNextSrc, lpPrevSrc, lpSrc, iPos, m_iTILIndex, iPos - 1);
			if(pos >= 0){
				return pos;
			}
		} else if(m_iTILIndex > 0){
			// 漢数字の位取りを示す読点の処理
			if(m_iTILIndex >= 2){
				int iPrev1Index = m_iTILIndex - 1; // TT_TEXTタイプでの一つ前
				int iPrev2Index = m_iTILIndex - 2; // TT_TEXTタイプでの二つ前
				const auto &prev1_str = text_list[iPrev1Index].str;
				const auto &prev2_str = text_list[iPrev2Index].str;
				if(!prev1_str.empty() && !prev2_str.empty()){
					auto *pNumPaddingStr = l_numPaddingStr;
					for(int inps_len = sizeof(l_numPaddingStr)/sizeof(LPCTSTR); inps_len>0; --inps_len, ++pNumPaddingStr){
						// 現在の文字が漢数字、一つ前の文字が「、」「・」、二つ前の文字が漢数字
						if(!CGrText::strChar(*pNumPaddingStr, lpSrc)){
							continue; // 現在の文字 漢数字ではない
						}
						if(!CGrText::strChar(*pNumPaddingStr,
											 CGrText::CharNextN(prev2_str.c_str(), CGrText::CharLen(prev2_str.c_str())-1))
						   ){
							continue; // 、二つ前の文字 漢数字ではない
						}
						lpPrevSrc = prev1_str.c_str();
						if(CGrText::strChar(_T("、・"), lpPrevSrc)){
							// 一つ前の文字が「、」「・」
							TxtMiru::TextOffset of = {/*offsetType=*/m_param.GetCharOffsetType(lpPrevSrc)};
							if(of.offsetType != 0){
								m_textOffsetMap[TxtMiru::TextPoint{m_iLLIndex, iPrev1Index, 0}] = of;
								return l_TextOffsetTable.byteArrayH[of.offsetType];
							}
						}
					}
				}
			}
			if(iPos == 0 && m_iCurChar > 0){
				const auto &prev_str = text_list[m_iTILIndex - 1].str;
				auto lpPrevBeginSrc = prev_str.c_str();
				auto lpPrevEndSrc   = lpPrevBeginSrc + prev_str.size();
				lpPrevSrc = CGrText::CharPrev(lpPrevBeginSrc, lpPrevEndSrc);
				if(lpPrevSrc){
					// 始め括弧類(cl-01)、終わり括弧類(cl-02)、読点類(cl- 07)、句点類(cl-06)が連続する場合の配置
					int pos = setArrangeSize(lpSrc, lpNextSrc, lpPrevSrc, lpPrevEndSrc, iPos, m_iTILIndex-1, getPrevPos(lpPrevBeginSrc, lpPrevEndSrc));
					if(pos >= 0){
						return pos;
					}
				}
			}
		}
		return nextHeight;
	}
	// iHeight : 元の高さ
	void getModHeight(int &iHeight, int iLLIndex, int iIndex, int iPos)
	{
		if(!m_textOffsetMap.empty()){
			TxtMiru::TextPoint tp_tom{iLLIndex, iIndex, iPos};
			auto it_tom=m_textOffsetMap.find(tp_tom);
			if(it_tom != m_textOffsetMap.end()){
				iHeight = l_TextOffsetTable.byteArrayH[it_tom->second.offsetType];
			}
		}
	}

	// 一つ前の iPos 取得
	int getPrevPos(LPCTSTR lpStart, LPCTSTR lpEnd)
	{
		return CGrText::CharLen(std::tstring(lpStart, lpEnd).c_str()) - 1;
	}
	struct PosInfo {
		int iIndex;
		int iPos  ;
		int iNum  ;
		LPCTSTR lpSrc;
	};
	void getPrevPosInfo(PosInfo &pi, const TxtMiru::TextInfoList &text_list)
	{
		auto lpPrevBeginSrc = text_list[pi.iIndex].str.c_str();
		auto lpPrevEndSrc   = lpPrevBeginSrc + text_list[pi.iIndex].str.size();
		pi.lpSrc = CGrText::CharPrev(lpPrevBeginSrc, lpPrevEndSrc);
		if(pi.lpSrc){
			pi.iPos = getPrevPos(lpPrevBeginSrc, lpPrevEndSrc);
		}
	}
	void getPrevPos(PosInfo &pi, const TxtMiru::TextInfoList &text_list)
	{
		getPrevPos(pi, text_list, pi.iIndex, pi.iPos, text_list[pi.iIndex].str.c_str(), pi.lpSrc);
	}
	void getPrevPos(PosInfo &pi, const TxtMiru::TextInfoList &text_list, int iIndex, int iPos, LPCTSTR lpBeginSrc, LPCTSTR lpSrc)
	{
		pi.iIndex = iIndex;
		pi.iPos   = 0;
		pi.lpSrc  = nullptr;
		pi.iNum   = 1;
		if(iPos > 0){
			pi.lpSrc = CGrText::CharPrev(lpBeginSrc, lpSrc);
			pi.iPos = iPos - 1;
		} else {
			while(pi.iIndex > 0){
				--pi.iIndex;
				switch(text_list[pi.iIndex].textType){
				case TxtMiru::TextType::OTHER          : // through
				case TxtMiru::TextType::TEXT           : // through
				case TxtMiru::TextType::LINE_CHAR      :
					getPrevPosInfo(pi, text_list);
					return;
					break;
				case TxtMiru::TextType::ROTATE_NUM     : // through // 縦中横
				case TxtMiru::TextType::ROTATE_NUM_AUTO: // through
				case TxtMiru::TextType::KU1_CHAR       : // through
				case TxtMiru::TextType::KU2_CHAR       :
					getPrevPosInfo(pi, text_list);
					pi.iPos = 0;
					pi.iNum = CGrText::CharLen(pi.lpSrc);
					return;
					break;
				}
			}
		}
	}

	void setPicture(TxtMiru::LineInfo &li, int num)
	{
		if(m_bImageNextLayout){
			auto &&ppm = m_doc.GetPictPosMap();
			TxtMiru::PictPos pp;
			if(num == 1){
				nextLayout();
			} else if(num == m_iPageLayoutNum){
				nextPage();
			} else if(num == m_iLayoutMaxNum){
				nextPaperFirst();
			}
			pp.iStartLayout = m_iLayout;
			pp.iEndLayout   = m_iLayout + num - 1;
			for(int l=m_iLayout; l<=pp.iEndLayout; ++l){
				m_imageList.insert(l);
				nextLayoutForce();
			}
			ppm[TxtMiru::TextPoint{m_iLLIndex, m_iTILIndex, 0}] = pp;
			li.iEndCol = m_iCurLine;
			m_bNextLayout = true;
		} else {
			//
			int iLayout = (m_iLayout+num-1) / num * num + 1; // 画像配置後のレイアウト番号
			// 本文よりも画像から始まる場合は、画像の方を優先的に配置
			if(m_iCurTextLayoutLines <= 0){
				// 画像に必要なレイアウト番号を進める
				--iLayout;
				for(int i=0; i<num; ++i){
					nextLayoutForce();
				}
			}
			for(const auto &item : m_imageList){
				int l = (item+num-1) / num * num + 1;
				if(iLayout <= l){
					iLayout = l;
				}
			}
			auto &&ppm = m_doc.GetPictPosMap();
			TxtMiru::PictPos pp;
			pp.iStartLayout = iLayout;
			pp.iEndLayout   = iLayout + num - 1;
			for(int l=m_iLayout; l<=pp.iEndLayout; ++l){
				m_imageList.insert(l);
			}
			ppm[TxtMiru::TextPoint{m_iLLIndex, m_iTILIndex, 0}] = pp;
			if(li.text_list.size() <= 1){
				li.iEndCol = li.iStartCol; // 改行は不要
			}
		}
	}
	// １行の高さが、m_iLineBottomより大きい場合 文字幅調整可能な文字がないかチェックして
	// あればそれの高さを調整する
	void arrangeLineSkip(TxtMiru::TextTurnPos &ttp, TxtMiru::TextPoint fromTp, const TxtMiru::TextPoint &toTp)
	{
		struct CGrArrange : public CGrTxtBuffer::CGrCharFunc {
			const CGrTxtParam &param;
			TxtMiru::TextOffsetMap &textOffsetMap;
			TxtMiru::TextTurnPos &pre_ttp;
			CGrArrange(const CGrTxtParam &p, TxtMiru::TextOffsetMap &t, TxtMiru::TextTurnPos &pre)
			: param(p), textOffsetMap(t), pre_ttp(pre){}
			virtual bool IsValid(TxtMiru::TextType tt){
				return tt == TxtMiru::TextType::SKIP_CHAR_AUTO;
			}
			virtual bool SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd){
				if(textOffsetMap.find(cur) == textOffsetMap.end()){
					const WORD offsetType = 0x05;
					textOffsetMap[cur] = TxtMiru::TextOffset{offsetType};
					pre_ttp.iHeight -= l_max_offsetnum-l_TextOffsetTable.byteArrayH[offsetType];
				}
				return true;
			};
		};
		++fromTp.iPos; // 先頭行は対象外
		m_doc.GetTxtBuffer().ForEach(fromTp, toTp, CGrArrange(m_param, m_textOffsetMap, ttp));
	}
	void forceSkipLine(bool &bSkipNextLine)
	{
		if(bSkipNextLine){
			nextLine();
			m_bForceNewLine = false;
			bSkipNextLine = false;
			m_ttpNewLine = TxtMiru::TextTurnPos{m_iTILIndex, 0, m_iCurLine};
			m_ttpNewLine.iHeight = m_iCurChar - m_iLineTop;
			m_ttpNewLine.iNum    = m_iLineCharNum;
			m_iLineCharNum = 0;
		}
	}
	void doLineList(TxtMiru::LineInfo &li)
	{
		m_pCurLi = &li;
		initLine(li);
		auto &&tlpTurnList = li.tlpTurnList;
		tlpTurnList.clear();
		// 折り返し時のフォントサイズを設定
		if(m_iCharSize != 0){
			TxtMiru::TextPoint curtp{m_iLLIndex, 0, 0};
			auto &&textStyleMap = m_doc.GetTextStyleMap();
			auto it = textStyleMap.find(curtp);
			if(textStyleMap.end() == it){
				textStyleMap[curtp].uStatus = TxtMiru::TTPStatusNone;
				textStyleMap[curtp].fontSize = m_iCharSize;
			} else {
				(it->second).fontSize = m_iCharSize;
			}
		}
		tlpTurnList.push_back(TxtMiru::TextTurnPos{0,0,m_iCurLine});
		m_ttpNewLine.iNum    = 0;
		m_ttpNewLine.iHeight = 0;
		//
		auto &&text_list = li.text_list;
		int tl_len = text_list.size();
		TxtMiru::TextInfo *it = nullptr;
		if(tl_len > 0){
			it = &text_list[0];
		}
		int iLineBoxStart = -1;
		bool bHangingLine = false;
		bool bSkipNextLine = false;
		bool bOldInComment = m_bInComment;
		for(; tl_len>0; --tl_len, ++it, ++m_iTILIndex){ // text_list の ループ
			m_bNextLayout = false;
			bHangingLine = false;
			auto &&ti = (*it);
			if(isComment(ti)){
				ti.textType = TxtMiru::TextType::COMMENT;
				continue;
			}
			if(!isNeedNewLine(ti)){
				nextLine();
				m_bNextLine = false;
			}
			m_bSkipNextLine = false;
			const auto &text = ti.str;
			auto lpBeginSrc = text.c_str();
			auto lpNextSrc  = lpBeginSrc;
			switch(ti.textType){
			case TxtMiru::TextType::LINE_BOX_START:
				iLineBoxStart = m_iCurLine;
				break;
			case TxtMiru::TextType::LINE_BOX_END:
				if(iLineBoxStart != m_iCurLine && m_iCurTextLayoutLines == m_iCurTextLayoutMaxLines){
					// 行頭に罫線の閉じは入れない
					m_bSkipNextLine = true;
				}
				break;
			case TxtMiru::TextType::CENTER    :
				if(m_iCurTextLayoutLines > 0){
					nextLayout();
				}
				break;
			case TxtMiru::TextType::TEXT_SIZE :
				setCharSize((signed short)ti.chrType);
				break;
			case TxtMiru::TextType::HORZ_START:
				m_bHorz = true;
				if(m_iTILIndex == 0 && tl_len == 1){
					m_bSkipNextLine = true;
				}
				break;
			case TxtMiru::TextType::HORZ_END:
				m_bHorz = false;
				if(m_iTILIndex == 0 && tl_len == 1){
					m_bSkipNextLine = true;
				}
				break;
			case TxtMiru::TextType::OTHER     :
				forceSkipLine(bSkipNextLine);
				m_iCurChar = maxCurChar();
				++m_iLineCharNum;
				if(text.empty()){
					m_iCurChar += HalfCharCur(ti.chrType);
				} else {
					int nextHeight = m_iCurTextL2byteSize;
					int iPos = 0;
					auto lpSrc = _T("あ");
					lpNextSrc = CGrText::CharNext(lpSrc);
					if((m_iLineBottom >= m_iCurChar+nextHeight)){ // 折り返し不要
						m_iCurChar += nextHeight;
						break;
					}
					// 一つ前の文字位置取得
					PosInfo prePi;
					getPrevPos(prePi, text_list, m_iTILIndex, iPos, lpBeginSrc, lpSrc);
					int iPrevIndex = prePi.iIndex;
					int iPrevPos   = prePi.iPos;
					auto lpPrevSrc = prePi.lpSrc;
					do {
						if(lpPrevSrc){
							if(isLineStartSkipCharacter(lpPrevSrc)){
								// ２行目以降の行頭の無視する文字の時 ※折り返した後の先頭行が、先頭行時、非表示文字の時は 属性を非表示にして その分の高さを引いておく
								// ※折り返しての「空白」は、無視する
								text_list[iPrevIndex].textType = TxtMiru::TextType::SKIP_CHAR_AUTO;
								m_iCurChar -= HalfCharCur(text_list[iPrevIndex].chrType);
								--m_iLineCharNum;
								setTurnupNextLine(tlpTurnList, m_iTILIndex, iPos, nextHeight);
								m_iLineCharNum = 0 + 1; /* 追い出した文字は無視 + 現在の文字 */
								break;
							} else if(isLineEndNGCharacter(lpPrevSrc)){
								// 行末禁則文字 は 次の行へ
								int prevNextHeight = HalfCharCur(text_list[iPrevIndex].chrType);
								// 前の文字が、文字幅調整をしていたら prevNextHeight も変更
								getModHeight(prevNextHeight, m_iLLIndex, iPrevIndex, iPrevPos);
								m_iLineCharNum -= prePi.iNum + 1;            /* prePi.iNum 文字追い出すので、その分 カレント行から一文字削除 */
								m_iCurChar     -= prevNextHeight*prePi.iNum; /*                                                              */
								setTurnupNextLine(tlpTurnList, iPrevIndex, iPrevPos, nextHeight + prevNextHeight);
								m_iLineCharNum = prePi.iNum + 1; /* 追い出した文字 + 現在の文字 */
								break;
							} else if(isHangingCharacter(lpPrevSrc)){
								//
							}
						}
						--m_iLineCharNum;
						if(isLineStartSkipCharacter(lpSrc)){
							// ２行目以降の行頭の無視する文字の時
							ti.textType = TxtMiru::TextType::SKIP_CHAR_AUTO;
							//DBGPRINT(_T("setTurnupNextLine"));
							setTurnupNextLine(tlpTurnList, m_iTILIndex, iPos, 0);
						} else {
							setTurnupNextLine(tlpTurnList, m_iTILIndex, iPos, nextHeight);
							m_iLineCharNum = 1;
						}
					} while(0);
				}
				break;
			case TxtMiru::TextType::ROTATE_NUM      : // 縦中横
				forceSkipLine(bSkipNextLine);
				m_iCurChar = maxCurChar();
				m_iCurChar += m_iCurTextL2byteSize;
				{
					do {
						if(!isTurnup()){/* 折り返し */
							break;
						}
						bool bNoWrap = false;
						auto lpSrc = lpNextSrc;
						PosInfo prePi;
						getPrevPos(prePi, text_list, m_iTILIndex, 0, lpBeginSrc, lpSrc);
						int iPrevIndex = prePi.iIndex;
						int iPrevPos   = prePi.iPos;
						auto lpPrevSrc = prePi.lpSrc;
						if(lpPrevSrc){
							int nextHeightOrg = m_iCurTextL2byteSize;
							int nextHeight = arrangePos(nextHeightOrg, lpSrc, lpNextSrc, 0, text_list);
							// 折り返し (画面外にいく場合は、改行)
							if(isHangingCharacter(lpPrevSrc)){
							} else if(isLineStartNGCharacters(m_param, it, tl_len, lpSrc)){
								//
								// 行頭禁則文字
								if(m_bNoPrevLine){ /* 追い込み禁止 */
									// 戻す余白がない場合は、引き連れて追い出す
									auto [prePi_iNum, iPrev2Index, iPrev2Pos, afterAddHeight] = getRunOutPos(tlpTurnList, text_list, m_iTILIndex, 0);
									if(prePi_iNum == -2){
										break;
									} else if(prePi_iNum > 0){
										m_iLineCharNum -= prePi_iNum;
										m_iCurChar     -= afterAddHeight;
										setTurnupNextLine(tlpTurnList, iPrev2Index, iPrev2Pos, afterAddHeight);
										m_iLineCharNum = prePi_iNum; /* 追いやった文字 + 現在の文字 */
										break;
									}
								} else {
									// 行頭禁則文字
									// ぶら下げ文字は、改行しない + 文字幅調整
									m_iCurChar += nextHeight;
									m_iLineCharNum -= prePi.iNum;
								}
								break;
							} else if(isLineEndNGCharacter(lpPrevSrc)){
								// 行末禁則文字 は 次の行へ
								int prevNextHeight = HalfCharCur(text_list[iPrevIndex].chrType);
								int afterAddHeight = prevNextHeight;
								// 前の文字が、文字幅調整をしていたら prevNextHeight も変更
								getModHeight(prevNextHeight, m_iLLIndex, iPrevIndex, iPrevPos);
								m_iLineCharNum -= (prePi.iNum + 1);              /* 一文字追いやるので、その分 カレント行から一文字削除 */
								m_iCurChar     -= m_iCurTextL2byteSize+prevNextHeight;
								setTurnupNextLine(tlpTurnList, iPrevIndex, iPrevPos, m_iCurTextL2byteSize + prevNextHeight);
								m_iLineCharNum = 1 + 1; /* 追い出した文字 + 現在の文字 */
								break;
							}
						}
						m_iCurChar -= m_iCurTextL2byteSize;
						setTurnupNextLine(tlpTurnList, m_iTILIndex, 0, m_iCurTextL2byteSize);
					} while(0);
					++m_iLineCharNum;
				}
				break;
			case TxtMiru::TextType::ROTATE_NUM_AUTO:
			case TxtMiru::TextType::TEXT           :
			case TxtMiru::TextType::LINE_CHAR      :
			case TxtMiru::TextType::SKIP_CHAR      :
			case TxtMiru::TextType::SKIP_CHAR_AUTO :
				forceSkipLine(bSkipNextLine);
				m_iCurChar = maxCurChar();
				if(!m_bHorz && isTateChuYoko(text_list, m_iTILIndex, ti.chrType, text.c_str(), m_iTateChuNum)/* 縦中横 */){
					m_iCurChar += m_iCurTextL2byteSize;
					do {
						if(!isTurnup()){/* 折り返し */
							break;
						}
						bool bNoWrap = false;
						auto lpSrc = lpNextSrc;
						PosInfo prePi;
						getPrevPos(prePi, text_list, m_iTILIndex, 0, lpBeginSrc, lpSrc);
						int iPrevIndex = prePi.iIndex;
						int iPrevPos   = prePi.iPos;
						auto lpPrevSrc = prePi.lpSrc;
						if(lpPrevSrc){
							int nextHeightOrg = m_iCurTextL2byteSize;
							int nextHeight = arrangePos(nextHeightOrg, lpSrc, lpNextSrc, 0, text_list);
							// 折り返し (画面外にいく場合は、改行)
							if(isHangingCharacter(lpPrevSrc)){
							} else if(isLineStartNGCharacters(m_param, it, tl_len, lpSrc)){
								//
								// 行頭禁則文字
								if(m_bNoPrevLine){ /* 追い込み禁止 */
									auto orgTextType = ti.textType;
									ti.textType = TxtMiru::TextType::ROTATE_NUM_AUTO; // 設定から縦中横自動設定
									auto [prePi_iNum, iPrev2Index, iPrev2Pos, afterAddHeight] = getRunOutPos(tlpTurnList, text_list, m_iTILIndex, 0); DBGPRINT(_T("prePi_iNum=%d"), prePi_iNum);
									if(prePi_iNum == -2){
										break;
									} else if(prePi_iNum > 0){
										m_iLineCharNum -= prePi_iNum;
										m_iCurChar     -= afterAddHeight;
										setTurnupNextLine(tlpTurnList, iPrev2Index, iPrev2Pos, afterAddHeight);
										m_iLineCharNum = prePi_iNum; /* 追いやった文字 + 現在の文字 */
										break;
									}
									ti.textType = orgTextType;

								} else {
									// 行頭禁則文字
									// ぶら下げ文字は、改行しない + 文字幅調整
									m_iCurChar += nextHeight;
									m_iLineCharNum -= prePi.iNum;
								}
								break;
							} else if(isLineEndNGCharacter(lpPrevSrc)){
								// 行末禁則文字 は 次の行へ
								int prevNextHeight = HalfCharCur(text_list[iPrevIndex].chrType);
								// 前の文字が、文字幅調整をしていたら prevNextHeight も変更
								getModHeight(prevNextHeight, m_iLLIndex, iPrevIndex, iPrevPos);
								m_iLineCharNum -= (prePi.iNum + 1);              /* 一文字追いやるので、その分 カレント行から一文字削除 */
								m_iCurChar     -= m_iCurTextL2byteSize+prevNextHeight;
								setTurnupNextLine(tlpTurnList, iPrevIndex, iPrevPos, m_iCurTextL2byteSize + prevNextHeight);
								m_iLineCharNum = 1 + 1; /* 追い出した文字 + 現在の文字 */
								break;
							}
						}
						m_iCurChar -= m_iCurTextL2byteSize;
						setTurnupNextLine(tlpTurnList, m_iTILIndex, 0, m_iCurTextL2byteSize);
					} while(0);
					ti.textType = TxtMiru::TextType::ROTATE_NUM_AUTO; // 設定から縦中横自動設定
					++m_iLineCharNum;
				} else {
					if(/**/ti.textType == TxtMiru::TextType::ROTATE_NUM_AUTO // 前回、自動で縦中横に設定されていたものを横書きに再設定(上の条件から外れているので)
					   ||  ti.textType == TxtMiru::TextType::SKIP_CHAR_AUTO ){
						ti.textType = TxtMiru::TextType::TEXT;
					}
					int nextHeightOrg = HalfCharCur(ti.chrType);
					//
					for(int iPos=0; *lpNextSrc; ++iPos){
						auto lpSrc = lpNextSrc;
						lpNextSrc = CGrText::CharNext(lpSrc);
						if(!(*lpNextSrc) && isHangingCharacter(lpSrc)){
							bHangingLine = true;
						}
						// 句読点や括弧類などの基本的な配置
						int nextHeight = arrangePos(nextHeightOrg, lpSrc, lpNextSrc, iPos, text_list);
						if(m_iLineCharNum == 0 && tlpTurnList.size() >= 2){ // 折り返し直後
							if(isLineStartSkipCharacter(lpSrc)){
								// ２行目以降の行頭の無視する文字の時
								ti.textType = TxtMiru::TextType::SKIP_CHAR_AUTO;
								m_iCurChar = m_iCurSubChar = m_iLineTop;
								continue;
							} else if(iPos == 0 && isLineStartNGCharacters(m_param, it, tl_len, lpSrc)){
								// スキップ文字の後、行頭禁則文字があると チェックから漏れるので再度 行頭禁止 設定
								tlpTurnList.pop_back();
								prevLine();
								auto &&ttl_last_pos = tlpTurnList[tlpTurnList.size() - 1];

								// 一行戻す
								if(ttl_last_pos.iNum == 0){
									ttl_last_pos.iNum = m_iLineCharNum + 1;
								} else {
									ttl_last_pos.iNum += 1;
								}
								m_iCurChar -= nextHeight;
								if(tl_len > 1){
									m_iLineCharNum = ttl_last_pos.iNum + 1; // この文字の分(ttl_last_pos.iNum)と スキップ文字の分(+1)
									TxtMiru::TextPoint pretp{m_iLLIndex, ttl_last_pos.iIndex, ttl_last_pos.iPos};
									TxtMiru::TextPoint curtp{m_iLLIndex, m_iTILIndex        , iPos + 1         };
									arrangeLineSkip(ttl_last_pos, pretp, curtp);
									setTurnupNextLine(tlpTurnList, ttl_last_pos.iHeight+nextHeight, TxtMiru::TextTurnPos{m_iTILIndex,iPos+1,m_iCurLine}, 0);
								}
								continue;
							}
						} // 折り返し直後
						if(TxtMiruType::isSkipChar(ti.textType)){ // 描画対象外文字は、スキップ
							continue;
						}
						++m_iLineCharNum;
						if(m_bForceNewLine){ // 途中で、天付きとかの設定をされた場合 強制的に改行
							m_bForceNewLine = false;
							m_ttpNewLine = TxtMiru::TextTurnPos{m_iTILIndex, iPos, m_iCurLine};
							m_ttpNewLine.iHeight = m_iCurChar - m_iLineTop;
							m_ttpNewLine.iNum    = m_iLineCharNum;
							m_iLineCharNum = 0;
						} else if(m_ttpNewLine.iHeight > 0){/* 折り返し */
							int num = m_iCurChar - m_ttpNewLine.iHeight;
							if(!tlpTurnList.empty() && m_ttpNewLine <= tlpTurnList.back()){
								// 強制改行を行う際に、同じ改行位置が 既に登録されている場合は 改行不要とみなす
								// ２回来ると 余計な空白行ができるため
								if(m_bBottomIndent){
									tlpTurnList.back().iRIndent = m_iBottomIndent / text_l_2byte_size;
								}
							} else {
								setTurnupNextLine(tlpTurnList, m_iLineBottom, m_ttpNewLine, nextHeight+nextHeightOrg);
							}
							m_ttpNewLine.iNum    = 0;
							m_ttpNewLine.iHeight = 0;
							// 折り返し (画面外にいく場合は、改行)
							continue;
						}
						if(m_iLineBottom >= m_iCurChar+nextHeight){ // 折り返し不要
							m_iCurChar += nextHeight;
							continue;
						}
						// 一つ前の文字位置取得
						PosInfo prePi;
						getPrevPos(prePi, text_list, m_iTILIndex, iPos, lpBeginSrc, lpSrc);
						int iPrevIndex = prePi.iIndex;
						int iPrevPos   = prePi.iPos;
						auto lpPrevSrc = prePi.lpSrc;
						//
						auto &&ttl_last_pos = tlpTurnList[tlpTurnList.size() - 1];
						if(//    分割禁止文字はできるだけ分割されないようにする
						   (isSeparateNGCharacter(lpSrc) && m_iCurChar < m_iLineBottom
							&& iPos > 0 && CGrText::nCmp(CGrText::CharPrev(lpBeginSrc, lpSrc), lpSrc, 1) == 0)
						   || // 分割禁止文字「―」 はできるだけ分割されないようにする
						   (TxtMiru::TextType::LINE_CHAR == ti.textType && m_iCurChar < m_iLineBottom
							&& m_iTILIndex > 0 && TxtMiru::TextType::LINE_CHAR == text_list[m_iTILIndex-1].textType)
						   ){
							if(m_bNoPrevLine){ /* 追い込み禁止 */
								// 戻す余白がない場合は、引き連れて追い出す
								int prevNextHeight = nextHeight;
								int afterAddHeight = nextHeightOrg-nextHeight+prevNextHeight;
								m_iLineCharNum -= (prePi.iNum + 1);          /* prePi.iNum 文字分 追いやるので、その分 カレント行から一文字削除 */
								m_iCurChar     -= prevNextHeight*prePi.iNum; /*                                                                 */
								setTurnupNextLine(tlpTurnList, iPrevIndex, iPrevPos, afterAddHeight);
								m_iLineCharNum = 1 + 1; /* 追いやった文字 + 現在の文字 */
								m_iCurChar += afterAddHeight;
							} else {
								// 一行戻す
								if(ttl_last_pos.iNum == 0){
									ttl_last_pos.iNum = m_iLineCharNum;
								} else {
									ttl_last_pos.iNum += 1;
								}
								m_iCurChar += nextHeight;
							}
							continue;
						}
						do {
							bool bNoWrap = false;
							if(lpPrevSrc){
								// 折り返し (画面外にいく場合は、改行)
								if(isHangingCharacter(lpPrevSrc)){
									// 折り返して、行頭が「ぶら下げ文字」の場合は 前の行に
									// ぶら下げ文字
									if((iPrevIndex > 0 || iPrevPos > 0) && isLineStartNGCharacters(m_param, it, tl_len, lpSrc)){
										// 戻す余白がない場合は、2つ前の文字から引き連れて追い出す
										int prevNextHeight = HalfCharCur(text_list[iPrevIndex].chrType);
										int afterAddHeight = nextHeight+prevNextHeight*prePi.iNum;
										int prePi_iNum = prePi.iNum;
										//
										m_iLineCharNum -= (prePi.iNum + 1);          /* prePi.iNum 文字分 追いやるので、その分 カレント行から一文字削除 */
										m_iCurChar     -= prevNextHeight*prePi.iNum; /*                                                                 */
										//
										int iPrev2Index = iPrevIndex;
										int iPrev2Pos   = iPrevPos;
										if(iPrev2Pos > 0){
											--iPrev2Pos;
											m_iLineCharNum -= prePi.iNum;
											m_iCurChar     -= prevNextHeight;
											afterAddHeight += prevNextHeight;
											prePi_iNum += 1;
										} else if(iPrev2Index > 0){
											--iPrev2Index;
											const auto *pPre2Pi = &text_list[iPrev2Index];
											for(; iPrev2Index >= 0; --pPre2Pi, --iPrev2Index){
												auto textType = pPre2Pi->textType;
												if(TxtMiruType::isTextOrSkipOrRotateNum(textType)){
													break;
												} else if(TxtMiruType::isSmallNote(textType)){
													/**/
												} else {
													pPre2Pi = nullptr;
													break;
												}
											}
											if(pPre2Pi){
												iPrev2Pos =  CGrText::CharLen(pPre2Pi->str.c_str()) - 1;
												int prev2NextHeight = HalfCharCur(pPre2Pi->chrType);
												m_iCurChar     -= prev2NextHeight;
												afterAddHeight += prev2NextHeight;
												prePi_iNum += 1;
												setTurnupNextLine(tlpTurnList, iPrev2Index, iPrev2Pos, afterAddHeight);
												m_iLineCharNum = prePi_iNum + 1; /* 追いやった文字 + 現在の文字 */
												break;
											}
										}
									}
									bNoWrap = true;
								} else if(isLineStartNGCharacters(m_param, it, tl_len, lpSrc)){
									// 行頭禁則文字
									if(m_bNoPrevLine){ /* 追い込み禁止 */
										// 戻す余白がない場合は、引き連れて追い出す
										{
											auto [prePi_iNum, iPrev2Index, iPrev2Pos, afterAddHeight] = getRunOutPos(tlpTurnList, text_list, m_iTILIndex, iPos);
											if(prePi_iNum == -2){
												break;
											} else if(prePi_iNum > 0){
												// Ruby付き文字が分かれるので対応予定
												m_iLineCharNum -= prePi_iNum;
												m_iCurChar     -= afterAddHeight - nextHeight/*m_iLineCharNumは先に追加されているがm_iCurCharはこれから追加予定だったので*/;
												setTurnupNextLine(tlpTurnList, iPrev2Index, iPrev2Pos, afterAddHeight);
												m_iLineCharNum = prePi_iNum; /* 追いやった文字 + 現在の文字 */
												break;
											}
										}
										//
										int prevNextHeight = nextHeightOrg; // +1する代わり ※1
										int afterAddHeight = nextHeight+prevNextHeight*prePi.iNum;
										//
										m_iLineCharNum -= (prePi.iNum + 1);          /* prePi.iNum 文字分 追いやるので、その分 カレント行から一文字削除 */
										m_iCurChar     -= prevNextHeight*(prePi.iNum + 0);
										setTurnupNextLine(tlpTurnList, iPrevIndex, iPrevPos, afterAddHeight);
										m_iLineCharNum = prePi.iNum + 1; /* 追いやった文字 + 現在の文字 */
									} else {
										// 行頭禁則文字
										// ぶら下げ文字は、改行しない + 文字幅調整
										m_iCurChar += nextHeight;
										m_iLineCharNum -= prePi.iNum;
										ttl_last_pos.iNum    -= prePi.iNum;
										ttl_last_pos.iHeight -= nextHeight;
									}
									break;
								} else if(isLineStartSkipCharacter(lpPrevSrc)){
									// ２行目以降の行頭の無視する文字の時 ※折り返した後の先頭行が、先頭行時、非表示文字の時は 属性を非表示にして その分の高さを引いておく
									// ※折り返しての「空白」は、無視する
									text_list[iPrevIndex].textType = TxtMiru::TextType::SKIP_CHAR_AUTO;
									m_iCurChar -= HalfCharCur(text_list[iPrevIndex].chrType);
									--m_iLineCharNum;
									setTurnupNextLine(tlpTurnList, m_iTILIndex, iPos, nextHeight);
									m_iLineCharNum = 0 + 1; /* 追い出し文字は無視 + 現在の文字 */
									break;
								} else if(isLineEndNGCharacter(lpPrevSrc)){
									// 行末禁則文字 は 次の行へ
									int prevNextHeight = HalfCharCur(text_list[iPrevIndex].chrType);
									int afterAddHeight = nextHeightOrg-nextHeight+prevNextHeight;
									// 前の文字が、文字幅調整をしていたら prevNextHeight も変更
									getModHeight(prevNextHeight, m_iLLIndex, iPrevIndex, iPrevPos);
									m_iLineCharNum -= prePi.iNum + 1;            /* prePi.iNum 文字追い出すので、その分 カレント行から一文字削除 */
									m_iCurChar     -= prevNextHeight*prePi.iNum; /*                                                              */
									setTurnupNextLine(tlpTurnList, iPrevIndex, iPrevPos, nextHeight + prevNextHeight);
									m_iLineCharNum = prePi.iNum + 1; /* 追い出した文字 + 現在の文字 */
									break;
								}
							}
							if(tlpTurnList.size() > 0){ // 一文字だけ改行されないように
								if (m_bBottomIndent && m_iTILIndex > 0) { // 字上で最初の一文字の場合は、改行する
									if (text_list[m_iTILIndex-1].textType == TxtMiru::TextType::RINDENT) {
										setTurnupNextLine(tlpTurnList, m_iLineBottom, m_ttpNewLine, nextHeight);
										m_ttpNewLine.iNum = 0;
										m_ttpNewLine.iHeight = 0;
										continue;
									}
								}
								int len = CGrText::CharLen(lpSrc);
								if(len <= 1 && tl_len <= 3){
									int iTmp_Index = m_iTILIndex;
									bool bArrangeLine = true;
									if(tl_len > 1){
										++iTmp_Index;
										bool bHanging = false;
										const auto *pNextPi = &text_list[iTmp_Index];
										int iTmp_tl_len = text_list.size();
										for(; iTmp_Index<iTmp_tl_len; ++iTmp_Index){
											auto textType = pNextPi->textType;
											if(TxtMiruType::isTextOrSkip(textType)){
												if(isHangingCharacter(pNextPi->str.c_str())){ // ぶら下げ文字が連続で続く場合は、不可
													if(bHanging){
														bArrangeLine = false;
														break;
													}
													bHanging = true;
												} else {
													bArrangeLine = false;
													break;
												}
											} else if(!TxtMiruType::isSmallNote(textType)){
												bArrangeLine = false;
												break;
											}
										}
									}
									if(bArrangeLine){
										int iHeight = m_iCurChar - m_iLineTop + nextHeight;
										auto pre_ttp = tlpTurnList.back();
										TxtMiru::TextTurnPos tmp_ttp{iTmp_Index,0,m_iCurLine};
										tmp_ttp.iHeight = iHeight;
										TxtMiru::TextPoint pretp{m_iLLIndex, pre_ttp.iIndex, pre_ttp.iPos};
										TxtMiru::TextPoint curtp{m_iLLIndex, tmp_ttp.iIndex, tmp_ttp.iPos};
										checkArrangeLine(tmp_ttp,pretp ,curtp);
										if(tmp_ttp.iHeight <= m_iLineBottom){
											m_iCurChar += nextHeight;
											continue;
										}
									}
								}
							}
							--m_iLineCharNum;
							if(isLineStartSkipCharacter(lpSrc)){ // 行頭スキップ文字
								// ２行目以降の行頭の無視する文字の時
								ti.textType = TxtMiru::TextType::SKIP_CHAR_AUTO;
								auto [iTmp_Index, iTmp_Pos, lpTmpChar] = getNextCharInfo(text_list, m_iTILIndex, iPos);
								if(lpTmpChar && isLineStartNGCharacter(lpTmpChar)){
									// 次の文字が、行頭禁則文字なら追い出す
									auto [iNum, iPrev2Index, iPrev2Pos, afterAddHeight] = getRunOutPosSub(tlpTurnList, text_list, m_iTILIndex, iPos);
									if(iNum == -2){
										break;
									} else if(iNum > 0){
										m_iLineCharNum -= iNum;
										m_iCurChar     -= afterAddHeight - nextHeight/*m_iLineCharNumは先に追加されているがm_iCurCharはこれから追加予定だったので*/;
										setTurnupNextLine(tlpTurnList, iPrev2Index, iPrev2Pos, afterAddHeight);
										m_iLineCharNum = iNum; /* 追いやった文字 + 現在の文字 */
										break;
									}
								}
								setTurnupNextLine(tlpTurnList, m_iTILIndex, iPos, 0);
							} else {
								auto iNum = getNextCharCount(text_list, m_iTILIndex, iPos);
								if(iNum < 1){
									auto [iNum, iPrev2Index, iPrev2Pos, afterAddHeight] = getRunOutPosSub(tlpTurnList, text_list, m_iTILIndex, iPos);
									if(iNum == -2){
										break;
									} else if(iNum > 0){
										m_iLineCharNum -= iNum;
										m_iCurChar     -= afterAddHeight - nextHeight/*m_iLineCharNumは先に追加されているがm_iCurCharはこれから追加予定だったので*/;
										setTurnupNextLine(tlpTurnList, iPrev2Index, iPrev2Pos, afterAddHeight);
										m_iLineCharNum = iNum; /* 追いやった文字 + 現在の文字 */
										break;
									}
								}
								setTurnupNextLine(tlpTurnList, m_iTILIndex, iPos, nextHeight);
								m_iLineCharNum = 1;
							}
							if(bNoWrap){
								int l = tlpTurnList.size();
								if(l >= 2){
									int prevNextHeight = HalfCharCur(text_list[iPrevIndex].chrType);
									tlpTurnList[l-2].iNum    += prePi.iNum;
									tlpTurnList[l-2].iHeight -= prevNextHeight*prePi.iNum;
								}
							}
						} while(0);
					}
				}
				break;
			case TxtMiru::TextType::SMALL_NOTE     :
			case TxtMiru::TextType::SMALL_NOTE_R   :
			case TxtMiru::TextType::SMALL_NOTE_L   :
			case TxtMiru::TextType::SUP_NOTE       :
			case TxtMiru::TextType::SUB_NOTE       :
			case TxtMiru::TextType::GUID_MARK      :
				forceSkipLine(bSkipNextLine);
				m_iCurSubChar = max(m_iCurSubChar, m_iCurChar + HalfCharLHalf(ti.chrType) * CGrText::CharLen(text.c_str()));
				break;
			case TxtMiru::TextType::RUBY:
			case TxtMiru::TextType::RUBY_L:
			case TxtMiru::TextType::NOTE_L:
			case TxtMiru::TextType::UNKOWN_ERROR:
				{
					// ルビを振る文字列をチェック(本文)
					int begin_col = 0;
					int end_col   = 0;
					TxtMiru::TextTurnPos *pttp1st = nullptr;
					TxtMiru::TextTurnPos *pttp2nd = nullptr;
					for(auto &&item : tlpTurnList){
						if(item <= ti.tpBegin){
							++begin_col;
							pttp1st = &item;
						}
						if(item <= ti.tpEnd){
							++end_col;
							pttp2nd = &item;
						}
					}
					if(!pttp1st || !pttp2nd){
						break;
					}
					int ruby_line_num = end_col-begin_col;
					//* □RUBY長対応
					if(ruby_line_num == 0){
						const auto &layout = m_doc.GetConstLayout();
						auto lsRuby = layout.GetLineSize(CGrTxtLayout::LineSizeType::Ruby);
						int ruby_len = 0;
						auto lpRuby=ti.str.c_str();
						while(*lpRuby){
							auto lpRubyNext = CGrText::CharNext(lpRuby);
							ruby_len += HalfCharLHalf(CGrText::GetStringTypeEx(lpRuby, lpRubyNext-lpRuby));
							lpRuby = lpRubyNext;
						}
						TxtMiru::TextPoint tp1st{m_iLLIndex, ti.tpBegin.iIndex, ti.tpBegin.iPos};
						TxtMiru::TextPoint tp2nd{m_iLLIndex, ti.tpEnd  .iIndex, ti.tpEnd  .iPos};
						std::tstring target_str;
						m_doc.ToString(target_str, tp1st, tp2nd);
						int target_len = 0;
						auto lpTarget=target_str.c_str();
						WORD wStrType = 0;
						while(*lpTarget){
							auto lpTargetNext = CGrText::CharNext(lpTarget);
							wStrType = CGrText::GetStringTypeEx(lpTarget, lpTargetNext-lpTarget);
							target_len += HalfCharCur(wStrType);
							lpTarget = lpTargetNext;
						}
						int diff_len = ruby_len - target_len;
						if(diff_len > 0){
							TxtMiru::TextOffset to = {0};
							if(m_param.GetBoolean(CGrTxtParam::PointsType::RubyPosition)){
								// ルビが中付の場合のみ
								// 前の文字が、記号 又は 平仮名のときは ルビ一文字分ははみ出てもＯＫに
								for(int iTmpIndex=ti.tpBegin.iIndex-1; iTmpIndex>=0; --iTmpIndex){
									const auto &item = text_list[iTmpIndex];
									if(CGrTxtParser::isText(item.textType)){
										auto lpTmpTarget=item.str.c_str();
										auto lpTmpTargetNext = CGrText::CharNext(lpTmpTarget);
										auto wPrevStrType = CGrText::GetStringTypeEx(lpTmpTarget, lpTmpTargetNext-lpTmpTarget);
										if(wPrevStrType != wStrType && wPrevStrType != 0x8100){
											to.y = -1;
										}
										break;
									}
								}
							}
							// 後の文字が、記号 又は 平仮名のときは ルビ一文字分ははみ出てもＯＫに
							bool bFind = false;
							int iTmpLen = text_list.size();
							for(int iTmpIndex=m_iTILIndex+1; iTmpIndex<iTmpLen; ++iTmpIndex){
								const auto &item = text_list[iTmpIndex];
								if(CGrTxtParser::isText(item.textType)){
									auto lpTmpTarget=item.str.c_str();
									auto lpTmpTargetNext = CGrText::CharNext(lpTmpTarget);
									auto wNextStrType = CGrText::GetStringTypeEx(lpTmpTarget, lpTmpTargetNext-lpTmpTarget);
									if(wNextStrType != wStrType && wNextStrType != 0x8100){
										to.h = 1;
									}
									bFind = true;
									break;
								}
							}
							if(!bFind){
								to.h = 1;
							}
							if(to.y != 0){
								if(to.h != 0){
									to.y = -min(diff_len / 2, text_l_half_2byte_size) / text_l_half_1byte_size;
									to.h = min(diff_len  / 2, text_l_half_2byte_size) / text_l_half_1byte_size;
								} else {
									to.y = -min(diff_len, text_l_half_2byte_size) / text_l_half_1byte_size;
								}
							} else if(to.h != 0){
								to.h = min(diff_len, text_l_half_2byte_size) / text_l_half_1byte_size;
							}
							ruby_len -= ((-to.y) + (to.h)) * text_l_half_1byte_size;
							m_textOffsetMap[TxtMiru::TextPoint{m_iLLIndex, m_iTILIndex, -1}] = to;
							m_iCurChar += (ruby_len-target_len);
							break;
						}
					}
					//*/
					if(ruby_line_num == 0){ break; } // (本文の)ルビを振る文字列が同一行の場合は、チェック不要
					// 分割禁止
					if(ruby_line_num != 1){ // (本文の)ルビを振る文字列が3行以上にまたがる場合は、無視
						break;
					}
					// (本文の)ルビを振る文字列が 2行にまたがっている際の処理
					TxtMiru::TextPoint tp1st     {m_iLLIndex, ti.tpBegin.iIndex, ti.tpBegin.iPos};
					TxtMiru::TextPoint tp2nd_pre {m_iLLIndex, pttp2nd->iIndex  , pttp2nd->iPos-1};
					TxtMiru::TextPoint tp2nd     {m_iLLIndex, pttp2nd->iIndex  , pttp2nd->iPos  };
					TxtMiru::TextPoint tp3rd     {m_iLLIndex, ti.tpEnd.iIndex  , ti.tpEnd.iPos  };
					std::tstring str_1st;
					std::tstring str_2nd;
					m_doc.ToString(str_1st, tp1st, tp2nd_pre);
					m_doc.ToString(str_2nd, tp2nd, tp3rd    );
					int len_1st = CGrText::CharLen(str_1st.c_str());
					int len_2nd = CGrText::CharLen(str_2nd.c_str());
					if(len_2nd == 1){
						// ルビを振っている文字は、改行させない
						if(m_bNoPrevLine){ /* 追い込み禁止 */
							// 戻す余白がない場合は、引き連れて追い出す
							auto lp1stStr = str_1st.c_str();
							int nextHeight = 0;
							int begin_iIndex = ti.tpBegin.iIndex;
							int begin_iPos   = ti.tpBegin.iPos  ;
							if(ti.tpBegin.iIndex > 0 && ti.tpBegin.iPos == 0){
								int iPrev2Index = ti.tpBegin.iIndex - 1;
								const auto *pPre2Pi = &text_list[iPrev2Index];
								for(; iPrev2Index >= 0; --pPre2Pi, --iPrev2Index){
									auto textType = pPre2Pi->textType;
									if(TxtMiruType::isTextOrSkipOrRotateNum(textType)){
										break;
									} else if(
										/**/TxtMiru::TextType::RUBY_SEP       == textType
										||  TxtMiru::TextType::RUBY           == textType
										||  TxtMiru::TextType::RUBY_L         == textType
										||  TxtMiru::TextType::NOTE_L         == textType
										){
										/**/
									} else {
										pPre2Pi = nullptr;
										break;
									}
								}
								if(pPre2Pi){
									auto lpPrev2Str = pPre2Pi->str.c_str();
									int iPrev2Pos = CGrText::CharLen(lpPrev2Str) - 1;
									lpPrev2Str += iPrev2Pos;
									if(isLineEndNGCharacter(lpPrev2Str)){
										nextHeight += HalfCharCur(pPre2Pi->chrType);
										begin_iIndex = iPrev2Index;
										begin_iPos   = iPrev2Pos  ;
									}
								}
							}
							while(*lp1stStr){
								auto lp1stStrNext = CGrText::CharNext(lp1stStr);
								nextHeight += HalfCharCur(CGrText::GetStringTypeEx(lp1stStr, lp1stStrNext-lp1stStr));
								lp1stStr = lp1stStrNext;
							}
							pttp1st->iNum    -= 1;
							pttp1st->iHeight -= nextHeight;
							*pttp2nd = TxtMiru::TextTurnPos{begin_iIndex, begin_iPos, m_iCurLine};
							m_iCurChar += nextHeight;
						} else {
							tlpTurnList.pop_back();
							prevLine();
							m_iLineCharNum += pttp1st->iNum;
							m_iCurChar += pttp1st->iHeight;
						}
					} else if(len_1st <= 2){
						// 追い出す
						int nextHeight = 0;
						// 追い出した結果、行末禁則文字が行末に来る場合は引き連れる
						int begin_iIndex = ti.tpBegin.iIndex;
						int begin_iPos   = ti.tpBegin.iPos  ;
						if(ti.tpBegin.iIndex > 0 && ti.tpBegin.iPos == 0){
							int iPrev2Index = ti.tpBegin.iIndex - 1;
							const auto *pPre2Pi = &text_list[iPrev2Index];
							for(; iPrev2Index >= 0; --pPre2Pi, --iPrev2Index){
								auto textType = pPre2Pi->textType;
								if(TxtMiruType::isTextOrSkipOrRotateNum(textType)){
									break;
								} else if(
									/**/TxtMiru::TextType::RUBY_SEP       == textType
									||  TxtMiru::TextType::RUBY           == textType
									||  TxtMiru::TextType::RUBY_L         == textType
									||  TxtMiru::TextType::NOTE_L         == textType
									){
									/**/
								} else {
									pPre2Pi = nullptr;
									break;
								}
							}
							if(pPre2Pi){
								auto lpPrev2Str = pPre2Pi->str.c_str();
								int iPrev2Pos = CGrText::CharLen(lpPrev2Str) - 1;
								lpPrev2Str += iPrev2Pos;
								if(isLineEndNGCharacter(lpPrev2Str)){
									nextHeight += HalfCharCur(pPre2Pi->chrType);
									begin_iIndex = iPrev2Index;
									begin_iPos   = iPrev2Pos  ;
								}
							}
						}
						auto lp1stStr = str_1st.c_str();
						while(*lp1stStr){
							auto lp1stStrNext = CGrText::CharNext(lp1stStr);
							nextHeight += HalfCharCur(CGrText::GetStringTypeEx(lp1stStr, lp1stStrNext-lp1stStr));
							lp1stStr = lp1stStrNext;
						}
						pttp1st->iNum    -= 1;
						pttp1st->iHeight -= nextHeight;
						*pttp2nd = TxtMiru::TextTurnPos{begin_iIndex, begin_iPos, m_iCurLine};
						m_iCurChar += nextHeight;
					}
				}
				break;
			case TxtMiru::TextType::PICTURE_LAYOUT   : setPicture(li, 1               ); break;
			case TxtMiru::TextType::PICTURE_HALF_PAGE: setPicture(li, m_iPageLayoutNum); break;
			case TxtMiru::TextType::PICTURE_FULL_PAGE: setPicture(li, m_iLayoutMaxNum ); break;
			case TxtMiru::TextType::NEXT_LAYOUT      : nextLayout    (); li.iEndCol = m_iCurLine; break; /* 改段     *//*                    *//* +------+------+ */
			case TxtMiru::TextType::NEXT_PAPER       : nextPaper     (); li.iEndCol = m_iCurLine; break; /* 改丁     *//* 必ず奇数ページから *//* |奇数**|  偶数| */
			case TxtMiru::TextType::NEXT_PAPER_FIRST : nextPaperFirst(); li.iEndCol = m_iCurLine; break; /* 改見開き *//* 必ず偶数ページから *//* |奇数  |**偶数| */
			case TxtMiru::TextType::NEXT_PAGE        : nextPage      (); li.iEndCol = m_iCurLine; break; /* 改頁     */                        /* +------+------+ */
			case TxtMiru::TextType::COMMENT_BEGIN    : m_bInComment = true ; break; // コメント開始
			case TxtMiru::TextType::COMMENT_END      : m_bInComment = false; break; // コメント終了
			case TxtMiru::TextType::RINDENT         : // 行末からインデント (該当行以降)
				setBottomIndent(ti);
				// ここまでの文字数を ti.tpEnd.iPos に入れる
				ti.tpEnd.iPos = m_iCurChar - m_iLineTop;
				m_bBottomIndent = true;
				break;
			case TxtMiru::TextType::RINDENT_START   : // 行末からインデント 範囲：開始
				setBottomIndent(ti);
				m_iNewBottomIndent = m_iBottomIndent;
				m_bNewBottomIndent = true;
				m_bSkipNextLine = true;
				bSkipNextLine = true;
				break;
			case TxtMiru::TextType::RINDENT_END     : // 行末からインデント 範囲：終了
				m_bBottomIndent = m_bNewBottomIndent = false;
				m_iNewBottomIndent = m_iLineBottom = 0;
				m_bSkipNextLine = true;
				if(m_iLimitChar > 0){
					setLineBottom();
				}
				break;
			case TxtMiru::TextType::INDENT          : // 行頭からインデント (該当行以降)
				setTopInent(ti);
				m_iCurChar += m_iTopIndent1st;
				break;
			case TxtMiru::TextType::INDENT_START    : // 行頭からインデント 範囲：開始
				setTopInent(ti);
				m_iNewTopIndent1st = m_iTopIndent1st;
				m_iNewTopIndent2nd = m_iTopIndent2nd;
				m_bSkipNextLine = true;
				break;
			case TxtMiru::TextType::INDENT_END      : // 行頭からインデント 範囲：終了
				m_iTopIndent1st = m_iTopIndent2nd = m_iNewTopIndent1st = m_iNewTopIndent2nd = 0;
				m_bSkipNextLine = true;
				break;
			case TxtMiru::TextType::LIMIT_CHAR_START: // 字詰め
				m_iLimitChar = ti.tpBegin.iIndex * text_l_2byte_size;
				setLineBottom();
				m_bSkipNextLine = true;
				break;
			case TxtMiru::TextType::KU1_CHAR        :
			case TxtMiru::TextType::KU2_CHAR        :
				forceSkipLine(bSkipNextLine);
				m_iCurChar += m_iCurTextL2byteSize*2;
				++m_iLineCharNum;
				break;
			}
		} // for(; tl_len>0; --tl_len, ++it, ++m_iTILIndex) // text_list の ループ
		// レイアウト番号を上げただけの時は、次の行に移動しない
		if(m_bNextLayout){
			while(m_iCurTextLayoutLines != 0){
				prevLine();
			}
			m_bNextLayout = false;
		}
		{
			if (m_bBottomIndent && tlpTurnList.size() > 0) { // 字上の時、前の行に入りそうなら入れる
				const auto &ttp = tlpTurnList.back();
				if (ttp.iIndex > 0 && text_list[ttp.iIndex-1].textType == TxtMiru::TextType::RINDENT) {
					// 途中で折り返し
					auto iHeight = text_list[ttp.iIndex - 1].tpEnd.iPos;
					auto iLeftSpace = m_iLineBottom - (m_iLineTop + iHeight);
					if (iLeftSpace > m_iCurChar) {
						prevLine();
					}
				}
				else if (ttp.iIndex == 0 && m_iLLIndex > 1) {
					// 範囲字上、開始
					const auto &pre1_ll = m_doc.GetLineList()[m_iLLIndex - 1];
					const auto &pre2_ll = m_doc.GetLineList()[m_iLLIndex - 2];
					const auto &pre1_tl = pre1_ll.text_list;
					const auto &pre2_tl = pre2_ll.text_list;
					const auto &pre2_tlptl = pre2_ll.tlpTurnList;
					if (pre2_ll.iRIndent <= 0 && // 前の行が字上なら行を戻さない
						(pre2_tl.size() == 0 || (pre2_tl.size() > 0 && isTextBlockArea(pre2_tl[0].textType))) &&
						pre1_tl.size() > 0 && pre2_tlptl.size() > 0 && pre1_tl[0].textType == TxtMiru::TextType::RINDENT_START) {
						const auto &pre2_ttp = pre2_tlptl.back();
						auto iHeight = pre2_ttp.iHeight;
						auto iLeftSpace = m_iLineBottom - (pre2_ll.iSecondIndent + iHeight);
						if (iLeftSpace > m_iCurChar) {
							int iPoint = 0;
							m_param.GetPoints(CGrTxtFuncIParam::PointsType::AozoraSetting, &iPoint, 1);
							if (iPoint == 0/* ブロック指定は、前の行に入れない */) {
								;
							}
							else if (iPoint == 1/* 青空文庫*/){
								// 青空文庫
								// 地付きは、前の行に戻すが、字上は戻さない
								if (pre1_tl[0].tpBegin.iIndex == 0) {
									prevLine();
								}
							}
							else {
								// 常に前の行に入れる
								prevLine();
							}
						}
					}
				}
			}
		}
		removeLastEmptyLine(tlpTurnList, text_list);
		m_iTopIndent1st = m_iNewTopIndent1st;
		m_iTopIndent2nd = m_iNewTopIndent2nd;
		m_iBottomIndent = m_iNewBottomIndent;
		m_bBottomIndent = m_bNewBottomIndent;
		if(li.iEndCol <= 0){
			if(!bOldInComment && !m_bInComment){
				// [!bOldInComment] : 今回の処理でコメントが終了した場合、改行はしない
				if (text_list.size() == 1 && TxtMiruType::isSubtitle(text_list[0].textType)){
					// SUBTITLExのみ場合は、改行しない
				}
				else {
					nextLine();
				}
			}
			li.iEndCol = m_iCurLine;
		}
		auto &&ttp = tlpTurnList.back();
		ttp.iLineNoInAll = m_iCurLine-1;
		ttp.iHeight = m_iCurChar - m_iLineTop; // 一件は必ずある
		// 行数0判定?
		if(ttp.iNum == 0){
			ttp.iNum = -m_iLineCharNum;
		}
		if(bHangingLine){
			ttp.iNum = -m_iLineCharNum;
			ttp.iHeight -= m_iCurTextL2byteSize;
		}
		if(ttp.iHeight > m_iLineBottom){
			arrangeLine(ttp, TxtMiru::TextPoint{m_iLLIndex, ttp.iIndex, ttp.iPos}, TxtMiru::TextPoint{m_iLLIndex, m_iTILIndex + 1, 0});
		}
		//
		li.iStartCol = tlpTurnList.front().iLineNoInAll;
	}
	bool isTextBlockArea(TxtMiru::TextType tt)
	{
		return !(
			tt == TxtMiru::TextType::INDENT ||
			tt == TxtMiru::TextType::INDENT2 ||
			tt == TxtMiru::TextType::INDENT3 ||
			tt == TxtMiru::TextType::INDENT_START ||
			tt == TxtMiru::TextType::INDENT_START2 ||
			tt == TxtMiru::TextType::INDENT_END ||
			tt == TxtMiru::TextType::RINDENT ||
			tt == TxtMiru::TextType::RINDENT_START ||
			tt == TxtMiru::TextType::RINDENT_END ||
			tt == TxtMiru::TextType::LIMIT_CHAR_START ||
			tt == TxtMiru::TextType::LIMIT_CHAR_END ||
			tt == TxtMiru::TextType::PICTURE_LAYOUT ||
			tt == TxtMiru::TextType::PICTURE_HALF_PAGE ||
			tt == TxtMiru::TextType::PICTURE_FULL_PAGE ||
			tt == TxtMiru::TextType::LINE_BOX_START ||
			tt == TxtMiru::TextType::LINE_BOX_END ||
			tt == TxtMiru::TextType::COMMENT_BEGIN ||
			tt == TxtMiru::TextType::COMMENT ||
			tt == TxtMiru::TextType::COMMENT_END ||
			tt == TxtMiru::TextType::NEXT_LAYOUT ||
			tt == TxtMiru::TextType::NEXT_PAGE ||
			tt == TxtMiru::TextType::NEXT_PAPER ||
			tt == TxtMiru::TextType::NEXT_PAPER_FIRST ||
			tt == TxtMiru::TextType::TITLE ||
			tt == TxtMiru::TextType::AUTHOR ||
			TxtMiruType::isSubtitle(tt) ||
			tt == TxtMiru::TextType::NOMBRE1 ||
			tt == TxtMiru::TextType::NOMBRE2 ||
			tt == TxtMiru::TextType::RUNNINGHEADS ||
			tt == TxtMiru::TextType::ENDOFCONTENTS ||
			tt == TxtMiru::TextType::TEXT_SIZE ||
			tt == TxtMiru::TextType::TEXT_SIZE_L ||
			tt == TxtMiru::TextType::TEXT_SIZE_S ||
			tt == TxtMiru::TextType::CENTER ||
			tt == TxtMiru::TextType::CAPTION ||
			tt == TxtMiru::TextType::AUTHOR ||
			tt == TxtMiru::TextType::HORZ ||
			tt == TxtMiru::TextType::HORZ_START ||
			tt == TxtMiru::TextType::HORZ_END
			);
	}
	// 折り返し後、TxtMiru::TextType::SKIP_CHARのみの行は tlpTurnList から抹消
	void removeLastEmptyLine(const std::vector<TxtMiru::TextTurnPos> &tlpTurnList, const TxtMiru::TextInfoList &text_list)
	{
		if(tlpTurnList.size() < 2){
			return;
		}
		int lastindex = tlpTurnList.back().iIndex;
		int it_tl_len = text_list.size();
		if(it_tl_len > lastindex){
			const auto *it_tl = &text_list[lastindex];
			it_tl_len -= lastindex;
			for(; it_tl_len>0; --it_tl_len, ++it_tl){
				if(it_tl->textType != TxtMiru::TextType::SKIP_CHAR && it_tl->textType != TxtMiru::TextType::SKIP_CHAR_AUTO){ return; /* 文字あり */ }
			}
		}
		prevLine();
	}
	// 改丁は、常に奇数ページに来るように設定する
	void nextPaper()
	{
		do {
			if((((m_iLayout-1) % m_iLayoutMaxNum) == m_iPageLayoutNum) && m_iCurTextLayoutLines == 0){
				// 改丁の先頭に配置された場合には 処理しない
				break;
			}
			nextLayoutForce();
			m_bNextLayout = true;
			bool bAllocated = false;
			int idx=m_iLayout;
			for(int len=m_iLayoutMaxNum; len>0; --len, ++idx){
				bAllocated = isAllocatedLayout(idx);
				if(bAllocated){ break; }
			}
			if(bAllocated){ continue; }
		} while(((m_iLayout-1) % m_iLayoutMaxNum) != m_iPageLayoutNum);
	}
	// 改見開き、常に偶数ページに来るように設定する
	void nextPaperFirst()
	{
		do {
			if(m_iCurTxtLayout == 0 && m_iCurTextLayoutLines == 0){
				// 見開きの先頭に配置された場合には 処理しない
				break;
			}
			nextLayoutForce();
			m_bNextLayout = true;
			bool bAllocated = false;
			int idx=m_iLayout;
			for(int len=m_iLayoutMaxNum; len>0; --len, ++idx){
				bAllocated = isAllocatedLayout(idx);
				if(bAllocated){ break; }
			}
			if(bAllocated){ continue; }
		} while(((m_iLayout-1) % m_iLayoutMaxNum) != 0);
	}
	void nextPage()
	{
		do {
			if((m_iLayout-1) % m_iPageLayoutNum == 0 && m_iCurTextLayoutLines == 0){
				// ページの先頭に配置された場合には 処理しない
				break;
			}
			nextLayoutForce();
			m_bNextLayout = true;
			bool bAllocated = false;
			int idx=m_iLayout;
			for(int len=m_iPageLayoutNum; len>0; --len, ++idx){
				bAllocated = isAllocatedLayout(idx);
				if(bAllocated){ break; }
			}
			if(bAllocated){ continue; }
		} while((m_iLayout-1) % m_iPageLayoutNum != 0);
	}
	// 既に画像がレイアウトに割り当てられているか
	bool isAllocatedLayout(int iLayout){ return m_imageList.end() != m_imageList.find(m_iLayout); }
	bool nextLayoutSub()
	{
		if(m_iCurTextLayoutMaxLines >= 0){
			++m_iCurTxtLayout;
		}
		int line = 0;
		if(m_iCurTextLayoutLines == 0){
			m_iCurLine += m_iCurTextLayoutMaxLines;
		} else {
			if(m_iCurTextLayoutMaxLines < 0){
			} else if(m_iCurTextLayoutLines >= m_iCurTextLayoutMaxLines){
				line = max(m_iCurTextLayoutLines - m_iCurTextLayoutMaxLines, 1);
				m_iCurLine += line;
			} else {
				m_iCurLine += max(m_iCurTextLayoutMaxLines - m_iCurTextLayoutLines, 1);
			}
		}
		if(m_iLayoutMaxNum <= m_iCurTxtLayout){
			m_iCurTxtLayout = 0;
		}
		m_iCurTextLayoutMaxLines = getCurrentTxtLayout().lines;
		m_iCurTextLayoutLines = line;
		++m_iLayout;
		bool bSkip = false;
		while(isAllocatedLayout(m_iLayout)){
			nextLayoutSub();
			bSkip = true;
		}
		return bSkip;
	}
	void nextLayoutForce()
	{
		if(nextLayoutSub() && m_pCurLi){
			if(m_iLineCharNum == 0){
				m_pCurLi->iStartCol = m_iCurLine;
			} else {
				nextLine();
				if (m_pCurLi->tlpTurnList.size() > 0) {
					auto&& ttp = m_pCurLi->tlpTurnList.back();
					ttp.iLineNoInAll = m_iCurLine - 1;
				}
			}
		}
	}
	void nextLayout()
	{
		if(m_iCurTextLayoutLines == 0){
			// レイアウトの先頭に配置された場合には 処理しない
			return;
		}
		nextLayoutForce();
	}
	void nextLine()
	{
		// ブロック指定とかあった場合、改行を抑制
		if(m_bSkipNextLine){
			m_bSkipNextLine = false;
			return;
		}
		if(m_bInComment){ return; }
		if(m_iCurTextLayoutLines >= m_iCurTextLayoutMaxLines){
			nextLayoutForce();
		} else {
			++m_iCurLine;
			++m_iCurTextLayoutLines;
		}
	}
	void prevLine()
	{
		--m_iCurLine;
		--m_iCurTextLayoutLines;
		if(m_iCurTextLayoutLines < 0){
			--m_iCurTxtLayout;
			if(m_iCurTxtLayout < 0){
				m_iCurTxtLayout = m_iLayoutMaxNum - 1;
			}
			m_iCurTextLayoutMaxLines = getCurrentTxtLayout().lines;
			m_iCurTextLayoutLines = m_iCurTextLayoutMaxLines;
		}
	}
	void checkArrangeLine(TxtMiru::TextTurnPos &ttp, TxtMiru::TextPoint fromTp, TxtMiru::TextPoint toTp)
	{
		struct CGrArrange : public CGrTxtBuffer::CGrCharFunc {
			const CGrTxtParam &param;
			const TxtMiru::TextOffsetMap &textOffsetMap;
			TxtMiru::TextTurnPos &pre_ttp;
			CGrArrange(const CGrTxtParam &p, const TxtMiru::TextOffsetMap &t, TxtMiru::TextTurnPos &pre)
			: param(p), textOffsetMap(t), pre_ttp(pre){}
			virtual bool IsValid(TxtMiru::TextType tt){
				return tt == TxtMiru::TextType::TEXT;
			}
			virtual bool SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd){
				auto offsetType = param.GetCharOffsetType(std::tstring(lpSrc, lpEnd));
				if(offsetType && textOffsetMap.find(cur) == textOffsetMap.end()){
					pre_ttp.iHeight -= l_max_offsetnum-l_TextOffsetTable.byteArrayH[offsetType];
				}
				return true;
			};
		};
		m_doc.GetTxtBuffer().ForEach(fromTp, toTp, CGrArrange(m_param, m_textOffsetMap, ttp));
	}
	// １行の高さが、m_iLineBottomより大きい場合 文字幅調整可能な文字がないかチェックして
	// あればそれの高さを調整する
	void arrangeLine(TxtMiru::TextTurnPos &ttp, TxtMiru::TextPoint fromTp, TxtMiru::TextPoint toTp, int limitHeight = 0)
	{
		struct CGrArrange : public CGrTxtBuffer::CGrCharFunc {
			const CGrTxtParam &param;
			TxtMiru::TextOffsetMap &textOffsetMap;
			TxtMiru::TextTurnPos &pre_ttp;
			CGrArrange(const CGrTxtParam &p, TxtMiru::TextOffsetMap &t, TxtMiru::TextTurnPos &pre)
			: param(p), textOffsetMap(t), pre_ttp(pre){}
			virtual bool IsValid(TxtMiru::TextType tt){
				return tt == TxtMiru::TextType::TEXT;
			}
			virtual bool SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd){
				auto offsetType = param.GetCharOffsetType(std::tstring(lpSrc, lpEnd));
				if(offsetType && textOffsetMap.find(cur) == textOffsetMap.end()){
					textOffsetMap[cur] = TxtMiru::TextOffset{offsetType};
					pre_ttp.iHeight -= l_max_offsetnum-l_TextOffsetTable.byteArrayH[offsetType];
				}
				return true;
			};
		};
		++fromTp.iPos; // 先頭行は対象外
		m_doc.GetTxtBuffer().ForEach(fromTp, toTp, CGrArrange(m_param, m_textOffsetMap, ttp));
	}
	//
	void setTurnupNextLine(std::vector<TxtMiru::TextTurnPos> &tlpTurnList, int iHeight, const TxtMiru::TextTurnPos &ttp, int nextHeight)
	{
		nextLine();
		if (tlpTurnList.empty()) {
			return;
		}
		auto &&pre_ttp = tlpTurnList.back();
		TxtMiru::TextPoint pretp{m_iLLIndex, pre_ttp.iIndex, pre_ttp.iPos};
		TxtMiru::TextPoint curtp{m_iLLIndex, ttp    .iIndex, ttp    .iPos};
		// 行頭文字には間調整を施さない
		auto it=m_textOffsetMap.find(curtp);
		if(it != m_textOffsetMap.end()){
			const auto of = (it->second);
			m_textOffsetMap.erase(it);
			// ここで文字間調整が入っている場合、m_iCurCharやnextHeightに先頭文字間後の値が入っているので
			// 高さを元に戻す
			int len = l_max_offsetnum-l_TextOffsetTable.byteArrayH[of.offsetType];
			iHeight += len;
			nextHeight += len;
		}
		pre_ttp.iNum    = m_iLineCharNum;
		pre_ttp.iHeight = iHeight;
		if(m_iLimitChar > 0){
			pre_ttp.iHeight = getCurrentTxtLayout().characters * text_l_2byte_size + text_l_1byte_size;
		}
		// １行の高さが、m_iLineBottomより大きい場合 文字幅調整可能な文字がないかチェックして
		// あればそれの高さを調整する
		if(iHeight >= m_iLineBottom){
			arrangeLine(pre_ttp, pretp, curtp);
		}
		// 折り返し時のフォントサイズを設定
		if(m_iCharSize != 0){
			auto &&textStyleMap = m_doc.GetTextStyleMap();
			auto tmpCurtp = curtp;
			for(int i=0; i<=curtp.iPos; ++i){
				tmpCurtp.iPos = i;
				auto it = textStyleMap.find(tmpCurtp);
				if(textStyleMap.end() == it){
					textStyleMap[tmpCurtp].uStatus = TxtMiru::TTPStatusNone;
					textStyleMap[tmpCurtp].fontSize = m_iCharSize;
				} else {
					(it->second).fontSize = m_iCharSize;
				}
			}
		}
		// 折り返し時の横組みを設定
		if (m_bHorz) {
			auto&& textStyleMap = m_doc.GetTextStyleMap();
			auto it = textStyleMap.find(curtp);
			if (textStyleMap.end() == it) {
				textStyleMap[curtp].uStatus = TxtMiru::TTPStatusHorz;
			}
			else {
				(it->second).uStatus |= TxtMiru::TTPStatusHorz;
			}
		}
		//
		tlpTurnList.push_back(ttp);
		tlpTurnList.back().iLineNoInAll = m_iCurLine;
		if(m_bBottomIndent){
			tlpTurnList.back().iRIndent = m_iBottomIndent / text_l_2byte_size;
		}
		m_iLineTop = m_iTopIndent2nd;
		m_iCurChar = m_iCurSubChar = m_iLineTop + nextHeight;
		m_iLineCharNum = 0;
	}
	// tlpTurnList  : 改行位置リスト
	// iIndex, iPos : 改行位置
	void setTurnupNextLine(std::vector<TxtMiru::TextTurnPos> &tlpTurnList, int iIndex, int iPos, int nextHeight)
	{
		setTurnupNextLine(tlpTurnList, m_iCurChar - m_iLineTop, TxtMiru::TextTurnPos{iIndex,iPos,m_iCurLine}, nextHeight);
	}
	// 折り返し?
	bool isTurnup(){
		return (m_iLineBottom < m_iCurChar);
	}
	int maxCurChar()
	{
		return max(m_iCurChar, m_iCurSubChar);
	}
	void setTopInent(const TxtMiru::TextInfo &ti)
	{
		int max_char_num = getCurrentTxtLayout().characters * text_l_2byte_size + text_l_1byte_size;
		m_iTopIndent1st = ti.tpBegin.iIndex * text_l_2byte_size;
		m_iTopIndent2nd = ti.tpEnd  .iIndex * text_l_2byte_size;
		// 一行の高さが、字下げより少ない場合 最低 一文字は表示されるように
		if(m_iTopIndent1st >= max_char_num){
			m_iTopIndent1st = max_char_num - text_l_2byte_size;
		}
		if(m_iTopIndent2nd >= max_char_num){
			m_iTopIndent2nd = max_char_num - text_l_2byte_size;
		}
		setLineBottom();
		if(m_iLineTop != m_iCurChar){ m_bForceNewLine = true; } // 行頭からのインデントが指定されている場合は、その位置が行頭であること (既に配置されていると、上から配置できない)
	}
	void setLineBottom()
	{
		int max_char_num = getCurrentTxtLayout().characters * text_l_2byte_size + text_l_1byte_size;
		m_iLineBottom = max_char_num - m_iBottomIndent; // *****
		if(m_iLimitChar > 0){
			if(m_iLineBottom <= 0){
				m_iLineBottom = max_char_num;
			}
			m_iLineBottom = min(m_iLineBottom, m_iLimitChar + m_iTopIndent1st);
		}
		// 一行の高さが、字下げより少ない場合 最低 一文字は表示されるように
		if(m_iLineBottom < m_iTopIndent1st || m_iLineBottom < m_iTopIndent2nd){
			m_iLineBottom = max(m_iTopIndent1st, m_iTopIndent2nd) + text_l_2byte_size;
		}
	}
	void setBottomIndent(const TxtMiru::TextInfo &ti)
	{
		m_iBottomIndent = ti.tpBegin.iIndex * text_l_2byte_size;
		setLineBottom();
		// 行末からのインデントが指定されている場合は、その位置が行頭であること (既に配置されていると、下から配置できない)
		//   対応するのであれば、先読みして 文字数が足りるかチェックする必要がある  ※A
		//   ※ただし、一文字の場合 m_bForceNewLine が処理されないので 同一行で 地付きになる
		if(m_iLineTop != m_iCurChar){ m_bForceNewLine = true; }
	}
	bool isNeedNewLine(const TxtMiru::TextInfo &ti) const
	{
		return m_bNextLine;
	}
	bool isComment(const TxtMiru::TextInfo &ti) const
	{
		return (m_bInComment && ti.textType != TxtMiru::TextType::COMMENT_END);
	}
	const TxtMiru::TxtLayout &getCurrentTxtLayout()
	{
		return m_textLayout[m_iCurTxtLayout];
	}
	void initLine(TxtMiru::LineInfo &li)
	{
		++m_iLLIndex;
		m_bSkipNextLine = false;
		m_bNextLine = true;
		m_iLineTop = m_iCurChar = m_iCurSubChar = m_iTopIndent1st;
		setLineBottom();
		m_iLineCharNum = 0;
		//
		li.iStartCol     = m_iCurLine;
		li.iFirstIndent  = m_iTopIndent1st / text_l_2byte_size;
		li.iSecondIndent = m_iTopIndent2nd / text_l_2byte_size;
		li.iRIndent      = (m_bBottomIndent) ? m_iBottomIndent / text_l_2byte_size : -1;
		li.iEndCol       = -1;
		//
		m_iTILIndex = 0;
		m_iNewTopIndent1st = m_iTopIndent1st;
		m_iNewTopIndent2nd = m_iTopIndent2nd;
		m_iNewBottomIndent = m_iBottomIndent;
		m_bNewBottomIndent = m_bBottomIndent;
	}
	bool isLineStartNGCharacter  (LPCTSTR lpSrc){ return (m_param.FindIF(CGrTxtParam::ValueType::LineStartNGCharacters  , CGrMapStringCompare(lpSrc)) >= 0); }
	bool isHangingCharacter      (LPCTSTR lpSrc){ return (m_param.FindIF(CGrTxtParam::ValueType::HangingCharacters      , CGrMapStringCompare(lpSrc)) >= 0); }
	bool isLineEndNGCharacter    (LPCTSTR lpSrc){ return (m_param.FindIF(CGrTxtParam::ValueType::LineEndNGCharacters    , CGrMapStringCompare(lpSrc)) >= 0); }
	bool isLineStartSkipCharacter(LPCTSTR lpSrc){ return (m_param.FindIF(CGrTxtParam::ValueType::LineStartSkipCharacters, CGrMapStringCompare(lpSrc)) >= 0); }
	bool isSeparateNGCharacter   (LPCTSTR lpSrc){ return (m_param.FindIF(CGrTxtParam::ValueType::SeparateNGCharacters   , CGrMapStringCompare(lpSrc)) >= 0); }
	//追い込み（run-in）
	//追い出し（run-out）
	// 指定文字以降の行に文字が残っているかどうか
	int getNextCharCount(const TxtMiru::TextInfoList &text_list, int iIndex, int iPos)
	{
		int iNum = 0;
		auto *it = &text_list[iIndex];
		int tl_len = text_list.size();
		auto lpSrc = CGrText::CharNextN(it->str.c_str(), iPos+1);
		for(int i=iIndex; i<tl_len; ++i, ++it, lpSrc=it->str.c_str(), iPos=0){
			if(TxtMiruType::isTextOrSkip(it->textType)){
				for(; *lpSrc; ++lpSrc){
					if(isHangingCharacter(lpSrc) || isLineStartNGCharacter(lpSrc) || isLineStartSkipCharacter(lpSrc)){
						return iNum;
					}
					++iNum;
				}
			} else if(TxtMiruType::isRotateNum(it->textType) || it->textType == TxtMiru::TextType::UNKOWN_ERROR){
				if(isHangingCharacter(lpSrc) || isLineStartNGCharacter(lpSrc) || isLineStartSkipCharacter(lpSrc)){
					return iNum;
				}
				++iNum;
			}
		}
		return iNum;
	}
	// 次の文字を取得
	std::tuple<int,int,LPCTSTR> getNextCharInfo(const TxtMiru::TextInfoList &text_list, int iIndex, int iPos)
	{
		auto *it = &text_list[iIndex];
		int tl_len = text_list.size();
		auto lpSrc = CGrText::CharNextN(it->str.c_str(), iPos+1);
		for(int i=iIndex; i<tl_len; ++i, ++it, lpSrc=it->str.c_str(), iPos=0){
			if(TxtMiruType::isTextOrSkip(it->textType)){
				if(*lpSrc){
					return {i,iPos+1,lpSrc};
				}
			} else if(TxtMiruType::isRotateNum(it->textType) || it->textType == TxtMiru::TextType::UNKOWN_ERROR){
				if(i != iIndex){
					return {i,0,lpSrc};
				}
			}
		}
		return {-1,-1,nullptr};
	}

	inline int GetCurCharHeight(const TxtMiru::TextInfo &tt)
	{
		return TxtMiruType::isRotateNum(tt.textType) ? m_iCurTextL2byteSize : HalfCharCur(tt.chrType);
	}
	std::tuple<int,int,int,int> getRunOutPosSub(std::vector<TxtMiru::TextTurnPos> &tlpTurnList, const TxtMiru::TextInfoList &text_list, int iIndex, int iPos)
	{
		struct CharInfo {
			int iIndex;
			int iPos;
			int iHeight;
			bool bLineStartNG;
			bool bLineEndNG;
			bool bHanging;
			bool operator<(const CharInfo& right) const {
				return iIndex == right.iIndex ? iPos < right.iPos : iIndex < right.iIndex;
			}
		};
		std::vector<CharInfo> charinfo_list;
		charinfo_list.reserve(10);
		const auto *pti = &text_list[iIndex];
		bool bContinue = true;
		for (int iPrevIndex = iIndex; iPrevIndex >= 0; --pti, --iPrevIndex) {
			switch (pti->textType) {
			case TxtMiru::TextType::OTHER:
			case TxtMiru::TextType::TEXT:
			case TxtMiru::TextType::LINE_CHAR:
			case TxtMiru::TextType::SKIP_CHAR:
			case TxtMiru::TextType::SKIP_CHAR_AUTO:
				{
					auto lpStr = pti->str.c_str();
					int iHeight = HalfCharCur(pti->chrType);
					for (int iPrevPos = 0; *lpStr; lpStr = CGrText::CharNext(lpStr), ++iPrevPos) {
						charinfo_list.push_back({ iPrevIndex, iPrevPos, iHeight, isLineStartNGCharacter(lpStr), isLineEndNGCharacter(lpStr), isHangingCharacter(lpStr) });
						if (iIndex == iPrevIndex && iPrevPos >= iPos) {
							break;
						}
					}
				}
				break;
			case TxtMiru::TextType::ROTATE_NUM:
			case TxtMiru::TextType::ROTATE_NUM_AUTO:
				charinfo_list.push_back({ iPrevIndex, 0, m_iCurTextL2byteSize, isLineStartNGCharacter(pti->str.c_str()), isLineEndNGCharacter(pti->str.c_str()), isHangingCharacter(pti->str.c_str()) });
				break;
				// 予定 Ruby付き文字が分かれるので対応予定
			case TxtMiru::TextType::RUBY:
			case TxtMiru::TextType::RUBY_L:
			case TxtMiru::TextType::NOTE_L:
			case TxtMiru::TextType::UNKOWN_ERROR:
				{
					TxtMiru::TextPoint tp1st{m_iLLIndex, pti->tpBegin.iIndex, pti->tpBegin.iPos};
					TxtMiru::TextPoint tp2nd{m_iLLIndex, pti->tpEnd  .iIndex, pti->tpEnd  .iPos};
					std::tstring target_str;
					m_doc.ToString(target_str, tp1st, tp2nd);
					if(target_str.size() < (pti->str.size() / 3)){
						if(charinfo_list.size() <= 1){
							return {-2,-1,-1,-1};
						} else {
							bContinue = false;
							break;
						}
					}
				}
				break;
			}
			if (charinfo_list.size() > 5) {
				// 5文字以上は戻さないのでこれ以上は不要
				break;
			}
			if (!bContinue){
				break;
			}
		}
		std::sort(charinfo_list.begin(), charinfo_list.end());
		// 行末禁則文字の次の文字は、行頭禁則文字とする
		{
			bool bLineEndNG = false;
			for (auto &&item : charinfo_list) {
				if(bLineEndNG && !item.bLineStartNG){
					item.bLineStartNG = true;
				}
				bLineEndNG = item.bLineEndNG;
			}
		}
		std::reverse(charinfo_list.begin(), charinfo_list.end());
		// できるだけ一文字では戻さない(2文字以上でチェックして、なければ1文字で再チェック)
		if(!charinfo_list.empty()){
			for (int i = charinfo_list[0].bLineStartNG ? 2 : 1; i > 0; --i) {
				int afterAddHeight = 0;
				int iNum = 0;
				for (const auto &item : charinfo_list) {
					if(item.bHanging && iNum > 0){
						return {-1,-1,-1,-1};
					}
					++iNum;
					afterAddHeight += item.iHeight;
					if (iNum > i && !item.bLineStartNG) {
						return { iNum, item.iIndex, item.iPos, afterAddHeight };
					}
					if (iNum > 5) {
						// 5文字以上は戻さない
						break;
					}
				}
			}
		}
		return {-1,-1,-1,-1};
	}
	std::tuple<int,int,int,int> getRunOutPos(std::vector<TxtMiru::TextTurnPos> &tlpTurnList, const TxtMiru::TextInfoList &text_list, int iIndex, int iPos)
	{
		auto lpBeginSrc = text_list[iIndex].str.c_str();
		auto lpSrc = CGrText::CharNextN(lpBeginSrc, iPos);
		// 改行した文字（カレント位置）が行頭禁則文字
		PosInfo prePi;
		getPrevPos(prePi, text_list, iIndex, iPos, lpBeginSrc, lpSrc);
		auto lpPrevSrc = prePi.lpSrc;
		if(lpPrevSrc && isLineStartNGCharacters(m_param, lpPrevSrc)){
			auto [iNum, iPrevIndex, iPrevPos, afterAddHeight] = getRunOutPosSub(tlpTurnList, text_list, prePi.iIndex, prePi.iPos);
			if(iNum > 0){
				return {iNum+1, iPrevIndex, iPrevPos, afterAddHeight+ GetCurCharHeight(text_list[iIndex])};
			}
		}
		auto [iNum, iPrevIndex, iPrevPos, afterAddHeight] = getRunOutPosSub(tlpTurnList, text_list, iIndex, iPos);
		if(iNum > 0){
			return {iNum, iPrevIndex, iPrevPos, afterAddHeight};
		}
		return {iNum,-1,-1,-1};
	}
};

// 改行位置判定
void CGrTxtMapper::CreateIndex(CGrTxtDocument &doc)
{
	CGrIndexCreator ic(doc, doc.GetConstLayout().GetConstLayoutList(CGrTxtLayout::LayoutListType::Text));
	ic.Create();
}
