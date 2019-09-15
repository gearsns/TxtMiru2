// Text.h
//
// 2バイト文字列操作
//
#ifndef __TEXT_H__
#define __TEXT_H__

#include "stltchar.h"

namespace CGrText
{
#ifdef UNICODE
#define GRCHAR TCHAR
#else
#define GRCHAR unsigned int
#endif
	// リードバイトかどうかを判定
	//   (漢字開始文字)
	//   ch     : 文字
	//   戻り値 : リードバイトの場合 TRUE
	BOOL isLead(GRCHAR ch);
	// トレーリングバイトかどうかを判定
	//  （漢字）
	//   ch     : 文字
	//   戻り値 : トレーリングバイトの場合 TRUE
	BOOL isTrail(GRCHAR ch);
	// 文字数を取得します。
	//   string : 文字列
	//   戻り値 : ２バイト文字を1文字とした文字数を返します。
	int len(LPCTSTR string);
	// 平仮名かどうか判定
	//   c      : 文字
	//   戻り値 : 平仮名の場合 TRUE
	BOOL isHira(GRCHAR c);
	// シンボルかどうか判定
	//   c      : 文字
	//   戻り値 : シンボルの場合 TRUE
	BOOL isSymbl(GRCHAR c);
	// シンボルかどうか判定
	//   文字が ヽ ヾ ゝ ゞ 〃 仝 々  のいづれかどうか
	//   c      : 文字
	//   戻り値 : シンボルの場合 TRUE
	BOOL isSymblKanji(GRCHAR c);
	// 片仮名かどうか判定
	//   c      : 文字
	//   戻り値 : 片仮名の場合 TRUE
	BOOL isKata(GRCHAR c);
	// 数字かどうか判定 : IsCharAlphaNumeric
	// 空白かどうか判定
	//   c      : 文字
	//   戻り値 : 空白の場合 TRUE
	BOOL isSpace(GRCHAR c);
	// カンマ区切り文字列に変換します。
	//   numStr    [out]: 数字を受け取るstring
	//   num        [in]: 数字
	//   bZeroDisp  [in]: ゼロの時、表示(=TRUE)
	void numCommaStr(std::tstring &numStr, int num, BOOL bZeroDisp);
	void FormatMessage(std::tstring &str, LPCTSTR lpszFormat, va_list *pargList);
	void FormatMessage(std::tstring &str, LPCTSTR lpszFormat, ...);
	void FormatMessage(std::tstring &str, UINT nFormatID, ...);
	void FormatMessage(HINSTANCE hInst, std::tstring &str, UINT nFormatID, ...);
#ifdef TXTMIRUFUNCLIB_DLL
	HINSTANCE GetDllModuleHandle();
	void SetDllModuleHandle(HINSTANCE hInst);
	void LoadString(UINT nFormatID, TCHAR *strFormat, int size);
	void LoadString(UINT nFormatID, std::tstring &str);
#else
	void LoadString(UINT nFormatID, TCHAR *strFormat, int size);
	void LoadString(UINT nFormatID, std::tstring &str);
#endif
	void LoadString(HINSTANCE hInst, UINT nFormatID, std::tstring &str);

	LPCTSTR CharNext(LPCTSTR lpStr);
	LPCTSTR CharPrev(LPCTSTR,LPCTSTR lpStr);
	LPCTSTR CharNextN(LPCTSTR str, int n);
	LPCTSTR CharPrevN(LPCTSTR begin, LPCTSTR str, int n);
	WORD GetStringTypeEx(LPCTSTR str, int len);
	int CharLen(LPCTSTR pSrc);
	// 文字一致確認
	//   pSrc    [in]: 文字列
	//   pDst    [in]: 検索文字(最大 pDstの文字数まで検索)
	//   戻り値 [out]: 一致、true
	bool isMatchChar(LPCTSTR pSrc, LPCTSTR pDst);
	//
	bool isMatchLast(LPCTSTR pSrc, LPCTSTR pDst);
	bool isMatchILast(LPCTSTR pSrc, LPCTSTR pDst);
	LPCTSTR Find(LPCTSTR lpSrc, LPCTSTR lpDst);
	LPCTSTR RFind(LPCTSTR lpBegin, LPCTSTR lpSrc, LPCTSTR lpDst);
	// 最短一致文字列の取得
	//   outstr [out]: 一致した文字列
	//   lpSrc   [in]: 走査文字列
	//   lpStart [in]: 開始文字
	//   lpEnd   [in]: 終了文字
	//   戻り値 [out]: 最終走査文字位置
	LPCTSTR GetShortMatchStr(std::tstring &outstr, LPCTSTR lpSrc, LPCTSTR lpStart, LPCTSTR lpEnd);
	// 最短一致文字列位置の取得
	//   lpSrc   [in]: 走査文字列
	//   lpStart [in]: 開始文字
	//   lpEnd   [in]: 終了文字
	//   戻り値 [out]: 最終走査文字位置
	LPCTSTR GetFirstShortMatchPos(LPCTSTR lpSrc, LPCTSTR lpStart, LPCTSTR lpEnd);
	// 大文字小文字を区別して、文字列を比較します。
	//   string1 : 文字列
	//   string2 : 文字列
	//   coount  : 文字数
	//   戻り値  : 正 string1 の部分文字列は string2 の部分文字列より小さい
	//              0 string1 の部分文字列は string2 の部分文字列と等しい
	//             負 string1 の部分文字列は string2 の部分文字列より大きい
	int nCmp(LPCTSTR string1, LPCTSTR string2, size_t count);
	// 大文字小文字を区別せずに、文字列を比較します。
	//   string1 : 文字列
	//   string2 : 文字列
	//   coount  : 文字数
	//   戻り値  : 正 string1 の部分文字列は string2 の部分文字列より小さい
	//              0 string1 の部分文字列は string2 の部分文字列と等しい
	//             負 string1 の部分文字列は string2 の部分文字列より大きい
	int niCmp(LPCTSTR string1, LPCTSTR string2, size_t count);
	// 小文字に変換します。
	//   ch     : 文字
	//   戻り値 : 小文字
	TCHAR toLower(TCHAR ch);
	// 文字列の中に文字があるかどうか
	//   striing : 文字列
	//   ch      : 文字
	//   戻り値  : 在った場合は 見つかった位置
	LPCTSTR strChar(LPCTSTR string, LPCTSTR ch);

	int toInt(const std::tstring &str);
	void itoa(int num, std::tstring &str);
	bool MultiByteToTString(UINT CodePage, LPCSTR mb, const DWORD len, std::tstring& text);
	bool isUTF16(BYTE *p, int len);
};

#endif