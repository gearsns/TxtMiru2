#ifndef __TXTBUFFER_H__
#define __TXTBUFFER_H__

#include "stltchar.h"
#include "TxtMiruDef.h"

class CGrTxtBuffer
{
public:
	struct CGrCharFunc {
		virtual bool IsValid(TxtMiru::TextType tt){ return true; }
		virtual bool SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd){ return true;};
	};
	struct CGrTypeFunc {
		virtual bool IsValid(TxtMiru::TextType tt){ return true; }
		virtual bool SetType(const TxtMiru::TextPoint &cur, const TxtMiru::TextInfo &text_info){ return true;};
	};
private:
	TxtMiru::LineList      m_lineList;
	TxtMiru::TextStyleMap  m_textStyleMap;
	TxtMiru::TextOffsetMap m_textOffsetMap;
	TxtMiru::PictPosMap    m_pictPosMap;
	std::tstring m_title;
	std::tstring m_author;
	std::tstring m_info;
	std::tstring m_lastWriteTime;
public:
	CGrTxtBuffer(){};
	virtual ~CGrTxtBuffer(){};

	TxtMiru::LineList &GetLineList(){ return m_lineList; }
	TxtMiru::TextStyleMap  &GetTextStyleMap (){ return m_textStyleMap ; }
	TxtMiru::TextOffsetMap &GetTextOffsetMap(){ return m_textOffsetMap; }
	TxtMiru::PictPosMap    &GetPictPosMap   (){ return m_pictPosMap   ; }
	//
	const TxtMiru::LineList &GetConstLineList() const { return m_lineList; }
	const TxtMiru::TextStyleMap  &GetConstTextStyleMap () const { return m_textStyleMap ; }
	const TxtMiru::TextOffsetMap &GetConstTextOffsetMap() const { return m_textOffsetMap; }
	const TxtMiru::PictPosMap    &GetConstPictPosMap   () const { return m_pictPosMap   ; }
	const bool IsBold(const TxtMiru::TextPoint &tp) const;
	void ForEach(const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, CGrCharFunc &&func) const;
	void ForEach(const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, CGrTypeFunc &&func) const;
	void ForEachAll(CGrCharFunc &&func) const;
	void ForEachAll(CGrTypeFunc &&func) const;
	TxtMiru::LineList::const_iterator FindConst(int start_line) const;
	//
	void AddLineInfo(const TxtMiru::LineInfo &li);
	void MoveBackLineInfo(TxtMiru::LineInfo &&li);
	void ToString(std::tstring &out_str, const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, int max_length = 0) const;
	void ToStringRuby(std::tstring &out_str, const TxtMiru::TextPoint &tpb, const TxtMiru::TextPoint &tpe, int max_length = 0) const;
	bool ToTextPoint(TxtMiru::TextPoint &tp, int start_line) const;
	LPCTSTR GetChar(const TxtMiru::TextPoint &pos) const;
	//
	void Clear();
	void SetTitle(LPCTSTR lpTitle){ m_title = lpTitle; }
	void SetAuthor(LPCTSTR lpAuthor){ m_author = lpAuthor; }
	void SetDocInfo(LPCTSTR lpDocInfo){ m_info = lpDocInfo; }
	void SetLastWriteTime(LPCTSTR lpTime){ m_lastWriteTime = lpTime; }
	const std::tstring &GetTitle() const { return m_title; }
	const std::tstring &GetAuthor() const { return m_author; }
	const std::tstring &GetDocInfo() const { return m_info; }
	const std::tstring &GetLastWriteTime() const { return m_lastWriteTime; }
};

#endif // __TXTBUFFER_H__
