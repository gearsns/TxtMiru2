#pragma warning( disable : 4786 )

#include "TxtParser.h"
#include "Text.h"
#include "TxtMiru.h"
#include "TxtMiruTextType.h"
//#define __DBG__
#include "Debug.h"

void CGrTxtParser::TextInfoList2String(std::tstring &out_str, const TxtMiru::TextInfoList &text_list)
{
	for(const auto &ti : text_list){
		if(ti.textType == TxtMiru::TextType::TEXT){
			out_str += ti.str;
		}
	}
}

TxtMiru::TextListPos CGrTxtParser::GetTextListPos(int offset, const TxtMiru::TextInfoList &text_list)
{
	int len=text_list.size();
	if(len <= 0){ return TxtMiru::TextListPos{-1, -1}; }
	const auto *it = &text_list[0];
	for(int index = 0; len>0; --len, ++it, ++index){
		const auto &ti = *it;
		if(ti.textType == TxtMiru::TextType::TEXT){
			int size = ti.str.size();
			if(offset < size){
				return TxtMiru::TextListPos{index, offset};
			}
			offset -= size;
		}
	}
	return TxtMiru::TextListPos{-1, -1};
}

int CGrTxtParser::GetTextListOffset(const TxtMiru::TextListPos &tlp, const TxtMiru::TextInfoList &text_list)
{
	int len = text_list.size();
	int last_index = tlp.iIndex;
	--len;
	if(last_index >= 0 && last_index < len){
		len = last_index;
	}
	if(len < 0){ return -1; }
	const auto *it = &text_list[0];
	// forで処理されない最終文字セット (iPos計算)
	int size = (it+len)->str.size();
	int pos = tlp.iPos;
	if(pos < 0 || pos > size){
		pos = size;
	}
	int offset = 0;
	for(; len>0; --len, ++it){
		const auto &ti = *it;
		if(ti.textType == TxtMiru::TextType::TEXT){
			offset += ti.str.size();
		}
	}
	return offset + pos;
}

bool CGrTxtParser::GetRFindText(TxtMiru::TextListPos &tlp, const TxtMiru::TextInfoList &text_list, LPCTSTR text)
{
	std::tstring body;
	TextInfoList2String(body, text_list);

	const auto lpSrc = body.c_str();
	const auto target_pos = CGrText::RFind(lpSrc, lpSrc + body.size() -1, text);
	if(!target_pos){
		return false;
	}
	tlp = GetTextListPos(target_pos - lpSrc, text_list);
	if(tlp.iIndex < 0){
		return false;
	}
	return true;
}

bool CGrTxtParser::GetRFindTextType(TxtMiru::TextListPos &tlp, const TxtMiru::TextInfoList &text_list, TxtMiru::TextType textType)
{
	int len=text_list.size();
	if(len <= 0){ return false; }
	const auto *it = &text_list[len-1];
	for(; len>0; --len, --it){
		const auto &ti = *it;
		if(ti.textType == textType){
			tlp = TxtMiru::TextListPos{len-1, CGrText::CharLen(ti.str.c_str())-1};
			if(tlp.iIndex < 0){
				return false;
			}
			return true;
		}
	}
	return false;
}

