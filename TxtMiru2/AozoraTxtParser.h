#ifndef __AOZORATXTPARSER_H__
#define __AOZORATXTPARSER_H__

#include "TxtParser.h"

class CGrAozoraTxtParser : public CGrTxtParser
{
public:
	struct Indent {
		int iIndent1st;
		int iIndent2nd;
	};
	struct LineBox {
		int top;
		int bottom;
	};
private:
	struct TextRangePoint {
		int iCount = 0;
		int iLine  = 0;
		int iIndex = 0;
		int iPos   = 0;
	};
	std::vector<Indent> m_Indent ;
	std::vector<Indent> m_RIndent;
	std::vector<LineBox> m_LineBox;
	TextRangePoint m_RangeTextPoint[TxtMiru::TT_MaxNum]; // タグの範囲指定用
	bool m_bIndentReset = false; // 当行のみの天からインデント指定有無
	bool m_bRIndentReset = false; // 当行のみの地からインデント指定有無
	std::vector<int> m_LimitChar;
	std::tstring m_title;
	std::tstring m_author;
	std::tstring m_info;
	std::tstring m_lastWriteTime;
	//
	int m_iFontSize = 0;
	std::vector<int> m_fontSizeS;
	std::vector<int> m_fontSizeL;
	std::vector<std::tstring> m_paramList;
public:
	CGrAozoraTxtParser();
	virtual ~CGrAozoraTxtParser();
	virtual bool ReadFile(LPCTSTR lpFileName, CGrTxtBuffer &buffer);
	virtual bool ReadBuffer(LPCTSTR lpBuffer, CGrTxtBuffer &buffer);
	virtual LPCTSTR Name() const { return _T("CGrAozoraTxtParser"); };
	virtual LPCTSTR GetTitle() const { return m_title.c_str(); };
	virtual LPCTSTR GetAuthor() const { return m_author.c_str(); };
	virtual LPCTSTR GetInfo() const { return m_info.c_str(); }
	virtual LPCTSTR GetLastWriteTime() const { return m_lastWriteTime.c_str(); }
private:
	bool ReadBuffer(LPCTSTR lpBuffer, CGrTxtBuffer &buffer, LPCTSTR lpFileName, bool bUsePreParser = true);
	bool ReadDataBuffer(LPBYTE lpData, DWORD fsize, CGrTxtBuffer &buffer, LPCTSTR lpFileName);
	LPCTSTR getPairText(LPCTSTR lpSrc, TxtMiru::TextType textType);
	void pushNote(TxtMiru::TextInfoList &text_list, LPCTSTR target, LPCTSTR note, TxtMiru::TextType textType);
	void addLine(CGrTxtBuffer &buffer, LPCTSTR lpSrc, LPCTSTR lpEnd, bool bTrans, int iFileLineNo = -1);
	bool parse(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, LPCTSTR lpLine, LPCTSTR lpEnd);
	void rindentEnd(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType);
	void indentEnd(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType);
	void rindentStart(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType, int iIndent);
	void indentStart(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType, int iIndent);
	void indentStart(CGrTxtBuffer &buffer, TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType, int iIndent1st, int iIndent2nd);
	bool convertNoteStr(std::tstring &note, LPCTSTR lpNote = nullptr);
private:
	enum CommentStatus {
		CSTAT_CONTENTS     ,
		CSTAT_BEGIN_COMMENT,
		CSTAT_COMMENT      ,
		CSTAT_SKIP_LINE    ,
		CSTAT_NO_MORE      ,
	} m_csComment = CSTAT_CONTENTS;
	bool m_bEndofContents = false;
	bool m_bUseOverlapChar = false;
};

#endif // __AOZORATXTPARSER_H__
