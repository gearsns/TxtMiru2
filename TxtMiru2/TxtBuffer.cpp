#include <windows.h>
#include "TxtBuffer.h"
#include "Text.h"
#include "TxtMiruTextType.h"

//#define __DBG__
#include "Debug.h"

const bool CGrTxtBuffer::IsBold(const TxtMiru::TextPoint &tp) const
{
	auto it=m_textStyleMap.find(tp);
	if(it != m_textStyleMap.end()){
		return ((it->second.uStatus & TxtMiru::TTPStatusBold) == TxtMiru::TTPStatusBold);
	}
	return false;
}

void CGrTxtBuffer::ForEachAll(CGrTypeFunc &&func) const
{
	const auto &line_list = GetConstLineList();
	int len = line_list.size();
	if(len <= 0){ return; }
	TxtMiru::TextPoint cur_point;
	const auto *it_ll=&line_list[0];
	for(; len>0; --len, ++it_ll, ++cur_point.iLine, cur_point.iIndex=0){
		const auto &text_list=it_ll->text_list;
		auto tl_len = text_list.size();
		if(tl_len <= 0){
			if(func.IsValid(TxtMiru::TextType::TEXT) && !func.SetType(cur_point, TxtMiru::TextInfo())){
				return;
			}
		} else {
			const auto *it_tl = &text_list[0];
			for(; tl_len>0; --tl_len, ++it_tl, ++cur_point.iIndex){
				const auto &text_info = *it_tl;
				if(func.IsValid(text_info.textType) && !func.SetType(cur_point, text_info)){
					return;
				}
			}
		}
	}
}

void CGrTxtBuffer::ForEachAll(CGrCharFunc &&func) const
{
	const auto &line_list = GetConstLineList();
	int len = line_list.size();
	if(len <= 0){ return; }
	TxtMiru::TextPoint cur_point;
	const auto *it_ll=&line_list[0];
	for(; len>0; --len, ++it_ll, ++cur_point.iLine, cur_point.iIndex=0, cur_point.iPos=0){
		const auto &text_list=it_ll->text_list;
		int tl_len = text_list.size();
		if(tl_len <= 0){ continue; }
		const auto *it_tl = &text_list[0];
		for(; tl_len>0; --tl_len, ++it_tl, ++cur_point.iIndex, cur_point.iPos=0){
			const auto &text_info = *it_tl;
			if(func.IsValid(text_info.textType)){
				auto lpNextSrc = text_info.str.c_str();
				while(*lpNextSrc){
					auto lpSrc = lpNextSrc;
					lpNextSrc = CGrText::CharNext(lpNextSrc);
					if(!func.SetChar(cur_point, lpSrc, lpNextSrc)){
						return;
					}
					++cur_point.iPos;
				}
			}
		}
	}
}

void CGrTxtBuffer::ForEach(const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, CGrCharFunc &&func) const
{
	const auto &line_list = GetConstLineList();

	auto it_ll=line_list.begin(), ite_ll=line_list.end();
	auto cur_point = tpb;
	if(cur_point.iLine < 0 || cur_point.iLine >= static_cast<int>(line_list.size())){
		cur_point.iLine = line_list.size() - 1;
		if(cur_point.iLine < 0){
			cur_point.iLine = 0;
		}
	}
	std::advance(it_ll, cur_point.iLine);
	for(; it_ll != ite_ll && cur_point <= tpe; ++it_ll, ++cur_point.iLine, cur_point.iIndex=0, cur_point.iPos=0){
		const auto &text_list=it_ll->text_list;
		auto it_tl=text_list.begin(), ite_tl=text_list.end();
		if(cur_point.iIndex < 0 || cur_point.iIndex >= static_cast<int>(text_list.size())){
			cur_point.iIndex = text_list.size() - 1;
			if(cur_point.iIndex < 0){
				cur_point.iIndex = 0;
			}
		}
		std::advance(it_tl, cur_point.iIndex);
		for(;it_tl != ite_tl && cur_point <= tpe; ++cur_point.iIndex, cur_point.iPos=0){
			const auto &text_info = *it_tl;
			++it_tl;
			if(func.IsValid(text_info.textType)){
				auto lpNextSrc = CGrText::CharNextN(text_info.str.c_str(), cur_point.iPos);
				while(*lpNextSrc && cur_point <= tpe){
					auto lpSrc = lpNextSrc;
					lpNextSrc = CGrText::CharNext(lpNextSrc);
					if(!func.SetChar(cur_point, lpSrc, lpNextSrc)){
						return;
					}
					++cur_point.iPos;
				}
			}
		}
	}
}