bool CGrTxtParser::isText(TxtMiru::TextType tt)
{
	return TxtMiruType::isTextOrSkipOrRotateNumOrKuChar(tt);
}
// 0:filename,1:note,2:size,3:pos,4:caption
void CGrTxtParser::GetPictInfo(const std::tstring &str, std::vector<std::tstring> &out_list)
{
	auto lpStart = str.c_str();
	auto lpEnd = lpStart + str.size();
	while(lpStart < lpEnd){
		out_list.push_back(lpStart);
		lpStart += lstrlen(lpStart) + 1;
	}
	if(out_list.size() < 5){
		out_list.resize(5);
	}
}
void CGrTxtParser::SetCaption(std::tstring &str, LPCTSTR caption)
{
	std::vector<std::tstring> string_list;
	GetPictInfo(str, string_list);
	string_list[4] = caption;
	str = _T("");
	for(const auto &item : string_list){
		str += item;
		str.append(1, '\0');
	}
}
TxtMiru::TextInfo *CGrTxtParser::GetPrevTextInfoPicture(TxtMiru::LineList &line_list, TxtMiru::TextInfoList &text_list)
{
	for(int iIndex=text_list.size()-1; iIndex>=0; --iIndex){
		if(TxtMiruType::isPicture(text_list[iIndex].textType)){
			return &text_list[iIndex];
		}
	}
	for(int iLine=line_list.size()-1; iLine>=0; --iLine){
		auto &&text_list_temp = line_list[iLine].text_list;
		for(int iIndex=text_list_temp.size()-1; iIndex>=0; --iIndex){
			if(TxtMiruType::isPicture(text_list_temp[iIndex].textType)){
				return &text_list_temp[iIndex];
			}
		}
	}
	return nullptr;
}

#include "AozoraTxtParser.h"
#include "HTMLTxtParser.h"
#include "stlutil.h"
#include "Shell.h"
enum class PARSER_FT {
	HTML,
	TEXT,
};
static PARSER_FT GetFileType(LPCTSTR lpFileName)
{
	if(CGrShell::IsURI(lpFileName)){
		return PARSER_FT::HTML;
	}
	const auto &param = CGrTxtMiru::theApp().Param();
	auto pext = CGrShell::GetFileExtConst(lpFileName);
	if(pext && param.GetFileType(pext) == CGrTxtParam::FileType::Html){
		return PARSER_FT::HTML;
	}
	WIN32_FIND_DATA wfd;
	if(CGrShell::getFileInfo(lpFileName, wfd) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
		return PARSER_FT::HTML;
	}
	return PARSER_FT::TEXT;
}
//

static bool ReadFile(CGrTxtParser &&parser, LPCTSTR lpFileName, CGrTxtBuffer &buffer)
{
	if(parser.ReadFile(lpFileName, buffer)){
		buffer.SetTitle(parser.GetTitle());
		buffer.SetAuthor(parser.GetAuthor());
		buffer.SetDocInfo(parser.GetInfo());
		buffer.SetLastWriteTime(parser.GetLastWriteTime());
		return true;
	}
	return false;
}

bool CGrTxtParserMgr::ReadFile(LPCTSTR lpFileName, CGrTxtBuffer &buffer)
{
	switch(GetFileType(lpFileName)){
	case PARSER_FT::HTML:
		if(::ReadFile(CGrHTMLTxtParser(), lpFileName, buffer)){
			return true;
		}
		break;
	case PARSER_FT::TEXT:
		if(::ReadFile(CGrAozoraTxtParser(), lpFileName, buffer)){
			return true;
		}
		break;
	}
	buffer.Clear(); // 途中までデータが入っているとおかしくなるので ここで初期化する
	buffer.SetTitle(lpFileName);
	buffer.SetAuthor(_T(""));
	buffer.SetDocInfo(_T(""));
	buffer.SetLastWriteTime(_T(""));
	return false;
}

