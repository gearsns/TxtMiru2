#ifndef __FONT_H__
#define __FONT_H__

#include "stltchar.h"
#include <string>

class CGrFont
{
public:
	CGrFont();
	~CGrFont();
	// HFONT�̎擾
	//   �߂�l [out]: HFONT
	const HFONT GetHFont() const { return m_hFont; }
	// Font�̍쐬
	//   height [in]: ����
	//   width  [in]: ��
	void CreateFont(int height, const int width = 0);
	void CreateFontIndirect(const LOGFONT &logfont);
	// �t�H���g���쐬���܂��B
	//   angle  [in]: �p�x
	void CreateAngleFont(int angle);

	// �����擾
	//   �߂�l [out]: ����
	int Height() const { return abs(m_logFont.lfHeight); }
	// �p�x�擾
	//   �߂�l [out]: �p�x
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
