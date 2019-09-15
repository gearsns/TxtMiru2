#pragma warning( disable : 4786 )

#include "TxtDocument.h"
#include "TxtSearch.h"
#include "WaitCursor.h"
#include <regex>
#include "TxtMiruTextType.h"

CGrTxtSearch::CGrTxtSearch()
{
}

CGrTxtSearch::~CGrTxtSearch()
{
}

void CGrTxtSearch::Set(const TxtMiru::TextPoint &pos, LPCTSTR lpSrc, bool bLoop, bool bUseRegExp, bool bDown)
{
	m_pos = pos;
	m_str = lpSrc;
	m_bLoop = bLoop;
	m_bUseRegExp = bUseRegExp;
	m_bDown = bDown;

	m_len = CGrText::CharLen(lpSrc);
	if(m_len > 0){
		LPCTSTR lpNextSrc = CGrText::CharNext(lpSrc);
		m_searchCharType = CGrText::GetStringTypeEx(lpSrc, lpNextSrc-lpSrc);
	}
	if(bDown){
		++m_pos.iPos;
	} else {
		--m_pos.iPos;
	}
}

void CGrTxtSearch::Clear()
{
	m_pos.iLine  = 0;
	m_pos.iIndex = 0;
	m_pos.iPos   = 0;
}
static bool isSearchTargetTextType(TxtMiru::TextType tt)
{
	return TxtMiruType::isTextOrSkipOrRotateNum(tt);
}
static void ToTextPoint(TxtMiru::TextPoint &tp, const TxtMiru::TextInfoList &li, int pos)
{
	int li_len = li.size();
	for(; tp.iIndex<li_len; ++tp.iIndex){
		const auto &it = li[tp.iIndex];
		if(isSearchTargetTextType(it.textType)){
			int len = it.str.size() - tp.iPos;
			if(len > 0){
				if(len > pos){
					tp.iPos += pos;
					break;
				}
				pos -= len;
			}
		}
		tp.iPos = 0;
	}
}
bool CGrTxtSearch::upSearch(TxtMiru::TextPoint &pos)
{
	if(!m_pDoc || m_len <= 0){ return false; }
	auto lpSrc = m_str.c_str();
	auto &line_list = m_pDoc->GetConstLineList();
	if(line_list.empty()){
		return false;
	}
	auto cur_point = m_pos;
	--cur_point.iPos;
	if(cur_point.iPos < 0){
		--cur_point.iIndex;
	}
	if(cur_point.iIndex < 0){
		--cur_point.iLine;
	}
	int ll_len = line_list.size();
	if(cur_point.iLine < 0 || cur_point.iLine >= ll_len){
		cur_point.iLine = ll_len - 1;
		if(cur_point.iLine < 0){
			cur_point.iLine = 0;
		}
	} else if(ll_len <= m_pos.iLine){
		return false;
	}
	auto it_ll=line_list.begin(), itb_ll=line_list.begin(), ite_ll=line_list.end();
	if(0 < cur_point.iLine && cur_point.iLine < ll_len){
		std::advance(it_ll, cur_point.iLine);
	}
	if(m_bUseRegExp){
		try {
			auto check_point = cur_point;
			std::wregex re(lpSrc, std::regex_constants::icase);
			for(;ll_len>0; --ll_len, --it_ll, --cur_point.iLine, cur_point.iIndex=-1, cur_point.iPos=0){
				auto &&text_list=it_ll->text_list;
				int tl_len = text_list.size();
				if(tl_len <= 0){
					continue;
				}
				std::tstring str;
				m_pDoc->ToString(str, TxtMiru::TextPoint{cur_point.iLine,0,0}, TxtMiru::TextPoint{cur_point.iLine + 1, -1, 0}, -1);
				if(str.size() <= 0){
					continue;
				}
				auto lpLine = str.c_str();
				TxtMiru::TextPoint hit_pos;
				int hit_len = 0;
				for(auto lpChar = lpLine; *lpChar; ++lpChar){
					std::wcmatch m;
					if(std::regex_search(lpChar, m, re)){
						cur_point.iIndex = 0;
						cur_point.iPos = 0;
						ToTextPoint(cur_point, text_list, m[0].first-lpLine);
						if(cur_point.iPos < 0){
							break;
						}
						bool bRangeOK = false;
						if(cur_point.iLine == check_point.iLine){
							if(cur_point.iIndex == check_point.iIndex){
								bRangeOK = cur_point.iPos <= check_point.iPos || check_point.iPos < 0;
							} else if(cur_point.iIndex < check_point.iIndex || check_point.iIndex < 0){
								bRangeOK = true;
							}
						} else if(cur_point.iLine < check_point.iLine){
							bRangeOK = true;
						}
						if(bRangeOK){
							hit_pos = cur_point;
							hit_len = m[0].second - m[0].first;
						} else {
							break;
						}
					} else {
						break;
					}
				}
				if(hit_len > 0){
					pos = hit_pos;
					m_pos = hit_pos;
					m_len = hit_len;
					return true;
				}
			}
		} catch(...){
			return false;
		}
	} else {
		bool bLoopLL = true;
		while(it_ll != ite_ll && bLoopLL){
			if(it_ll == itb_ll){ bLoopLL = false; }
			auto &&text_list = it_ll->text_list;
			if(bLoopLL){
				--it_ll;
			}
			if(!text_list.empty()){
				int tl_len = text_list.size();
				if(cur_point.iIndex < 0 || cur_point.iIndex >= tl_len){
					cur_point.iIndex = text_list.size() - 1;
					if(cur_point.iIndex < 0){
						cur_point.iIndex = 0;
					}
				}
				auto it_tl=text_list.begin(), itb_tl=text_list.begin(), ite_tl=text_list.end();
				if(0 < cur_point.iIndex && cur_point.iIndex < tl_len){
					std::advance(it_tl, cur_point.iIndex);
				}
				auto next_point = cur_point;
				bool bLoopTL = true;
				while(it_tl != ite_tl && bLoopTL){
					if(it_tl == itb_tl){ bLoopTL = false; }
					auto &&text_info = *it_tl;
					if(bLoopTL){
						--it_tl;
					}
					cur_point = next_point;
					--next_point.iIndex;
					next_point.iPos = -1;
					if(!isSearchTargetTextType(text_info.textType)){
						continue;
					}
					auto lpFirstDst = text_info.str.c_str();
					if(cur_point.iPos < 0){
						cur_point.iPos = CGrText::CharLen(lpFirstDst) - 1;
						if(cur_point.iPos < 0){
							cur_point.iPos = 0;
						}
					}
					auto lpNextDst = CGrText::CharNextN(lpFirstDst, cur_point.iPos);
					while(lpFirstDst != lpNextDst && lpNextDst && *lpNextDst && *lpSrc != *lpNextDst){
						lpNextDst = CGrText::CharPrev(lpFirstDst, lpNextDst);
						--cur_point.iPos;
					}
					if(!lpNextDst || !(*lpNextDst)){ continue; }
					int len = m_len + CGrText::CharLen(lpFirstDst);
					std::tstring str;
					m_pDoc->ToString(str, TxtMiru::TextPoint{cur_point.iLine, cur_point.iIndex, 0}, TxtMiru::TextPoint{cur_point.iLine + 1, -1, 0}, len);
					auto lpLineStr = CGrText::CharNextN(str.c_str(), cur_point.iPos);
					do {
						if(CGrText::isMatchChar(lpLineStr, lpSrc)){
							pos = cur_point;
							m_pos = pos;
							return true;
						}
						lpNextDst = CGrText::CharPrev(lpFirstDst, lpNextDst);
						lpLineStr = CGrText::CharPrev(str.c_str(), lpLineStr);
					} while(lpFirstDst != lpNextDst && lpNextDst && *lpNextDst);
				}
			}
			--cur_point.iLine;
			cur_point.iIndex = -1;
			cur_point.iPos   = -1;
		}
	}
	return false;
}