bool CGrTxtParserMgr::ReadBuffer(LPCTSTR lpBuffer, CGrTxtBuffer &buffer)
{
	CGrAozoraTxtParser aozoara_parser;
	if(aozoara_parser.ReadBuffer(lpBuffer, buffer)){
		buffer.SetTitle(aozoara_parser.GetTitle());
		buffer.SetAuthor(aozoara_parser.GetAuthor());
		buffer.SetDocInfo(aozoara_parser.GetInfo());
		return true;
	} else {
		buffer.SetTitle(_T(""));
		buffer.SetAuthor(_T(""));
		buffer.SetDocInfo(_T(""));
		return false;
	}
}
////////////////////////////////////
/*半角文字*/TextInfoHalfChar::TextInfoHalfChar(LPCTSTR l, TxtMiru::TextType t) : TextInfo(l, 0x0040, 0,0,0,0, t){};
/*本文    */TextInfoLPTSTR::TextInfoLPTSTR(LPCTSTR s, LPCTSTR e, WORD c) : TextInfo(std::tstring(s, CGrText::CharNext(e)), c){};
/*記号    */TextInfoSpec::TextInfoSpec(TxtMiru::TextType t, int l) : TextInfo(_T(""), 0xffff, l, 0, l, -1, t){};
/*        */TextInfoSpec::TextInfoSpec(TxtMiru::TextType t, int s, int e) : TextInfo(_T(""), 0xffff, s, 0, e,-1, t){};
/*        */TextInfoSpec::TextInfoSpec(std::tstring &&s, TxtMiru::TextType t, int l) : TextInfo(static_cast<std::tstring &&>(s), 0xffff, l, 0, l, -1, t){};
/*        */TextInfoSpec::TextInfoSpec(std::tstring &&s, TxtMiru::TextType t, TxtMiru::TextListPos &tlp) : TextInfo(static_cast<std::tstring&&>(s), 0xffff, tlp, tlp, t){};
/*        */TextInfoSpec::TextInfoSpec(std::tstring &&s, TxtMiru::TextType t, int bi, int bp, int ei, int ep) : TextInfo(static_cast<std::tstring&&>(s), 0xffff, bi, bp, ei, ep, t){};
/*        */TextInfoSpec::TextInfoSpec(LPCTSTR s, TxtMiru::TextType t, int bi, int bp, int ei, int ep) : TextInfo(s, 0xffff, bi, bp, ei, ep, t){};
////////////////////////////////////
TextComment::TextComment(std::tstring &s, const CGrTxtBuffer &b) : str(s), begin_pos{-1,-1,-1}, lpSrcBegin(nullptr), buffer(b){}
TextComment::~TextComment()
{
	if(lpSrcBegin){
		str += std::tstring(lpSrcBegin, lpSrcEnd+1);
	}
}
bool TextComment::IsValid(TxtMiru::TextType tt)
{
	return (tt == TxtMiru::TextType::COMMENT_BEGIN || tt == TxtMiru::TextType::COMMENT_END || tt == TxtMiru::TextType::COMMENT);
}
bool TextComment::SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd)
{
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
		lpSrcBegin = lpSrc;
		begin_pos = cur;
	}
	lpSrcEnd = lpSrc;
	return true;
}
////////////////////////////////////
static const int max_title_line = 6;
TitleType::TitleType(CGrTxtBuffer &b, std::tstring &t, std::tstring &a, std::tstring &i) : buffer(b), title(t), author(a), info(i){}
TitleType::~TitleType()
{
	auto &line_list = buffer.GetLineList();
	if(1 <= last_point.iLine && last_point.iLine <= max_title_line){
		// 訳 で終わる行は、翻訳者として処理
		//   最終行から順に、チェックして戻す
		auto lpTranslator = _T("訳"); // 翻訳者
		int translator_len = CGrText::CharLen(lpTranslator);
		std::tstring str;
		int author_point = last_point.iLine;
		do {
			if(author_point <= 1){
				break;
			}
			buffer.ToString(str, TxtMiru::TextPoint{author_point, 0, 0}, TxtMiru::TextPoint{author_point + 1, -1, -1});
			auto lpAuthor = str.c_str();
			int len = CGrText::CharLen(lpAuthor);
			if(len < translator_len){
				break;
			}
			auto lpAuthorLast = CGrText::CharPrevN(lpAuthor, CGrText::CharNextN(lpAuthor, len), translator_len);
			if(!CGrText::isMatchChar(lpAuthorLast, lpTranslator)){
				break;
			}
			--author_point;
		} while(1);
		buffer.ToString(title, TxtMiru::TextPoint{0, 0, 0}, TxtMiru::TextPoint{author_point, -1, -1});
		buffer.ToString(author, TxtMiru::TextPoint{author_point, 0, 0}, TxtMiru::TextPoint{last_point.iLine + 1, -1, -1});
		// タイトルや、著者が 500文字を超えるものは 設定無しとして除外する
		if(title.size() > 500 || author.size() > 500){
			title.clear();
			author.clear();
			return;
		}
		// タイトル 装飾
		auto ttsize = TextInfoSpec(TxtMiru::TextType::TEXT_SIZE , 0);
		ttsize.chrType = 2;
		line_list[0].text_list.push_back(TextInfoSpec(TxtMiru::TextType::TITLE, 0));
		line_list[0].text_list.push_back(ttsize);
		line_list[0].text_list.push_back(TextInfoSpec(TxtMiru::TextType::BOLD_START, 0));
		line_list[author_point-1].text_list.push_back(TextInfoSpec(TxtMiru::TextType::BOLD_END , 0));
		ttsize.chrType = 0;
		line_list[author_point-1].text_list.push_back(ttsize);
		line_list[author_point-1].text_list.push_back(TextInfoSpec(TxtMiru::TextType::AUTHOR, 0));
		// 改頁挿入位置を調べる
		//   Title
		//   Author
		// ブランク
		//   --------------------
		//   ...
		//   --------------------
		// ブランク /______ 改頁を入れる
		// 本文
		int len = line_list.size();
		int iNextPageLine = last_point.iLine;
		int iLine = iNextPageLine+1; // Author の次の行
		if(isEmptyLine(iLine, len, line_list)){
			iNextPageLine = iLine;
			++iLine;
		}
		// 3行先までチェックして ブランク行の後にコメントがあればそこまで表紙とみなす
		for(int iBlankCount=min(3, len-1); iBlankCount>0 && iLine < len; --iBlankCount, ++iLine){
			if(!line_list[iLine].text_list.empty() && line_list[iLine].text_list[0].textType == TxtMiru::TextType::COMMENT_BEGIN){
				// コメント行読み飛ばし
				while(iLine < len && (line_list[iLine].text_list.empty() || line_list[iLine].text_list[0].textType != TxtMiru::TextType::COMMENT_END)){
					++iLine;
				}
				iNextPageLine = iLine;
				++iLine;
				break;
			} else if(!isEmptyLine(iLine, len, line_list)){
				break;
			}
		}
		info += title;
		info += _T("\r\n");
		info += author;
		info += _T("\r\n");
		// 情報取得(前半部分)
		buffer.ForEach(TxtMiru::TextPoint{0, 0, 0}, TxtMiru::TextPoint{iNextPageLine+1, -1, -1}, TextComment(info, buffer));
		// iNextPageLine が line_listの範囲を超えないように
		if(iNextPageLine >= 0 && iNextPageLine < static_cast<signed int>(line_list.size())){
			line_list[iNextPageLine].text_list.push_back(TextInfoSpec(TxtMiru::TextType::NEXT_PAGE, 0));
		}
	} else {
		line_list[0].text_list.push_back(TextInfoSpec(TxtMiru::TextType::NEXT_PAGE, 0));
	}
}
bool TitleType::isEmptyLine(int iLine, int len, const TxtMiru::LineList &line_list)
{
	return (iLine < len && (line_list[iLine].text_list.empty() || line_list[iLine].text_list[0].str.empty()));
}
bool TitleType::IsValid(TxtMiru::TextType tt){
	return (tt == TxtMiru::TextType::TEXT || tt == TxtMiru::TextType::LINE_CHAR || tt == TxtMiru::TextType::COMMENT_BEGIN);
}
bool TitleType::SetType(const TxtMiru::TextPoint &cur_point, const TxtMiru::TextInfo &text_info){
	switch(text_info.textType){
	case TxtMiru::TextType::TEXT:
	case TxtMiru::TextType::LINE_CHAR:
		if(cur_point.iLine != 0 && text_info.str.empty()){ return false; }
		last_point = cur_point;
		if(cur_point.iLine > max_title_line){ return false; }
		break;
	case TxtMiru::TextType::COMMENT_BEGIN:
		return false;
		break;
	default:
		break;
	}
	return true;
};
////////////////////////////////////
InfoType::InfoType(CGrTxtBuffer &b, std::tstring &i) : buffer(b), info(i), line_list(buffer.GetLineList()){}
InfoType::~InfoType()
{
	if(last_point.iLine > 0){
		std::tstring str;
		buffer.ToString(str, last_point, TxtMiru::TextPoint{static_cast<signed int>(line_list.size())+1, -1, -1});
		info += _T("\r\n");
		info += str;
		line_list[last_point.iLine].text_list.push_back(TextInfoSpec(TxtMiru::TextType::COMMENT_BEGIN, 0));
	}
}
bool InfoType::isEmptyLine(const TxtMiru::TextInfoList &text_list)
{
	return (text_list.empty() || text_list[0].str.empty());
}
bool InfoType::IsValid(TxtMiru::TextType tt)
{
	return tt == TxtMiru::TextType::ENDOFCONTENTS;
}
bool InfoType::SetType(const TxtMiru::TextPoint &cur_point, const TxtMiru::TextInfo &text_info)
{
	last_point = cur_point;
	return false;
}
////////////////////////////////////
BoldSet::BoldSet(TxtMiru::TextStyleMap &tsm) : textStyleMap(tsm){}
bool BoldSet::IsValid(TxtMiru::TextType tt)
{
	return (tt == TxtMiru::TextType::TEXT);
}
bool BoldSet::SetChar(const TxtMiru::TextPoint &cur, LPCTSTR lpSrc, LPCTSTR lpEnd)
{
	auto it = textStyleMap.find(cur);
	if(textStyleMap.end() == it){
		textStyleMap[cur].uStatus = TxtMiru::TTPStatusBold;
	} else {
		textStyleMap[cur].uStatus |= TxtMiru::TTPStatusBold;
		textStyleMap[cur].fontSize = 0;
	}
	return true;
}
////////////////////////////////////
BoldType::BoldType(CGrTxtBuffer &b) : buffer(b), boldSet(b.GetTextStyleMap()), tpBeginBold{-1,0,0}{}
bool BoldType::IsValid(TxtMiru::TextType tt)
{
	return (tt == TxtMiru::TextType::BOLD || tt == TxtMiru::TextType::BOLD_START || tt == TxtMiru::TextType::BOLD_END);
}
bool BoldType::SetType(const TxtMiru::TextPoint &cur_point, const TxtMiru::TextInfo &text_info)
{
	switch(text_info.textType){
	case TxtMiru::TextType::BOLD:
		buffer.ForEach(TxtMiru::TextParserPoint(cur_point.iLine, text_info.tpBegin), TxtMiru::TextParserPoint(cur_point.iLine, text_info.tpEnd), static_cast<CGrTxtBuffer::CGrCharFunc&&>(boldSet));
		break;
	case TxtMiru::TextType::BOLD_START:
		if(tpBeginBold.iLine == -1){
			tpBeginBold = cur_point;
		}
		break;
	case TxtMiru::TextType::BOLD_END:
		if(tpBeginBold.iLine >= 0){
			buffer.ForEach(tpBeginBold, cur_point, static_cast<CGrTxtBuffer::CGrCharFunc&&>(boldSet));
			tpBeginBold.iLine = -1;
		}
		break;
	default:
		break;
	}
	return true;
}
////////////////////////////////////
#include "stlutil.h"
#include "CSVText.h"
#include "shell.h"
// 外字
class CGrCharConvertor
{
public:
	static CGrCharConvertor &Convert()
	{
		static CGrCharConvertor cc;
		return cc;
	}
	bool Accent(std::tstring &line, int iStart, int iEnd)
	{
		return convertAccent(line, iStart, iEnd);
	}
	bool Gaiji(std::tstring &line, LPCTSTR lpStr)
	{
		auto lpEnd = lpStr+_tcslen(lpStr);
		auto lpSearchEnd = CGrText::RFind(lpStr, CGrText::CharPrev(lpStr, lpEnd), _T("、"));
		if(lpSearchEnd && lpStr != lpSearchEnd){
			lpEnd = lpSearchEnd;
		}
		return convertGaiji(line, lpStr, lpEnd);
	}
private:
	CGrCharConvertor() : m_bAccentLoaded(false), m_bGaijiLoaded(false){}
	bool m_bAccentLoaded;
	bool m_bGaijiLoaded;
	std::map<std::tstring,std::tstring> m_accent_list;
	std::map<std::tstring,std::tstring> m_gaiji_list;
	bool open_data(std::map<std::tstring,std::tstring> &char_list, TCHAR *lpFolder, LPCTSTR lpTypeName)
	{
		std::tstring accent_file_name;
		if(CGrShell::EndBackSlash(lpFolder)){
			CGrText::FormatMessage(accent_file_name, _T("%1!s!%2!s!.txt"), lpFolder, lpTypeName);
		} else {
			CGrText::FormatMessage(accent_file_name, _T("%1!s!/%2!s!.txt"), lpFolder, lpTypeName);
		}
		CGrCSVText csv;
		csv.SetSplitChar(_T('\t'));
		csv.Open(accent_file_name.c_str());
		int max_row = csv.GetRowSize();
		for(int row=0; row<max_row; ++row){
			if(csv.GetColmnSize(row) == 2){
				char_list[csv.GetText(row, 0)] = csv.GetText(row, 1);
			}
		}
		return (max_row > 0);
	}
	void open(std::map<std::tstring,std::tstring> &char_list, LPCTSTR lpTypeName)
	{
		TCHAR folderName[MAX_PATH];
		_tcscpy_s(folderName, CGrTxtMiru::GetDataPath());
		// Application Dataからファイルを探す
		if(!open_data(char_list, folderName, lpTypeName)){
			// 無ければ、本体のパスから
			CGrShell::GetExePath(folderName);
			open_data(char_list, folderName, lpTypeName);
		}
	}
	bool convertGaiji(std::tstring &line, LPCTSTR lpStr, LPCTSTR lpEnd)
	{
		if(!m_bGaijiLoaded){
			open(m_gaiji_list, _T("gaiji"));
			m_bGaijiLoaded = true;
		}
		std::tstring target(lpStr, lpEnd);
		auto it=m_gaiji_list.find(target);
		if(it != m_gaiji_list.end()){
			line = it->second;
			return true;
		}
		return false;
	}
	bool convertAccent(std::tstring &line, int iStart, int iEnd)
	{
		if(!m_bAccentLoaded){
			open(m_accent_list, _T("accent"));
			m_bAccentLoaded = true;
		}
		auto lpBegin = line.c_str();
		auto lpStr = lpBegin + iStart;
		auto lpEnd = lpBegin + iEnd;
		std::tstring target(CGrText::CharNext(lpStr), lpEnd);
		int from_size = target.size();
		for(const auto &item : m_accent_list){
			std::replace(target, item.first.c_str(), item.second.c_str());
		}
		if(target.size() == from_size){
			return false;
		} else {
			line.replace(lpStr-lpBegin, CGrText::CharNext(lpEnd)-lpStr/*文字数*/, target.c_str());
			return true;
		}
	}
};

