#include <windows.h>
#include "Font.h"
#include "Text.h"
#include "csvtext.h"

#define DELETE_OBJECT(__obj_h__) { \
	if(__obj_h__) { \
		DeleteObject(__obj_h__); \
		__obj_h__ = NULL; \
	} \
}

LOGFONT CGrFont::m_def_logFont = {
	-12, 0,
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	SHIFTJIS_CHARSET, OUT_STRING_PRECIS, CLIP_STROKE_PRECIS,
	DRAFT_QUALITY, FIXED_PITCH, _T("ÇlÇr ñæí©")
	};

CGrFont::CGrFont()
{
	CreateFontIndirect(m_def_logFont);
}

CGrFont::~CGrFont()
{
	::DeleteObject(m_hFont);
}

// FontÇÃçÏê¨
//   height [in]: çÇÇ≥
//   width  [in]: ïù
void CGrFont::CreateFont(const int height, const int width)
{
	auto hDC = ::GetDC(NULL);
	DELETE_OBJECT(m_hFont);
	m_logFont.lfHeight = height;
	m_logFont.lfWidth  = width;
	CreateFontIndirect(m_logFont);
	::DeleteObject(hDC);
}

void CGrFont::CreateFontIndirect(const LOGFONT &logfont)
{
	DELETE_OBJECT(m_hFont);
	m_logFont = logfont;
	m_hFont = ::CreateFontIndirect(&logfont);
}

// ÉtÉHÉìÉgÇçÏê¨ÇµÇ‹Ç∑ÅB
//   angle  [in]: äpìx
void CGrFont::CreateAngleFont(int angle)
{
	DELETE_OBJECT(m_hFont);
	m_logFont.lfWidth = 0;
	m_logFont.lfEscapement = angle;
	CreateFontIndirect(m_logFont);
}

std::tstring const CGrFont::LOGFONT2String(const LOGFONT &logfont)
{
	std::tstring buf;

	CGrText::FormatMessage(buf, _T("%1!d!,%2!d!,%3!d!,%4!d!,%5!d!,%6!d!,%7!d!,%8!d!,%9!d!,%10!d!,%11!d!,%12!d!,%13!d!,\"%14!s!\""),
						   logfont.lfHeight , logfont.lfWidth         , logfont.lfEscapement   , logfont.lfOrientation,
						   logfont.lfWeight , logfont.lfItalic        , logfont.lfUnderline    , logfont.lfStrikeOut  ,
						   logfont.lfCharSet, logfont.lfOutPrecision  , logfont.lfClipPrecision,
						   logfont.lfQuality, logfont.lfPitchAndFamily, logfont.lfFaceName     );
	return buf;
}

LOGFONT const CGrFont::String2LOGFONT(const std::tstring &str)
{
	LOGFONT logfont;
	CGrCSVText csv(str);

	static LOGFONT l_logFont = {
		-15, 0,
		FALSE, FALSE, FW_NORMAL, FALSE, FALSE, FALSE,
		SHIFTJIS_CHARSET, OUT_STRING_PRECIS, CLIP_STROKE_PRECIS,
		DRAFT_QUALITY, FIXED_PITCH, _T("ÇlÇr ÇoÉSÉVÉbÉN")
		};

	if(csv.GetColmnSize(0) < 13){
		return l_logFont;
	}
	auto *lfFaceName = csv.GetString(0, 13);
	if(!lfFaceName){
		return l_logFont;
	}
	logfont.lfHeight         = csv.GetInteger(0,  0, 0);
	logfont.lfWidth          = csv.GetInteger(0,  1, 0);
	logfont.lfEscapement     = csv.GetInteger(0,  2, 0);
	logfont.lfOrientation    = csv.GetInteger(0,  3, 0);
	logfont.lfWeight         = csv.GetInteger(0,  4, 0);
	logfont.lfItalic         = csv.GetInteger(0,  5, 0);
	logfont.lfUnderline      = csv.GetInteger(0,  6, 0);
	logfont.lfStrikeOut      = csv.GetInteger(0,  7, 0);
	logfont.lfCharSet        = csv.GetInteger(0,  8, 0);
	logfont.lfOutPrecision   = csv.GetInteger(0,  9, 0);
	logfont.lfClipPrecision  = csv.GetInteger(0, 10, 0);
	logfont.lfQuality        = csv.GetInteger(0, 11, 0);
	logfont.lfPitchAndFamily = csv.GetInteger(0, 12, 0);

	_tcscpy_s(logfont.lfFaceName, lfFaceName->c_str());
	return logfont;
}

void CGrFont::operator =(const CGrFont &font)
{
	m_logFont = font.m_logFont;
}
