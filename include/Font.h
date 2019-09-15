#ifndef __FONT_H__
#define __FONT_H__

#include "stltchar.h"
#include <string>

class CGrFont
{
public:
	CGrFont();
	~CGrFont();
	// HFONTの取得
	//   戻り値 [out]: HFONT
	const HFONT GetHFont() const { return m_hFont; }
	// Fontの作成
	//   height [in]: 高さ
	//   width  [in]: 幅
	void CreateFont(int height, const int width = 0);
	void CreateFontIndirect(const LOGFONT &logfont);
	// フォントを作成します。
	//   angle  [in]: 角度
	void CreateAngleFont(int angle);

	// 高さ取得
	//   戻り値 [out]: 高さ
	int Height() const { return abs(m_logFont.lfHeight); }
	// 角度取得
	//   戻り値 [out]: 角度
	int Angle() const { return m_logFont.lfEscapement; }
	LOGFONT GetLogFont() const { return m_logFont; }
	
	static std::tstring const LOGFONT2String(const LOGFONT &logfont);
	static LOGFONT const String2LOGFONT(const std::tstring &str);
	void operator =(const CGrFont &font);
public:
	operator HFONT() const { return m_hFont; }
private:
	HFONT          m_hFont = NULL;
	LOGFONT        m_logFont = {};
	static LOGFONT m_def_logFont;
};

#endif