static const TCHAR *numStr[] = { _T(""), _T("Ⅰ"), _T("Ⅱ"), _T("Ⅲ"), _T("Ⅳ"), _T("Ⅴ"), _T("Ⅵ"), _T("Ⅶ"), _T("Ⅷ"), _T("Ⅸ"), _T("Ⅹ"), };
LPCTSTR ConvertNoteChar(LPCTSTR lpSrc)
{
	if(CGrText::isMatchChar(lpSrc, _T("ローマ数字"))){
		lpSrc = CGrText::CharNextN(lpSrc, 5);
		int i = CGrText::toInt(lpSrc);
		if(i <= 10){
			return numStr[i];
		}
	}
	static std::tstring line;
	if(CGrCharConvertor::Convert().Gaiji(line, lpSrc)){
		return line.c_str();
	}
	const auto lpFindSrc = CGrText::GetFirstShortMatchPos(lpSrc, _T("「"), _T("」"));
	if(lpFindSrc){
		std::tstring str(CGrText::CharNext(lpSrc), lpFindSrc);
		return ConvertNoteChar(str.c_str());
	}
	return nullptr;
}

void ConvertAccentChar(std::tstring &line)
{
	int index = 0;
	do {
		const auto lpText = line.c_str();
		const auto lpFindBegin = CGrText::Find(lpText + index, _T("〔"));
		if(!lpFindBegin){
			break;
		}
		if(lpFindBegin - lpText < index){
			break;
		}
		index = lpFindBegin - lpText;
		auto lpFindEnd = CGrText::Find(lpFindBegin, _T("〕"));
		if(!lpFindEnd){
			break;
		}
		if(!CGrCharConvertor::Convert().Accent(line, lpFindBegin-lpText, lpFindEnd-lpText)){
			// 変換対象文字が無い場合は、〔〕も削除しない
			index = lpFindEnd - lpText;
		}
	} while(1);
}