void CGrTxtBuffer::ForEach(const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, CGrTypeFunc &&func) const
{
	const auto &line_list = GetConstLineList();

	auto it_ll=line_list.begin(), ite_ll=line_list.end();
	auto cur_point = tpb;
	if(cur_point.iLine < 0 || cur_point.iLine >= static_cast<int>(line_list.size())){
		cur_point.iLine = line_list.size() - 1;
		if(cur_point.iLine < 0){
			cur_point.iLine = 0;
		}
	}
	std::advance(it_ll, cur_point.iLine);
	for(; it_ll != ite_ll && cur_point <= tpe; ++it_ll, ++cur_point.iLine, cur_point.iIndex=0, cur_point.iPos=0){
		const auto &text_list=it_ll->text_list;
		auto it_tl=text_list.begin(), ite_tl=text_list.end();
		if(cur_point.iIndex < 0 || cur_point.iIndex >= static_cast<int>(text_list.size())){
			cur_point.iIndex = text_list.size() - 1;
			if(cur_point.iIndex < 0){
				cur_point.iIndex = 0;
			}
		}
		std::advance(it_tl, cur_point.iIndex);
		for(;it_tl != ite_tl && cur_point <= tpe; ++cur_point.iIndex, cur_point.iPos=0){
			const auto &text_info = *it_tl;
			++it_tl;
			if(func.IsValid(text_info.textType)){
				if(!func.SetType(cur_point, text_info)){
					return;
				}
			}
		}
	}
}

LPCTSTR CGrTxtBuffer::GetChar(const TxtMiru::TextPoint &pos) const
{
	auto it_ll=m_lineList.begin();
	std::advance(it_ll, pos.iLine);
	//
	const auto &text_list = it_ll->text_list;
	auto it_tl=text_list.begin();
	std::advance(it_tl, pos.iIndex);
	//
	return CGrText::CharNextN(it_tl->str.c_str(), pos.iPos);
}

struct RubyMap : public CGrTxtBuffer::CGrTypeFunc {
	struct TextRange {
		TxtMiru::TextPoint tpLast;
		TxtMiru::TextPoint tpBegin;
		TxtMiru::TextPoint tpEnd;
		bool isRange(const TxtMiru::TextPoint &cur) const{
			return (tpBegin <= cur && cur <= tpEnd);
		}
	};
	std::vector<TextRange> ruby_map;
	RubyMap(){}
	virtual bool IsValid(TxtMiru::TextType tt){
		return tt == TxtMiru::TextType::RUBY;
	}
	virtual bool SetType(const TxtMiru::TextPoint &cur, const TxtMiru::TextInfo &text_info){
		ruby_map.push_back({
			cur,
			TxtMiru::TextPoint{cur.iLine, text_info.tpBegin.iIndex, text_info.tpBegin.iPos},
			TxtMiru::TextPoint{cur.iLine, text_info.tpEnd  .iIndex, text_info.tpEnd  .iPos}
		});
		return true;
	};
	TxtMiru::TextPoint getRubyTextPoint(const TxtMiru::TextPoint &cur) const{
		for(const auto &item : ruby_map){
			if(item.isRange(cur)){
				return item.tpLast;
			}
		}
		return TxtMiru::TextPoint{-1,-1,-1};
	}
};
struct TextRuby : public CGrTxtBuffer::CGrCharFunc {
	std::tstring &str;
	const CGrTxtBuffer &buffer;
	int maxLength;
	TxtMiru::TextPoint curPos;
	TxtMiru::TextPoint maxPos;
	const RubyMap &ruby_map;
	TextRuby(std::tstring &s, const CGrTxtBuffer &b, const TxtMiru::TextPoint &mp, int l, const RubyMap &m) : str(s), buffer(b), maxLength(l), maxPos(mp), ruby_map(m){
		curPos.iLine = -1;
	}
	~TextRuby(){
	}
	virtual bool IsValid(TxtMiru::TextType tt){
		return (/**/tt == TxtMiru::TextType::TEXT
				||  tt == TxtMiru::TextType::LINE_CHAR
				||  tt == TxtMiru::TextType::SKIP_CHAR_AUTO
				||  tt == TxtMiru::TextType::ROTATE_NUM
				||  tt == TxtMiru::TextType::ROTATE_NUM_AUTO
				||  tt == TxtMiru::TextType::RUBY
				);
	}
	virtual bool SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd){
		if(curPos.iLine >= 0 && curPos.iLine != cur.iLine){
			// 改行
			for(int len=cur.iLine-curPos.iLine; len>0; --len){
				str.append(_T("\r\n"));
			}
		}
		if(maxPos < cur){
			return false;
		}
		auto rtp = ruby_map.getRubyTextPoint(cur);
		if(rtp.iLine < 0){
			str += std::tstring(lpSrc, lpEnd);
		} else if(maxPos < rtp){
			maxPos.iLine  = rtp.iLine;
			maxPos.iIndex = rtp.iIndex + 1;
			maxPos.iPos   = -1;
		}
		if(maxLength > 0 && maxLength < CGrText::CharLen(str.c_str())){
			return false;
		}
		curPos = cur;
		return true;
	};
};

