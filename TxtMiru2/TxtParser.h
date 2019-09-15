#ifndef __TXTPARSER_H__
#define __TXTPARSER_H__

#include <windows.h>
#include <windowsx.h>
#include "stltchar.h"
#include "Text.h"
#include "TxtMiruDef.h"
#include <map>
#include <vector>
#include "TxtBuffer.h"

class CGrTxtParser
{
public:
	//
	static void TextInfoList2String(std::tstring &out_str, const TxtMiru::TextInfoList &text_list);
	static TxtMiru::TextListPos GetTextListPos(int offset, const TxtMiru::TextInfoList &text_list);
	static int GetTextListOffset(const TxtMiru::TextListPos &tlp, const TxtMiru::TextInfoList &text_list);
	static bool GetRFindText(TxtMiru::TextListPos &tlp, const TxtMiru::TextInfoList &text_list, LPCTSTR text);
	static bool GetRFindTextType(TxtMiru::TextListPos &tlp, const TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType);
	static bool isText(TxtMiru::TextType tt);
public:
	// 0:filename,1:note,2:size,3:pos,4:caption
	static void GetPictInfo(const std::tstring &str, std::vector<std::tstring> &out_list);
	static void SetCaption(std::tstring &str, LPCTSTR caption);
	static TxtMiru::TextInfo *GetPrevTextInfoPicture(TxtMiru::LineList &line_list, TxtMiru::TextInfoList &text_list);
public:
	CGrTxtParser(){};
	virtual ~CGrTxtParser(){};
	virtual bool ReadFile(LPCTSTR lpFileName, CGrTxtBuffer &buffer) = 0;
	virtual bool ReadBuffer(LPCTSTR lpBuffer, CGrTxtBuffer &buffer) = 0;
	virtual LPCTSTR Name() const = 0;
	virtual LPCTSTR GetTitle() const = 0;
	virtual LPCTSTR GetAuthor() const = 0;
	virtual LPCTSTR GetInfo() const = 0;
	virtual LPCTSTR GetLastWriteTime() const = 0;
};
//////////
class CGrTxtParserMgr
{
public:
	CGrTxtParserMgr(){}
	virtual ~CGrTxtParserMgr(){}
	bool ReadFile(LPCTSTR lpFileName, CGrTxtBuffer &buffer);
	bool ReadBuffer(LPCTSTR lpBuffer, CGrTxtBuffer &buffer);
};
//////////
//
struct TextInfoHalfChar /*îºäpï∂éö*/: public TxtMiru::TextInfo { TextInfoHalfChar(LPCTSTR l, TxtMiru::TextType t); };
struct TextInfoLPTSTR   /*ñ{ï∂    */: public TxtMiru::TextInfo { TextInfoLPTSTR(LPCTSTR s, LPCTSTR e, WORD c);};
struct TextInfoSpec     /*ãLçÜ    */: public TxtMiru::TextInfo {
	TextInfoSpec(TxtMiru::TextType t, int l);
	TextInfoSpec(TxtMiru::TextType t, int s, int e);
	TextInfoSpec(std::tstring &&s, TxtMiru::TextType t, int l);
	TextInfoSpec(std::tstring &&s, TxtMiru::TextType t, TxtMiru::TextListPos &tlp);
	TextInfoSpec(std::tstring &&s, TxtMiru::TextType t, int bi, int bp, int ei, int ep);
	TextInfoSpec(LPCTSTR s, TxtMiru::TextType t, int bi, int bp, int ei, int ep);
};
struct TextComment : public CGrTxtBuffer::CGrCharFunc {
	std::tstring &str;
	TxtMiru::TextPoint begin_pos;
	LPCTSTR lpSrcBegin = nullptr;
	LPCTSTR lpSrcEnd = nullptr;
	const CGrTxtBuffer &buffer;
	TextComment(std::tstring &s, const CGrTxtBuffer &b);
	~TextComment();
	virtual bool IsValid(TxtMiru::TextType tt);
	virtual bool SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd);
};
struct InfoType : public CGrTxtBuffer::CGrTypeFunc {
	CGrTxtBuffer &buffer;
	TxtMiru::LineList &line_list;
	TxtMiru::TextPoint last_point;
	std::tstring &info;

	InfoType(CGrTxtBuffer &b, std::tstring &i);
	~InfoType();
	bool isEmptyLine(const TxtMiru::TextInfoList &text_list);
	virtual bool IsValid(TxtMiru::TextType tt);
	virtual bool SetType(const TxtMiru::TextPoint &cur_point, const TxtMiru::TextInfo &text_info);
};
struct TitleType : public CGrTxtBuffer::CGrTypeFunc {
	CGrTxtBuffer &buffer;
	TxtMiru::TextPoint last_point;
	std::tstring &title;
	std::tstring &author;
	std::tstring &info;

	TitleType(CGrTxtBuffer &b, std::tstring &t, std::tstring &a, std::tstring &i);
	~TitleType();
	bool isEmptyLine(int iLine, int len, const TxtMiru::LineList &line_list);
	virtual bool IsValid(TxtMiru::TextType tt);
	virtual bool SetType(const TxtMiru::TextPoint &cur_point, const TxtMiru::TextInfo &text_info);
};
struct BoldSet : public CGrTxtBuffer::CGrCharFunc {
	TxtMiru::TextStyleMap &textStyleMap;
	BoldSet(TxtMiru::TextStyleMap &tsm);
	virtual bool IsValid(TxtMiru::TextType tt);
	virtual bool SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd);
};
struct BoldType : public CGrTxtBuffer::CGrTypeFunc {
	CGrTxtBuffer &buffer;
	BoldSet boldSet;
	TxtMiru::TextPoint tpBeginBold;

	BoldType(CGrTxtBuffer &b);
	virtual bool IsValid(TxtMiru::TextType tt);
	virtual bool SetType(const TxtMiru::TextPoint &cur_point, const TxtMiru::TextInfo &text_info);
};
//
LPCTSTR ConvertNoteChar(LPCTSTR lpSrc);
void ConvertAccentChar(std::tstring &line);
class CGrJScript;
bool LoadPreParser(CGrJScript &script, LPCTSTR lpType);
bool PreParse(CGrJScript &script, std::tstring &outstr, LPCTSTR lpFileName, LPCTSTR lpBuffer);
//
#endif // __TXTPARSER_H__