//
#include "JScript.h"
#include "CurrentDirectory.h"
bool LoadPreParser(CGrJScript &script, LPCTSTR lpType)
{
	const auto &param = CGrTxtMiru::theApp().Param();
	if(!param.GetBoolean(CGrTxtParam::PointsType::UsePreParser)){
		return false;
	}
	CGrCurrentDirectory cur;
	TCHAR *lpfilepart;
	std::tstring filename;
	param.GetText(CGrTxtParam::TextType::PreParserFolder, filename);
	do {
		if(!filename.empty()){
			TCHAR foldername[MAX_PATH + 1];
			::GetFullPathName(filename.c_str(), sizeof(foldername)/sizeof(TCHAR), foldername, &lpfilepart);
			CGrShell::AddBackSlash(foldername);
			filename = foldername;
			filename += lpType;
			filename += _T(".txt");
			if(CGrShell::GetSearchDir(filename.c_str())){
				break;
			} else {
				filename = foldername;
				filename += lpType;
				filename += _T(".js");
				if(CGrShell::GetSearchDir(filename.c_str())){
					break;
				}
			}
		}
		TCHAR folderName[MAX_PATH];
		CGrShell::GetExePath(folderName);
		if(folderName[0]){
			TCHAR datapath[MAX_PATH+1];
			::GetFullPathName(folderName, sizeof(datapath)/sizeof(TCHAR), datapath, &lpfilepart);
			CGrShell::AddBackSlash(datapath);
			filename = datapath;
			filename += _T("Script\\");
			if(_tcsicmp(folderName, datapath) == 0){
				return false;
			}
			filename += lpType;
			std::tstring basefilename(filename);
			filename += _T(".txt");
			if(CGrShell::GetSearchDir(filename.c_str())){
				break;
			} else {
				filename = basefilename;
				filename += _T(".js");
				if(CGrShell::GetSearchDir(filename.c_str())){
					break;
				} else {
					return false;
				}
			}
		}
	} while(0);
	return script.Load(filename.c_str());
}
bool PreParse(CGrJScript &script, std::tstring &outstr, LPCTSTR lpFileName, LPCTSTR lpBuffer)
{
	TCHAR exeFolderName[MAX_PATH];
	CGrShell::GetExePath(exeFolderName);
	TCHAR dataFolderName[MAX_PATH];
	_tcscpy_s(dataFolderName, CGrTxtMiru::GetDataPath());
	LPCTSTR argv[] = {lpFileName, lpBuffer, exeFolderName, dataFolderName};

	return script.Run(outstr, _T("Parse"), argv, sizeof(argv)/sizeof(LPCTSTR));
}
