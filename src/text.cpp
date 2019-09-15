// Text.c
//
// 2バイト文字列操作(J-JIS)
//
#define STRICT // 型を宣言および使用時に、より厳密な型チェックが行われます。

#include <windows.h>
#include "Text.h"
#include <sstream>
#include <stack>

#ifdef TXTMIRUFUNCLIB_DLL
static HINSTANCE g_hInst = NULL;
#endif
namespace CGrText
{
	// リードバイトバイトかどうかを判定
	//   (漢字開始文字)
	//   ch     : 文字
	//   戻り値 : リードバイトの場合 TRUE
	BOOL isLead(GRCHAR ch)
	{
#ifdef UNICODE
		return FALSE;
#else
		return ((unsigned int) (ch ^ 0x20) - 0xa1 < 0x3c);/*((ch >= 0x80 && ch <= 0x9F) || (ch >= 0xE0 && ch <= 0xFC));*/
#endif
	}

	// トレーリングバイトかどうかを判定
	//  （漢字）
	//   ch     : 文字
	//   戻り値 : トレーリングバイトの場合 TRUE
	BOOL isTrail(GRCHAR ch)
	{
#ifdef UNICODE
		return FALSE;
#else
		return ((ch >= 0x40 && ch <= 0x7E) || (ch >= 0x80 && ch <= 0xFC));
#endif
	}

	// 文字数を取得します。
	//   string : 文字列
	//   戻り値 : ２バイト文字を1文字とした文字数を返します。
	int len(LPCTSTR string)
	{
		int len = 0;
		while(*string){
			len++;
			string = CharNext(string);
		}
		return len;
	}

	// 平仮名かどうか判定
	//   c      : 文字
	//   戻り値 : 平仮名の場合 TRUE
	BOOL isHira(unsigned int c)
	{
		return (c >= 0x829F && c <= 0x82F1);
	}

	// シンボルかどうか判定
	//   c      : 文字
	//   戻り値 : シンボルの場合 TRUE
	BOOL isSymbl(unsigned int c)
	{
		return (c >= 0x8141 && c <= 0x81AC);
	}

	// 片仮名かどうか判定
	//   c      : 文字
	//   戻り値 : 片仮名の場合 TRUE
	BOOL isKata(unsigned int c)
	{
		return (c >= 0x8340 && c <= 0x8396);
	}

	// シンボルかどうか判定
	//   文字が ヽ ヾ ゝ ゞ 〃 仝 々  のいづれかどうか
	//   c      : 文字
	//   戻り値 : シンボルの場合 TRUE
	BOOL isSymblKanji(unsigned int c)
	{
		if(c >= 0x8151 && c <= 0x8158){
			return TRUE;
		}
		return FALSE;
	}
	// 数字かどうか判定
	//   c      : 文字
	//   戻り値 : 数字の場合 TRUE
	BOOL isDigit(unsigned int c)
	{
		return _istdigit(c);
	}

	// 空白かどうか判定
	//   c      : 文字
	//   戻り値 : 空白の場合 TRUE
	BOOL isSpace(unsigned int c)
	{
		return (c == 0x20 || c == 0x8140 || (0x09 <= c && c <= 0x0D));
	}