struct TextOnly : public CGrTxtBuffer::CGrCharFunc {
	std::tstring &str;
	TxtMiru::TextPoint begin_pos;
	LPCTSTR lpSrcBegin = nullptr;
	LPCTSTR lpSrcEnd = nullptr;
	const CGrTxtBuffer &buffer;
	int maxLength;
	TextOnly(std::tstring &s, const CGrTxtBuffer &b, int l) : str(s), begin_pos{-1,-1,-1}, lpSrcBegin(nullptr), buffer(b), maxLength(l){}
	~TextOnly(){
		if(lpSrcBegin){
			str += std::tstring(lpSrcBegin, lpSrcEnd+1);
		}
	}
	virtual bool IsValid(TxtMiru::TextType tt){
		return TxtMiruType::isTextOrSkipOrRotateNum(tt);
	}
	virtual bool SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd){
		if(!lpSrcBegin){
			lpSrcBegin = lpSrc;
			begin_pos = cur;
		} else if(cur.iLine != begin_pos.iLine || cur.iIndex != begin_pos.iIndex){
			str += std::tstring(lpSrcBegin, lpSrcEnd+1);
			if(cur.iLine != begin_pos.iLine){
				for(int len=cur.iLine-begin_pos.iLine; len>0; --len){
					str.append(_T("\r\n"));
				}
			}
			if(maxLength > 0 && maxLength < CGrText::CharLen(str.c_str())){
				lpSrcBegin = nullptr;
				return false;
			}
			lpSrcBegin = lpSrc;
			begin_pos = cur;
		}
		lpSrcEnd = lpSrc;
		return true;
	};
};

void CGrTxtBuffer::ToString(std::tstring &out_str, const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, int max_length) const
{
	out_str.clear();
	ForEach(tpb, tpe, static_cast<CGrTxtBuffer::CGrCharFunc&&>(TextOnly(out_str, *this, max_length)));
}

void CGrTxtBuffer::ToStringRuby(std::tstring &out_str, const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, int max_length) const
{
	const auto &ll = GetConstLineList();
	TxtMiru::TextPoint tpe2{tpe.iLine, static_cast<signed int>(ll.size()),0};
	out_str.clear();
	RubyMap m;
	ForEach(tpb, tpe2, static_cast<CGrTxtBuffer::CGrTypeFunc&&>(m));
	ForEach(tpb, tpe2, static_cast<CGrTxtBuffer::CGrCharFunc&&>(TextRuby(out_str, *this, tpe, max_length, m)));
}

TxtMiru::LineList::const_iterator CGrTxtBuffer::FindConst(int start_line) const
{
	auto it_li=m_lineList.begin(), it_lie=m_lineList.end();
	for(;it_li != it_lie; ++it_li){
		if(it_li->iStartCol <= start_line && start_line < it_li->iEndCol){
			break;
		}
	}
	return it_li;
}

bool CGrTxtBuffer::ToTextPoint(TxtMiru::TextPoint &tp, int start_line) const
{
	if(m_lineList.empty()){
		return false;
	}
	auto it_li = FindConst(start_line);
	if(it_li == m_lineList.end()){
		return false;
	}
	//
	const auto &li = (*it_li);
	int line = start_line-li.iStartCol;
	if(line < 0){
		return false;
	} else if(static_cast<int>(li.tlpTurnList.size()) <= line){
		line = li.tlpTurnList.size() - 1; // 改ページ、画像などがあるので
		if(line < 0){
			return false;
		}
	}
	auto it_ttp=li.tlpTurnList.begin();
	std::advance(it_ttp, line);
	tp.iLine  = std::distance(m_lineList.begin(), it_li);
	tp.iIndex = it_ttp->iIndex;
	tp.iPos   = it_ttp->iPos  ;
	return true;
}

void CGrTxtBuffer::Clear()
{
	m_lineList.clear();
	m_textStyleMap.clear();
	m_textOffsetMap.clear();
	m_pictPosMap.clear();
	m_title.clear();
	m_author.clear();
	m_lastWriteTime.clear();

	m_lineList.shrink_to_fit();
}

void CGrTxtBuffer::AddLineInfo(const TxtMiru::LineInfo &li)
{
	m_lineList.push_back(li);
}

void CGrTxtBuffer::MoveBackLineInfo(TxtMiru::LineInfo &&li)
{
	m_lineList.push_back(std::forward<decltype(li)>(li));
}
