#ifndef __AOZORADATA_H__
#define __AOZORADATA_H__

#include "TxtParser.h"
//
struct MatchStr {
	LPCTSTR str;
	int str_len ;
	int str_size;
	TxtMiru::TextType text_type;
public:
	MatchStr() : str(_T("")), str_len(0), str_size(0), text_type(TxtMiru::TextType::TEXT){}
	MatchStr(LPCTSTR p, TxtMiru::TextType t=TxtMiru::TextType::TEXT) :str(p), str_len(CGrText::CharLen(p)), str_size(_tcslen(p)), text_type(t){}
	void Set(LPCTSTR p, TxtMiru::TextType t=TxtMiru::TextType::TEXT);
	operator LPCTSTR() const { return str; }
	operator std::tstring() const { return str; }
	const int len() const { return str_len; }
	const int size() const { return str_size; }
	const TxtMiru::TextType type() const { return text_type; }
};
//
struct MatchPairStr {
	MatchPairStr(){}
	MatchPairStr(LPCTSTR  f, LPCTSTR  s) : first(f), second(s){}
	MatchPairStr(MatchStr f, MatchStr s) : first(f), second(s){}
	MatchPairStr(TxtMiru::TextType t, LPCTSTR  f, LPCTSTR  s) : first(f, t), second(s, t){}
	MatchPairStr(TxtMiru::TextType t, MatchStr f, MatchStr s) : first(f, t), second(s, t){}
	void Set(TxtMiru::TextType t, LPCTSTR f, LPCTSTR s);
	MatchStr first ;
	MatchStr second;
};

class CGrAozoraData
{
public:
	static CGrAozoraData &Data();
	~CGrAozoraData();
	//
	const MatchPairStr* GetPairTTList() const;
	const MatchStr* GetTTList() const;
	const MatchStr* GetTT() const;
	//
	int GetPairTTListLength() const;
	int GetTTListLength() const;
	int GetTTLength() const;
	//
	bool IsPairTTListFistChar(LPCTSTR lpSrc) const;
	bool IsTTListFistChar(LPCTSTR lpSrc) const;
private:
	CGrAozoraData();
};

#endif // __AOZORADATA_H__
