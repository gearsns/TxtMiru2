#pragma warning( disable : 4786 )

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "TxtSubtitle.h"
#include "TxtDocument.h"
#include "TxtMiruTextType.h"

CGrTxtSubtitle::CGrTxtSubtitle()
{
}

CGrTxtSubtitle::~CGrTxtSubtitle()
{
}

void CGrTxtSubtitle::Attach(const CGrTxtDocument *pDoc)
{
	m_pDoc = pDoc;
}

void CGrTxtSubtitle::RebuildPageNo()
{
	if(m_pDoc){
		for(auto &&item : m_subtitles){
			item.iPage = m_pDoc->TextPointToPage(item.tpb);
		}
	}
}

void CGrTxtSubtitle::ClearLeaf()
{
	m_subtitles.clear();
	m_subtitles.shrink_to_fit();
}

bool CGrTxtSubtitle::AddLeaf(int order, LPCTSTR lpName, LPCTSTR lpFilename)
{
	Subtitle subtitle;
	subtitle.order    = order     ;
	subtitle.str      = lpName    ;
	subtitle.filename = lpFilename;
	m_subtitles.push_back(std::move(subtitle));
	return true;
}

int CGrTxtSubtitle::Count() const
{
	return m_subtitles.size();
}

CGrTxtFuncISubTitleLeaf* CGrTxtSubtitle::GetLeaf(int idx)
{
	if(idx >= 0 && idx < static_cast<signed int>(m_subtitles.size())){
		return &(m_subtitles[idx]);
	}
	return nullptr;
}
const CGrTxtFuncISubTitleLeaf* CGrTxtSubtitle::GetConstLeaf(int idx) const
{
	if(idx >= 0 && idx < static_cast<signed int>(m_subtitles.size())){
		return &(m_subtitles[idx]);
	}
	return nullptr;
}

struct CGrTxtOutLinerFunc : public CGrTxtBuffer::CGrTypeFunc {
	CGrTxtBuffer &m_txtBuffer;
	std::vector<CGrTxtSubtitle::Subtitle> &m_subtitles;
	bool m_bComment = false;
	CGrTxtOutLinerFunc(CGrTxtBuffer &t, std::vector<CGrTxtSubtitle::Subtitle> &s) : m_txtBuffer(t), m_subtitles(s){}
	~CGrTxtOutLinerFunc()
	{
		auto &&lineList = m_txtBuffer.GetLineList();
		for(auto &&subtitle : m_subtitles){
			auto &&li = lineList[subtitle.tpb.iLine];
			auto &&text_list = li.text_list;
			int len=text_list.size();
			if(len <= 0){
				continue;
			}
			auto &&str = text_list[0].str;
			if(CGrText::CharLen(str.c_str()) <= subtitle.tpe.iPos){
				str = str.substr(subtitle.tpe.iPos);
			} else {
				str.resize(0);
			}
			for(auto &&ti : text_list){
				setpos(subtitle.tpe.iIndex, subtitle.tpe.iPos, ti.tpBegin);
				setpos(subtitle.tpe.iIndex, subtitle.tpe.iPos, ti.tpEnd  );
			}
			subtitle.tpe = TxtMiru::TextPoint{subtitle.tpb.iLine+1,-1,-1};
		}
	}
	void setpos(int iIndex, int iPos, TxtMiru::TextListPos &tlp){
		if(iIndex == tlp.iIndex){
			if(iPos > tlp.iPos){
				++tlp.iIndex;
			} else {
				tlp.iPos = max(tlp.iPos - iPos, 0);
			}
		}
	}
	virtual bool IsValid(TxtMiru::TextType tt){
		return TxtMiruType::isTextOrRotateNumOrComment(tt);
	}
	virtual bool SetType(const TxtMiru::TextPoint &cur, const TxtMiru::TextInfo &text_info){
		switch(text_info.textType){
		case TxtMiru::TextType::COMMENT_BEGIN: m_bComment = true ; break;
		case TxtMiru::TextType::COMMENT_END  : m_bComment = false; break;
		case TxtMiru::TextType::COMMENT      : break;
		default:
			if(!m_bComment && cur.iIndex == 0){
				auto lpSrc = text_info.str.c_str();
				int n = 0;
				while(*lpSrc && CGrText::isMatchChar(lpSrc, _T("."))){
					++n;
					lpSrc = CGrText::CharNext(lpSrc);
				}
				if(n > 0){
					CGrTxtSubtitle::Subtitle subtitle;
					auto tbp = cur;
					tbp.iPos += n;
					m_txtBuffer.ToString(subtitle.str, tbp, TxtMiru::TextPoint{tbp.iLine+1,-1,-1});
					subtitle.tpb   = cur;
					subtitle.tpe   = tbp;
					subtitle.level = n  ;
					subtitle.order = 0  ;
					m_subtitles.push_back(std::move(subtitle));
				}
			}
		}
		return true;
	};
};

