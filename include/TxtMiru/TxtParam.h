#ifndef __TXTPARAM_H__
#define __TXTPARAM_H__

#include <windows.h>
#include <windowsx.h>
#include "stltchar.h"

#include <map>
#include <vector>
#include "stlutil.h"
#include <regex>

#ifndef MemberSizeOf
#define MemberSizeOf(type, member)       sizeof(((type*)0)->member)
#endif

#include "TxtFuncConfig.h"
class CGrTxtParam : public CGrTxtFuncIParam
{
public:
	using PointArray = std::vector<int>;
	struct Points {
		LPCTSTR    key = nullptr;
		PointArray array;
		int toIntArray(int points[], int size) const;
		Points(){}
	};
	using StringArray = std::vector<std::tstring>;
	struct Value {
		LPCTSTR     key = nullptr;
		StringArray array;
		void toString(std::tstring &out) const;
		Value(){}
	};
	struct CharInfo {
		LPCTSTR  key = nullptr;
		TCHAR    fontName[MemberSizeOf(LOGFONT, lfFaceName)] = {};
		COLORREF color         = 0       ;
		double   fontSpacing   = 0       ;
		bool     fontCentering = false   ;
		long     weight        = FW_NORMAL;
		CharInfo(){}
	};
	struct TextInfo {
		LPCTSTR      key;
		std::tstring str;
		TextInfo() : key(nullptr){};
		TextInfo(LPCTSTR k) : key(k){};
	};
	using CharOffsetTypeMap = std::map<std::tstring, WORD>;
	using CharOffsetMap = std::map<std::tstring, POINT> ;
	struct CGrStringCompare
	{
		virtual bool operator == (LPCTSTR lpSrc) {return true;}
	};
	struct ValueSetter {
		virtual void operator()(Value &v){};
		virtual void operator()(CharInfo &v){};
		virtual void operator()(Points &v){};
		virtual void operator()(TextInfo &v){};
		virtual void operator()(CharOffsetMap::value_type &v){};
	};
	struct FileTypeInfo {
		FileType ft = FileType::Html;
		std::tstring ext;
	};
	using FileTypeMap = std::simple_array<FileTypeInfo>;
	struct CookieInfo {
		std::wregex re;
		std::tstring cookie;
	};
	using CookieInfoMap = std::simple_array<CookieInfo>;
public:
	CGrTxtParam();
	virtual ~CGrTxtParam();

	int FindIF(ValueType type, CGrStringCompare &&sc) const;

	bool Save(LPCTSTR lpFileName);
	bool Load(LPCTSTR lpFileName);

	//
	virtual void SetFontName(CharType type, LPCTSTR fontName);
	virtual void SetColor(CharType type, COLORREF color);
	virtual void SetFontSpacing(CharType type, double fontSpacing);
	virtual void SetFontCentering(CharType type, bool fontCentering);
	virtual void SetFontWeight(CharType type, long weight);
	virtual void SetBoolean(PointsType type, bool val);
	virtual void SetPoints(PointsType type, const int points[], int size);
	virtual void SetTextList(ValueType type, LPCTSTR lpStr);
	virtual void SetText(TextType type, LPCTSTR lpStr);
	//
	virtual LPCTSTR GetFontName(CharType type);
	virtual COLORREF GetColor(CharType type);
	virtual double GetFontSpacing(CharType type);
	virtual bool GetFontCentering(CharType type);
	virtual long GetFontWeight(CharType type);
	virtual bool GetBoolean(PointsType type) const;
	virtual int GetPoints(PointsType type, int points[], int size) const;
	virtual void GetTextList(ValueType type, LPTSTR outstr, int numberOfElements);
	virtual void GetText(TextType type, LPTSTR outstr, int numberOfElements) const;
	void GetTextList(ValueType type, std::tstring &out) const;
	void GetText(TextType type, std::tstring &out) const;
	//
	virtual POINT GetCharOffset(LPCTSTR str) const;
	virtual WORD GetCharOffsetType(LPCTSTR str) const;
	POINT GetCharOffset(const std::tstring &str) const;
	WORD GetCharOffsetType(const std::tstring &str) const;
	//
	virtual void UpdateKeybord(HWND hWnd);
	virtual void UpdateConfig(HWND hWnd);
	virtual void UpdateLayout(HWND hWnd);
	virtual void UpdateStyleList(HWND hWnd);
	virtual bool SaveStyle(LPCTSTR lpFileName, LPCTSTR lpStyleName);
	//
	void ForEachValueType(ValueSetter &vs);
	void ForEachCharInfo(ValueSetter &vs);
	void ForEachTextInfo(ValueSetter &vs);
	void ForEachPoints(ValueSetter &vs);
	void ForEachCharOffset(ValueSetter &vs);
	//
	FileType GetFileType(LPCTSTR lpExt) const;
	const FileTypeMap &GetFileTypeMap() const { return m_fileTypeMap; }
	//
	LPCTSTR GetCookie(LPCTSTR lpURL) const;
private:
	Value             m_valueTypeMap[static_cast<int>(ValueType::MaxNum)];
	CharInfo          m_charInfoMap [static_cast<int>(CharType::MaxNum)];
	TextInfo          m_textInfoMap [static_cast<int>(TextType::MaxNum)];
	Points            m_pointsMap   [static_cast<int>(PointsType::MaxNum)];
	CharOffsetMap     m_charOffsetMap; // ï\é¶à íuí≤êÆÇçsÇ§ï∂éö áÄ Ç∆Ç© offset.lisÇÊÇËéÊìæ
	CharOffsetTypeMap m_charOffsetTypeMap;
	FileTypeMap       m_fileTypeMap;
	CookieInfoMap     m_cookieInfoMap;
};

#endif // __TXTPARAM_H__