	// カンマ区切り文字列に変換します。
	//   numStr    [out]: 数字を受け取るstring
	//   num        [in]: 数字
	//   bZeroDisp  [in]: ゼロの時、表示(=TRUE)
	void numCommaStr(std::tstring &numStr, int num, BOOL bZeroDisp)
	{
		std::tstring buf;
		std::tstring bufStr;

		if(!bZeroDisp && num == 0){
			numStr.resize(0);
			return;
		}
		itoa(num, bufStr);

		int len = bufStr.length();
		if(!_istdigit(bufStr[0])){
			len--;
		}
		int count = len % 3;
		if(count == 0){
			count = 3;
		}

		auto *src = const_cast<TCHAR*>(bufStr.c_str());
		while(*src){
			if(_istdigit(*src)){
				if(count == 0){
					buf += _T(',');
					count = 3;
				}
				count--;
			}
			buf += *src; // 数値を一文字ずつ表示する
			src++;
		}
		numStr = buf;
	}
#ifdef TXTMIRUFUNCLIB_DLL
	HINSTANCE GetDllModuleHandle()
	{
		return g_hInst;
	}
	void SetDllModuleHandle(HINSTANCE hInst)
	{
		g_hInst = hInst;
	}
	void LoadString(UINT nFormatID, TCHAR *strFormat, int size)
	{
		::LoadString(g_hInst, nFormatID, strFormat, size);
	}
	void LoadString(UINT nFormatID, std::tstring &str)
	{
		LoadString(g_hInst, nFormatID, str);
	}
#else
	void LoadString(UINT nFormatID, TCHAR *strFormat, int size)
	{
		::LoadString(GetModuleHandle(NULL), nFormatID, strFormat, size);
	}
	void LoadString(UINT nFormatID, std::tstring &str)
	{
		LoadString(GetModuleHandle(NULL), nFormatID, str);
	}
#endif
	void LoadString(HINSTANCE hInst, UINT nFormatID, std::tstring &str)
	{
		TCHAR strFormat[2048];
		LoadString(hInst, nFormatID, strFormat, _countof(strFormat));
		str = strFormat;
	}
	// formatting (using FormatMessage style formatting)
	void FormatMessage(std::tstring &str, LPCTSTR lpszFormat, va_list *pargList)
	{
		// format message into temporary buffer lpszTemp
		LPTSTR lpszTemp = nullptr;
		if (::FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
							lpszFormat, 0, 0, reinterpret_cast<LPTSTR>(&lpszTemp), 0, pargList) == 0 ||
			lpszTemp == nullptr)
		{
			;//AfxThrowMemoryException();
		} else {
			// assign lpszTemp into the resulting string and free the temporary
			str = lpszTemp;
			LocalFree(lpszTemp);
		}
	}
	void FormatMessage(std::tstring &str, LPCTSTR lpszFormat, ...)
	{
		va_list argList;
		va_start(argList, lpszFormat);
		FormatMessage(str, lpszFormat, &argList);
		va_end(argList);
	}
	void FormatMessage(std::tstring &str, UINT nFormatID, ...)
	{
		std::tstring strFormat;
		LoadString(nFormatID, strFormat);
		va_list argList;
		va_start(argList, nFormatID);
		FormatMessage(str, strFormat.c_str(), &argList);
		va_end(argList);
	}
	void FormatMessage(HINSTANCE hInst, std::tstring &str, UINT nFormatID, ...)
	{
		// get format string from string table
		TCHAR strFormat[2048];
		LoadString(hInst, nFormatID, strFormat, _countof(strFormat));

		// format message into temporary buffer lpszTemp
		va_list argList;
		va_start(argList, nFormatID);
		LPTSTR lpszTemp = nullptr;
		if (::FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
							strFormat, 0, 0, reinterpret_cast<LPTSTR>(&lpszTemp), 0, &argList) == 0 ||
			lpszTemp == nullptr)
		{
			;//AfxThrowMemoryException();
		} else {
			// assign lpszTemp into the resulting string and free lpszTemp
			str = lpszTemp;
			LocalFree(lpszTemp);
		}
		va_end(argList);
	}