struct CGrTxtSubtileFunc : public CGrTxtBuffer::CGrTypeFunc {
	CGrTxtBuffer &m_txtBuffer;
	std::vector<CGrTxtSubtitle::Subtitle> &m_subtitles;
	CGrTxtSubtileFunc(CGrTxtBuffer &t, std::vector<CGrTxtSubtitle::Subtitle> &s) : m_txtBuffer(t), m_subtitles(s){}
	~CGrTxtSubtileFunc(){}
	virtual bool IsValid(TxtMiru::TextType tt){
		return TxtMiruType::isSubtitle(tt) || tt == TxtMiru::TextType::ID || tt == TxtMiru::TextType::FILE;
	}
	virtual bool SetType(const TxtMiru::TextPoint &cur, const TxtMiru::TextInfo &text_info){
		int lvl = 1;
		switch(text_info.textType){
		case TxtMiru::TextType::SUBTITLE1: lvl=1; break;
		case TxtMiru::TextType::SUBTITLE2: lvl=2; break;
		case TxtMiru::TextType::SUBTITLE3: lvl=3; break;
		}
		CGrTxtSubtitle::Subtitle subtitle;
		subtitle.order = 0;
		if(text_info.textType == TxtMiru::TextType::ID){ // ID ページ内リンク用
			subtitle.tpb   = cur;
			subtitle.tpe   = cur;
			subtitle.str   = text_info.str;
			subtitle.level = 999;
		} else if(text_info.textType == TxtMiru::TextType::FILE){ // NextFile用
			const auto &lineList = m_txtBuffer.GetLineList();
			if(cur.iLine < 0 || cur.iLine >= static_cast<signed int>(lineList.size())){
				return true;
			}
			const auto &li = lineList[cur.iLine];
			if(text_info.tpEnd.iIndex < 0 || text_info.tpEnd.iIndex >= static_cast<signed int>(li.text_list.size())){
				return true;
			}
			const auto &ti_link=li.text_list[text_info.tpEnd.iIndex];
			m_txtBuffer.ToString(subtitle.str,
								 TxtMiru::TextPoint{cur.iLine, ti_link.tpBegin.iIndex, ti_link.tpBegin.iPos},
								 TxtMiru::TextPoint{cur.iLine, ti_link.tpEnd.iIndex  , ti_link.tpEnd.iPos  });
			subtitle.str      += text_info.str;
			subtitle.filename = ti_link.str;
			subtitle.level    = -1;
			subtitle.order    = static_cast<signed short>(text_info.chrType);
		} else {
			/**/subtitle.tpb = TxtMiru::TextPoint{cur.iLine, text_info.tpBegin.iIndex, text_info.tpBegin.iPos  };
			if(text_info.tpEnd.iPos < 0){
				subtitle.tpe = TxtMiru::TextPoint{cur.iLine, text_info.tpEnd  .iIndex, text_info.tpEnd  .iPos  };
			} else {
				subtitle.tpe = TxtMiru::TextPoint{cur.iLine, text_info.tpEnd  .iIndex, text_info.tpEnd  .iPos+1};
			}
			m_txtBuffer.ToString(subtitle.str, subtitle.tpb, subtitle.tpe);
			if (subtitle.str.empty()) {
				return true;
			}
			subtitle.level = lvl;
		}
		m_subtitles.push_back(std::move(subtitle));
		return true;
	};
};

bool CGrTxtSubtitle::AddLeaf(CGrTxtBuffer &txtBuffer)
{
	const auto &param = CGrTxtMiru::theApp().Param();
	if(param.GetBoolean(CGrTxtParam::PointsType::WzMemoMode)){
		CGrTxtOutLinerFunc outline(txtBuffer, m_subtitles);
		txtBuffer.ForEachAll(static_cast<CGrTxtBuffer::CGrTypeFunc&&>(outline));
	}
	CGrTxtSubtileFunc subtitle(txtBuffer, m_subtitles);
	txtBuffer.ForEachAll(static_cast<CGrTxtBuffer::CGrTypeFunc&&>(subtitle));

	return true;
}

int CGrTxtSubtitle::GetPageById(LPCTSTR lpID) const
{
	for(const auto &item : m_subtitles){
		if(_tcsicmp(item.str.c_str(), lpID) == 0){
			return item.iPage;
		}
	}
	return -1;
}

LPCTSTR CGrTxtSubtitle::GetNameByPage(int page) const
{
	LPCTSTR lpName = nullptr;
	const auto &param = CGrTxtMiru::theApp().Param();
	int points[2] = {1,1};
	param.GetPoints(CGrTxtParam::PointsType::RunningHeadLevel, points, _countof(points));
	bool b1st_title = (points[0] == 1);
	int max_title_level = points[1];
	for(const auto &item : m_subtitles){
		if(item.level <= max_title_level && item.level >= 1 && item.iPage <= page && !item.str.empty()){
			lpName = item.str.c_str();
			// 指定したページの最初の見出しを返す
			if(b1st_title && item.iPage == page){
				break;
			}
		}
	}
	return lpName;
}