bool CGrTxtSearch::downSearch(TxtMiru::TextPoint &pos)
{
	if(!m_pDoc || m_len <= 0){ return false; }
	auto lpSrc = m_str.c_str();
	auto &&line_list = m_pDoc->GetConstLineList();
	int ll_len = line_list.size();
	if(ll_len <= m_pos.iLine){
		return false;
	}
	auto cur_point = m_pos;
	++cur_point.iPos;
	auto *it_ll=&line_list[0];
	if(0 < cur_point.iLine && cur_point.iLine < ll_len){
		it_ll += cur_point.iLine;
		ll_len -= cur_point.iLine;
	}
	if(m_bUseRegExp){
		// ³‹K•\Œ»‘Î‰ž
		try {
			std::wregex re(lpSrc, std::regex_constants::icase);
			for(;ll_len>0; --ll_len, ++it_ll, ++cur_point.iLine, cur_point.iIndex=0, cur_point.iPos=0){
				auto &&text_list=it_ll->text_list;
				int tl_len = text_list.size();
				if(tl_len <= 0){
					continue;
				}
				std::tstring str;
				m_pDoc->ToString(str, cur_point, TxtMiru::TextPoint{cur_point.iLine + 1, -1, 0}, -1);
				std::wcmatch m;
				auto lpLine = str.c_str();
				if(std::regex_search(lpLine, m, re)){
					ToTextPoint(cur_point, text_list, m[0].first-lpLine);
					pos = cur_point;
					m_pos = pos;
					m_len = m[0].second - m[0].first;
					return true;
				}
			}
		} catch(...){
			return false;
		}
	} else {
		for(;ll_len>0; --ll_len, ++it_ll, ++cur_point.iLine, cur_point.iIndex=0, cur_point.iPos=0){
			auto &&text_list=it_ll->text_list;
			int tl_len = text_list.size();
			if(tl_len <= 0){
				continue;
			}
			auto *it_tl=&text_list[0];
			if(0 < cur_point.iIndex && cur_point.iIndex < tl_len){
				it_tl += cur_point.iIndex;
				tl_len -= cur_point.iIndex;
			}
			auto next_point = cur_point;
			for(;tl_len>0; --tl_len, ++it_tl){
				auto &&text_info = *it_tl;
				cur_point = next_point;
				++next_point.iIndex;
				next_point.iPos = 0;
				if(!isSearchTargetTextType(text_info.textType)){
					continue;
				}
				auto lpNextDst = CGrText::CharNextN(text_info.str.c_str(), cur_point.iPos);
				while(*lpNextDst && *lpSrc != *lpNextDst){
					lpNextDst = CGrText::CharNext(lpNextDst);
					++cur_point.iPos;
				}
				if(!(*lpNextDst)){ continue; }
				int len = m_len + CGrText::CharLen(lpNextDst);
				std::tstring str;
				m_pDoc->ToString(str, cur_point, TxtMiru::TextPoint{cur_point.iLine + 1, -1, 0}, len);
				auto lpLineStr = str.c_str();
				do {
					if(CGrText::isMatchChar(lpLineStr, lpSrc)){
						pos = cur_point;
						m_pos = pos;
						return true;
					}
					lpNextDst = CGrText::CharNext(lpNextDst);
					lpLineStr = CGrText::CharNext(lpLineStr);
					++cur_point.iPos;
				} while(*lpNextDst);
			}
		}
	}
	return false;
}

bool CGrTxtSearch::next(TxtMiru::TextPoint &pos, bool bDown)
{
	CGrWaitCursor wait;
	bool bFind = false;
	if(bDown){
		bFind = downSearch(pos);
	} else {
		bFind = upSearch(pos);
	}
	if(!bFind && m_bLoop){
		if(bDown){
			m_pos = TxtMiru::TextPoint{0,0,0};
			bFind = downSearch(pos);
		} else {
			m_pos = TxtMiru::TextPoint{-1,-1,-1};
			bFind = upSearch(pos);
		}
	}
	return bFind;
}

bool CGrTxtSearch::Search(TxtMiru::TextPoint &pos, LPCTSTR lpSrc, bool bLoop, bool bUseRegExp, bool bDown)
{
	Set(pos, lpSrc, bLoop, bUseRegExp, bDown);
	return next(pos, bDown);
}

bool CGrTxtSearch::Next(TxtMiru::TextPoint &pos)
{
	return next(pos, true);
}

bool CGrTxtSearch::Prev(TxtMiru::TextPoint &pos)
{
	return next(pos, false);
}