#ifdef UNICODE
#ifndef IS_HIGH_SURROGATE
#define HIGH_SURROGATE_START 0xd800
#define HIGH_SURROGATE_END 0xdbff
#define LOW_SURROGATE_START 0xdc00
#define LOW_SURROGATE_END 0xdfff
#define IS_HIGH_SURROGATE(wch) (((wch) >= HIGH_SURROGATE_START) && ((wch) <= HIGH_SURROGATE_END))
#define IS_LOW_SURROGATE(wch) (((wch) >= LOW_SURROGATE_START) && ((wch) <= LOW_SURROGATE_END))
#define IS_SURROGATE_PAIR(hs, ls) (IS_HIGH_SURROGATE(hs) && IS_LOW_SURROGATE(ls))
#endif
#endif

	LPCTSTR CharNext(LPCTSTR lpStr)
	{
#ifdef UNICODE
		if(IS_HIGH_SURROGATE(*lpStr)){
			++lpStr;
		}
		return lpStr+1;
#else
		return ::CharNext(lpStr);
#endif
	}

	LPCTSTR CharPrev(LPCTSTR lpStart, LPCTSTR lpStr)
	{
#ifdef UNICODE
		if(IS_LOW_SURROGATE(*lpStr)){
			--lpStr;
		}
		--lpStr;
		if(IS_LOW_SURROGATE(*lpStr)){
			--lpStr;
		}
		if(lpStr < lpStart){
			return nullptr;
		} else {
			return lpStr;
		}
#else
		return ::CharPrev(lpStart, lpStr);
#endif
	}

	WORD GetStringTypeEx(LPCTSTR str, int len)
	{
		WORD type = 0;
#ifdef UNICODE
		if(str[0] == _T('ヶ')){
			return C3_KATAKANA|0x8100|0xf000;
		} else if(str[0] == _T('々')){
			return 0x8100; //str = _T("漢");
		} else if(str[0] == _T('ー')){
			return 0x0010; //str = _T("ア");
		} else if(str[0] == _T('｢') || str[0] == _T('｣')){ 
			return 0x0048; // str = _T("(");
		} else if(
			// ×が半角扱いになるので  /} else if(str[0] >= 161 && str[0] <= 363){
			/**/str[0] == 0x00a7 /* §*/
			||  str[0] == 0x00a8 /* ¨*/
			||  str[0] == 0x00b0 /* °*/
			||  str[0] == 0x00b1 /* ±*/
			||  str[0] == 0x00b4 /* ´*/
			||  str[0] == 0x00d7 /* ×*/
			||  str[0] == 0x00f7 /* ÷*/
			){
			// NOP
		} else if(str[0] >= 0x00A1 && str[0] <= 0x016b){
			// 発音記号付き?
			return 0x0048;
		}
#endif
		if(::GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE3, str, 1, &type)){

		}
		if(type == 0x0048){
			if(CGrText::isMatchChar(str, _T("!"))){
				return 0xF048;
			}
			if(CGrText::isMatchChar(str, _T("?"))){
				return 0xF048;
			}
		}
		if(type & C3_HIRAGANA){
			return C3_HIRAGANA;
		} else if(type & C3_KATAKANA){
			return C3_KATAKANA;
		} else if(type & C3_FULLWIDTH){
			WORD type2 = 0;
			::GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE2, str, 1, &type2);
			if(type2 & C2_WHITESPACE){
				return type | type2;
			}
		}
		return type;
	}

	int CharLen(LPCTSTR pSrc)
	{
		int len = 0;
		while(*pSrc){ pSrc = CharNext(pSrc); ++len; }
		return len;
	}

	bool isMatchChar(LPCTSTR pSrc, LPCTSTR pDst)
	{
		while(*pSrc && *pDst){
			if(*pDst != *pSrc){
				return false;
			}
			++pDst;
			++pSrc;
		}
		return (*pDst) ? false : true;
	}
	bool isMatchLast(LPCTSTR pSrc, LPCTSTR pDst)
	{
		auto srcLen = _tcslen(pSrc);
		auto dstLen = _tcslen(pDst);
		if(srcLen < dstLen){ return false; }
		return _tcscmp(&(pSrc[srcLen-dstLen]), pDst) == 0;
	}
	bool isMatchILast(LPCTSTR pSrc, LPCTSTR pDst)
	{
		auto srcLen = _tcslen(pSrc);
		auto dstLen = _tcslen(pDst);
		if(srcLen < dstLen){ return false; }
		return _tcsicmp(&(pSrc[srcLen-dstLen]), pDst) == 0;
	}
	LPCTSTR CharNextN(LPCTSTR str, int n)
	{
		while(--n >= 0 && str){ str = CharNext(str); }
		return str;
	}

	LPCTSTR CharPrevN(LPCTSTR begin, LPCTSTR str, int n)
	{
		while(--n >= 0 && str && str != begin){ str = CharPrev(begin, str); }
		return str;
	}
	LPCTSTR Find(LPCTSTR lpSrc, LPCTSTR lpDst)
	{
		bool bFind = false;
		while(*lpSrc && !(bFind = isMatchChar(lpSrc, lpDst))){
			lpSrc = CharNext(lpSrc);
		}
		if(bFind){
			return lpSrc;
		} else {
			return nullptr;
		}
	}
	LPCTSTR RFind(LPCTSTR lpBegin, LPCTSTR lpSrc, LPCTSTR lpDst)
	{
		while(lpBegin != lpSrc && lpSrc && *lpSrc && !isMatchChar(lpSrc, lpDst)){
			lpSrc = CharPrev(lpBegin, lpSrc);
		}
		if(lpSrc && lpBegin == lpSrc && !isMatchChar(lpSrc, lpDst)){
			return nullptr;
		}
		return lpSrc;
	}
	// 最短一致文字列の取得
	//   outstr [out]: 一致した文字列
	//   lpSrc   [in]: 走査文字列
	//   lpStart [in]: 開始文字
	//   lpEnd   [in]: 終了文字
	//   戻り値 [out]: 最終走査文字位置
	LPCTSTR GetShortMatchStr(std::tstring &outstr, LPCTSTR lpSrc, LPCTSTR lpStart, LPCTSTR lpEnd)
	{
		while(!isMatchChar(lpSrc, lpStart)){
			lpSrc = CharNext(lpSrc);
			if(!*lpSrc){ return nullptr; }
		}
		while(*lpStart){
			lpSrc   = CharNext(lpSrc  );
			lpStart = CharNext(lpStart);
		}
		auto lpoutstr = lpSrc;
		while(!isMatchChar(lpSrc, lpEnd)){
			lpSrc = CharNext(lpSrc);
			if(!*lpSrc){ return nullptr; }
		}
		outstr.assign(lpoutstr, lpSrc);

		return lpSrc;
	}
	// 最短一致文字列位置の取得
	//   lpSrc   [in]: 走査文字列
	//   lpStart [in]: 開始文字
	//   lpEnd   [in]: 終了文字
	//   戻り値 [out]: 最終走査文字位置
	LPCTSTR GetFirstShortMatchPos(LPCTSTR lpSrc, LPCTSTR lpStart, LPCTSTR lpEnd)
	{
		if(!isMatchChar(lpSrc, lpStart)){
			return nullptr;
		}
		while(*lpStart){
			lpSrc   = CharNext(lpSrc  );
			lpStart = CharNext(lpStart);
		}
		auto lpoutstr = lpSrc;
		while(!isMatchChar(lpSrc, lpEnd)){
			lpSrc = CharNext(lpSrc);
			if(!*lpSrc){ return nullptr; }
		}
		return lpSrc;
	}
	// 文字列を比較します。
	//   string1 : 文字列
	//   string2 : 文字列
	//   coount  : 文字数
	//   bLower  : TRUEの時、小文字に変換
	//   戻り値  : 正 string1 の部分文字列は string2 の部分文字列より小さい
	//              0 string1 の部分文字列は string2 の部分文字列と等しい
	//             負 string1 の部分文字列は string2 の部分文字列より大きい
	static int stringnCompare(LPCTSTR string1, LPCTSTR string2, size_t count, BOOL bLower)
	{
		int ret;

		while(count > 0){
			// 漢字
			if(isLead(*string1)){
				if(!isLead(*string2)){
					// string1 漢字 != string2 非漢字
					return -1;
				}
				ret = *string1 - *string2;
				if(ret != 0)
					return ret;
				string1++;
				string2++;
				ret = *string1 - *string2;
			} else {
				if(bLower){
					// 小文字に変換して比較
					ret = toLower(*string1) - toLower(*string2);
				} else {
					ret = *string1 - *string2;
				}
			}
			if(ret != 0)
				return ret;
			// 終端チェック
			if(*string1 == _T('\0')){
				// 終端まで来ていれば同じ
				return 0;
			}
			string1++;
			string2++;
			count--;
		}
		// 最後まで同じ
		return 0;
	}

	// 大文字小文字を区別して、文字列を比較します。
	//   string1 : 文字列
	//   string2 : 文字列
	//   coount  : 文字数
	//   戻り値  : 正 string1 の部分文字列は string2 の部分文字列より小さい
	//              0 string1 の部分文字列は string2 の部分文字列と等しい
	//             負 string1 の部分文字列は string2 の部分文字列より大きい
	int nCmp(LPCTSTR string1, LPCTSTR string2, size_t count)
	{
		return stringnCompare(string1, string2, count, FALSE);
	}

	// 大文字小文字を区別せずに、文字列を比較します。
	//   string1 : 文字列
	//   string2 : 文字列
	//   coount  : 文字数
	//   戻り値  : 正 string1 の部分文字列は string2 の部分文字列より小さい
	//              0 string1 の部分文字列は string2 の部分文字列と等しい
	//             負 string1 の部分文字列は string2 の部分文字列より大きい
	int niCmp(LPCTSTR string1, LPCTSTR string2, size_t count)
	{
		return stringnCompare(string1, string2, count, TRUE);
	}
	// 小文字に変換します。
	//   ch     : 文字
	//   戻り値 : 小文字
	TCHAR toLower(TCHAR ch)
	{
		return (ch >= _T('A') && ch <= _T('Z')) ? ch - _T('A') + _T('a') : ch;
	}
	// 文字列の中に文字があるかどうか
	//   striing : 文字列
	//   ch      : 文字
	//   戻り値  : 在った場合は 見つかった位置
	LPCTSTR strChar(LPCTSTR string, LPCTSTR ch)
	{
		// ２バイト文字（リード文字）かどうかを判定
		auto lead = isLead(*ch);

		auto p = string;
		auto len = static_cast<signed int>(_tcslen(p));
		while((p-string)<len && *p){
			// 文字は１バイト、２バイト文字混在なので
			if(isLead(*p)){
				// ２バイト文字
				if(lead && *ch == *p && *(ch+1) == *(p+1)){
					// 一致
					return p;
				}
			} else {
				// １バイトの場合はhighChのみを
				if(!lead && *ch == *p){
					// 一致
					return p;
				}
			}
			p = CharNext(p);
		}
		return nullptr;
	}

	int toInt(const std::tstring &str)
	{
		return _ttoi(str.c_str());
	}

	void itoa(int num, std::tstring &str)
	{
		if(num == 0){ str = _T("0"); return; }
		bool bAsc = true;
		if(num < 0){ // 数字がマイナスの場合
			bAsc = false;
			num = -num;
		}
		std::stack<TCHAR> st;
		TCHAR numStr[] = _T("0123456789");
		div_t div_result = {num};
		int &quot = div_result.quot;
		int &rem  = div_result.rem;
		while(quot > 0){
			div_result = div(quot, 10);
			st.push(numStr[rem]);
		}
		int index=0;
		auto len = st.size();
		if(bAsc){
			str.resize(len);
		} else {
			str.resize(len+1);
			str[0] = '-';
			++index;
		}
		while(!st.empty()){
			str[index] = st.top();
			st.pop();
			++index;
		}
	}

	bool MultiByteToTString(UINT CodePage, LPCSTR mb, const DWORD len, std::tstring& text)
	{
		bool result = false;

		auto wlen = ::MultiByteToWideChar(CodePage, 0, mb, len, NULL, 0);
		if(wlen == 0){
			return false;
		}
		auto* buff = new WCHAR[wlen + 1];
		if(::MultiByteToWideChar(CodePage, 0, mb, len, buff, wlen)){
			result = true;
			buff[wlen] = L'\0';
			text = buff;
		}
		delete[] buff;

		return result;
	}
	bool isUTF16(BYTE *p, int len)
	{
		while(--len > 0){
			if(*p == '\0'){
				return true;
			}
			++p;
		}
		return false;
	}
};
